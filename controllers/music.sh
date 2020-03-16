#!/bin/bash

# The filesystem socket to connect to
socketName="/tmp/musiccontroller.sock"

# Check that we got exactly one command
if [[ $# -ne 1 ]]
then
	echo "Usage: $0 <command>"
	exit
fi

# Send the commands on to MusicController
echo -n "$1" | nc -U "$socketName"
