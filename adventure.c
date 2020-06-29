#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define DIR_PREFIX "duellje.rooms."

char* findNewestDirectory(char* path, char* prefix);
void game(char* path);
char* getStart(char* rooms[7], char* dirName);
void printfile(char* dirName, char* roomName);

int main(void){
    //gets the newest directory
    char* latestRoomDirectory = findNewestDirectory(".", DIR_PREFIX);

    game(latestRoomDirectory);

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

//starts the adventure game 
void game(char* path){
    char *files [9];
    //opens the directory and copies the room names into the rooms array
    DIR* currDir = opendir(path);
    struct dirent *ent;
    int count = 0;

    while((ent = readdir(currDir)) != NULL){
        files[count] = ent->d_name;
        count++;
    }

    files[0] = files[8];
    files[1] = files[7];

    //copies the room name files into the rooms array so that the array doesn't have extra files such as . or ..
    char *rooms[7];
    for(int i = 0; i < 7; i++){
        rooms[i] = files[i];
    }

    char* start = getStart(rooms, path);
    printf("CURRENT LOCATION: %s\n", start);
    printf("POSSIBLE CONNECTIONS: ");
    printfile(path, start);


}

//returns the filename of the start room 
char* getStart(char* rooms[7], char* dirName){
    char* line;
    size_t len = 0;
    ssize_t read;
    FILE *file_descriptor;
    char* start = malloc(22);

    for(int i = 0; i < 7; i++){
        char* roomName = rooms[i];

        char* filePath = malloc(strlen(dirName) + strlen(roomName) + 2);
        dirName = strcat(dirName, "/");
        filePath = strcpy(filePath, dirName);
        filePath = strcat(filePath, roomName);

        //opens the file
        file_descriptor = fopen(filePath, "r");

        if(file_descriptor == NULL){
            printf("open() failed on %s \n", filePath);
            perror("Error");
            exit(1);
        }

        while((read = getline(&line, &len, file_descriptor)) != -1){
            line[strcspn(line, "\n")] = 0;
            if(!strcmp(line, "ROOM TYPE: START_ROOM")){
                start = strcat(start, roomName);
                return start;
            }
        }
        fclose(file_descriptor);
        
    }
    
    return start;
}

//prints information from the file in the correct format for the interface 
void printfile(char* dirName, char* roomName){
    
}
