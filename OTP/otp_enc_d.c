#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<time.h>
#define SIZE 100000

int charToInt(char c)
{
	if (c == ' ')
	{
		return 26;
	}
	else
	{
		return (c - 'A');
	}
	return 0;
}

char intToChar(int i)
{
	if (i == 26)
	{
		return ' ';
	}
	else
	{
		return (i + 'A');
	}
}
//encrypts message according to assignment
void encrypt_message(char *message, char *key)
{
	int i;
	char n;
	for (i = 0; i < strlen(message); i++)
	{
		n = (charToInt(message[i]) + charToInt(key[i])) % 27;
		message[i] = intToChar(n);
	}
	message[i] = '\0';
}

void error(const char *msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[]){
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead,check_e;
	socklen_t sizeOfClientInfo;
	
	
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) {
		fprintf(stderr,"USAGE: %s port\n", argv[0]);
		exit(1);
	} // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	//-------------------------------------------------------------------------------------------------------------------------
	pid_t pid;
	pid_t child;
	int children = 0;
	int childExitMethod = -5;

	while(1){
		while ((child = waitpid(-1, &childExitMethod, WNOHANG)) > 0){
			child += -1;
		}
		if (children < 5){ //onl 5!

			// Accept a connection, blocking if one is not available until one connects
			sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
			establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
			if (establishedConnectionFD < 0) error("ERROR on accept");

			pid = fork();
			if(pid < 0){ //errir
				perror("Hull breach");
			}
			if(pid == 0){
				//Checks if correct thing to connect to
				char buffer[11];
				memset(buffer, '\0',11);
				check_e = recv(establishedConnectionFD, buffer, 2, 0); // Read the client's message from the socket
				if (check_e < 0) error("ERROR reading from socket");

				//Checks if write connection (I added E. at the beg of message to check)
				if (strcmp("E.",buffer) != 0){
					char output[] = "Incorrect Connection - from Encrypt";
					write(establishedConnectionFD, output, sizeof(output));
					exit(2);
				}

				//Reads how long the message is
				check_e = recv(establishedConnectionFD, buffer, 10, 0);
				if (check_e < 0){
					error("ERROR reading from socket");
				}
				int length_of_message = atoi(buffer);

				char message[length_of_message+1];
				memset(message, '\0', length_of_message+1);

				//Reads in the Message
				check_e = recv(establishedConnectionFD, message, length_of_message, 0); // Read the client's message from the socket
				if (check_e < 0)
					error("ERROR reading from socket");
				

				//Reads how long the key is
				memset(buffer, '\0', 11);
				check_e = recv(establishedConnectionFD, buffer, 10, 0); // Read the client's message from the socket
				if (check_e < 0)
					error("ERROR reading from socket");

				int length_of_key = atoi(buffer);
				char key[length_of_key+1];
				memset(key, '\0', length_of_key+1);

				//Reads in the key
				check_e = recv(establishedConnectionFD, key, length_of_key, 0); // Read the client's message from the socket
				if (check_e < 0)
					error("ERROR reading from socket");

				//Passes message and key to encrypt_message and encrypts it
				char * message_pointer = message;
				char * key_pointer = key;
				
				encrypt_message(message_pointer,key_pointer);

				//Sends encrypted message back to client
				charsRead = send(establishedConnectionFD, message, strlen(message)+1, 0); // Send success back
				if (charsRead < 0) error("ERROR writing to socket");
				close(establishedConnectionFD); // Close the existing socket which is connected to the client
				exit(0);
				
			}
			else{
				if ((child = waitpid(-1, &childExitMethod, WNOHANG)) < 0)
					child += -1;
			}
		}
	}
	//-----------------------------------------------------------------------------------------------------------------------

	close(listenSocketFD); // Close the listening socket
	return 0;
}
