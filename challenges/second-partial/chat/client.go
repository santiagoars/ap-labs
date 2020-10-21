package main

import (
	"fmt"
	"io"
	"log"
	"net"
	"os"
)

//!+
func main() {
	if len(os.Args) != 5 {
		log.Fatal("Wrong parameters, please try again")
	}
	user := os.Args[2]
	server := os.Args[4]
	conn, err := net.Dial("tcp", server)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Fprintln(conn, user)
	done := make(chan bool)
	go func() {
		io.Copy(os.Stdout, conn)
		log.Println("Kicked from the channel")
		done <- true
	}()
	mustCopy(conn, os.Stdin)
	conn.Close()
	<-done
}

func mustCopy(dst io.Writer, src io.Reader) {
	if _, err := io.Copy(dst, src); err != nil {
		log.Fatal(err)
	}
}
