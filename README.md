# MusicController

A simple script to control media on webpages using a filesystem socket. The python app waits for socket connections and upon receipt sends their messages on to `background.js`, which reads the message and injects javascript into certain pages to perform the required actions

## To Install

 - Modify `app/music_controller.json` so the path points to your installation
 - Copy `app/music_controller.json` to `~/.mozilla/native-messaging-hosts/`
 - Install the extension from `about:debugging`

## To Use

Just write commands to the socket file. I use netcat, like so: `echo -n pause | nc -U ./musiccontroller.sock`. This can also be done programmatically, so for instance an i3 keybind would look like `bindsym XF86AudioPause exec --no-startup-id echo -n "pause" | nc -U /tmp/musiccontroller.sock`.
