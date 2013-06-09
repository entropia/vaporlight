import math
import time
import itertools

import login

dev = login.connect()
last_j = 0
for i in xrange(20):
    dev.set_rgba(i, (0, 0, 0, 255))
max_led = 20
for i in itertools.count():
    j = int(math.sin(i / float(max_led)) * float(max_led)/2 + float(max_led)/2)
    dev.set_rgb(last_j, (0, 0, 0))
    dev.set_rgb(j, (255, 255, 255))
    last_j = j
    dev.strobe()
    time.sleep(0.01)

