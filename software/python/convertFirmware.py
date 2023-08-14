#!/usr/bin/python3

import sys
import math

FILE = open(sys.argv[1])
amount = int(sys.argv[2])

print("Conversion for: ",str(amount),file=sys.stderr)

lines = FILE.readlines()

if(amount == 64):
   for i in range(math.ceil(len(lines) / 2)):
      if(i*2 + 1 >= len(lines)):
         print("00000000",lines[i*2].strip(),sep="")
      else:
         print(lines[i*2+1].strip(),lines[i*2].strip(),sep="")
if(amount == 128):
   for i in range(math.ceil(len(lines) / 4)):
      if(i*4 + 1 >= len(lines)):
         print("00000000",lines[i*4].strip(),sep="")
      else:
         print(lines[i*4+3].strip(),lines[i*4+2].strip(),lines[i*4+1].strip(),lines[i*4].strip(),sep="")
   
