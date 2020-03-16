#!/usr/bin/env python3

import sys
import json
import struct
import socket
import os

# Encode a message for transmission,
# given its content.
def encodeMessage(messageContent):
    encodedContent = json.dumps(messageContent).encode('utf-8')
    encodedLength = struct.pack('@I', len(encodedContent))
    return {'length': encodedLength, 'content': encodedContent}

# Send an encoded message to stdout
def sendMessage(encodedMessage):
    sys.stdout.buffer.write(encodedMessage['length'])
    sys.stdout.buffer.write(encodedMessage['content'])
    sys.stdout.buffer.flush()

# Open a file socket to communicate with other userspace
# programs to control media
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
socketName = "/tmp/musiccontroller.sock"
# Delete the socket if it already exists
try:
    os.remove(socketName)
except OSError:
    pass
# Listen to our socket
s.bind(socketName)
s.listen(1)
while True:
    # Wait for socket connections, and when we get one
    # send it on to the background script
    conn, addr = s.accept()
    data = conn.recv(1024).decode('utf-8')
    conn.close()
    sendMessage(encodeMessage(data))
