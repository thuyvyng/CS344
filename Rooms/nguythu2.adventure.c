#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>

#define NUM_THREADS 2

//used to create room array
struct room{
    char * f;                                 //file name
    char * name;                              //room name
    int num_connections;                      //number of connections
    struct room * outboundConnections[6];     //array of room connections
    char * room_type;                            //room type - 0 = start | 1 = mid | 2 = end
};

//used for creating dynamic arrays
typedef struct {
  int *array;
  size_t used;
  size_t size;
} Array;

//used for initializing dynamic array
void initArray(Array *a, size_t initialSize) {
  a->array = (int *)malloc(initialSize * sizeof(int));
  a->used = 0;
  a->size = initialSize;
}

//insert into dynamic array
void insertArray(Array *a, int element) {
  //if array is getting too smaller
  if (a->used == a->size) {
    a->size *= 2;
    a->array = (int *)realloc(a->array, a->size * sizeof(int));
  }
  a->array[a->used++] = element;
}

//deleting memory
void freeArray(Array *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

//congrats message + prints out the path taken
void PrintArray(Array *a,struct room * RoomAr){
  int i;
  printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
  printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS\n", a->used);
  for(i = 0; i <a->used; i++){
    printf("%s\n",RoomAr[a->array[i]].name);
  }
}

//taken from lecture
//prints out all the rooms connected to
void PrintRoomOutboundConnections(struct room* input){
  printf("The rooms connected to %s are:\n",input->name);
  int i;
  for (i = 0; i < input->num_connections; i++){
    printf("  %s \n", input->outboundConnections[i]->name);
  }
  return;
}
//taken from lecture
//finds most recently modified dir
void recentDir(char * newestDirName){
  int newestDirTime = -1; // Modified timestamp of newest subdir examined
  char targetDirPrefix[32] = "nguythu2.rooms."; // Prefix we're looking for
  memset(newestDirName, '\0', sizeof(newestDirName));

  DIR* dirToCheck; // Holds the directory we're starting in
  struct dirent *fileInDir; // Holds the current subdir of the starting dir
  struct stat dirAttributes; // Holds information we've gained about subdir

  dirToCheck = opendir("."); // Open up the directory this program was run in

  if (dirToCheck > 0) // Make sure the current directory could be opened
  {
    while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
    {
      if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
      {
        stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

        if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
        {
          newestDirTime = (int)dirAttributes.st_mtime;
          memset(newestDirName, '\0', sizeof(newestDirName));
          strcpy(newestDirName, fileInDir->d_name);
        }
      }
    }
  }
  closedir(dirToCheck); // Close the directory we opened

}
//finds room index
int findRoomFile(struct room * RoomAr, char * roomName){
  int i = 0;

  for(i = 0; i < 7; i++){
    if(strncmp(RoomAr[i].name,roomName,5) == 0){
      return i;
    }
  }
  printf("NOPE");
  return -1;
}

//creates a giant room array from reading in files
void create_RoomArr(struct room * RoomAr){
    char newestDirName[256]; // Holds the name of the newest dir that contains prefix
    recentDir(newestDirName);
    char *room_files[7] = { "/room0.txt", "/room1.txt", "/room2.txt", "/room3.txt", "/room4.txt", "/room5.txt", "/room6.txt"};

    int i;
    char buffer [256];
    strcpy(buffer,newestDirName);

    FILE *fp;

    //this for loop established all of the room names in giant array
    for (i = 0; i < 7; i++){
        strcat(buffer,room_files[i]);
        fp = fopen(buffer, "r");

        int c =0;
        char sub [10];
        char room_name [100];
        fscanf(fp, "%[^\n]", room_name);
        while (c < 5) {
          sub[c] = room_name[12 +c];
          c++;
        }
        //sub[c] = '\0';
        RoomAr[i].name = calloc(16, sizeof(char));
        strcpy(RoomAr[i].name,sub); 
        
        strcpy(buffer,newestDirName);
        fclose (fp);
    }

    for (i = 0; i < 7; i++){
        strcat(buffer,room_files[i]);
        fp = fopen(buffer, "r");

        //counts how many lines
        int ch;
        int lines = -1;
        int connections;
        while(!feof(fp)){
          ch = fgetc(fp);
          if(ch == '\n'){
            lines++;
          }
        }
        
        //goes to top of file
        fseek(fp,0L,0);


        //subtracts 2 from file to find out how many connections are in each room
        RoomAr[i].num_connections = (lines -2);

        //since the format of the line is
        char try [19];
        char try2 [19];
        char try3 [19];
        int c;
        fscanf (fp, "%s", try);
        fscanf (fp, "%s", try2);
        fscanf (fp, "%s", try3);

        for( c = 0; c < RoomAr[i].num_connections; c++){
            // split up the string
            fscanf (fp, "%s", try);
            fscanf (fp, "%s", try2);
            fscanf (fp, "%s", try3); //this one is the name
            
            int roomArrayIndex = findRoomFile(RoomAr,try3);
            RoomAr[i].outboundConnections[c] = &RoomAr[roomArrayIndex];

        }
        // finding room type
        fscanf (fp, "%s", try);
        
        RoomAr[i].room_type = calloc(16, sizeof(char));
        strcpy(RoomAr[i].room_type,try);

        strcpy(buffer,newestDirName);
        fclose (fp);
      }
}
//helps to debug - prints all contents of Room
void printRoomArray(struct room * RoomArray){
  int i;
  for(i = 0; i < 7; i ++){
    printf("%s %s \n","Room Name: ",RoomArray[i].name);
    printf("%s %s \n","Room type: ",RoomArray[i].room_type);
    printf("%s %d \n","Num connections: ",RoomArray[i].num_connections);
    PrintRoomOutboundConnections(&RoomArray[i]);
  }
}

//returns name that is the end room
char * findEnd(struct room * RoomArray){
  int i = 0;

  char * buffer = (char *)malloc(32 * sizeof(char));
  for(i = 0; i < 7; i++){
    if( strncmp(RoomArray[i].room_type,"END_ROOM",8) == 0){
      strcpy(buffer,RoomArray[i].name );
      return buffer;
    }
  }
  printf("end doesn't work");
  return buffer;
}
//returns index of start room
int findStart(struct room * RoomArray){
  int i = 0;
  for(i = 0; i < 7; i++){
    if( strncmp(RoomArray[i].room_type,"START_ROOM",10) == 0){
      return i;
    }
  }
  printf("start doesn't work");
  return -1;
}

//finds room index
int findRoomIndex(struct room * RoomArray, char *roomName){
  int i = 0;
  for(i = 0; i < 7; i++){
    if( strncmp(RoomArray[i].name,roomName,5) == 0){
      return i;
    }
  }
  printf("find Room Index doesn't work");
  return -1;
}

//Prints our current and possible location and deals with input
char * CurrentLocation(struct room* current){
  printf("%s %s \n%s", "CURRENT LOCATION: ",current->name, "POSSIBLE LOCATIONS: ");
  int i = 0;
  int num_c = (current->num_connections)-1;

  //prints list of possible locations
  for (i = 0; i < num_c; i++){
    printf("%s, ", current->outboundConnections[i]->name);
  }
  printf("%s.", current->outboundConnections[num_c]->name);
  printf("\n");

   char *buffer;
   size_t bufsize = 32;
   size_t characters;

   buffer = (char *)malloc(bufsize * sizeof(char));


   printf("WHERE TO? >");
   characters = getline(&buffer,&bufsize,stdin);
   printf("\n");
   int lengthCh = strlen(buffer);

   for(i = 0; i<current->num_connections;i++){
     if ((strncmp(buffer,current->outboundConnections[i]->name,5) == 0) && (strlen(buffer) == 6)){ //if valid name
       return buffer;
     }
   }

  printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
  //will only get called if answer was invalid
  char * y = CurrentLocation(current);
}
// void* printtimetofile(){
//     FILE *fp = fopen("currentTime.txt", "w");
//     time_t t; 
//     struct tm *tmp; 
//     char MY_TIME[40]; 
//     time(&t); 

//     printf("DO YOU")
//     tmp = localtime( &t ); 
      
//     // using strftime to display time 
//     strftime(MY_TIME, sizeof(MY_TIME), "%I:%M%p, %A, %B %d, %Y", tmp); 
      
//     fprintf(fp, MY_TIME );

//     fclose(fp); 
// }

// void printtimefromfile(){
//   FILE *fp = fopen("currentTime.txt", "r");
//   char time [100];
//   fscanf(fp, "%[^\n]", time);
//   printf("  %s",time);
// }

//main game function
void game(struct room * RoomArray){
  int start = findStart(RoomArray);
  char * end = findEnd(RoomArray);
  char *returned_str  = (char *)malloc(32 * sizeof(char));
  int location = start;

  Array a;
  initArray(&a, 1);

  while(strncmp(end,returned_str,5) != 0){
    returned_str = CurrentLocation(&RoomArray[location]);
    location = findRoomIndex(RoomArray,returned_str);
    insertArray(&a,location);
  }
  PrintArray(&a,RoomArray);
  freeArray(&a);
}

int main () {
    struct room RoomArr[7];
    create_RoomArr(RoomArr); //filles in array
    game(RoomArr);

    // pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
    // pthread_mutex_lock(&myMutex);
    
    // pthread_t threads1;

    // pthread_create(&threads1, NULL, printtimetofile, NULL);
    int i;
    for(i = 0; i <7; i++){
      free(RoomArr[i].name);
      free(RoomArr[i].room_type);
      free(RoomArr[i].f);
    }
    exit(0);
}