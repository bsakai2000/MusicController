#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <dbus/dbus.h>

int main(int argc, char* argv[])
{
	// We want just one command
	if(argc != 2)
	{
		printf("Usage: %s <command>\n", argv[0]);
		return 1;
	}

	DBusMessage* msg;
	DBusConnection* conn;
	DBusError err;
	DBusPendingCall* pending;
	unsigned int serial = 0;

	dbus_error_init(&err);

	// connect to the system bus and check for errors
	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Connection Error (%s)\n", err.message);
		dbus_error_free(&err);
	}
	if (NULL == conn) {
		exit(1);
	}

#define NUM_COMMANDS 5
	// List of commands we can run
	char* commands[NUM_COMMANDS][2] = 
	{
		{"Play", "play"},
		{"Pause", "pause"},
		{"PlayPause", "togglepause"},
		{"Next", "next"},
		{"Previous", "prev"}
	};

	char method[20] = "";

	// Check if passed command matches known commands, then find alias
	for(int i = 0; i < NUM_COMMANDS; ++i)
	{
		if(strcmp(commands[i][1], argv[1]) == 0)
		{
			strcpy(method, commands[i][0]);
			break;
		}
	}

	// Make call to musiccontroller
	msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.musiccontroller", // target for the method call
			"/org/mpris/MediaPlayer2", // object to call on
			"org.mpris.MediaPlayer2.Player", // interface to call on
			method); // method name

	// send the reply && flush the connection
	if (!dbus_connection_send_with_reply(conn, msg, &pending, -1))
	{
		fprintf(stderr, "Out Of Memory!\n");
		exit(1);
	}

	// clean up
	dbus_connection_flush(conn);

	if(pending)
	{
		dbus_pending_call_block(pending);
	}

	// free the reply
	dbus_message_unref(msg);

	return 0;
}
