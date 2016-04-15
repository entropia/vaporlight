import serial

FRAME_START = 0x55

class VaporBus:
    def __init__(self, dev="/dev/ttyUSB0", baudrate=500000):
        self.bus = serial.Serial(dev, baudrate=baudrate)
    def send_frame(self, data):
        if type(data) == list:
            data = "".join(map(chr, data))
        data = data.replace("\x54", "\x54\x00").replace("\x55", "\x54\x01")

        self.bus.write(
            chr(FRAME_START) + data
        )