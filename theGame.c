/******************************************************************************
 * theGame 
 * Author: ElenaNick
 * Date: 2017/07/24
 * Description: this program finds the newest subdirectory with room files,
 *              reads contet of those files, and prompt user to find the path 
 *              from start room to end room. When end room is found, 
 *              the program prints congratulation message together with number
 *              of steps and path taken. During the game the program will print
 *              out current time if user would input "time" instead of room name
 * ***************************************************************************/

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

struct room{
	int id;
	char* name;
	char* connections[6];
	int numOfConnections;
	char* type;
};

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER; //init mutex

//function to write current time into specified file. 
//The file will be opened, written into and closed
void* timeKeeper(void* a){
	pthread_mutex_lock(&myMutex);   //lock the mutex, so it doesn't start yet
	char timeStr[64]; //will store a time string here
	memset(timeStr, '\0', sizeof(timeStr));
	time_t currTime = time(NULL); //get current time
	struct tm *locTime = localtime(&currTime); //convert to tm as local time
	strftime(timeStr, sizeof(timeStr)-1, "%I:%M%P, %A, %B %d, %Y", locTime); //make the string of time
	int file_descriptor = open("./currentTime.txt", O_WRONLY | O_TRUNC | O_CREAT, 0600); //open or create the file
	write(file_descriptor, timeStr, strlen(timeStr)*sizeof(char));
	close(file_descriptor);
	pthread_mutex_unlock(&myMutex);
	return NULL;
};

int main(){
	int result_code;
	pthread_t timer;
	pthread_mutex_lock(&myMutex); //lock the mutex
	result_code = pthread_create(&timer, NULL, timeKeeper, NULL);//start a thread
	if(result_code !=0 )
		printf("timeKeeper was not created\n");
	//get space for  array of rooms for the gme
	struct room* gameRooms;
	int i;
	gameRooms = malloc(7*sizeof(struct room));
		
/*find the newest directory with rooms in current directory
 * adapted from 2.4 Manipulating directories*/
	int newestDirTime = -1; //timestamp of newest subdir
	char targetDirPrefix[32] = "rooms.";//prefix for dirs to go through
	char newestDirName[256];//to hold the name of the newest dir
	memset(newestDirName, '\0', sizeof(newestDirName));//fill  it with \0s

	DIR* dirToCheck; //hold the directory we are starting in
	struct dirent *fileInDir; //holds the current subdir of the starting dir
	struct stat dirAttributes; //holds information about subdir

	dirToCheck = opendir("."); //open the current directory

	if(dirToCheck > 0){  //if dir was opened successfully
		while((fileInDir = readdir(dirToCheck)) != NULL){ //check each entry in te dir
			if(strstr(fileInDir->d_name, targetDirPrefix) != NULL){ //If entry had the prefix
				stat(fileInDir->d_name, &dirAttributes); //get attributes of the entry
				if((int)dirAttributes.st_mtime > newestDirTime){ //if this time is bigger = the subdir is newer
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName, '\0', sizeof(newestDirName));
					strcpy(newestDirName, fileInDir->d_name);
				}
			}
		}

	}

/* Open the newest subdir we found and read all the files in it*/
	//open the newest subdir we found
	DIR* dirToRead;
	char buffer[256]; //bufer to read the files into
	memset(buffer, '\0', sizeof(buffer));
	int roomNumber = 0; //to count our rooms
	int file_descriptor;
	dirToRead = opendir(newestDirName); //open the newwest dir
	if(dirToRead > 0){                  //if it opened fine
		while((fileInDir = readdir(dirToRead)) != NULL){ //get the files in dir one by one
			if(strcmp(fileInDir->d_name, ".") != 0 && strcmp(fileInDir->d_name, "..") != 0){ //if it's not a reference to current dir or subdir, but actual file name
			       //let's make a path to the file
			        strcpy(buffer, "./");
		       		strcat(buffer, newestDirName);
		 		strcat(buffer, "/");
				strcat(buffer, fileInDir->d_name);		
				file_descriptor = open(buffer, O_RDONLY); //open file for reading
				if(file_descriptor < 0){
					printf("Error: Could not open a file");
					exit(1);
				}	
				//read file byte by byte till we fund first :
				do{
					read(file_descriptor, buffer, 1);
				}
				while(buffer[0] != ':');
				memset(buffer, '\0', sizeof(buffer));
				lseek(file_descriptor, 1, SEEK_CUR); //move position for 1 byte - the space
				int counter=0; //to count how many chars we read
				do{           //read name char by char till end of the line
					read(file_descriptor, &buffer[counter], 1);
					counter++;
				}
				while(buffer[counter-1] != '\n');
				buffer[counter-1] = '\0'; //whipe the \n
				gameRooms[roomNumber].name = calloc(10, sizeof(char));
				strcpy(gameRooms[roomNumber].name, buffer);
				//now look for connections 
				gameRooms[roomNumber].numOfConnections = 0;
				do{
				//clear buffer and read next char in the file
				memset(buffer, '\0', sizeof(buffer));
				read(file_descriptor, buffer, 1);
				if(buffer[0] == 'C'){ //we are in line with connection
					lseek(file_descriptor, 13, SEEK_CUR); //move pointer to connection name
					counter = 0;
					do{
						read(file_descriptor, &buffer[counter], 1);
						counter++;
					}
					while(buffer[counter-1] != '\n');
					buffer[counter-1] = '\0';
					//allocate memory for one connection and copy the name from the file into that memory
					gameRooms[roomNumber].connections[gameRooms[roomNumber].numOfConnections] = calloc(16, sizeof(char));
					strcpy(gameRooms[roomNumber].connections[gameRooms[roomNumber].numOfConnections], buffer);
					gameRooms[roomNumber].numOfConnections++;
				}
				}
				while(buffer[0] != 'R');//if line begins with R, it's room type line
				//get to the room type in the line
				lseek(file_descriptor, 10, SEEK_CUR);
				memset(buffer, '\0', sizeof(buffer));
				counter = 0;
				do{ //read the type byte by byte till \n is encountered
					read(file_descriptor, &buffer[counter], 1);
					counter++;
				}
				while(buffer[counter-1] != '\n');
				buffer[counter-1] = '\0';//whipe '\n' from buffer with  '\0'
				//allocate memory for room type and copy there what was read from the file
				gameRooms[roomNumber].type = calloc(16, sizeof(char));
				strcpy(gameRooms[roomNumber].type, buffer);
				gameRooms[roomNumber].id = roomNumber;//room is = index in the array of rooms
				roomNumber++;
				close(file_descriptor);
			}
		}
	}
	closedir(dirToRead); //close the subdir
	closedir(dirToCheck); //Close the dir


