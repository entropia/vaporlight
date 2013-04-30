import colorsys
import itertools
import math
import sys
import time

import login


def main():
    colors = [
        (0, 0, 0, 0),
        (0, 255, 0, 255),
        (255, 0, 0, 255),
        (0, 0, 255, 255)]
    light = login.connect()
    for i in itertools.count(-1):
        for k, color in enumerate(colors):
            light.set_rgba((i + k) % login.NUM_LEDS, color)
        light.strobe()
        time.sleep(0.1)

if __name__ == "__main__":
    main()

