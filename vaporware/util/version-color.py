#!/usr/bin/env python2

"""
usage: version-color.py [git_refspec]

when the vaporlight firmware, it shows the git version it was built
from for a few seconds as a color code. this script can be used to
find out the color code for some (default: HEAD) git commit.
"""

import subprocess
import sys

rev = "HEAD" if len(sys.argv) < 2 else sys.argv[1]
sha1 = subprocess.check_output(["git", "rev-parse", rev]).strip()
num = int(sha1[0:4], 16)

b0_r = (num & (1<<15)) != 0
b0_g = (num & (1<<14)) != 0
b0_b = (num & (1<<13)) != 0
b1_r = (num & (1<<12)) != 0
b1_g = (num & (1<<11)) != 0
b1_b = (num & (1<<10)) != 0
b2_r = (num & (1<<9)) != 0
b2_g = (num & (1<<8)) != 0
b2_b = (num & (1<<7)) != 0
b3_r = (num & (1<<6)) != 0
b3_g = (num & (1<<5)) != 0
b3_b = (num & (1<<4)) != 0
b4_r = (num & (1<<3)) != 0
b4_g = (num & (1<<2)) != 0
b4_b = (num & (1<<1)) != 0

colors = {
    (0,0,0): "black",
    (0,0,1): "blue",
    (0,1,0): "green",
    (0,1,1): "cyan",
    (1,0,0): "red",
    (1,0,1): "purple",
    (1,1,0): "yellow",
    (1,1,1): "some kind of white",
}

print "git commit: %s" % sha1
print "=> [%s]_10" % (num & 0b1111111111111110)
print "=> [%s]_2" % bin(num & 0b1111111111111110)[2:]
print

print "%i%i%i => %s" % (b0_r, b0_g, b0_b, colors[b0_r, b0_g, b0_b])
print "%i%i%i => %s" % (b1_r, b1_g, b1_b, colors[b1_r, b1_g, b1_b])
print "%i%i%i => %s" % (b2_r, b2_g, b2_b, colors[b2_r, b2_g, b2_b])
print "%i%i%i => %s" % (b3_r, b3_g, b3_b, colors[b3_r, b3_g, b3_b])
print "%i%i%i => %s" % (b4_r, b4_g, b4_b, colors[b4_r, b4_g, b4_b])

