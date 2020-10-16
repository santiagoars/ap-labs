// Copyright © 2016 Alan A. A. Donovan & Brian W. Kernighan.
// License: https://creativecommons.org/licenses/by-nc-sa/4.0/

// See page 254.
//!+

// Chat is a server that lets clients chat with each other.
package main

import (
	"bufio"
	"fmt"
	"log"
	"net"
	"os"
	"strings"
	"time"
)

//!+broadcaster
type clientChan chan<- string // an outgoing message channel
type client struct {
	channel clientChan
	name    string
	ip      string
	admin   bool
	conn    net.Conn
}

var (
	clients  = make(map[clientChan]*client)
	entering = make(chan client)
	leaving  = make(chan clientChan)
	messages = make(chan string) // all incoming client messages
)

func broadcaster() {
	for {
		select {
		case msg := <-messages:
			for cli := range clients {
				cli <- msg
			}

		case clientInfo := <-entering:
			if len(clients) == 0 {
				clientInfo.admin = true
			}
			clients[clientInfo.channel] = &clientInfo

		case cli := <-leaving:
			delete(clients, cli)
			close(cli)
		}
	}
}

func admin(who string, cli clientChan) {
	fmt.Println(serverInfo + "[" + who + "]" + "now has admin permissions")
	clients[cli].admin = true
	cli <- serverInfo + "You are now the server admin!"
}

func clientChat(conn net.Conn, ch <-chan string) {
	for msg := range ch {
		fmt.Println(conn, msg)
	}
}

//!-broadcaster

//!+handleConn
func handleConn(conn net.Conn) {
	var username = false
	input := bufio.NewScanner(conn)
	input.Scan()
	who := input.Text()
	for _, person := range clients {
		if person.name == who {
			username = true
			fmt.Println(conn, "Username is taken, choose another one")
			conn.Close()
			break
		}
	}
	if !username {
		ch := make(chan string)
		go clientChat(conn, ch)
		ip := conn.RemoteAddr().String()
		ch <- serverInfo + "Welcome to this simple IRC server"
		ch <- serverInfo + who + "has logged to the server"
		messages <- serverInfo + who + "is a new user"
		entering <- client{ch, who, ip, false, conn}

		if clients[ch].admin == true {
			ch <- serverInfo + "You are the first user in this server!"
		}

		for input.Scan() {
			msg := input.Text()
			if msg != "" {
				if msg[0] == '/' {
					values := strings.Split(msg, " ")
					switch values[0] {
					case "/time":
						location, _ := time.LoadLocation("Local")
						loc := location.String()
						if loc == "Local" {
							curr, _ := time.LoadLocation("America/MexicoCity")
							loc = curr.String()
						}
						ch <- serverInfo + "The local time is " + loc + " " + time.Now().Format("14:03")
					case "/users":
						var user_name string
						for _, person := range clients {
							user_name += person.name + ", "
						}
						ch <- serverInfo + user_name[:len(user_name)-2]

					case "/user":
						if len(values) != 2 {
							ch <- serverInfo + "Wrong parameters"
						} else {
							var foundUser = false
							for _, person := range clients {
								if person.name == values[1] {
									foundUser = true
									ch <- serverInfo + "username: " + person.name + ", IP: " + person.ip
									break
								}
							}
							if !foundUser {
								ch <- serverInfo + "User has not been found inside this server, sorry"
							}
						}
					case "/msg":
						if len(values) < 3 {
							ch <- serverInfo + "Wrong parameters"
						} else {
							var foundUser = false
							for _, p := range clients {
								if p.name == values[1] {
									foundUser = true
									p.channel <- who + " (direct) > " + msg[6+len(p.name):]
									break
								}
							}
							if !foundUser {
								ch <- serverInfo + "User not found, sorry"
							}
						}
					case "/kick":
						if clients[ch].admin {
							if len(values) != 2 {
								ch <- serverInfo + "Wrong parameters"
							} else {
								var foundUser = false
								for _, person := range clients {
									if person.name == values[1] {
										foundUser = true
										person.channel <- serverInfo + "You've been kicked"
										leaving <- person.channel
										messages <- serverInfo + person.name + "has been kicked"
										person.conn.Close()
										break
									}
								}
								if !foundUser {
									ch <- serverInfo + "User was not found, try again with an existing user"
								}
							}
						} else {
							ch <- serverInfo + "You don't have the permissions to kick"
						}
					default:
						ch <- serverInfo + "Command incorrect"
					}
				} else {
					messages <- who + " > " + msg
				}
			}
		}
		if clients[ch] != nil {
			leaving <- ch
			messages <- serverInfo + who + "has left the channel"
			conn.Close()
		}
	}
}

var serverInfo string

func main() {
	if len(os.Args) != 5 {
		log.Fatal("Wrong parameters, try again")
	}
	serverInfo = "irc--server"
	server := os.Args[2] + ":" + os.Args[4]
	listener, err := net.Listen("tcp", server)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println(serverInfo + "IRC Server started at: " + server)
	go broadcaster()
	fmt.Println(serverInfo + "Ready")
	for {
		conn, err := listener.Accept()
		if err != nil {
			log.Print(err)
			continue
		}
		go handleConn(conn)
	}
}
