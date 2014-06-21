#! /usr/bin/python3

import sys

#        SET_OP(0x60, "LD H,B", op_ld_8r_8r, H, B, "NONE", 1, 4, 0);
model = 'SET_OP_CB(0x%02X, "%s", %s, %s, %s, %s, %s, %s, %s);'


for line in sys.stdin:
	line = line.split(" ")
	code = int(line[0], 16)
	if len(line) > 2:
		opt = "NONE"
		desc = line[1]
		if len(line) == 9:
			desc = desc + " " + line[2]
			pars = line[2].split(",")
			if len(pars) == 2:
				a, b = pars[0], pars[1]
			else:
				a, b = pars[0], "NULL"
		else:
			a, b = "NULL", "NULL"
		op = "op_" + line[1].lower()
		length = line[-6]
		
		
		if (a[0] == "("):
			opt = "MEM_RW_16"
		if (b[0] == "("):
			opt = "MEM_RW_16"
		
		a = a.replace("(", "").replace(")", "")
		b = b.replace("(", "").replace(")", "")
		
		if (b != "NULL"):
			a, b = b, "&C_" + a
				
		
		cycl = line[-5].split("/")
		c0 = cycl[0]
		c1 = "0"
		
		print(model % (code, desc, op, a, b, opt, length, c0, c1))
	else:
		print(model % (code, "-", "op_undef", "NULL", "NULL", "NONE", "1", "0", "0"))