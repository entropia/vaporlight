import colorsys
import socket, thread, copy

CMD_AUTH = 0x02
CMD_SET_LED_16 = 0x03
CMD_SET_LED_8 = 0x01
CMD_STROBE = 0xff

bin2float = lambda x: (x[0] << 8 | x[1]) / float(0xFFFF)
bin2int = lambda x: (x[0] << 8 | x[1])

class VaporClient:
    def __init__(self):
        self.led_data = {}
        self.buffer = {}

    def set_led_rgb(self, id, r, g, b, a):
        self.buffer[id] = [r, g, b, a]

    def set_led_hsv(self, id, h, s, v, a):
        r, g, b = colorsys.hsv_to_rgb(h, s, v)
        self.set_led_rgb(id, r, g, b, a)

    def strobe(self):
        self.led_data = copy.deepcopy(self.buffer)

    def get_led_data(self):
        return self.led_data

class VaporCompatibilityClient(VaporClient):
    def __init__(self, sock, addr, server):
        VaporClient.__init__(self)
        print "new Connection from", addr
        self.sock = sock
        self.addr = addr
        self.authenticated = False
        self.server = server
        self.led_data = {}
        self.buffer = {}
        thread.start_new(self.workThread, tuple())

    def isAuthenticated(self):
        return self.authenticated

    def get_list_of_connections(self):
        return {'auth': self.isAuthenticated(), 'addr': self.addr}

    def auth(self, data):
        self.authenticated = True

    def cmd_set_led_16(self, data):
        led_id = bin2int(data[0:2])
        r = bin2float(data[2:4])
        g = bin2float(data[4:6])
        b = bin2float(data[6:8])
        a = bin2float(data[8:10])
        self.set_led_rgb(led_id, r, g, b, a)

    def cmd_set_led_8(self, data):
        led_id = bin2int(data[0:2])
        r = data[2] / 255.0
        g = data[3] / 255.0
        b = data[4] / 255.0
        a = data[5] / 255.0
        self.set_led_rgb(led_id, r, g, b, a)

    def cmd_strobe(self, tmp=None):
        self.strobe()

    def get_led_data(self):
        return self.led_data

    def workThread(self):
        cmds = {
            CMD_AUTH: [self.auth, 16],
            CMD_SET_LED_16: [self.cmd_set_led_16, 10],
            CMD_SET_LED_8: [self.cmd_set_led_8, 6],
            CMD_STROBE: [self.cmd_strobe, 0]
        }
        while 1:
            try:
                data = self.sock.recv(1024)
            except:
                break
            data = [ord(n) for n in data]
            if len(data) == 0:
                break
            while len(data) > 0:
                if data[0] in cmds:
                    try:
                        cmds[data[0]][0](data[1:1 + cmds[data[0]][1]])
                    except:
                        print "Error in function", data
                        break
                    data = data[1 + cmds[data[0]][1]:]
                else:
                    data = data[1:]
        print "Connection closed", self.addr
        self.server.remove_me(self)


class VaporCompatibilityServer:
    def __init__(self, acceptAllTokens=True):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.bind(("", 7534))
        self.socket.listen(2)
        self.connections = []
        thread.start_new(self.main_loop, tuple())

    def main_loop(self):
        while 1:
            newConnection, addr = self.socket.accept()
            con = VaporCompatibilityClient(newConnection, addr, self)
            self.connections.append(con)

    def remove_me(self, to_remove):
        self.connections.remove(to_remove)

    def get_list_of_connections(self):
        return [n.get_connection_info() for n in self.connections[::]]

    def get_connection_data(self):
        return [n.get_led_data() for n in self.connections[::]]
