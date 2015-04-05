#! /usr/bin/env python3

l = open("io_regs.txt", "r").readlines()

for m in l:
    n = m.split(" ")[0][3:]
    space = " " * (10 - len(n))
    print("""printf("%02X ["""+n+"""]:"""+space+"""%02x (%s)\\n",""")
    print("""IO_"""+n+""", mm[IO_"""+n+"""], byte2bin_str(mm[IO_"""+n+"""], s));""")
