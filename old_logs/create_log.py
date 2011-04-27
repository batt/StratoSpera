#!/usr/bin/python
import random

start_lat = 43.606463
start_lon = 11.311823
end_lat = 43.253265
end_lon = 11.857652
alt = [[0, 260],
	   [3600, 260],
	   [7200+3600, 27500],
	   [5400+3600+3600, 200],
	   [9000+3600+3600, 260],
	   ]
delay = 3
alt_err = 100
press_err = 10

delta_lat = end_lat - start_lat
delta_lon = end_lon - start_lon
end = alt[-1][0]
alt_idx = 0

for t in range(0, end, delay):
	if (t >= alt[alt_idx+1][0]):
		alt_idx += 1
	prev_t = alt[alt_idx][0]
	next_t = alt[alt_idx+1][0]
	delta_t = next_t - prev_t

	prev_alt = alt[alt_idx][1]
	next_alt = alt[alt_idx+1][1]
	delta_alt = next_alt - prev_alt

	curr_alt = int((float(t - prev_t) / float(delta_t)) * delta_alt + prev_alt)
	curr_press = int(1013.25 * ((1 - 2.25577e-5 * curr_alt) ** 5.25588))
	curr_alt += random.randint(-alt_err, alt_err)
	curr_press += random.randint(-press_err, press_err)

	now = t / float(end)
	curr_lat = now * delta_lat + start_lat
	curr_lon = now * delta_lon + start_lon
	print "%d;1;%f;%f;%d;%d" % (t, curr_lat, curr_lon, curr_alt, curr_press)
