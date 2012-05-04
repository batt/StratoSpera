#!/usr/bin/env python

import sys
import struct
import math

f = open(sys.argv[1])
print f.readline(),
i = 0
while 1:
	raw = f.read(6)
	if not raw:
		break

	acc = struct.unpack(">hhh", raw)
	x, y, z = map(lambda a: 9.80665/128 * a / 64, acc)
	tot = math.sqrt(x*x+y*y+z*z)
	print "%d;%.2f;%.2f;%.2f;%.2f" % (i, x, y, z, tot)
	i += 1
