import math
import time
import itertools

import llvp


def main(light, num_leds):
    for i in xrange(num_leds):
        light.set_rgba(i, (0, 0, 0, 255))

    for j in itertools.count():
        for i in xrange(num_leds):
            light.set_rgb(i, wheel((i * 256 / num_leds + (j % 256)) & 255))
        light.strobe()
        time.sleep(0.04)


def wheel(pos):
    if pos < 85:
        return (pos * 3, 255 - pos * 3, 0)
    elif pos < 170:
        pos -= 85
        return (255 - pos * 3, 0, pos * 3)
    else:
        pos -= 170
        return (0, pos * 3, 255 - pos * 3)


if __name__ == "__main__":
    llvp.main(main)

