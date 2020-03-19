#!/bin/bash

# The filesystem socket to connect to
socketName="/tmp/musiccontroller.sock"

# Check that we got exactly one command
if [[ $# -ne 1 ]]
then
	echo "Usage: $0 <command>"
	exit
fi

# Parse our command and find the equivalent MPRIS command
cmd=''
case $1 in
	"pause")
		cmd="Pause"
		;;
	"play")
		cmd="Play"
		;;
	"togglepause")
		cmd="PlayPause"
		;;
	"next")
		cmd="Next"
		;;
	"prev")
		cmd="Previous"
		;;
esac

# Call musiccontroller over DBus
dbus-send --dest=org.mpris.MediaPlayer2.musiccontroller --type=method_call --print-reply /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.$cmd
