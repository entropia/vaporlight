#[macro_use] extern crate log;
extern crate env_logger;
extern crate vaporlight;

use vaporlight::Color;
use std::thread::sleep;
use std::time::Duration;

fn animate(con: &mut vaporlight::Controller, num_leds: u16) {
    for i in 0u16..num_leds {
        con.set(i, Color::RGB8{r: 0, g: 0, b: 0})
            .expect("connection error!");
        con.strobe().expect("connection error!");
    }
    let mut count= 0u8;
    loop {
        count = (count as u16 + 1) as u8;
        for i in 0u16..num_leds {
            con.set(i, wheel((((i*256/num_leds) as u16) + count as u16) as u8))
                .expect("connection error!");
        }
        con.strobe().expect("connection error!");
        sleep(Duration::from_millis(40));
    }
}

fn wheel(pos: u8) -> Color {
    let mut pos = pos;
	if pos < 85 {
		Color::RGB8{r: pos * 3, g: 255 - pos*3, b: 0}
	} else if pos < 170 {
		pos -= 85;
		Color::RGB8{r: 255 - pos*3, g: 0, b: pos * 3}
	} else {
		pos -= 170;
		Color::RGB8{r: 0, g: pos * 3, b: 255 - pos*3}
	}
}

fn main() {
    env_logger::init().unwrap();
    vaporlight::run(animate);
}
