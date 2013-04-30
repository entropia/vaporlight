import colorsys
import math
import sys
import time

import login


RAINBOW = [
    (255, 0, 0),
    (255, 255, 0), # or should it be (128, 128, 0)?
    (0, 255, 0),   # we couldnt really make out a difference
    (0, 255, 255), # so whatever
    (0, 0, 255),
    (255, 0, 255),
]

def main():
    light = login.connect()
    j = 1e9
    while True:
        j -= 0.01
        time.sleep(0.005)
        for module in xrange(4):
            for color_num in xrange(5):
                color_a = RAINBOW[(int(j) + color_num) % len(RAINBOW)]
                color_b = RAINBOW[(int(j) + color_num + 1) % len(RAINBOW)]
                color = interpolate(color_a, color_b, 1. - j % 1.0)
                light.set_rgb(module * 5 + color_num, color)
        light.strobe()
    while True:
        time.sleep(10)
    light.close()

def interpolate(c1, c2, i): # i = 0...1
    return (
        int(c1[0] * i + c2[0] * (1-i)),
        int(c1[1] * i + c2[1] * (1-i)),
        int(c1[2] * i + c2[2] * (1-i)))

if __name__ == "__main__":
    main()

