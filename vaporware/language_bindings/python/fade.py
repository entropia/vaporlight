import time
import llvp


def main(light, num_leds):
    while True:
        for intensity in xrange(255): # fade up
            apply(light, num_leds, intensity)
        for intensity in xrange(255, -1, -1): # fade down
            apply(light, num_leds, intensity)

def apply(light, num_leds, intensity):
    for led in xrange(num_leds):
        light.set_rgb(led, (intensity, intensity, intensity))
    light.strobe()
    time.sleep(0.02)


if __name__ == "__main__":
    llvp.main(main)

