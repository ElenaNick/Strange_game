/******************************************************************************
 * Author: ElenaNick
 * Date: 2017/07/24
 * Description: this program creates a directory rooms.<process ID> and
 *              reate 7 files in this directory with rooms description. Names
 *              for the rooms are chosen randomly from 10 pre-defined options. 
 *              Each room has 3 to 6 connections to other rooms.
 *              This program adapts pseudocode from "2.2 Program Outline"
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>


struct room
{
	int id;
	char* name;
	char* type;
	struct room* connections[6];
	int numOfConnections;
};

//Returns 1 if all rooms have 3 to 6 outbond connections, 0 otherwise
int IsGraphFull(struct room graph[]){
	int i;
	for(i=0; i<7; i++)
		if(graph[i].numOfConnections <3)
			return 0;
	return 1;
}

//Returns  a random room
struct room* GetRandomRoom(struct room graph[]){
	int index = rand()%7;
	return &graph[index];
}

//Returns 1 if a connection can be added from a room, 0 otherwise
int  CanAddConnectionFrom (struct room* A){
	if((*A).numOfConnections < 6)
		return 1;
	return 0;
}

//Connects rooms A to a room B, doesnt check if this connection is valid
void ConnectRoom(struct room* A, struct room* B){
	(*A).connections[(*A).numOfConnections] = B;
	(*A).numOfConnections++;
}

//return 1 if rooms are the same room or if there is connection between them already, 0 otherwise
int IsSameRoom(struct room* A, struct room* B){
	//check if the same room
	if((*A).id == (*B).id)
		return 1;
	int i;
	//check if there is a connection already
	for(i=0; i<A->numOfConnections; i++)
		if(A->connections[i]->id == B->id)
			return 1;
	return 0;
}

//Adds a random, valid outbond connection from one room to another room
void AddRandomConnection(struct room graph[]){
	struct room* A;
	struct room* B;
	//assign A to a random room
	do{
		A = GetRandomRoom(graph);
	}
	while(CanAddConnectionFrom(A) == 0);
	//assign B to another room
	do{
		B = GetRandomRoom(graph);
	}
	//Check if B can add a connection and if A is connected to B already
	while(CanAddConnectionFrom(B) == 0 || IsSameRoom(A,B) == 1);
	//connect A and B
	ConnectRoom(A, B);
	ConnectRoom(B, A);
}


int main(){
/*Make rooms for the game*/
	//make rooms names and types, and all other standard stuff
	char* roomNames[] = {"Sputnik", "Luna", "Vostok", "Molnia", 
	             "Kosmos", "Voshod", "Proton", "Souz",
		     "Tsiklon", "Zenit"};
	char* roomType[] = {"START_ROOM", "END_ROOM", "MID_ROOM"};
	char* fileName[] = {"Room1.txt", "Room2.txt", "Room3.txt", "Room4.txt", 
			    "Room5.txt", "Room6.txt", "Room7.txt"};
	char* writeName = "ROOM NAME: ";
	char* writeConnection = "CONNECTION ";
	char* writeType = "ROOM TYPE: ";

	//get meemory for rooms
	struct room* roomsForGame = malloc(7*sizeof(struct room));
	srand(getpid());
	int i;
	int collector[7];
	//initialize 7 rooms with names and types, and numbr of connections = 0
	for(i=0; i<7; i++){
		//assign room type
		if(i<2)
			roomsForGame[i].type = roomType[i];
		else
			roomsForGame[i].type = roomType[2];
		//assign id to a room
		roomsForGame[i].id = i;
		int index;
		//choose randomly 1 name out of 10
		int newIndexFound;
		do{
			//get random index
			index = rand()%10;
			newIndexFound = 1;
			int j;
			//check if we used this index before
			for(j=0; j<i && newIndexFound == 1; j++){
				if(collector[j] == index)
					newIndexFound = 0;
			}}
		while(newIndexFound == 0);
		collector[i] = index;
		//create space for room's name and assign chosen name
		roomsForGame[i].name = calloc(10, sizeof(char)); 
		strcpy(roomsForGame[i].name, roomNames[index]);
		//set number of connections = 0
		roomsForGame[i].numOfConnections = 0;
	}

/*Connect rooms randomly*/
	while(!IsGraphFull(roomsForGame)){
		AddRandomConnection(roomsForGame);
	}

	
/*Make a directory with rooms.<pid> name*/
	//adapted from Piazza responce by Ryan Gambord
	//get the process id
	int pid = getpid();
	//get length of pid
	int pid_len = snprintf(NULL, 0, "%d", pid);
	//create dirName with enough space to contain it
	char* dirName = calloc((20+pid_len), sizeof(char));
	//combine dirName with process id
	sprintf(dirName, "./rooms.%d", pid);
	//create the directory
	int dirResult = mkdir(dirName, 0755);
	if(dirResult == -1){
		printf("Error: Could not create new directory\n");
		return 1;
	}
	//open up the created directory
	DIR* dirToWrite = opendir(dirName);
	if(dirToWrite > 0){
		for(i=0; i<7; i++){
			//make a path name for i-th file
			char newFilePath[255];
			strcpy(newFilePath, dirName);
			//add file name to dir name
			strcat(newFilePath, "/");
			strcat(newFilePath, fileName[i]);
			//create new file and open it for writing
			int file_descriptor = open(newFilePath, O_WRONLY | O_CREAT, 0600);
			if(file_descriptor == -1)
				printf("Error: Could not create a file\n");
			//write room's name into the file
			//get temporary storage to build strings to write 
			char* tempStr = calloc(30, sizeof(char));
			//build room name line for a file
			strcpy(tempStr, writeName);
			strcat(tempStr, roomsForGame[i].name);
			strcat(tempStr, "\n");
			//write it to the file
			write(file_descriptor, tempStr, strlen(tempStr));
			//loop to make up conections lines and write it to the file
			int j;
			for(j=0; j<roomsForGame[i].numOfConnections; j++){
				//clear tempStr
				memset(tempStr, '\0', strlen(tempStr));
				//make up a line
				strcpy(tempStr, writeConnection);
				char tempStr2[2];
				sprintf(tempStr2, "%d", (j+1));
				strcat(tempStr, tempStr2);
				strcat(tempStr, ": ");
				strcat(tempStr, roomsForGame[i].connections[j]->name);
				strcat(tempStr, "\n");	
				//write the line into the file
				write(file_descriptor, tempStr, strlen(tempStr));	
			}
			//make up a line with room type
			memset(tempStr, '\0', strlen(tempStr));
			strcpy(tempStr, writeType);
			strcat(tempStr, roomsForGame[i].type);
			strcat(tempStr, "\n");
			//write it to the file
			write(file_descriptor, tempStr, strlen(tempStr));
			//close the file
			close(file_descriptor);
			free(tempStr);
		}

	}
	closedir(dirToWrite);
	for(i=0; i<7; i++)
		free(roomsForGame[i].name);
	free(roomsForGame);
	free(dirName);
	
return 0;
}
