ADDR_BROADCAST = 0xFF

CMD_STROBE = 0xFF
CMD_RAW = 0x00

int_to_bin = lambda i: [(i >> 8) & 0xFF, i & 0xFF]

class VaporProtocol:
    def __init__(self, bus=None):
        self.bus = bus
    def set_bus(self, bus):
        self.bus = bus
    def send_strobe(self, addr=ADDR_BROADCAST):
        self.bus.send_frame([addr, CMD_STROBE])

    def send_raw(self, addr, raw_data=[0.0] * 16):
        if len(raw_data) < 16:
            raw_data += [0.0] * (16 - len(raw_data))
        bin_out = [addr, CMD_RAW]
        for i in raw_data:
            bin_out += int_to_bin(int(min(1.0, max(0.0, i)) * 0xFFFF))
        self.bus.send_frame(bin_out)