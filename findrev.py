#!/usr/bin/env python

f = open("bertos/verstag.h")
for l in f:
	if l.startswith("#define VERS_NAME"):
		name = l.split()[2]
	if l.startswith("#define VERS_REV"):
		rev = l.split()[2]
if rev == '0':
	rev = ''
print "%s%s" % (name.strip('"'), rev)




