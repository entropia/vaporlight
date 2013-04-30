import colorsys
import math
import sys
import time

import login


RAINBOW = [
    (128, 0, 0),
    (128, 128, 0),
    (0, 255, 0),
    (0, 0, 255),
    (128, 0, 255)
]

def main():
    light = login.connect()
    sleep = 0.01
    while True:
        for intensity in xrange(255):
            for led in xrange(login.NUM_LEDS):
                light.set_rgb(led, (intensity, intensity, intensity))
            light.strobe()
            time.sleep(sleep)
        for intensity in xrange(255, -1, -1):
            for led in xrange(login.NUM_LEDS):
                light.set_rgb(led, (intensity, intensity, intensity))
            light.strobe()
            time.sleep(sleep)

if __name__ == "__main__":
    main()

