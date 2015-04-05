 
regs = ["B", "C", "D", "E", "H", "L", "(HL)", "A"]
d = 0x80
for i in range(0, 8):
	print('\tSET_OP(0x%02X, "ADD A,%s", op_add, A, %s, 4);' % (i+d, regs[i % 8], regs[i % 8]));
	
for i in range(8, 16):
	print('\tSET_OP(0x%02X, "ADC A,%s", op_adc, A, %s, 4);' % (i+d, regs[i % 8], regs[i % 8]));
	
d = 0x90
for i in range(0, 8):
	print('\tSET_OP(0x%02X, "SUB %s", op_sub, %s, NULL, 4);' % (i+d, regs[i % 8], regs[i % 8]));
	
for i in range(8, 16):
	print('\tSET_OP(0x%02X, "SBC A,%s", op_sbc, A, %s, 4);' % (i+d, regs[i % 8], regs[i % 8]));	

d = 0xA0
for i in range(0, 8):
	print('\tSET_OP(0x%02X, "AND %s", op_and, %s, NULL, 4);' % (i+d, regs[i % 8], regs[i % 8]));
	
for i in range(8, 16):
	print('\tSET_OP(0x%02X, "XOR %s", op_xor, %s, NULL, 4);' % (i+d, regs[i % 8], regs[i % 8]));
	
d = 0xB0
for i in range(0, 8):
	print('\tSET_OP(0x%02X, "OR %s", op_or, %s, NULL, 4);' % (i+d, regs[i % 8], regs[i % 8]));
	
for i in range(8, 16):
	print('\tSET_OP(0x%02X, "CP %s", op_cp, %s, NULL, 4);' % (i+d, regs[i % 8], regs[i % 8]));