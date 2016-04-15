import time

import math

import vaporconfig
cfg = vaporconfig.VaporManager()

steps = 1500
i = 0

while 1:
    for n in range(0, steps):
        m = n / float(steps)
        for id in range(0, 35):
            cfg.setHSV(id, [(m + 0.2 * (id / 20.0)) % 1, 1, 0.4])
        i = (i + 1) % 600
        cfg.setMaster((1 + math.cos(math.pi * 2 * i / 200.0)) * 0.5 + 0.5)
        cfg.update()
        time.sleep(1/60.0)
