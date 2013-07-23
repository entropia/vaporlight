import itertools
import time

import llvp


def main(light, num_leds):
    colors = [
        (0, 0, 0, 0),
        (0, 255, 0, 255),
        (255, 0, 0, 255),
        (0, 0, 255, 255)]
    for i in itertools.count(-1):
        for k, color in enumerate(colors):
            light.set_rgba((i + k) % num_leds, color)
        light.strobe()
        time.sleep(0.3)

if __name__ == "__main__":
    llvp.main(main)

