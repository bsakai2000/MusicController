// Big thanks to https://github.com/wware/stuff/blob/master/dbus-example/dbus-example.c
// for lots of the DBus routines used here
#include <dbus/dbus.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

int* playback_status = 0;

// Initialize the DBus connection and check for errors
void init_dbus(DBusConnection** conn, DBusError* err)
{
	// initialise the error struct
	dbus_error_init(err);

	// connect to the bus and check for errors
	*conn = dbus_bus_get(DBUS_BUS_SESSION, err);

	if (dbus_error_is_set(err))
	{
		fprintf(stderr, "Connection Error (%s)\n", err->message);
		dbus_error_free(err);
	}
	if (NULL == conn)
	{
		fprintf(stderr, "Connection Null\n");
		exit(1);
	}
}

// Request our desired name and check for errors
void request_name(DBusConnection* conn, DBusError* err, char* mpris_name)
{
	// request our name on the bus and make sure there is only one version of this code running
	int name_request = dbus_bus_request_name(conn, mpris_name, DBUS_NAME_FLAG_REPLACE_EXISTING , err);
	if (dbus_error_is_set(err))
	{
		fprintf(stderr, "Name Error (%s)\n", err->message);
		dbus_error_free(err);
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != name_request)
	{
		fprintf(stderr, "Not Primary Owner (%d)\n", name_request);
		exit(1);
	}
}

// Adds one string/boolean dictionary entry to dict
void add_dict_entry(DBusMessageIter* dict, char* attribute, void* attr_value, int type)
{
	char type_string[2] = {type, '\0'};

	DBusMessageIter dict_entry, dict_val;
	// Create our entry in the dictionary
	dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry);
	// Add the attribute string
	dbus_message_iter_append_basic(&dict_entry, DBUS_TYPE_STRING, &attribute);
	// Create the value for this entry
	dbus_message_iter_open_container(&dict_entry, DBUS_TYPE_VARIANT, type_string, &dict_val);
	dbus_message_iter_append_basic(&dict_val, type, &attr_value);
	// Clean up and return
	dbus_message_iter_close_container(&dict_entry, &dict_val);
	dbus_message_iter_close_container(dict, &dict_entry);
}

// TODO - Combine these two functions into one, and hold the attributes in an array
// or something. Since they're bools, we could use a char* to hold their values,
// but that's kinda hack-y

// Pretend to be a functioning MPRIS media player that has few capabilities
void getall_mediaplayer(DBusMessage* msg, DBusConnection* conn)
{
	dbus_uint32_t serial = 0;
	// Generate a message to return
	DBusMessage* reply = dbus_message_new_method_return(msg);
	DBusMessageIter reply_args, dict;
	dbus_message_iter_init_append(reply, &reply_args);

	// Create our array to hold dictionary entries
	dbus_message_iter_open_container(&reply_args, DBUS_TYPE_ARRAY, "{sv}", &dict);

	// Add dictionary entries claiming we don't have these capabilities
	add_dict_entry(&dict, "CanQuit", (void*) 0, DBUS_TYPE_BOOLEAN);
	add_dict_entry(&dict, "CanRaise", (void*) 0, DBUS_TYPE_BOOLEAN);
	add_dict_entry(&dict, "HasTrackList", (void*) 0, DBUS_TYPE_BOOLEAN);

	// Clean up our array
	dbus_message_iter_close_container(&reply_args, &dict);


	// send the reply && flush the connection
	if (!dbus_connection_send(conn, reply, &serial))
	{
		fprintf(stderr, "Out Of Memory!\n");
		exit(1);
	}

	// free the reply
	dbus_message_unref(reply);
}

// Tell about our capabilities, few as they are
void getall_mediaplayer_player(DBusMessage* msg, DBusConnection* conn)
{
	dbus_uint32_t serial = 0;
	// Generate a message to return
	DBusMessage* reply = dbus_message_new_method_return(msg);
	DBusMessageIter reply_args, dict;
	dbus_message_iter_init_append(reply, &reply_args);

	// Create our array to hold dictionary entries
	dbus_message_iter_open_container(&reply_args, DBUS_TYPE_ARRAY, "{sv}", &dict);

	// Add dictionary entries telling we have these capabilities
	add_dict_entry(&dict, "CanGoNext", (void*) 1, DBUS_TYPE_BOOLEAN);
	add_dict_entry(&dict, "CanGoPrevious", (void*) 1, DBUS_TYPE_BOOLEAN);
	add_dict_entry(&dict, "CanPlay", (void*) 1, DBUS_TYPE_BOOLEAN);
	add_dict_entry(&dict, "CanPause", (void*) 1, DBUS_TYPE_BOOLEAN);
	add_dict_entry(&dict, "CanControl", (void*) 1, DBUS_TYPE_BOOLEAN);
	add_dict_entry(&dict, "CanSeek", (void*) 0, DBUS_TYPE_BOOLEAN);
	add_dict_entry(&dict, "PlaybackStatus", (*playback_status?"Playing":"Paused"), DBUS_TYPE_STRING);

	// Clean up our array
	dbus_message_iter_close_container(&reply_args, &dict);

	// send the reply && flush the connection
	if (!dbus_connection_send(conn, reply, &serial))
	{
		fprintf(stderr, "Out Of Memory!\n");
		exit(1);
	}

	// free the reply
	dbus_message_unref(reply);
}

