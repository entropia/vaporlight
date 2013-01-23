import colorsys
import math
import sys
import time

import login


def main():
    light = login.connect()
    for i in xrange(login.NUM_LEDS):
        light.set_rgb(i, (255, 255, 255))
    light.strobe()
    while True:
        time.sleep(10)
    light.close()


if __name__ == "__main__":
    main()

