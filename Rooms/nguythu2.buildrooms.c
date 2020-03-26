#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

struct room{
    char * f;                                 //file name
    char * name;                              //room name
    int num_connections;                      //number of connections
    struct room * outboundConnections[6];     //array of room connections
    char * room_type;                            //room type - 0 = start | 1 = mid | 2 = end
};


//finds if value is in array (returns 0 if not)
int valueinarray(struct room val, struct room* input){
    int i;
    for (i = 0; i < input->num_connections; i++){
        if(strncmp(input->outboundConnections[i]->name,val.name,5) == 0){
            return 1;
        }
    }
    return 0;
}

//shuffles array (helps to randomly select room names)
void shuffle(char *array[], int n) {
    srand((unsigned)time(NULL));
    int j;
    int i;
    char buffer [50];
    for (j = 0; j < n; j++) {
        i = rand() % 10;
        char * t = array[i];
        array[i] = array[j];
        array[j] = t;
    }
}

//taken from lecture
void PrintRoomOutboundConnections(struct room* input){
  printf("The rooms connected to %s are:\n",input->name);
  int i;
  for (i = 0; i < input->num_connections; i++)
    printf("  %s \n", input->outboundConnections[i]->name);
  return;
}

int main () {
    char *room_names[10] = { "RoomA", "RoomB", "RoomC", "RoomD", "RoomE", "RoomF", "RoomG", "RoomH", "RoomI", "RoomJ"};

    //Creates Directory w/ PID
    int x = getpid();
    char buffer [50];
    char direct [50];
    int n = sprintf (buffer, "%d", x);
    char str[80];
    strcpy (str,"nguythu2.rooms.");
    strcat (str,buffer);
    strcpy(direct,str);
    if (mkdir(str, 0777) == -1){ 
        printf("Could not create");
    }else{
        // What I did was put room_names in array, shuffled it and then selected the first 5 elements for Room Names
        shuffle(room_names,10);
        struct room RoomArr[7];

        //MAKING ROOMS STRUCTS
        struct room r0;
        strcat(str,"/room0.txt");
        r0.name = calloc(16, sizeof(char));
        strcpy(r0.name,room_names[0]);
        r0.f = calloc(50, sizeof(char));
        strcpy(r0.f,str);
        RoomArr[0] = r0;

        strcpy(str,direct);

        struct room r1;
        strcat(str,"/room1.txt");
        r1.name = calloc(16, sizeof(char));
        strcpy(r1.name,room_names[1]);
        r1.f = calloc(50, sizeof(char));
        strcpy(r1.f,str);
        RoomArr[1] = r1;

        strcpy(str,direct);

        struct room r2;
        strcat(str,"/room2.txt");
        r2.name = calloc(16, sizeof(char));
        strcpy(r2.name,room_names[2]);
        r2.f = calloc(50, sizeof(char));
        strcpy(r2.f,str);
        RoomArr[2] = r2;

        strcpy(str,direct);

        struct room r3;
        strcat(str,"/room3.txt");
        r3.name = calloc(16, sizeof(char));
        strcpy(r3.name,room_names[3]);
        r3.f = calloc(50, sizeof(char));
        strcpy(r3.f,str);
        RoomArr[3] = r3;

        strcpy(str,direct);

        struct room r4;
        strcat(str,"/room4.txt");
        r4.name = calloc(16, sizeof(char));
        strcpy(r4.name,room_names[4]);
        r4.f = calloc(50, sizeof(char));
        strcpy(r4.f,str);
        RoomArr[4] = r4;

        strcpy(str,direct);

        struct room r5;
        strcat(str,"/room5.txt");
        r5.name = calloc(16, sizeof(char));
        strcpy(r5.name,room_names[5]);
        r5.f = calloc(50, sizeof(char));
        strcpy(r5.f,str);
        RoomArr[5] = r5;

        strcpy(str,direct);

        struct room r6;
        strcat(str,"/room6.txt");
        r6.name = calloc(16, sizeof(char));
        strcpy(r6.name,room_names[6]);
        r6.f = calloc(50, sizeof(char));
        strcpy(r6.f,str);
        RoomArr[6] = r6;

        for(x=0; x< 7;x++){
            RoomArr[x].num_connections = 0;
        }


        char t [16];
        strcpy(t,"START_ROOM");
        //Creating Room Types
        int start = rand() % 7;

        RoomArr[start].room_type = calloc(16, sizeof(char));
        strcpy(RoomArr[start].room_type,t);
        

        int end = start;
        while(end == start){
            end = rand() % 7;
        }
        strcpy(t,"END_ROOM");
        RoomArr[end].room_type = calloc(16, sizeof(char));
        strcpy(RoomArr[end].room_type,t);
        int i;
        strcpy(t,"MID_ROOM");
        for(i = 0; i < 7; i++){
            if ((i !=start) && (i != end)){
                RoomArr[i].room_type = calloc(16, sizeof(char));
                strcpy(RoomArr[i].room_type,t);
            }
        }
    
        //Creating Room Connections
        for(i = 0; i < 7; i++){
            int rand_connections = (rand() % 4) + 3; //3-6 connections per room
            while(RoomArr[i].num_connections < rand_connections){
                int add_room = rand() % 7;
                if ( (valueinarray(RoomArr[add_room],&RoomArr[i]) == 0) && (valueinarray(RoomArr[i],&RoomArr[add_room]) == 0) && (add_room != i)){
                    RoomArr[i].outboundConnections[RoomArr[i].num_connections] = &RoomArr[add_room];
                    RoomArr[add_room].outboundConnections[RoomArr[add_room].num_connections] = &RoomArr[i];

                    RoomArr[i].num_connections = RoomArr[i].num_connections +1;
                    RoomArr[add_room].num_connections = RoomArr[add_room].num_connections +1;
                }
            }
        }
        int in;

        //prints everything to file in correct order
        for(i = 0; i < 7; i++){
            FILE *fp = fopen(RoomArr[i].f, "w");
            fprintf(fp,"%s %s \n", "ROOM NAME: ",RoomArr[i].name);
            for(in = 0; in <RoomArr[i].num_connections; in++ ){
                fprintf(fp,"%s%d%s %s \n", "Connection ",in,":", RoomArr[i].outboundConnections[in]->name);
            }
            fprintf(fp,"%s \n \n", RoomArr[i].room_type);
            fclose (fp);
        }
    }
    return 0;
}