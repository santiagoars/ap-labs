package main

import "golang.org/x/tour/pic"

func main() {
	pic.Show(Picture)
}

func Picture(x, y int) [][]uint8 {

	myPic := make([][]uint8, y)

	for i := 0; i < y; i++ {
		myPic[i] = make([]uint8, x)
		for j := 0; j < x; j++ {
			myPic[i][j] = uint8(i ^ j)
		}
	}
	return myPic
}
