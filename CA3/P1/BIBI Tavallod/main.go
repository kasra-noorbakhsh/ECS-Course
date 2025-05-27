package main

import (
	"fmt"
	"log"
	"net/http"
)

func handler(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "Happy Birthday BIBI JOON!")
}

func main() {
	http.HandleFunc("/", handler)
	log.Println("Starting server on :80")
	err := http.ListenAndServe(":80", nil)
	if err != nil {
		log.Fatal("Error starting server: ", err)
	}
}