// Prints the command in a way that the Native Messaging will understand
void print_player_command(char* command)
{
	// Disable output buffering to avoid issues with it
	setbuf(stdout, NULL);
	// Length is command length plus two quotes
	u_int32_t len = strlen(command) + 2;
	// Print the length as bytes, little endian
	for(int i = 0; i < 4; ++i)
	{
		printf("%c", ((char*) &len)[i]);
	}
	// Print the command in quotes
	printf("\"%s\"", command);
}

// Holds the number of commands we can perform
#define NUM_PLAYER_COMMANDS 5
// If we recieve a player command, output the magic word to make our extension do it
int check_player_command(DBusMessage* msg)
{
	char* commands[NUM_PLAYER_COMMANDS][2] =
	{
		{"Pause", "pause"},
		{"Play", "play"},
		{"PlayPause", "togglepause"},
		{"Next", "next"},
		{"Previous", "prev"}
	};

	// Loop through our list of commands. When we find a match, output the associated magic word
	for(int i = 0; i < NUM_PLAYER_COMMANDS; ++i)
	{
		if (dbus_message_is_method_call(msg, "org.mpris.MediaPlayer2.Player", commands[i][0]))
		{
			print_player_command(commands[i][1]);
			//printf(commands[i][1]);
			return 1;
		}
	}

	return 0;
}

void* listen_for_status(void* args)
{
	while(1)
	{
		unsigned int message_length;
		for(int i = 0; i < 4; ++i)
		{
			if(scanf("%c", ((char*) &message_length) + i) == EOF)
			{
				abort();
			}
		}

		char* message = calloc(message_length + 1, sizeof(char));
		char format_string[100];
		snprintf(format_string, 100, "%%%us\0", message_length);
		scanf(format_string, message);

		if(strcmp(message, "\"Audible\"") == 0 && *playback_status == 0)
		{
			*playback_status = 1;
		}
		else if(strcmp(message, "\"Inaudible\"") == 0 && *playback_status == 1)
		{
			*playback_status = 0;
		}
		free(message);
	}
	return args;
}

int main()
{
	DBusError err;
	DBusConnection* conn;
	char mpris_name[] = "org.mpris.MediaPlayer2.musiccontroller";
	
	// Setup DBus connection and ensure we own our name
	init_dbus(&conn, &err);
	request_name(conn, &err, mpris_name);
	
	playback_status = calloc(1, sizeof(int));
	*playback_status = 0;

	pthread_t tid;
	pthread_create(&tid, 0, listen_for_status, NULL);
	
	// loop, testing for new messages
	while (1)
	{
		// non blocking read of the next available message
		dbus_connection_read_write(conn, 0);
		DBusMessage* msg = dbus_connection_pop_message(conn);

		// loop again if we haven't got a message
		if (NULL == msg)
		{
			usleep(10000);
			continue;
		}

		dbus_uint32_t serial = 0;
		// check this is a method call for the right interface & method
		if(dbus_message_is_method_call(msg, "org.freedesktop.DBus.Properties", "GetAll"))
		{
			// Read arguments to determine which response to give
			char* param;
			DBusMessageIter args;
			if (!dbus_message_iter_init(msg, &args))
			{
				fprintf(stderr, "Message has no arguments!\n");
			}
			else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
			{
				fprintf(stderr, "Argument is not string!\n");
			}
			else
			{
				// Default to giving information about our capabilities as a player,
				// but if asked give information about our capabilities through MPRIS
				dbus_message_iter_get_basic(&args, &param);
				if(strcmp(param, "org.mpris.MediaPlayer2") == 0)
				{
					getall_mediaplayer(msg, conn);
				}
				else
				{
					getall_mediaplayer_player(msg, conn);
				}
			}
		}
		// If this is a player command, run it and return nothing
		else if(check_player_command(msg))
		{
			DBusMessage* reply = dbus_message_new_method_return(msg);

			// send the reply && flush the connection
			if (!dbus_connection_send(conn, reply, &serial))
			{
				fprintf(stderr, "Out Of Memory!\n");
				exit(1);
			}

			// free the reply
			dbus_message_unref(reply);
		}
		// If we don't recognize the call, say so
		else
		{
			char error_string[127];
			snprintf(error_string, 127, "Illegal Call: %s.%s on %s", dbus_message_get_interface(msg), dbus_message_get_member(msg), dbus_message_get_path(msg));

			DBusMessage* error_reply = dbus_message_new_error(msg, DBUS_ERROR_FAILED, error_string);

			if (!dbus_connection_send(conn, error_reply, &serial))
			{
				fprintf(stderr, "Out Of Memory!\n");
				exit(1);
			}
			
			// free the reply
			dbus_message_unref(error_reply);
		}

		// free the message
		dbus_message_unref(msg);
	}

}
