#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (int argc, char *argv[]) {
	if(argc < 1) {
		fprintf(stderr, "Invalid input.\n"); 
		return 1;
	}
	
	srand(time(0));

	//Possible Characters Allowed
	char valid_char[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	char outStr[1000];

	//Length of Key
	int charCount = atoi(argv[1]);	

	int i;
    int rand_char;

	//choosing random
	for(i = 0; i < charCount; i++) {
		rand_char = rand() % 27; 
		printf("%c", valid_char[rand_char]);
		
	}

	printf("\n"); // End with a newline.
	
	return 0;	
}