#!/bin/python2
import time

import math

from vapormanager import VaporManager
from vapornet import VaporCompatibilityServer
from vacuum import ExecJSManager
cfg = VaporManager()
server = VaporCompatibilityServer()
js = ExecJSManager()
js.run_file("vaporscripts/rainbow.js")
js.run_file("vaporscripts/fire.js")

steps = 1500
i = 0
n = 0

initial_fade_in = 0.0



while 1:
    if initial_fade_in < 1.0:
        initial_fade_in += 0.002
    cfg.setMaster(initial_fade_in)
    js.render()

    for client in server.get_connection_data() + js.get_data():
        for light_id in client:
            cfg.setRGB(light_id, client[light_id][0:3], client[light_id][3])
    cfg.update()
    time.sleep(1/50.0)
