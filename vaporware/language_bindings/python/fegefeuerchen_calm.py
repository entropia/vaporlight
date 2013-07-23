import itertools
import random
import colorsys
import math
import sys
import time

import llvp


class Led(object):
    LIMIT = 64
    PALETTE = (
        [(i*4, i*1, 0) for i in xrange(64)] +
        [(255,i*3,0) for i in xrange(64)] +
        [(180,0,0)])

    def __init__(self):
        self.step = 1
        self.steps = 200
        self.color_a = (0, 0, 0)
        self.color_b = random.choice(self.PALETTE)

    def next(self):
        self.step += 1
        if self.step > self.steps:
            self.step = 0
            self.color_a = self.color_b
            #self.color_b = (255, 0, 0) #random.choice(self.PALETTE)
            self.color_b = random.choice(self.PALETTE)
        x = math.pi * 0.5 * self.step / self.steps
        a_factor = (math.cos(x) ** 2.0)
        b_factor = (math.sin(x) ** 2.0)
        r0, g0, b0 = self.color_a
        r1, g1, b1 = self.color_b
        return (
            int(r0 * a_factor + r1 * b_factor),
            int(g0 * a_factor + g1 * b_factor),
            int(b0 * a_factor + b1 * b_factor))


def main(light, num_leds):
    leds = [Led() for i in xrange(num_leds)]
    for i in itertools.count():
        for j, led in enumerate(leds):
            light.set_rgb(j, led.next())
        light.strobe()
        #if i % 20 == 0: # Flickering
        #    swap2(leds)
        #time.sleep(0.005)
        time.sleep(random.random() * 0.005 + 0.001)

    light.close()

def swap2(array):
    if len(array) <= 1:
        return
    a = random.randrange(0, len(array))
    while True:
        b = random.randrange(0, len(array))
        if a != b:
            array[a], array[b] = array[b], array[a]
            return


if __name__ == "__main__":
    llvp.main(main)

