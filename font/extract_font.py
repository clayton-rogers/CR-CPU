#!/usr/bin/python3

import sys
from PIL import Image

filename = sys.argv[1]
print("Filename: ", filename)

im = Image.open(filename)
(x,y) = im.size

if x != 1280:
	print("Image width should be 1280")
	sys.exit(1)
if y != 25:
	print("Image height should be 25")
	sys.exit(1)

CHAR_WIDTH = 8
CHAR_HEIGHT = 16
for char in range(128):
	for sub_y in range(CHAR_HEIGHT):
		value = 0
		for sub_x in range(CHAR_WIDTH):
			x = char*10 + 1 + (7 - sub_x)
			y = 7 + sub_y
			value = value * 2 # bitshift left
			if im.getpixel((x,y))[0] == 0: # r value is black
				value = value + 1

		print("{:02X} ".format(value), end='')
	print()

