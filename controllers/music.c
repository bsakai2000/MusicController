#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// The filesystem socket to communicate with Music Controller on
char socketName[] = "/tmp/musiccontroller.sock";

int main(int argc, char* argv[])
{
	// We want just one command
	if(argc != 2)
	{
		printf("Usage: %s <command>\n", argv[0]);
		return 1;
	}

	// Try to create a Unix socket
	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sock < 0)
	{
		printf("Socket did not create!\n");
		return 1;
	}

	// Allocate a socket address struct and fill it with correct values
	struct sockaddr_un* socketAddress = calloc(1, sizeof(struct sockaddr_un));
	socketAddress->sun_family = AF_UNIX;
	strcpy(socketAddress->sun_path, socketName);

	// Try to connect to the Music Controller socket
	int conn = connect(sock, (struct sockaddr*) socketAddress, sizeof(struct sockaddr_un));
	if(conn < 0)
	{
		printf("Connection did not create!\n");
		// Explicit error messages for a few of the more likely errors
		switch(errno)
		{
			case ECONNREFUSED:
				printf("Connection refused on socket %s\n", socketName);
				break;
			case ENOENT:
				printf("No such socket %s\n", socketName);
				break;
			default:
				printf("errno: %d\n", errno);
		}
		return 1;
	}

	// Try to send the command to the Music Controller
	int didSend = send(sock, argv[1], strlen(argv[1]), 0);
	if(didSend < 0)
	{
		printf("Sending failed!\n");
		return 1;
	}
	return 0;
}
