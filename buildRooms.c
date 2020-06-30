#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define _GNU_SOURCE
#define DIR_PREFIX "rooms."

char* findNewestDirectory(char* path, char* prefix);
struct AM** createRooms();
void createConnections(struct AM** graph);
void createRoomFiles(struct AM** graph, char* directoryName);
int connect(struct AM* row, int randRoom);

struct Node {
    int room; 
    struct Node* next;
};

struct AM{
    int numConnections;
    int room;
    struct Node* list;
};

int main(void){
    srand(time(NULL));
    
    //calculates a random number from 0 to 100,000
    int randomNum = rand() % 100000 + 1;

    //creates a new directory 
    char makeDir[40];
    sprintf(makeDir,"mkdir duellje.rooms.%d", randomNum);
    system(makeDir);

    //gets the newest directory
    char* latestRoomDirectory = findNewestDirectory(".", DIR_PREFIX);

    //creates the graph of the rooms
    struct AM** graph = createRooms();

    //creates the room files
    createRoomFiles(graph, latestRoomDirectory);

    return 0;
}

/*
    Goes to the directory specified by path
    Finds the directories that start with the prefix 
    Returns the name of the directory that has the latest modification time
*/
char* findNewestDirectory(char* path, char* prefix){
    struct stat dirStat;
    char directoryName[256];
    char* latestDirName;

    //opens the directory
    DIR* currDir = opendir(path);
    struct dirent *aDir;
    time_t lastModified;
    int i = 0; 

    while((aDir = readdir(currDir)) != NULL){
        //uses strncmp to check if the directory name matches the prefix
        if(strncmp(prefix, aDir->d_name, strlen(prefix)) == 0){
            stat(aDir->d_name, &dirStat);

            //checks if this directory has the latest modification date
            if(i == 0 || difftime(dirStat.st_mtime, lastModified) > 0){
                lastModified = dirStat.st_mtime;
                memset(directoryName, '\0', sizeof(directoryName));
                strcpy(directoryName, aDir->d_name);
            }
            i++;
        }
    }

    latestDirName = malloc(sizeof(char) * strlen(directoryName) + 1);
    strcpy(latestDirName, directoryName);

    closedir(currDir);
    return latestDirName;
}

/* 
    Generates 7 different room files which contains one room per file
    Each room name is hardcoded into the program 
    Each room is made up of:
        A room name
            each name must be unique and at max 8 characters long 
            only uppercase and lowercase letters are allowed 
        Outbound connections to other rooms 
            must be at least 3 outbound connections 
            max outbound connections is 6 
            outbound connections are assigned randomly 
            outbound connections have matching connections coming back
            a room can't have an outbound connection to itself
            a room can't have more than 1 connection to the same room
        A room type
            possible types: START_ROOM, END_ROOM and MID_ROOM
            each room type is randomly assigned to each room generated
            only one room will be assigned the start and end room types
*/
void createRoomFiles(struct AM** graph, char* directoryName){
    //list of 10 different room names 
    char *rooms [10] = {"Dungeon", "Basement", "Attic", "Living", "Plant", "Master", "Bath", "Study", "Laundry", "Dining"};

    //array to keep track of which room names have been taken (0 is available, 1 room name taken)
    int file_descriptor;
    struct Node* cur = NULL;
    int roomCount = 0;

    //creates 7 room files 
    for(int i = 0; i < 7; i++){
        char* room = rooms[graph[i]->room];

        //creates the file path for each room
        char *filepath = malloc(strlen(directoryName) + strlen(room) + 2);
        directoryName = strcat(directoryName, "/");
        filepath = strcpy(filepath, directoryName);
        filepath = strcat(filepath, room);

        //opens/creates a new file
        file_descriptor = open(filepath, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        
        if(file_descriptor == -1){
            printf("open() failed on \"%s\"\n", room);
            perror("Error");
            exit(1);
        }

        //writes the room details to the room file
        char* title = "ROOM NAME: ";
        char* message = malloc(strlen(title) + strlen(room) + 1);
        strcpy(message, title);
        strcat(message, room);

        write(file_descriptor, message, strlen(message));

        //writes the connections to the room file
        int count = 1;
        cur = graph[i]->list;
        if(cur != NULL){
            for(int i = 0; i < graph[i]->numConnections; i++){
                while(cur != NULL){
                char* connection = malloc(15 + strlen(rooms[cur->room]) + 1);
                char number[50];
                sprintf(number, "%d", count);
                strcpy(connection, "\nCONNECTION ");
                strcat(connection, number);
                strcat(connection, ":");
                strcat(connection, rooms[cur->room]);

                write(file_descriptor, connection, strlen(connection));
                cur = cur->next;

                count++;
                }
            }
        }
        //writes the room type to the file
        if(roomCount == 0){
            char roomType[] =  "\nROOM TYPE:START_ROOM\n";
            write(file_descriptor, roomType, strlen(roomType)); 
            
        } else if(roomCount == 6){
            char roomType[] =  "\nROOM TYPE:END_ROOM\n";
            write(file_descriptor, roomType, strlen(roomType)); 
        } else {
            char roomType[] =  "\nROOM TYPE:MID_ROOM\n";
            write(file_descriptor, roomType, strlen(roomType)); 
        }

        roomCount++;
        //closes the file 
        close(file_descriptor);
    }
}

//creates a graph of 7 rooms that are connected to each other by a minium of 3 connections and at most 6 connections
struct AM** createRooms(){
    struct AM** graph = malloc(sizeof(struct AM*)*7);
    struct AM* roomInit = NULL;
    int roomTaken[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int randRoom = 0;
    int roomCount = 0;

    while(roomCount < 7){
        randRoom = rand() % 10;
        if(roomTaken[randRoom] == 0){
            roomInit = malloc(sizeof(struct AM));
            roomInit->numConnections = 0;
            roomInit->room = randRoom;
            roomInit->list = NULL;
            graph[roomCount] = roomInit;
            roomCount++;
            roomTaken[randRoom] = 1;
        }
    }

    createConnections(graph);
    return graph;
}

//creates connections between the rooms created
void createConnections(struct AM** graph){
    int rooms[7];
    int randNum = 0;
    int numConn = 0;
    int randRoom = 0;

    for(int i = 0; i < 7; i++){
        rooms[i] = graph[i]->room;
    }

    struct Node* cur;

    for(int j = 0; j < 7; j++){
        numConn = graph[j]->numConnections;
        while(numConn < 3){
            randNum = rand() % 7;
            randRoom = rooms[randNum];
            //checks if the connection is equal to itself or is already a connection
            if(randRoom != graph[j]->room && connect(graph[j], randRoom)){
                graph[j]->numConnections++;
                if(connect(graph[randNum], graph[j]->room)){
                    graph[randNum]->numConnections++;
                }
                numConn++;
            }
        }
    }
}

//checks if two rooms can be connected
//returns 1 if the room isn't already in connections otherwise it returns 0
int connect(struct AM* row, int randRoom){
    struct Node* cur = row->list;
    struct Node* prev = NULL;
    struct Node* newNode = malloc(sizeof(struct Node));

    //checks if this is the first link in the list
    if(cur == NULL){
        newNode->next = NULL;
        newNode->room = randRoom;
        row->list = newNode;
        return 1;
    }

    //checks if the room is already connected
    while(cur != NULL){
        if(cur->room == randRoom){
            return 0;
        }
        prev = cur;
        cur = cur->next;
    }

    //adds room to the end of the list
    newNode->next = NULL;
    newNode->room = randRoom;
    prev->next = newNode;

    return 1;
}
