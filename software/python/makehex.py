#!/usr/bin/python2

from sys import argv

byteSize = 8

binfile = argv[1]
mem_size = 2**(int(argv[2]))

with open(binfile, "rb") as f:
    bindata = f.read()

assert len(bindata) <= mem_size
assert len(bindata) % byteSize == 0

for i in range(mem_size/byteSize):
    if i < (len(bindata)/byteSize):
        w = bindata
        print('%02x%02x%02x%02x%02x%02x%02x%02x' % (ord(w[8*i+7]), ord(w[8*i+6]), ord(w[8*i+5]), ord(w[8*i+8]),ord(w[8*i+3]), ord(w[8*i+2]), ord(w[8*i+1]), ord(w[8*i+0])))
    else:
        print("0000000000000000")

