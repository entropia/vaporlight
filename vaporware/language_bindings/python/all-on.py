import llvp


def main(light, num_leds):
    for i in xrange(num_leds):
        light.set_rgb(i, (255, 255, 255))
    light.strobe()
    light.done()


if __name__ == "__main__":
    llvp.main(main)

