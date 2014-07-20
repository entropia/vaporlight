import time
import llvp


BRIGHTNESS = 1.0
RAINBOW = [
    (128, 0, 0),
    (128, 128, 0),
    (0, 255, 0),
    (0, 0, 255),
    (128, 0, 255)
]


def main(light, num_lights):
    j = 1e9
    while True:
        j += 0.025
        time.sleep(0.01)
        for module in xrange(num_lights // 5):
            for color_num in xrange(4):
                color_a = RAINBOW[(int(j) + color_num) % len(RAINBOW)]
                color_b = RAINBOW[(int(j) + color_num + 1) % len(RAINBOW)]
                color = interpolate(color_a, color_b, j % 1.0)
                color = [int(x * BRIGHTNESS) for x in color]
                light.set_rgb(module * 5 + color_num, color)
        light.strobe()

def interpolate(c2, c1, i): # i = 0...1
    return (
        int(c1[0] * i + c2[0] * (1-i)),
        int(c1[1] * i + c2[1] * (1-i)),
        int(c1[2] * i + c2[2] * (1-i)))


if __name__ == "__main__":
    llvp.main(main)

