"""
ugly ugly code

${someone} should rewrite this
"""

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
#        [(i*4, i*1, 0) for i in xrange(64)] +
        [(255,i*3,0) for i in xrange(64)] +
        [(180,0,0)])
#    PALETTE = (
#        #[(i*4, i*1, 0) for i in xrange(64)] +
#        #[(255*j,(64+i)*j,0) for i in xrange(96) for j in (0.95,0.975,1)])
#        [(0,(128+4*i),0) for i in xrange(96) for j in (0.95,0.975,1)])
    BRIGHTNESS = 1.0


    def __init__(self):
        self.steps = 200
        self.step = random.randrange(0, 200)
        self.color_a = (0, 0, 0)
        self.color_b = random.choice(self.PALETTE)

    def next(self, current_auto_brightness):
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
            int(current_auto_brightness * self.BRIGHTNESS * (r0 * a_factor + r1 * b_factor)),
            int(current_auto_brightness * self.BRIGHTNESS * (g0 * a_factor + g1 * b_factor)),
            int(current_auto_brightness * self.BRIGHTNESS * (b0 * a_factor + b1 * b_factor)))


def main(light, num_leds):
    MIN_AUTO_BRIGHTNESS = 0.15
    MAX_AUTO_BRIGHTNESS = 1.0
    AUTO_BRIGHTNESS_HYSTERESIS = 0.9995

    leds = [Led() for i in xrange(num_leds)]
    last_auto_brightness = 0.0
    for i in itertools.count():
        last_auto_brightness = last_auto_brightness * AUTO_BRIGHTNESS_HYSTERESIS \
            + (1.0 - AUTO_BRIGHTNESS_HYSTERESIS) * (MIN_AUTO_BRIGHTNESS + random.random()*(MAX_AUTO_BRIGHTNESS-MIN_AUTO_BRIGHTNESS))
        for j, led in enumerate(leds):
            light.set_rgb(j, led.next(last_auto_brightness))
            #light.set_rgba(j, (50, 100, 150, 200)) # FIXME this is debug code
        light.strobe()

        #if i % 20 == 0: # Flickering
        #    swap2neighbors(leds)

        time.sleep(random.random() * 0.00005 + 0.00001)

    light.close()


def swap2neighbors(array):
    if len(array) <= 1:
        return
    a = random.randint(0, len(array) - 2)
    array[a], array[a+1] = array[a+1], array[a]

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

