import math
import time
import itertools

import login

dev = login.connect()
last_j = 0
for i in itertools.count():
    j = int(math.sin(i / 15.) * 7.5 + 7.5)
    dev.set_rgb(last_j, (0, 0, 0))
    dev.set_rgb(j, (255, 255, 255))
    last_j = j
    dev.strobe()
    time.sleep(0.01)

