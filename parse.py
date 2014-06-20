#! /usr/bin/python3

import sys

for i in range(0, 16):
	line = sys.stdin.readline()
	sp = line.split("</td>")
	for j in range(0, 16):
		print("%02x " % (i * 0x10 + j), end="")
		op = sp[j + 1][41:]
		op = op.replace("<br>", " ").replace("&nbsp;&nbsp;", " ")
		print(op)