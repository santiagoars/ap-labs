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

type clientChannel chan<- string

var (
	entering = make(chan client)
	leaving  = make(chan clientChannel)
	clients  = make(map[clientChannel]*client)
	messages = make(chan string)
)

type client struct {
	channel  clientChannel
	name     string
	ip       string
	conn     net.Conn
	admin    bool
	connTime string
}

func broadcaster() {
	for {
		select {
		case msg := <-messages:
			for cli := range clients {
				cli <- msg
			}

		case clientInformation := <-entering:
			if len(clients) == 0 {
				clientInformation.admin = true
			}
			clients[clientInformation.channel] = &clientInformation

		case cli := <-leaving:
			delete(clients, cli)
			close(cli)
		}
	}
}

func handleConn(conn net.Conn) {
	var username = false
	var connTime = getTimeOfConnection()
	input := bufio.NewScanner(conn)
	input.Scan()
	who := input.Text()
	for _, person := range clients {
		if person.name == who {
			username = true
			fmt.Fprintln(conn, "Sorry the username you choosed is already taken, try again")
			conn.Close()
			break
		}
	}
	if !username {
		ch := make(chan string)
		go chat(conn, ch)
		ip := conn.RemoteAddr().String()

		ch <- infoServer + "Welcome to the Simple IRC Server"
		ch <- infoServer + "The user [" + who + "] is succesfully logged"
		messages <- infoServer + "New user: [" + who + "] has connected to the server"
		fmt.Println(infoServer + "New user: [" + who + "] has connected to the server")

		entering <- client{ch, who, ip, conn, false, connTime}
		if clients[ch].admin == true {
			ch <- infoServer + "Congratulations, you were the first user."
			makeAdmin(who, ch)
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
							curr, _ := time.LoadLocation("America/Mexico_City")
							loc = curr.String()
						}
						ch <- infoServer + "Local Time: " + loc + " " + time.Now().Format("15:04")

					case "/users":
						var logInfo string
						for _, person := range clients {
							logInfo += person.name + ", " + person.connTime + ", "
						}
						ch <- infoServer + logInfo[:len(logInfo)-2]
					case "/user":
						if len(values) != 2 {
							ch <- infoServer + "Error writting parameters, the correct way isusage: /user <user>"
						} else {
							var foundUser = false
							for _, person := range clients {
								if person.name == values[1] {
									foundUser = true
									ch <- infoServer + "username: " + person.name + ", IP: " + person.ip
									fmt.Println(infoServer + "User: [" + who + "]")
									break
								}
							}
							if !foundUser {
								ch <- infoServer + "Error, user not found"
							}
						}
					case "/msg":
						if len(values) < 3 {
							ch <- infoServer + "Error parameters not in order, usage: /msg <user> <msg>"
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
								ch <- infoServer + "Error, user not found"
							}
						}
					case "/kick":
						if clients[ch].admin {
							if len(values) != 2 {
								ch <- infoServer + "Error in parameters, correct parameters: /kick <user>"
							} else {
								var foundUser = false
								for _, person := range clients {
									if person.name == values[1] {
										foundUser = true
										person.channel <- infoServer + "You're kicked from this channel"
										leaving <- person.channel
										messages <- infoServer + person.name + " has been kicked from channel"
										fmt.Println(infoServer + person.name + " has been kicked")

										person.conn.Close()
										break
									}
								}
								if !foundUser {
									ch <- infoServer + "Error, the user you requested was not found on this Server"
								}
							}
						} else {
							ch <- infoServer + "Error, your user doesn't have the permissions to kisk a user, you need to be the admin"
						}
					default:
						ch <- infoServer + "Error, command not not found"
					}
				} else {
					messages <- who + " > " + msg
				}
			}
		}
		if clients[ch] != nil {
			leaving <- ch
			messages <- infoServer + "[" + who + "] left the chat"
			fmt.Println(infoServer + "[" + who + "] left the chat")

			conn.Close()
		}
	}
}

func chat(conn net.Conn, ch <-chan string) {
	for msg := range ch {
		fmt.Fprintln(conn, msg)
	}
}

func makeAdmin(who string, cli clientChannel) {
	fmt.Println(infoServer + "[" + who + "] has been promoted as the Admin!")
	clients[cli].admin = true
	cli <- infoServer + "You're the new IRC Server Admin!"
}

func getTimeOfConnection() string {
	_, err := time.LoadLocation("America/Mexico_City")
	if err != nil {
		return infoServer + "Failed loading timezone, try again later. "
	}
	return time.Now().Format("2020-01-01 15:04:05")
}

var infoServer string

func main() {
	if len(os.Args) != 5 {
		log.Fatal("Error introducing the parameters the correct way is, usage: go run client.go -host [host] -port [port]")
	}
	infoServer = "irc-server> "
	server := os.Args[2] + ":" + os.Args[4]
	listener, err := net.Listen("tcp", server)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println(infoServer + "Simple IRC Server started at " + server)
	go broadcaster()
	fmt.Println(infoServer + "Ready for receiving new clients")
	for {
		conn, err := listener.Accept()
		if err != nil {
			log.Print(err)
			continue
		}
		go handleConn(conn)
	}
}
