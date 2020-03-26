#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>
#define MAX_ARGUMENTS 512
#define MAX_LENGTH 2048


//GLOBAL VARIABLES

//keeps track of pids
pid_t pid_array[100];
int pid_array_size = 0;

//global variable for background/foregorund
int allowBackground =1;

//I referenced StackOverflow for this function
//https://stackoverflow.com/questions/53311499/c-replace-in-string-with-id-from-getpid/53311604
void get_pid(char* input){
    int i = 0;
    char* formatString = strdup(input); //copies string
    for (i = 0; i < strlen(formatString); i++){
        if (formatString[i] == '$' && (i + 1 < strlen(formatString))){
            if( formatString[i+1] == '$'){
                formatString[i] = '%';
                formatString[i+1] = 'd';
                break;
            }
        }
    }
    sprintf(input,formatString,getpid());
    free(formatString);    
}
//this function checks all child processes and utilizies waitpid
void check_background(){
	int exitStatus = 0;
	int i;
	pid_t spawnpid = -5;

	//checks every pid in the array
	for(i = pid_array_size; i >= 0; i--){
		if ((spawnpid = waitpid(pid_array[i], &exitStatus, WNOHANG)) > 0){ 
			//TA Mike explained I had to switch the one I wanted with the last one
			pid_t temp = pid_array[i];
			pid_array[i] = pid_array[pid_array_size -1];
			pid_array[pid_array_size -1] = temp;
			
			//keep track of actual current amount of pid
			pid_array_size--;
			
			//if exit status is clean
			if (WIFEXITED(exitStatus) != 0){
				printf("exited w/ value %d\n", WEXITSTATUS(exitStatus));
				
				fflush(stdout);
			}
			else{ //exit with status
				printf("done w/ signal:  %d\n", WTERMSIG(exitStatus));
				printf("background pid is %d\n", spawnpid);
				fflush(stdout);
			}
		}
	}
}

int set_arg_array(char**arg_array, char * user_input){
    int num_arguments = 0;
    char* token = strtok(user_input, " \n");
    int i =0;

	//initialize here/clean up (gets reused every time cant have hanging arguments)
    for(i = 0; i < MAX_ARGUMENTS; i++){
        arg_array[i] = NULL;
    }	
	//fills up the argument array
	while(token != NULL) {
		arg_array[num_arguments] = strdup(token);
		num_arguments++;
		token = strtok(NULL, " \n");
	}
	return num_arguments;
}


//clean up memory
void free_arg_array(char** argList, int numberOfArg) {
	int i;
	for (i = 0; i < numberOfArg; i++) {
		free(argList[i]);
	}
}

//used for debugging
void print_arg_array(char** argList, int numberOfArg) {
	int i;
	for (i = 0; i < numberOfArg; i++) {
		printf(argList[i]);
        printf("\n");
		fflush(stdout);
	}
}
//this function validates and gets correct input from user
int get_commands(char ** arg_array){
    int num_arguments;
    char* user_input = malloc(MAX_LENGTH * sizeof(char));

    do{
		check_background(); //checks for background processes (check before printing!!)
        printf(": ");
        fflush(stdout);
        fgets(user_input, MAX_LENGTH, stdin);
    }while(user_input[0] == '#' || strlen(user_input) <= 1 || strlen(user_input) > MAX_LENGTH);
	//keep getting input until input is valid (valid length/ is not a comment (just ignore))

    get_pid(user_input);

    num_arguments = set_arg_array(arg_array, user_input); //parses input correctly
    free(user_input);
    return num_arguments;
}


//this is used when :status is called
void printexitstatus(int exit) {
	if (WIFEXITED(exit)) {
		printf("exit value %d\n", WEXITSTATUS(exit));
	} else {
		printf("terminated by signal %d\n", WTERMSIG(exit));
	}
	fflush(stdout);
}

//does all the file handling
void fix_file(char ** argList,int numberOfArg,int run_on_back){
	int file_name;
	char* open_file = NULL;
	int redirect = 0;

	int i;
	for (i = 1; i < numberOfArg; i++) {
		if (argList[i] == NULL) break; 
		if ((strcmp(argList[i], "<") == 0) || (strcmp(argList[i], ">") == 0)) {
			redirect = 1;
			open_file = strdup(argList[i + 1]);
			if (run_on_back) {
				file_name = open("/dev/null", O_RDWR);	
				if (dup2(file_name, STDIN_FILENO) == -1){ 
					fprintf(stderr, "error redirecting"); 
					exit(1); 
				};
				if (dup2(file_name, STDOUT_FILENO) == -1){ 
					fprintf(stderr, "error redirecting"); 
					exit(1); 
				};
			}
			else {	
				file_name = open(open_file, O_RDWR);
				if ((strcmp(argList[i], "<") == 0)) {
					//file_name = open(open_file, O_RDWR);
					if (file_name == -1) { 
						fprintf(stderr, "can't open %s \n", open_file); 
						exit(1); 
					}
					if (dup2(file_name, STDIN_FILENO) == -1) { 
						fprintf(stderr, "error");
						exit(1); 
					}
				}else  {
					//file_name = open(open_file, O_RDWR);
					if (file_name == -1) { 
						fprintf(stderr, "can't open %s \n", open_file); 
						exit(1); 
					}
					if (dup2(file_name, STDOUT_FILENO) == -1) { 
						fprintf(stderr, "error"); 
						exit(1); 
					};
				}
			}
			close(file_name);
			free(open_file);
		}
	}
	if (redirect == 1){ 
		for(i = 1; i < numberOfArg; i++)
			argList[i] = NULL;
	}

}

