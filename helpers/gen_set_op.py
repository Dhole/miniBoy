#! /usr/bin/python3

import sys

#        SET_OP(0x60, "LD H,B", op_ld_8r_8r, regs.H, regs.B, "NONE", 1, 4, 0);
model = 'SET_OP(0x%02X, "%s", %s, %s, %s, %s, %s, %s, %s);'


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
		
		if line[1] == "SUB" or line[1] == "AND" or line[1] == "XOR" or \
			line[1] == "OR" or line[1] == "CP":
			a, b = "A", pars[0]
		
		if (op == "op_ld"):
			if (len(a) == 2 or b == "SP"):
				op = "op_ld_16"
			else:
				op = "op_ld_8"
		
		elif (op == "op_inc"):
			if (len(a) == 2):
				op = "op_inc_16"
			else:
				op = "op_inc_8"
		
		elif (op == "op_dec"):
			if (len(a) == 2):
				op = "op_dec_16"
			else:
				op = "op_dec_8"
		elif (op == "op_add"):
			if (a == "SP"):
				op = "op_addsp"
			elif (len(a) == 2):
				op = "op_add_16"
			else:
				op = "op_add_8"
		
		elif (op == "op_prefix"):
			op = "op_pref_cb"
			a = "imm_8"
			b = "NULL"
			length = 2
			
		elif (op == "op_jr"):
			if (b == "NULL"):
				a, b = "NULL", a
			else:
				a = "&C_" + a
				
		elif (op == "op_ret"):
			if (a != "NULL"):
				a = "&C_" + a
			else:
				a, b = "NULL", a
				
		elif (op == "op_jp"):
			if (b == "NULL"):
				a, b = "NULL", a
			else:
				a = "&C_" + a
		
		elif (op == "op_call"):
			if (len(a) < 3):
				a = "&C_" + a
			else:
				a, b = "NULL", a
				
		elif (op == "op_stop"):
			a = "&C_" + a
		
		elif (op == "op_ldh"):
			op = "op_ld_8"
		
		if (a[0] == "("):
			if (len(a) == 1 + 2 or a == "(a8)"):
				opt = "MEM_W_8"
			else:
				opt = "MEM_W_16"
		if (b[0] == "("):
			if (len(b) == 1 + 2 or b == "(a8)"):
				opt = "MEM_R_8"
			else:
				opt = "MEM_R_16"
		
		a = a.replace("(", "").replace(")", "")
		b = b.replace("(", "").replace(")", "")
		
		if (op == "op_rst"):
			a = "&C_" + a
		
		if (b[0] == "d" or b[0] == "a" or b[0] == "r"):
			if (b[1] == "8"):
				b = "imm_8"
			else:
				b = "imm_16"
		elif (a[0] == "d" or a[0] == "a" or a[0] == "r"):
			if (a[1] == "8"):
				a = "imm_8"
			else:
				a = "imm_16"
		
		if (code == 0x22):
			op = "op_ldi"
			a = "HL"
		elif (code == 0x2A):
			op = "op_ldi"
			b = "HL"
		elif (code == 0x32):
			op = "op_ldd"
			a = "HL"
		elif (code == 0x3A):
			op = "op_ldd"
			b = "HL"
		elif (code == 0xF8):
			op = "op_ldhl"
			a = "SP"
			b = "imm_8"

		if (a[0] != "i" and a[0] != "&" and a[0] != "N"):
                        a = "regs." + a
		if (b[0] != "i" and b[0] != "&" and b[0] != "N"):
                        b = "regs." + b
		
		cycl = line[-5].split("/")
		if (len(cycl) == 2):
			c0 = cycl[1]
			c1 = cycl[0]
		else:
			c0 = cycl[0]
			c1 = "0"
		print(model % (code, desc, op, a, b, opt, length, c0, c1))
	else:
		print(model % (code, "-", "op_undef", "NULL", "NULL", "NONE", "1", "0", "0"))
