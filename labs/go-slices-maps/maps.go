package main

//Done

import (
	"strings"

	"golang.org/x/tour/wc"
)

func main() {
	wc.Test(WordCount)
}

func WordCount(s string) map[string]int {
	palabras := strings.Split(s, " ")
	mapa := make(map[string]int)
	for i := 0; i < len(palabras); i++ {

		mapa[palabras[i]] += 1
	}
	return mapa
}
