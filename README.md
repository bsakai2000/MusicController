# MusicController

A simple script to control media on webpages using the MPRIS API. The C app waits for DBus connections and upon receipt sends their messages on to `background.js`, which reads the message and injects javascript into certain pages to perform the required actions

## To Install

 - Build `app/music_controller` with `gcc -o music_controller music_controller.c $(pkg-config --libs --cflags dbus-1) -pthread`
 - Modify `app/music_controller.json` so the path points to your installation
 - Copy `app/music_controller.json` to `~/.mozilla/native-messaging-hosts/`
 - Install the extension from `about:debugging`

## To Use

Use DBus to send commands to MusicController. I use `dbus-send`, like so:

	dbus-send --dest=org.mpris.MediaPlayer2.musiccontroller --type=method_call --print-reply /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Pause

DBus endpoints are `Play`, `Pause`, `PlayPause`, `Next`, `Previous`. For logistical reasons involving `mpris-proxy`, `Play` is an alias for `PlayPause` 

This can also be automated. There are a couple simple examples for cli interfaces in different languanges in `/controllers`, so it would be relatively simple to adapt one into a custom script. One could also choose to bind keyboard keys to trigger media events, so for instance an i3 keybind would look like

	bindsym XF86AudioPause exec --no-startup-id dbus-send --dest=org.mpris.MediaPlayer2.musiccontroller --type=method_call --print-reply /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Pause

Commands for the controllers are `togglepause`, `pause`, `play`, `next`, `prev`

In addition, because we implement the MPRIS API, several third party controllers ought to work out of the box. This setup is tested only against `mpris-proxy`, but it works well.
