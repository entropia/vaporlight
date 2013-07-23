import math
import time
import itertools

import llvp


def main(light, num_leds):
    last_j = 0
    for i in xrange(num_leds):
        light.set_rgba(i, (0, 0, 0, 255))
    for i in itertools.count():
        j = int(math.sin(i / float(num_leds)) * float(num_leds)/2 + float(num_leds)/2)
        light.set_rgb(last_j, (0, 0, 0))
        light.set_rgb(j, (255, 255, 255))
        last_j = j
        light.strobe()
        time.sleep(0.01)


if __name__ == "__main__":
    llvp.main(main)

