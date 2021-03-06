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
char* getCurrRoom(FILE* fd);
char* getCurrConnec(FILE* fd);
int checkIfValid(char nextRoom[], char currConnection[]);

/*
    NEED TO: 
    Trying to go to an incorrect location does not increment the path history or the step count. Once the user has reached the End Room, the game should indicate that it has been reached. It should also print out the path the user has taken to get there (this path should not include the start room), the number of steps taken (not the number of rooms visited, which would be one higher because of the start room), a congratulatory message, and then exit
*/

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
    FILE *fd;
    int steps = 0;
    char currRoom[20];
    char currConnec[256];
    memset(currConnec, '\0', sizeof(currConnec));
    char roomPath[256];
    char nextRoom[30];
    char *roomT;
    int error;
    char passPath[256];
    char *room;

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

    strcpy(passPath, path);
    char* start = getStart(rooms, passPath);
    
    //opens the start room file
    char *filePath = malloc(strlen(path) + strlen(start)+2);
    path = strcat(path, "/");
    filePath = strcpy(filePath, path);
    filePath = strcat(filePath, start);
    fd = fopen(filePath, "r");

    //checks that the file open correctly
    if(fd == NULL){
        printf("open() failed on %s\n", filePath);
        perror("Error");
        exit(1);
    }

    //makes the start room to the current room
    strcpy(currRoom, start);
    char newPath[256];
    strcpy(newPath, "");

    do{
        strcpy(currRoom, getCurrRoom(fd));

        strcpy(currConnec, getCurrConnec(fd));

        roomT = strtok(currConnec, "#");
        roomT = strtok(NULL, "#");

        if(strncmp("END_ROOM", roomT, sizeof(roomT)) == 0){
            break;
        }

        printf("CURRENT LOCATION: %s\n", currRoom);
        printf("POSSIBLE CONNECTIONS: %s\n", currConnec);
        printf("WHERE TO? >");
        scanf("%s", nextRoom);
        printf("\n");

        error = checkIfValid(nextRoom, currConnec);
        while(error == 0){
            if(error == 0){
                printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN \n \n");
            }
            printf("CURRENT LOCATION: %s\n", currRoom);
            printf("POSSIBLE CONNECTIONS: %s\n", currConnec);
            printf("WHERE TO? >");
            scanf("%s", nextRoom);
            printf("\n");
            error = checkIfValid(nextRoom, currConnec);
        };
        
        strcpy(newPath, path);
        strcat(newPath, nextRoom);
        //closes previous room file
        fclose(fd);
        //opens new room file
        fd = fopen(newPath, "r");

        //adds the room picked by the user to the roomPath string
        strcat(roomPath, nextRoom);
        strcat(roomPath, " ");
        steps++;
        

        if(fd == NULL){
            printf("open() failed on %s\n", filePath);
            perror("Error");
            exit(1);
        }
        
    }while(strncmp("END_ROOM", roomT, sizeof(roomT)) != 0);

    printf("YOU HAVE FOUND THE END ROOM CONGRATUALTIONS!\n");
    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);

    room = strtok(roomPath, " ");
    while(room != NULL){
        printf("%s\n", room);
        room = strtok(NULL, " ");
    }


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
            if(!strcmp(line, "ROOM TYPE:START_ROOM")){
                start = strcat(start, roomName);
                return start;
            }
        }
        fclose(file_descriptor);
        
    }
    
    return start;
}


//returns the name of the room the user is currently in
char* getCurrRoom(FILE* fd){
    char line[256];
    char* word;
    fgets(line, sizeof(line), fd);
    word = strtok(line, ":");
    word = strtok(NULL, "");
    word[strcspn(word, "\n")] = 0;

    return word;
}

//returns the names of the connections for the room file passed in as the argument 
char* getCurrConnec(FILE *fd){
    char line[256];
    memset(line, '\0', sizeof(line));
    char* word;
    char* route = malloc(sizeof(char)*256);
    memset(route, '\0', sizeof(route));
    char next;
    int count = 0;
    char* prefix = "ROOM NAME:";

    while(1){
        fgets(line, sizeof(line), fd);
        prefix = strtok(line, ":");
        word = strtok(NULL, ":");

        if(strcmp(prefix, "ROOM TYPE") != 0){
            word[strcspn(word, "\n")] = 0;
            strcat(route, word);
            strcat(route, ", ");
        } else {
            word[strcspn(word, "\n")] = 0;
            strcat(route, "#");
            strcat(route, word);
            next = route[count];
            while(next != '#'){
                count++;
                next = route[count];
            }

            route[count-2] = '.';
            route[count] = '#';
            break;

        }
    }
    return route;
}

//returns 1 if the word matches a connection and 0 if the word doesn't
int checkIfValid(char nextRoom[], char currConnection[]){
    char *word = NULL;
    char copyConnection[200];

    strcpy(copyConnection, currConnection);

    //separates into words based on spaces, commas or periods 
    word = strtok(copyConnection, " ,.");

    while(word != NULL){
        //returns the word if it matches a word in the connection list
        if(strncmp(nextRoom, word, sizeof(nextRoom)) == 0){
            return 1;
        }
        word = strtok(NULL, " ,.");
    }
    //returns empty string if word doesn't match
    return 0;
}
