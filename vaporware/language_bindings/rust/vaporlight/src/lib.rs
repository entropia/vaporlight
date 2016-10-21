#[macro_use]
extern crate log;
extern crate docopt;
extern crate rustc_serialize;

use std::error::Error;
use std::fmt;
use std::net::TcpStream;
use std::io::prelude::*;
use docopt::Docopt;

type Token = [u8; 16];

/// Color types
pub enum Color {
    RGB8 { r: u8, g: u8, b: u8 },
    RGBA8 { r: u8, g: u8, b: u8, a: u8 },
    RGB16 { r: u16, g: u16, b: u16 },
    RGBA16 { r: u16, g: u16, b: u16, a: u16 },
}

/// Vaporlight controller
///
/// represents a connection to the vporlight server
pub struct Controller {
    writer: Box<Write>,
    token: Token,
}

#[derive(Debug)]
struct InvalidTokenError;

impl fmt::Display for InvalidTokenError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Invalid token error")
    }
}

impl Error for InvalidTokenError {
    fn description(&self) -> &str {
        "The given token is invalid (must be 16 bytes)."
    }
}

impl Controller {
    fn send(&mut self, cmd: &[u8]) -> Result<(), std::io::Error> {
        match self.writer.write(cmd) {
            Ok(_) => Ok(()),
            Err(e) => Err(e),
        }
    }

    /// Send the authentication token
    pub fn authenticate(&mut self) -> Result<(), std::io::Error> {
        info!("Authenticating using token {}",
              String::from_utf8_lossy(&self.token[..]));

        let opcode: u8 = 0x02;
        let mut cmd = [0u8; 17];
        cmd[0] = opcode;
        cmd[1..17].clone_from_slice(&self.token[..]);

        self.send(&cmd[..])
    }

    /// Send the strobe command
    pub fn strobe(&mut self) -> Result<(), std::io::Error> {
        let opcode: u8 = 0xff;
        let cmd = [opcode];

        self.send(&cmd[..])
    }

    /// Set color for one led
    pub fn set(&mut self, led: u16, c: Color) -> Result<(), std::io::Error> {
        match c {
            Color::RGB8 { r, g, b } => self.set_rgb_8(led, r, g, b),
            Color::RGBA8 { r, g, b, a } => self.set_rgba_8(led, r, g, b, a),
            Color::RGB16 { r, g, b } => self.set_rgb_16(led, r, g, b),
            Color::RGBA16 { r, g, b, a } => self.set_rgba_16(led, r, g, b, a),
        }
    }

    /// Set led as 8 bit RGB values
    pub fn set_rgb_8(&mut self, led: u16, r: u8, g: u8, b: u8) -> Result<(), std::io::Error> {
        self.set_rgba_8(led, r, g, b, 255)
    }

    /// Set led as 8 bit RGBA values
    pub fn set_rgba_8(&mut self, led: u16, r: u8, g: u8, b: u8, a: u8) -> Result<(), std::io::Error> {
        let opcode: u8 = 0x01;

        let mut cmd = [0u8; 7];
        cmd[0] = opcode;
        cmd[1] = (led >> 8) as u8;
        cmd[2] = (led & 0xff) as u8;
        cmd[3] = r;
        cmd[4] = g;
        cmd[5] = b;
        cmd[6] = a;

        self.send(&cmd[..])
    }

    /// Set led as 16 bit RGB values
    pub fn set_rgb_16(&mut self, led: u16, r: u16, g: u16, b: u16) -> Result<(), std::io::Error> {
        self.set_rgba_16(led, r, g, b, 65535)
    }

    /// Set led as 16 bit RGBA values
    pub fn set_rgba_16(&mut self, led: u16, r: u16, g: u16, b: u16, a: u16) -> Result<(), std::io::Error> {
        let opcode: u8 = 0x03;

        let mut cmd = [0u8; 11];
        cmd[0] = opcode;
        cmd[1] = (led >> 8) as u8;
        cmd[2] = (led & 0xff) as u8;
        cmd[3] = (r >> 8) as u8;;
        cmd[4] = (r & 0xff) as u8;;
        cmd[5] = (g >> 8) as u8;;
        cmd[6] = (g & 0xff) as u8;;
        cmd[7] = (b >> 8) as u8;;
        cmd[8] = (b & 0xff) as u8;;
        cmd[9] = (a >> 8) as u8;;
        cmd[10] = (a & 0xff) as u8;;

        self.send(&cmd[..])
    }
}

fn new_tcp_controller(host: &str, port: u16, token: Token) -> std::io::Result<Controller> {
    let sock = try!(TcpStream::connect((host, port)));
    Ok(
        Controller {
            writer: Box::new(sock), // consume sock, but use Box to have a Sized object (traits like Write are not Sized)
            token: token,
        }
      )
}

#[derive(RustcDecodable)]
struct Args {
    flag_leds: u16,
    flag_host: String,
    flag_port: u16,
    flag_token: String,
}

fn token_from_string(s: &str) -> Result<Token, InvalidTokenError> {
    if s.len() < 16 {
        return Err(InvalidTokenError)
    }
    let bytes = s.as_bytes();
    let mut token = [0u8; 16];
    for i in 0..16 {
        token[i] = bytes[i];
    }
    Ok(token)
}

fn exit_error(e: &Error) -> ! {
    error!("{}", e);
    std::process::exit(1);
}

/// Main function.
/// Parses cli args, sets up a connection and runs the passed animation function.
/// Run this with your animation.
pub fn run<F>(animation: F)
    where F: Fn(&mut Controller, u16)
{
    let args: Vec<_> = std::env::args().collect();
    let exec_name = &args[0];
    let usage = format!("
Usage: 
    {0} --help
    {0} [options]

Options:
  --help                      Print help.
  -l LEDS, --leds=LEDS        Number of leds. [default: 5]
  -h HOST, --host=HOST        Hostname of vaporlight server. [default: localhost]
  -p PORT, --port=PORT        Port number of vaporlight server. [default: 7534]
  -t TOKEN, --token=TOKEN     Authentication token (16 bytes, rest is ignored). [default: sixteen letters.]
", exec_name);

    let args: Args = Docopt::new(usage)
        .and_then(|d| d.argv(&args).help(true).decode())
        .unwrap_or_else(|e| e.exit());
    let token: [u8; 16] = token_from_string(args.flag_token.as_str())
        .unwrap_or_else(|e| exit_error(&e));

    let mut con = new_tcp_controller(args.flag_host.as_str(), args.flag_port, token)
        .unwrap_or_else(|e| exit_error(&e));
    con.authenticate().expect("Connection error!");
    animation(&mut con, args.flag_leds);
}