void execute_commands(char ** arg_array,int num_arguments, int background, int * exitStatus, struct sigaction ignore, struct sigaction ignore_action){
    pid_t spawnpid = -5;
    spawnpid = fork();

    switch (spawnpid){
        case -1:  //error forking
            perror("Hull Breach!");
            exit(1);
            break;
        case 0: //children processes
			if (background && allowBackground){
				sigaction(SIGTSTP, &ignore,NULL);
			}
			else{
				sigaction(SIGTSTP, &ignore,NULL);
				sigaction(SIGINT, &ignore_action,NULL);
			}
			fix_file(arg_array, num_arguments, background);
			execvp(arg_array[0], arg_array);
			fprintf(stderr, "%s: command not found\n", arg_array[0]); 
            exit(1);
			
            break;
        default: //parent process
            if (background && allowBackground){
				pid_array[pid_array_size++] = spawnpid;
				printf("background pid is %d\n", spawnpid);
				fflush(stdout);
			}
			else{	
				spawnpid = waitpid(spawnpid, exitStatus, 0);
				if(WIFSIGNALED(*exitStatus) != 0){
					printf("terminated by signal %d",  WTERMSIG(*exitStatus));
					fflush(stdout);
				}
			}
    }
}

//signals
void catchSIGINT(int signo){
}

//signals for foreground and background
void catchSIGTSP(){
	if (allowBackground == 0) {
		char* message = "Entering foreground-only mode (& is now ignored)\n";
		write(1, message, 49);
		fflush(stdout);
		allowBackground = 1;
	}
	else {
		char* message = "Exiting foreground-only mode\n";
		write (1, message, 29);
		fflush(stdout);
		allowBackground = 0;
	}
}

//these run the built commands (status, chdir, cd)
void build_commands(char ** arg_array, int num_arguments, int exitStatus){
	char* home = getenv("HOME");
	if (!strncmp(arg_array[0], "exit",4)){
		free_arg_array(arg_array, num_arguments);
		exit(0);
	}
    else if(!strcmp(arg_array[0], "cd")){
        if(num_arguments == 1){
            chdir(home);
        }else{
            chdir(arg_array[1]);
        }
    }
    else if(strcmp(arg_array[0], "status")==0){
        printexitstatus(exitStatus);
    } 
}

//this runs all the commands - seperates them between buildcommands and execute commands
void run_commands(char ** arg_array, int num_arguments, int *exitStatus, struct sigaction ignore, struct sigaction int_action){
    int background = 0;

	//changes global variable of background to be true
    if ((strcmp(arg_array[num_arguments - 1], "&") == 0)) {	
		background = 1;
		arg_array[num_arguments - 1] = NULL;
	}
	//check if its a build command
	if( (!strncmp(arg_array[0], "exit",4)) || (!strcmp(arg_array[0], "cd")) || (!strcmp(arg_array[0], "status") )){
		build_commands(arg_array,num_arguments,*exitStatus);
	}
    else{
		// if its not a build command its gonna be an non built command
        execute_commands(arg_array,num_arguments, background,exitStatus, ignore, int_action);
    }
    free_arg_array(arg_array,num_arguments);
}


int main(){
	int exitStatus = 0;
    int num_arguments;
    char* arg_array[512];

	//signals
	struct sigaction SIGINT_action = { 0 };		//SIGINT initialization
	struct sigaction SIGTSTP_action = { 0 };	//SIGTSTP initialization
	
	struct sigaction ignore_action = { 0 };

	ignore_action.sa_handler = SIG_IGN;
	sigaction(SIGINT,&ignore_action,NULL);
	
	SIGINT_action.sa_handler = catchSIGINT;		//Handler to catch the SIGINT signal
	//SIGINT_action.sa_handler = SIG_DFL; // changed
	SIGINT_action.sa_flags = 0;

	sigfillset(&SIGINT_action.sa_mask);
	sigaction(SIGINT, &SIGINT_action, NULL);	//Create sigaction for SIGINT

	SIGTSTP_action.sa_handler = catchSIGTSP;	//Handler to catch the SIGTSTP signal
	sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = 0;
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);	//Create sigaction for SIGTSTP

	pid_t spawnpid = -5;

    while(1){
        num_arguments = get_commands(arg_array);
        run_commands(arg_array, num_arguments,&exitStatus,ignore_action, SIGINT_action);
    }
    return 0;
}