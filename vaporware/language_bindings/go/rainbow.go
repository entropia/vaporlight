package main

import (
	"./vaporlight"
	"time"
)

func rainbow(con vaporlight.Controller, leds int, speed int) {
	for i := 0; i < leds; i++ {
		con.SetRgba8(i, [4]uint8{0, 0, 0, 255})
	}
	sleepTime := time.Duration(8*(11-speed)^2) * time.Millisecond
	for loop := 0; true; loop++ {
		for i := 0; i < leds; i++ {
			con.SetRgb8(i, wheel(uint8((i*256/leds+(loop%256))&255)))
		}
		con.Strobe()
		time.Sleep(sleepTime)
	}
}

func wheel(pos uint8) [3]uint8 {
	if pos < 85 {
		return [3]uint8{pos * 3, 255 - pos*3, 0}
	} else if pos < 170 {
		pos -= 85
		return [3]uint8{255 - pos*3, 0, pos * 3}
	} else {
		pos -= 170
		return [3]uint8{0, pos * 3, 255 - pos*3}
	}
}

func main() {
	vaporlight.Main(rainbow)
}
