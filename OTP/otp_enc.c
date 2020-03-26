#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) {
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[]){
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;

	if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args
	//Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number

	//Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");


	//Opens files, reads how long files are, and puts them into a string
	FILE *file = fopen(argv[1], "r");
	fseek(file, 0, SEEK_END);
	int file_size = ftell(file);

	fseek(file, 0, SEEK_SET);
	char *text = (char *)malloc(sizeof(char) * file_size);
	int length = strlen(text);

	//this checks for ending w/ new line character
	size_t size_size = fread(text, 1, file_size, file);
	//replaces newline character
	if (text[file_size - 1] == '\n')
	{
		text[file_size - 1] = '\0';
	}
	int i;

	//checks if they are bad files
	for (i = 0; i < file_size; i++)
	{
		if (text[i] != ' ' && text[i] != '\0')
		{
			if (text[i] < 'A' || text[i] > 'Z')
			{
				//printf("%s%d\n","HERE", i);
				fprintf(stderr, "%s", "Bad characters exist in message \n");
				exit(1);
			}
		}
	}
	fclose(file);

	//reads file

	FILE *file2 = fopen(argv[2], "r");
	fseek(file2, 0, SEEK_END);
	int file_size2 = ftell(file2);

	fseek(file2, 0, SEEK_SET);
	char *text2 = malloc(file_size2 + 1);
	size_t size_size2 = fread(text2, 1, file_size2, file2);

	//replaces new line characters
	if (text2[file_size2 - 1] == '\n'){
		text2[file_size2 - 1] = '\0';
	}

	fclose(file2);
	
	//checking for bad characters
	for (i = 0; i < file_size2; i++)
	{
		if (text2[i] != ' ' && text2[i] != '\0')
		{
			if (text2[i] < 'A' || text2[i] > 'Z')
			{
				fprintf(stderr, "%s", "Bad characters exist in key\n");
				exit(1);
			}
		}
	}
	//making sure message > key length
	if (file_size > file_size2)
	{
		fprintf(stderr, "%s", "Invalid file lengths!\n");
		exit(1);
	}


	//creating buffer to send over
	//My format: "E._lengthofmessage_message_lengthofkey_key"

	char buffer[2+10+4+file_size+file_size2];
	memset(buffer, '\0', sizeof(buffer));
	strcpy(buffer, "E."); //this is used to differentiate decrypt and encrypt

	char str[100];
	sprintf(str, "%10d", strlen(text));
	strcat(buffer, str);
	strcat(buffer, text);

	char str2[100];
	sprintf(str2, "%10d", strlen(text2));
	strcat(buffer, str2);
	strcat(buffer, text2);

	//send info over
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

	// Get return message from server
	char encrypted_message[strlen(text)+1];
	memset(encrypted_message, '\0', sizeof(encrypted_message)+1);											 // Clear out the buffer again for reuse
	charsRead = recv(socketFD, encrypted_message, sizeof(encrypted_message), 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	
	printf("%s\n", encrypted_message);
	close(socketFD); // Close the socket
	return 0;
}
