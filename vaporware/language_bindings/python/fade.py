import time
import llvp


def main(light, num_leds):
    # we only use 1/10th of the possible resolution
    while True:
        for intensity in xrange(0, 65535, 10): # fade up
            apply(light, num_leds, intensity)
            print(intensity)
        for intensity in xrange(65535, -1, -10): # fade down
            apply(light, num_leds, intensity)

def apply(light, num_leds, intensity):
    for led in xrange(num_leds):
        light.set_rgb_hi(led, (intensity, intensity, intensity))
    light.strobe()
    time.sleep(0.001)


if __name__ == "__main__":
    llvp.main(main)

