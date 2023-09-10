#!/usr/bin/env python3

"""
sha256_test.py
Python script to send input data and receive output data from a single SHA256 
test
"""

# Import Ethernet package
import sys
from os.path import getsize
sys.path.append('../../')
from submodules.ETHERNET.software.python.ethBase import CreateSocket, SyncAckFirst, SyncAckLast
from submodules.ETHERNET.software.python.ethSendVariableData import SendVariableFile
from submodules.ETHERNET.software.python.ethRcvVariableData import RcvVariableFile

if __name__ == "__main__":
    print("\nSHA 256 Single Test\n")

    # Check input arguments
    if len(sys.argv) != 4:
        print(f'Usage: ./{sys.argv[0]} [RMAC_INTERFACE] [RMAC] [input.bin]')
        quit()
    else:
        send_file = sys.argv[3]

    try:
        fileSize = getsize(send_file)

        # Send Variable Data File
        print("\nStarting file transmission...")

        socket = CreateSocket()

        SyncAckFirst(socket)
        SendVariableFile(socket,send_file)

        # Close Socket
        socket.close()
    except OSError:
        print("Path '%s' does not exists or is inaccessible" % send_file)