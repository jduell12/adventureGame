#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define _GNU_SOURCE
#define DIR_PREFIX "duellje.rooms."

char* findNewestDirectory(char* path, char* prefix);
void createRoomFiles(char* directoryName);


int main(void){
    srand(time(NULL));
    
    //calculates a random number from 0 to 100,000
    int randomNum = rand() % 100000 + 1;

    //creates a new directory 
    char makeDir[40];
    sprintf(makeDir,"mkdir duellje.rooms.%d", randomNum);
    system(makeDir);

    //enters the new directory
    char* latestRoomDirectory = findNewestDirectory(".", DIR_PREFIX);
    
    //creates room files 
    createRoomFiles(latestRoomDirectory);

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
        A room type
            possible types: START_ROOM, END_ROOM and MID_ROOM
            each room type is randomly assigned to each room generated
            only one room will be assigned the start and end room types
        Outbound connections to other rooms 
            must be at least 3 outbound connections 
            max outbound connections is 6 
            outbound connections are assigned randomly 
            outbound connections have matching connections coming back
            a room can't have an outbound connection to itself
            a room can't have more than 1 connection to the same room
*/
void createRoomFiles(char* directoryName){
    //list of 10 different room names 
    char *rooms [10] = {"Dungeon", "Basement", "Attic", "Living", "Plant", "Master", "Bath", "Study", "Laundry", "Dining"};

    //array to keep track of which room names have been taken (0 is available, 1 room name taken)
    int roomTaken[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int roomCount = 0;
    int file_descriptor;

    //creates 7 room files 
    while(roomCount < 7){
        //gets a random number from 0 to 9
        int randomNum = rand() % 10;

        //checks if the room name has already been taken
        while(roomTaken[randomNum] == 1){
            randomNum = rand() % 10;
        }

        char* room = rooms[randomNum];
        roomTaken[randomNum] = 1;

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

        //writes the room type to the file
        if(roomCount == 0){
            char roomType[] =  "\nROOM TYPE: START_ROOM";
            write(file_descriptor, roomType, strlen(roomType)); 
            
        } else if(roomCount == 6){
            char roomType[] =  "\nROOM TYPE: END_ROOM";
            write(file_descriptor, roomType, strlen(roomType)); 
        } else {
            char roomType[] =  "\nROOM TYPE: MID_ROOM";
            write(file_descriptor, roomType, strlen(roomType)); 
        }

        //closes the file 
        close(file_descriptor);

        roomCount++;
    }
}
