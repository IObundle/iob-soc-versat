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
if(amount == 256):
   for i in range(math.ceil(len(lines) / 8)):
      if(i*8 + 1 >= len(lines)):
         print("00000000",lines[i*8].strip(),sep="")
      else:
         print(lines[i*8+7].strip(),lines[i*8+6].strip(),lines[i*8+5].strip(),lines[i*8+4].strip(),lines[i*8+3].strip(),lines[i*8+2].strip(),lines[i*8+1].strip(),lines[i*8].strip(),sep="")
if(amount == 512):
   for i in range(math.ceil(len(lines) / 16)):
      if(i*16 + 1 >= len(lines)):
         print("00000000",lines[i*16].strip(),sep="")
      else:
         print(lines[i*16+15].strip(),lines[i*16+14].strip(),lines[i*16+13].strip(),lines[i*16+12].strip(),lines[i*16+11].strip(),lines[i*16+10].strip(),lines[i*16+9].strip(),lines[i*16+8].strip(),lines[i*16+7].strip(),lines[i*16+6].strip(),lines[i*16+5].strip(),lines[i*16+4].strip(),lines[i*16+3].strip(),lines[i*16+2].strip(),lines[i*16+1].strip(),lines[i*16].strip(),sep="")
   
   
