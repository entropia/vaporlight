import config, colorsys

#FIXME: Dynamic channel asignment through config, adding support for rgbw etc
from vaporbus import VaporBus
from vaporprotocol import VaporProtocol


class VaporManager:
    def __init__(self):
        self.bus = VaporBus(config.port['device'], config.port['baudrate'])
        self.proto = VaporProtocol(self.bus)
        self.device_buffer = {}
        self.devices = config.lights
        for light, n in enumerate(self.devices):
            devAddr = self.devices[light][0]
            if devAddr not in self.device_buffer:
                self.device_buffer[devAddr] = [0.0] * 16
        self.master = 1.0
    def setRGB(self, id, rgb=[], alpha=1.0):
        if len(rgb) != 3:
            return
        if id not in range(0, len(self.devices)):
            return
        addr, rgb_ids = self.devices[id]
        for i, n in enumerate(rgb_ids):
            self.device_buffer[addr][n] = alpha * rgb[i] + (1 - alpha) * self.device_buffer[addr][n]
    def setHSV(self, id, hsv=[]):
        if len(hsv) != 3:
            return
        h, s, v = hsv
        r, g, b = colorsys.hsv_to_rgb(h, s, v)
        self.setRGB(id, [r, g, b])
    def setMaster(self, master=1.0):
        self.master = master
    def update(self):
        for device in self.device_buffer:
            self.proto.send_raw(device, [n  * self.master for n in self.device_buffer[device]])
        self.proto.send_strobe()