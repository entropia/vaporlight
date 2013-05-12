"""Implementation of the Low-Level Vaporlight Protocol."""

import socket


__version__ = "0.2.0"


class FilelikeController(object):

    def __init__(self, f, token):
        self._filelike = f
        self._send(AuthenticateCommand(token))

    def _send(self, command):
        self._filelike.sendall(command.to_str())

    def set_rgb(self, led, rgb, a=255):
        self._send(SetLedCommand(led, (rgb[0], rgb[1], rgb[2], a)))

    def set_rgba(self, led, rgba):
        self._send(SetLedCommand(led, rgba))

    def set_rgb_a(self, led, rgb, a=255):
        """deprecated. for backwards compatibility only."""
        self.set_rgb(led, rgb, a)

    def strobe(self):
        self._send(StrobeCommand())

    def close(self):
        self._filelike.close()


class SocketController(FilelikeController):

    def __init__(self, token, host, port=7534):
        sck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print("connecting to %s, %s" % (host, port))
        sck.connect((host, port))
        super(SocketController, self).__init__(sck, token)


class NetProtocol(object):

    def read(self):
        opcode = yield
        while True:
            if opcode == SetLedCommand.opcode:
                opcode = yield SetLedCommand(ord((yield))<<8|ord((yield)),
                    (ord((yield)), ord((yield)), ord((yield)), ord((yield))))
            elif opcode == StrobeCommand.opcode:
                opcode = yield StrobeCommand()
            elif opcode == AuthenticateCommand.opcode:
                opcode = yield AuthenticateCommand([(yield) for i in range(16)])
            else:
                raise ProtocolError()

class UnexpectedSyncError(Exception):
    pass

class BusProtocol(object):

    STROBE_PADDR = 0xfe
    START_OF_CMD_MARKER = 0x55
    ESCAPE_BYTE = 0x54
    UNESCAPE = {0x00: 0x54,
                0x01: 0x55}

    module_lens = []

    def __init__(self, lens):
        self.module_lens = lens

    def read(self):
        first_byte_after_cmd = 0x00
        while True:
            if first_byte_after_cmd != 0x55:
                while ord((yield)) != 0x55:
                    print("skipping")
                    pass

            addr = ord((yield))
            if addr == self.ESCAPE_BYTE:
                addr = self.UNESCAPE[(yield)]
            elif addr == self.START_OF_CMD_MARKER:
                raise UnexpectedSyncError()

            if addr == self.STROBE_PADDR:
                first_byte_after_cmd = ord((yield BusStrobeCommand()))
            elif addr == self.START_OF_CMD_MARKER:
                raise UnexpectedSyncError()
            else:
                continue_reading = self.module_lens[addr]
                values = []
                while continue_reading:
                    new_values = [ord((yield)) for _ in range(continue_reading)]

                    if self.START_OF_CMD_MARKER in new_values:
                        raise UnexpectedSyncError()

                    num_of_escapes = len([True for val in new_values if val == self.ESCAPE_BYTE])
                    continue_reading = num_of_escapes

                    values.extend(new_values)

                unescaped = []
                escaped = False
                for val in values:
                    if val == self.ESCAPE_BYTE:
                        escaped = True
                    else:
                        if escaped:
                            unescaped.append(self.UNESCAPE[val])
                            escaped = False
                        else:
                            unescaped.append(val)

                first_byte_after_cmd = ord((yield BusSetChannelsCommand(addr, unescaped)))


class StrobeCommand(object):
    opcode = "\xff"

    def to_str(self):
        return self.opcode


class BusStrobeCommand(object):
    opcode = BusProtocol.STROBE_PADDR

    def to_str(self):
        return chr(BusProtocol.START_OF_CMD_MARKER) +\
               chr(BusProtocol.STROBE_PADDR)


class BusSetChannelsCommand(object):
    def __init__(self, mod_id, channels):
        self.mod_id = mod_id
        self.channels = channels

    def to_str(self):
        res = chr(BusProtocol.START_OF_CMD_MARKER)
        for value in [self.mod_id] + self.channels:
            if value in BusProtocol.UNESCAPE.values():
                res.append(chr(BusProtocol.ESCAPE_BYTE) +
                           chr(value - BusProtocol.ESCAPE_BYTE))
        return res


class SetLedCommand(object):
    opcode = "\x01"

    def __init__(self, led, color):
        self.led = led
        self._color = color

    def to_str(self):
        return "".join((self.opcode,
            chr(self.led >> 8),
            chr(self.led & 255),
            chr(self._color[0]),
            chr(self._color[1]),
            chr(self._color[2]),
            chr(self._color[3])))

    @property
    def rgb(self):
        return self._color[:3]

    @property
    def rgba(self):
        return self._color


class AuthenticateCommand(object):
    opcode = "\x02"

    def __init__(self, token):
        self._token = token;

    def to_str(self):
        return self.opcode + self._token

    @property
    def token(self):
        return self._token


class ProtocolError(Exception):
    pass

