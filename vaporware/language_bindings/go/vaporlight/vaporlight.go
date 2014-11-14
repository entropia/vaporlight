package vaporlight

import (
	"io"
	"net"
	"flag"
	"log"
	"strconv"
	"os"
	"fmt"
)

// Function type for animations.
// Implement and run vaporlight.Main(yourfunction)
type Animation func(Controller, int, int)

type Token [16]byte

// Create a token from a string.
func TokenFromStr(str string) (token Token) {
    copy(token[:], []byte(str))
    return
}

//   ____                                          _      //
//  / ___|___  _ __ ___  _ __ ___   __ _ _ __   __| |___  //
// | |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` / __| //
// | |__| (_) | | | | | | | | | | | (_| | | | | (_| \__ \ //
//  \____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|___/ //
//                                                        //

type command []byte

func strobeCommand() command {
	opcode := byte(0xff)
	return command{opcode}
}

func authenticateCommand(token Token) command {
    opcode := byte(0x02)
    cmd := make([]byte, len(token) + 1)
    cmd[0] = opcode
	copy(cmd[1:], token[:])
    return cmd
}

func setLedCommand(led int, rgba [4]uint8) command {
    opcode := byte(0x01)
    cmd := make([]byte, 7)
    cmd[0] = opcode
    cmd[1] = byte(led >> 8)
    cmd[2] = byte(led & 255)
    cmd[3] = rgba[0]
    cmd[4] = rgba[1]
    cmd[5] = rgba[2]
    cmd[6] = rgba[3]
    return cmd
}

func hiResSetLedCommand(led int, rgba [4]uint16) command {
    opcode := byte(0x03)
    cmd := make([]byte, 11)
    cmd[0] = opcode
    cmd[1] = byte(led >> 8)
    cmd[2] = byte(led & 255)
    cmd[3] = byte(rgba[0] >> 8)
    cmd[4] = byte(rgba[0] & 255)
    cmd[5] = byte(rgba[1] >> 8)
    cmd[6] = byte(rgba[1] & 255)
    cmd[7] = byte(rgba[2] >> 8)
    cmd[8] = byte(rgba[2] & 255)
    cmd[9] = byte(rgba[3] >> 8)
    cmd[10] = byte(rgba[3] & 255)
    return cmd
}

//   ____            _             _ _            //
//  / ___|___  _ __ | |_ _ __ ___ | | | ___ _ __  //
// | |   / _ \| '_ \| __| '__/ _ \| | |/ _ \ '__| //
// | |__| (_) | | | | |_| | | (_) | | |  __/ |    //
//  \____\___/|_| |_|\__|_|  \___/|_|_|\___|_|    //
//                                                //

type Controller struct {
	writer io.WriteCloser
	token Token
}

func (con Controller) send(cmd command) {
	con.writer.Write(cmd)
}

// Authenticate against the Vaporlight daemon.
// Needs to be done first
func (con Controller) Authenticate() {
	log.Println("Authenticating using token", string(con.token[:]))
	con.send(authenticateCommand(con.token))
}

// Set one led to 8 bit rgb values, alpha at max
func (con Controller) SetRgb8(led int, rgb [3]uint8) {
	rgba := [4]uint8{rgb[0], rgb[1], rgb[2], 255}
	con.SetRgba8(led, rgba)
}

// Set one led to 8 bit rgba values
func (con Controller) SetRgba8(led int, rgba [4]uint8) {
	con.send(setLedCommand(led, rgba))
}

// Set one led to 16 bit rgb values, alpha at max
func (con Controller) SetRgb16(led int, rgb [3]uint16) {
	rgba := [4]uint16{rgb[0], rgb[1], rgb[2], 65535}
	con.SetRgba16(led, rgba)
}

// Set one led to 16 bit rgba values
func (con Controller) SetRgba16(led int, rgba [4]uint16) {
	con.send(hiResSetLedCommand(led, rgba))
}

// Apply set values
func (con Controller) Strobe() {
	con.send(strobeCommand())
}

// Close connection.
// Writing after close will fail.
func (con Controller) Close() {
	con.Close()
	log.Println("Connection closed")
}

// Create a Controller connecting via TCP
func SocketController(token Token, host string, port int) Controller {
	addr := net.JoinHostPort(host, strconv.Itoa(port))
	log.Println("Connecting to", addr)
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		log.Fatal("Could not connect!")
	}
	return Controller{conn, token}
}


//  __  __       _        //
// |  \/  | __ _(_)_ __   //
// | |\/| |/ _` | | '_ \  //
// | |  | | (_| | | | | | //
// |_|  |_|\__,_|_|_| |_| //
//                        //

func Main(f Animation) {
	var leds = flag.Int("leds", 5, "number of leds")
	var host = flag.String("host", "localhost", "the vaporlight server's hostname or IP address")
	var port = flag.Int("port", 7534, "port the vaporlight server runs on")
	var token = TokenFromStr(*flag.String("token", "sixteen letters.", "token to authenticate with"))
	var speed = flag.Int("speed", 5, "animation speed between 1 and 10")
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, "Usage of %s:\n", os.Args[0])
		flag.PrintDefaults()
	}
	flag.Parse()
	if *speed < 1 || *speed > 10 {
		log.Fatal("animation speed not in range 1..10")
	}
	dev := SocketController(token, *host, *port)
	dev.Authenticate()

	f(dev, *leds, *speed)
}
