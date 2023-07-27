#!/usr/bin/python3

import sys
import math

FILE = open(sys.argv[1])

lines = FILE.readlines()

for i in range(math.ceil(len(lines) / 2)):
   if(i*2 + 1 >= len(lines)):
      print("00000000",lines[i*2].strip(),sep="")
   else:
      print(lines[i*2+1].strip(),lines[i*2].strip(),sep="")
   
