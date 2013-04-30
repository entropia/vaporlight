import colorsys
import math
import sys
import time

import login


def main():
    light = login.connect()

    # warmup
    j = 1e9
    RAINBOW = [
        (128, 0, 0),
        (128, 128, 0),
        (0, 255, 0),
        (0, 0, 255),
        (128, 0, 255)
    ]
    for i in xrange(256):
        j -= 0.01
        time.sleep(0.005)
        for module in xrange(4):
            for color_num in xrange(5):
                color_a = RAINBOW[(int(j) + color_num) % len(RAINBOW)]
                color_b = RAINBOW[(int(j) + color_num + 1) % len(RAINBOW)]
                color = interpolate(color_a, color_b, j % 1.0)
                light.set_rgb(module * 5 + color_num, color)
        light.strobe()

    # magic
    for i in xrange(20):
        light.set_rgb(i, (23,42,123))
    light.strobe()

    # pause before test
    time.sleep(2)

    # test
    light.set_rgb(1, (255,0,255))
    light.strobe()

    while True:
        time.sleep(10)
    light.close()

def interpolate(c2, c1, i): # i = 0...1
    return (
        int(c1[0] * i + c2[0] * (1-i)),
        int(c1[1] * i + c2[1] * (1-i)),
        int(c1[2] * i + c2[2] * (1-i)))


if __name__ == "__main__":
    main()

