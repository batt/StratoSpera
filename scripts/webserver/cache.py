#!/usr/bin/env python
from globalmaptiles import GlobalMercator
import urllib
import os
import random

def ensure_dir(f):
	d = os.path.dirname(f)
	if not os.path.exists(d):
		os.makedirs(d)

radius = 150 #km
lat, lon = 43.606167, 11.311833
tzmax = 13
mercator = GlobalMercator()
radius *= 1000

mx, my = mercator.LatLonToMeters( lat, lon )
print "Spherical Mercator (ESPG:900913) coordinates for lat/lon: "
print (mx, my)

for tz in range(tzmax + 1):
	tminx, tminy = mercator.MetersToTile(mx - radius, my - radius, tz )
	tmaxx, tmaxy = mercator.MetersToTile(mx + radius, my + radius, tz )

	for ty in range(tminy, tmaxy+1):
		for tx in range(tminx, tmaxx+1):
			tilefilename = "%s/%s/%s" % (tz, tx, ty)
			gx, gy = mercator.GoogleTile(tx, ty, tz)
			path = "maps/%d/%d/%d.png" % (tz, gx, gy)

			if not os.path.exists(path):
				url = "http://" + random.choice("abc") + ".tile.opencyclemap.org/cycle" + path.replace("maps", "")

				print "GETTING", url, "-->",
				u = urllib.urlopen(url)
				if u.getcode() != 404:
					ensure_dir(path)
					f = open(path, "wb").write(u.read())
					print "OK"
				else:
					print "FAIL"
				u.close()
			else:
				print path, "--> PRESENT"