/************************************************************
 * It's the game time!!
 * ********************************************************/

	int numCharsEntered = -5; //how many chars user have entered
	size_t bufferSize = 0; //size of allocated buffer
	char* userInput = NULL; //points to buffer where user's input is
	int pathSize = 8;
	int* path = malloc(pathSize*sizeof(int));

	struct room* currentRoom = &gameRooms[0];
	int steps = 0;
	i = 0;
	//find the start room
	while(strcmp(currentRoom->type, "START_ROOM") != 0){
		i++;
		currentRoom = &gameRooms[i];
	}
	do{
		int goodFlag = 0;//flag will be =1 if entered connection name is valid
		//print the prompt
		printf("CURRENT LOCATION: %s\n", currentRoom->name);
		printf("POSSIBLE CONNECTIONS: ");
		for(i=0; i<currentRoom->numOfConnections-1; i++)
			printf("%s, ", currentRoom->connections[i]);
		printf("%s.\n", currentRoom->connections[i]);
		printf("WHERE TO? >");
		//get the user's input
		numCharsEntered = getline(&userInput, &bufferSize, stdin);
		userInput[numCharsEntered-1] = '\0';

		//if user input is"time"
		while(strcmp(userInput, "time") == 0){
			pthread_mutex_unlock(&myMutex);   //start timer
			result_code = pthread_join(timer, NULL);  //join timer
			pthread_mutex_lock(&myMutex);   //timer is done, so lock the mutex again
			result_code = pthread_create(&timer, NULL, timeKeeper, NULL);
			//open the time file
			int file_descriptor = open("./currentTime.txt", O_RDONLY);
			char readBuffer[64];
			memset(readBuffer, '\0', sizeof(readBuffer));
			ssize_t nread;
			i = 0;
			//read the time file byte by byte till EOF is found
			do{
				nread = read(file_descriptor, &readBuffer[i], 1);
				i++;
			}
			while(nread > 0);
			//print out time and prompt
			printf("\n%s\n", readBuffer);
			printf("\nWHERE TO? >");
			//get another user's input
			free(userInput);
			userInput = NULL;
			bufferSize  = 0;
			numCharsEntered = getline(&userInput, &bufferSize, stdin);
			userInput[numCharsEntered-1] = '\0';
		}
		//if input was not "time", check if it was valid name
		for(i=0; i<currentRoom->numOfConnections; i++){
			if(strcmp(currentRoom->connections[i], userInput) == 0){
				int j;//find the requested room and reassign currentRoom pointer to it
				for(j=0; j<7; j++)
					if(strcmp(gameRooms[j].name, userInput) == 0){
						currentRoom = &gameRooms[j];
						goodFlag = 1;//set flag if connection found
					}
			}
		}
		if(goodFlag == 0)//if name is not valid, print error message
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
		else{
			//else check if path array has space for another step name
			printf("\n");
			if(steps >= pathSize){//if true, need to allocate more memory
				pathSize *=2; //just normal dynamic array stuff
				int* temp = malloc(pathSize*sizeof(int));
				int k;
				for(k=0; k<pathSize/2; k++)
					temp[k] = path[k];
				free(path);
				path = temp;
				temp = NULL;
			}
			path[steps] = currentRoom->id;//save the current room in the path array
			steps++;
		}
	}//till find the end room
	while(strcmp(currentRoom->type, "END_ROOM") != 0);

	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEP. YOUR PATH TO VICTORY WAS:\n", steps);
	for(i=0; i<steps; i++)
		printf("%s\n", gameRooms[path[i]].name);



	//game ofer. Free stuff.
	free(path);
	free(userInput);
	for(i=0; i<7; i++){
		free(gameRooms[i].name);
		free(gameRooms[i].type);
		int j;
		for(j=0; j<gameRooms[i].numOfConnections; j++)
			free(gameRooms[i].connections[j]);
	}
	free(gameRooms);
	pthread_mutex_destroy(&myMutex);
	return 0;
}
