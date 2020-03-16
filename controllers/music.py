#!/usr/bin/env python3

import os
import socket
import sys

# Check that we got exactly one command
if(len(sys.argv) != 2):
    print("Usage: " + sys.argv[0] + " <command>")
    sys.exit(1)

# Connect to the MusicController socket
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
socketName = "/tmp/musiccontroller.sock"
s.connect(socketName)

# Send the command
s.send(sys.argv[1].encode("utf-8"))
