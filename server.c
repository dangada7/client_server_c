/*
 ============================================================================
 Name        : server.c
 Author      : dan
 Version     : 1
 Copyright   : Your copyright notice
 Description : server bla bla
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>   		 //strlen
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h> 		//inet_addr
#include <unistd.h>     	//write
#include <math.h>


#define CONTROL_PORT 50006

int mallocNum = 0;

struct file{
	int fileNameSize;
	char *fileName;
	int fileContentSize;
	char *fileContent;
};

struct clientNode{
	int countFiles; //count the number of filed the server get from the client
	int sd;
	int numOfFiles;
	struct clientNode* next;
	int* filesSd;
};

/********************************************************
 * closeSocket
 * do:			(1)close socket
 * 				(2)print socket id
 ********************************************************/
void closeSocket(int sd){
	close(sd);
	printf("close socketId-%d\n",sd);
}

/********************************************************
 * closeSockets
 * do:		get array of socketId and call closeSocket with each socket in the array
 ********************************************************/
void closeSockets(int* socket, int len, char* varName)
{
	int i;
	printf("close %s:\n",varName);
	for(i=0; i < len; i++)
	{
		printf("(%d) ",i);
		closeSocket(socket[i]);
	}
}

/********************************************************
 * myFree
 * do			(1)free  var
 * 				(2)print varName
 ********************************************************/
void myFree(void* var, char* varName)
{
	printf("free %s\n",varName);
	mallocNum--;
	free(var);
}

/********************************************************
 * freeFiles
 ********************************************************/
void freeFiles(struct file* files, int len ,char* varName)
{
	int i;

	puts("free files array:");
	for(i=0; i<len; i++ )
	{
		printf("(%d) file:\n",i);
		myFree(files[i].fileContent,"fileContent");
		myFree(files[i].fileName,"fileName");
	}
	myFree(files,varName);
}

/********************************************************
 * freeClient
 * do			(1) arrange the clientList
 * 				(2) free the freeNode variable
 ********************************************************/
void freeClient(struct clientNode* freeNode,struct clientNode** clientsList)
{
	struct clientNode* currentNode;

	puts("**free client:");
	//start from the head of the list
	currentNode = *clientsList;
	// free node is the first node in the list
	if(*clientsList == freeNode)
	{
		*clientsList = (*clientsList)->next;

	// free node is not the first node in the list
	}else
	{
		//for each client in clientlist
		while(currentNode->next!=NULL)
		{
			if(currentNode->next== freeNode)
			{
				currentNode->next = currentNode->next->next;
				break;
			}
		}
	}

	// free freeNode
	closeSockets(freeNode->filesSd, freeNode->countFiles,"filesSd");
	myFree(freeNode->filesSd,"filesSd");
	myFree(freeNode,"clientFree");
}

/********************************************************
 * arrIntMalloc
 * do			(1)Allocates (size*int) memory to var
 * 				(2)print the name of the var
 ********************************************************/
void arrIntMalloc(int** var, int size, char* varName){
	printf("malloc %s\n",varName);
	*var =(int* )malloc(size*(sizeof(int)));
	mallocNum++;
}

/********************************************************
 * mallocFilesArr
 * do			(1)Allocates (size*sizeof(struct file)) memory to var
 * 				(2)print the name of the var
 ********************************************************/
void mallocFilesArr(struct file** var,int size, char* varName)
{
	printf("malloc %s\n",varName);
	*var =(struct file*)malloc(size*(sizeof(struct file)));
	mallocNum++;
}

/********************************************************
 * mallocString
 ********************************************************/
void mallocString(char** fileName,int size,char* varName)
{
	printf("malloc %s\n",varName);
	*fileName =(char*)malloc(size*(sizeof(char)));
	mallocNum++;
}

/********************************************************
 * mallocNewClient
 ********************************************************/
void mallocNewClient(struct clientNode** newClient)
{
	puts("malloc new client");
	*newClient =(struct clientNode*)malloc((sizeof(struct clientNode)));
	(*newClient)->next = NULL;
	mallocNum++;
}

/********************************************************
 * listenOnPort
 * do:			create, bind and listen to new socket on Localhost and port
 * return: 		socketId
 ********************************************************/
int listenOnPort(int port){
	struct sockaddr_in address;
	int new_socket;

	socklen_t addrsize;
	struct sockaddr_storage their_addr;
	addrsize = sizeof (their_addr);

	//init the address with port
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if( (new_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
	{
		perror("socket failed");
		return -1;
	}

	//bind the socket to address with port = 0 , return a bind socket to a free port.
	if (bind(new_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		return -1;
	}

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(new_socket, 3) < 0)
	{
		perror("listen");
		return -1;
	}

	printf("Listener on port %d, socketId-%d \n", port,new_socket);
	return new_socket;
}
int listenOnPort2(int portNumber)
{
	int master_socket;
	struct addrinfo hints, *res;

	//get address info
	memset(&hints, 0, sizeof (hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, portNumber, &hints, &res);

	//create a master socket
	if( (master_socket = socket(res->ai_family , res->ai_socktype , res->ai_protocol)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	//bind the socket to localhost port 50006
	if (bind(master_socket, res->ai_addr, res->ai_addrlen)<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	//listen
	if (listen(master_socket, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	//accept the incoming connection
	printf("Listener on port %d \n", master_socket);


	return master_socket;
}

/********************************************************
 * acceptConnect
 * do:			accept connection from sd
 * return:		sd that return from accept(sd)
 ********************************************************/
int acceptConnection(int sd){

	int new_socket;
	struct sockaddr_storage their_addr;
	socklen_t addrsize = sizeof (their_addr);

	printf("waiting for connection on socket-%d\n",sd);
	// accept
	if ((new_socket = accept(sd, (struct sockaddr *)&their_addr, &addrsize))<0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}

	//inform user of socket number - used in send and receive commands
	printf("New connection , socket-%d\n" , new_socket);

	return new_socket;
}

/********************************************************
 * getNumberOfFilesFromClient
 * do:		read int from sd
 ********************************************************/
int getIntFromClient(int sd, char* varName){
	int valread, msg;

	//Check if it was for closing , and also read the incoming message
	printf("read int from socket-%d\n",sd);
	if ((valread = read( sd , &msg, sizeof(int))) < 0)
	{
		perror("read");
	}else
	{
		printf("%s=%d\n",varName ,msg);
	}

	return msg;
}

/********************************************************
 * listenOnSocketWithFreePorts
 * return:  sockets array
 ********************************************************/
int* listenOnSocketWithFreePorts(int numberOfPorts)
{
	int * FilesSockets,i;
	arrIntMalloc(&FilesSockets, numberOfPorts, "FilesSocket");

	puts("listenOnFreePorts");
	for(i=0; i< numberOfPorts; i++){
		printf("(%d) ",i);
		FilesSockets[i] = listenOnPort(0);
	}

	return FilesSockets;
}

/********************************************************
 * getPortNumberFromSockets
 * do:		get array of socketId and call closeSocket with each socekt in the array
 ********************************************************/
int* getPortNumberFromSockets(int* filesSocketsArr, int len)
{
	int* freePorts, i;

	struct sockaddr_in sin;
	socklen_t sinLen = sizeof(sin);

	arrIntMalloc(&freePorts, len,"freePorts");

	puts("free ports:");
	for(i=0; i<len; i++)
	{
		if (getsockname(filesSocketsArr[i], (struct sockaddr *)&sin, &sinLen) == -1)
		{
		    perror("getsockname");
		}//end if
		else
		{
			freePorts[i] = ntohs(sin.sin_port);
		    printf("%d)port number %d\n", i, freePorts[i]);
		}//end else
	}//end for

	return freePorts;
}

/********************************************************
 * getPortNumberFromSockets
 * do:		get array of socketId and call closeSocket with each socekt in the array
 ********************************************************/
char* getStringFromClient(int sd,int len ,char* varName)
{
	char* fileName;
	mallocString(&fileName, len, varName);

	//Check if it was for closing , and also read the incoming message
	printf("read string from socket-%d\n",sd);
	if (read( sd , fileName, len*sizeof(char)) < 0)
	{
		perror("read");
	}else
	{
		printf("%s=%s\n",varName ,fileName);
	}

	return fileName;
}

/********************************************************
 * SendIntArrayToClient
 ********************************************************/
int SendIntArrayToClient(int sd, int* msg, int len, char* varName)
{

	printf("send %s, sockedId-%d\n",varName, sd);
	if( send(sd , msg , len*sizeof(int), 0) < 0)
	{
		perror("Send");
		return -1;
	}

	return 1;
}

/********************************************************
 * accept_connection_from_array_of_sockets
 ********************************************************/
int* accept_connection_from_array_of_sockets(int* filesSdArr, int len,char* varName)
{
	int* connectedFilesSocketsArr, i;

	arrIntMalloc(&connectedFilesSocketsArr, len, "connectedFilesSocketsArr");


	printf("accept connection to %s\n", varName);
	for(i=0; i<len; i++)
	{
		printf("(%d)",i);
		connectedFilesSocketsArr[i] = acceptConnection(filesSdArr[i]);
	}

	return connectedFilesSocketsArr;
}

/********************************************************
 * get_files_from_array_of_sockets
 ********************************************************/
struct file* get_files_from_array_of_sockets(int* sdArr, int len)
{
	int i;
	struct file* files;

	mallocFilesArr(&files,len, "files");

	puts("get files from user");
	for(i=0; i<len; i++)
	{
		printf("(%d) file:\n",i);
		files[i].fileNameSize = getIntFromClient(sdArr[i],"fileNameSize");
		files[i].fileName = getStringFromClient(sdArr[i],files[i].fileNameSize,"fileName");
		files[i].fileContentSize = getIntFromClient(sdArr[i],"fileContentSize");
		files[i].fileContent = getStringFromClient(sdArr[i],files[i].fileContentSize,"fileContent");
	}


	return files;
}

/********************************************************
 * writeToFile
 ********************************************************/
void writeToFile(char* fileName,char* fileContent ){
	FILE* fp;

	//open file
	if ( (fp = fopen(fileName,"w")) == NULL ){
		printf("fails to open the file: %s \n", fileName);
		return;
	}
	printf("open file: %s \n", fileName);

	//write to file
	fputs(fileContent,fp);
	printf("write to file:%s\n",fileContent);

	//close file
	fclose(fp);
	printf("close file: %s \n", fileName);
}

/********************************************************
 * writeFiles
 ********************************************************/
void writeFiles(struct file* files,int len)
{
	int i;

	puts("write to files");
	for (i=0; i<len; i++)
	{
		printf("file %d\n",i);
		writeToFile(files[i].fileName, files[i].fileContent);
	}

}

/********************************************************
 * initSet
 * do		insert the msd and all the cliend sd to the set readfds
 * return 	max_sd
 ********************************************************/
int initSet(fd_set* readfds, struct clientNode* clientsList, int msd){
	int max_sd, i, j;
	struct clientNode* currentClient;

	//Clear all entries from the set
	FD_ZERO(readfds);
	max_sd=0;
	puts("set - clear");

	//add master sd to the set
	FD_SET(msd, readfds);
	max_sd=msd;
	puts("set - add msd");

	//add client to the set
	currentClient = clientsList;

	//for each client
	j=1;
	puts("set - add clients:");
	while(currentClient!= NULL)
	{
		//for each client add all the filesSd to the set.
		printf("client %d:\n",j);
		for(i=0; i< currentClient->numOfFiles; i++)
		{
			// add client to the set
			FD_SET(currentClient->filesSd[i], readfds);

			//get the max sd number
			if(currentClient->filesSd[i] > max_sd)
				max_sd = currentClient->filesSd[i];

			//print stuff
			printf("(%d) add fileSd %d to the set\n",i ,currentClient->filesSd[i]);
		}

		//go to the next client
		j++;
		currentClient = currentClient->next;
	}

	return max_sd;
}

/********************************************************
 * handleOneClient
 ********************************************************/
void handleOneClient(int sd){
	int clientSd, numberOfFiles;
	struct file* file;
	numberOfFiles = 1;

	//(7)accept connection from each of the socket in filesSocketsArr
	puts("\n------------(3.4)accept connection;--------------");
	clientSd = acceptConnection(sd);
	//connectedFilesSocketsArr = accept_connection_from_array_of_sockets(filesSocketsArr, numberOfFiles, "filesSocketsArr");

	//(8)get file from each of the socket in connectedFilesSocketsArr
	puts("\n------------(3.5)get files--------------");

	file = get_files_from_array_of_sockets(&clientSd, numberOfFiles);

	//(8)get file from each of the socket in connectedFilesSocketsArr
	puts("\n------------(3.6)save files--------------");
	writeFiles(file, numberOfFiles);

	//(9)close master_socket, client_socket and all the socket in filesSockets
	puts("\n------------(3.7)close sockets and free file--------------");
	closeSocket(clientSd);
	freeFiles(file,numberOfFiles,"file");


}

/********************************************************
 * handleOneClient
 * do				add new client to the head of the list
 ********************************************************/
void addToList(struct clientNode** newclient,struct clientNode** clientsList)
{
	(*newclient)->next = *clientsList;
	*clientsList = *newclient;
}

/********************************************************
 * handleNewConnectionEvnet
 ********************************************************/
void handleControlPortEvent(int masterSocket,struct clientNode** clientsList)
{
	int* freePortsArr;
	struct clientNode* newclient;

	puts("\n------------(2.1)create new client--------------");
	//(2.1.1) allocate memory for new client
	mallocNewClient(&newclient);
	newclient->countFiles = 0;
	//(2.1.2)add client to the list of clients
	addToList(&newclient, clientsList);

	puts("\n------------(2.2)accept new connection--------------");
	//(2.2) accept new connection and save the sd in the client struct
	newclient->sd = acceptConnection(masterSocket);

	puts("\n------------(2.3)get number of files--------------");
	//(2.3)get port Number from client
	newclient->numOfFiles = getIntFromClient(newclient->sd,"numberOfFiles");

	puts("\n------------(2.4)listen on random ports--------------");
	//(2.4)listen to socket on random free ports
	newclient->filesSd  = listenOnSocketWithFreePorts(newclient->numOfFiles);

	//(2.5)get filesSockets port
	puts("\n------------(2.5)random ports--------------");
	freePortsArr = getPortNumberFromSockets(newclient->filesSd, newclient->numOfFiles);

	//(2.6)send to client the free ports
	puts("\n------------(2.6)send free ports to client--------------");
	SendIntArrayToClient(newclient->sd,freePortsArr, newclient->numOfFiles ,"freePortsArr");

	//(2.7)close client socket
	puts("\n------------(2.7)free freePortsArr--------------");
	myFree(freePortsArr,"freePortsArr");
	closeSocket(newclient->sd);
}

/********************************************************
 * handleClientsEvent
 * do					for each client in clientList check if his socket is ready for reading (and if so call handleOneClient(Sd)
 ********************************************************/
void handleFilesPortsEvents(struct clientNode** clientsList,fd_set readfds)
{
	int i=0;
	struct clientNode* currentClient;

	//start from the head of the list
	currentClient = *clientsList;
	//for each client in the client list
	while(currentClient!=NULL)
	{
		//for each fileSd in client
		for(i=0; i<currentClient->numOfFiles ; i++)
		{
			// if currentClient->filesSd[i] is ready for reading
			if(FD_ISSET(currentClient->filesSd[i], &readfds))
			{
				handleOneClient(currentClient->filesSd[i]);
				currentClient->countFiles++;

				//if we get all the files from the client free the client
				if(currentClient->countFiles == currentClient->numOfFiles)
				{
					freeClient(currentClient, clientsList);
				}
				return;
			}

		}
		//else go to the next client
		currentClient = currentClient->next;
	}


}

/********************************************************
 * server1
 * general			i divided the socket to two groups:
 * 					(1) socket that bind to the control port
 * 					(2) all the other socket (upload the files)
 ********************************************************/
void server1()
{
	fd_set readfds;
	int max_sd, masterSocket;
	struct clientNode* clientsList;

	clientsList=NULL;

	//listen on control port
	puts("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	puts("~~~~~~~~~~~~(0)listen on control port~~~~~~~~~~~~");
	puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	masterSocket = listenOnPort(CONTROL_PORT);

	while(1)
	{
		printf("\ndebug mallocNum = %d\n",mallocNum);

		//initSet
		puts("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
		puts("~~~~~~~~~~~~(1)initSet~~~~~~~~~~~~");
		puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
		max_sd = initSet(&readfds, clientsList, masterSocket);

		//select
		if (select( max_sd + 1 , &readfds , NULL , NULL , NULL) < 0) {
			perror("select");
		}

		//handle connection event
		if(FD_ISSET(masterSocket, &readfds))
		{
			puts("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
			puts("~~~~~~~~~~~~(2)handleControlPortEvent~~~~~~~~~~~~");
			puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
			handleControlPortEvent(masterSocket,&clientsList);

		//handle clients event
		}else
		{
			puts("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
			puts("~~~~~~~~~~~~(3)handleFilesPortsEvents~~~~~~~~~~~~");
			puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
			handleFilesPortsEvents(&clientsList, readfds);
		}
	}

	closeSocket(masterSocket);

}

/********************************************************
 * server2
 * general		without select
 ********************************************************/
void server2()
{
	int masterSocket, connectedMasterSocket, numberOfFiles, *filesSocketsArr, *freePortsArr, * connectedFilesSocketsArr;
	struct file* files;

	//(1)listen on control master_socket
	masterSocket = listenOnPort(CONTROL_PORT);

	while(1){
		//(2)accept new connection form master_socket
		puts("\n------------(1)wait for connection--------------");
		connectedMasterSocket  = acceptConnection(masterSocket);

		//(3)get port Number from client
		puts("\n------------(2)get number of files--------------");
		numberOfFiles = getIntFromClient(connectedMasterSocket,"numberOfFiles");

		//(4)listen to socket on random free ports
		puts("\n------------(3)listen on random ports--------------");
		filesSocketsArr  = listenOnSocketWithFreePorts(numberOfFiles);

		//(5)get filesSockets port
		puts("\n------------(4)random ports--------------");
		freePortsArr = getPortNumberFromSockets(filesSocketsArr, numberOfFiles);

		//(6)send to client the free ports
		puts("\n------------(5)send free ports to client--------------");
		SendIntArrayToClient(connectedMasterSocket,freePortsArr, numberOfFiles ,"freePortsArr");

		//(7)accept connection from each of the socket in filesSocketsArr
		puts("\n------------(6)accept connection from new sockets--------------");
		connectedFilesSocketsArr = accept_connection_from_array_of_sockets(filesSocketsArr, numberOfFiles, "filesSocketsArr");

		//(8)get file from each of the socket in connectedFilesSocketsArr
		puts("\n------------(7)get files--------------");
		files = get_files_from_array_of_sockets(connectedFilesSocketsArr, numberOfFiles);

		//(8)get file from each of the socket in connectedFilesSocketsArr
		puts("\n------------(8)save files--------------");
		writeFiles(files, numberOfFiles);

		//(9)close master_socket, client_socket and all the socket in filesSockets
		puts("\n------------(9)close sockets--------------");
		closeSocket(connectedMasterSocket);
		closeSockets(filesSocketsArr, numberOfFiles, "filesSocketsArr");
		closeSockets(connectedFilesSocketsArr, numberOfFiles, "connectedFilesSocketsArr");

		//(10)free filesSocketsArr and freePortsArr
		puts("\n------------(10)free memory--------------");
		myFree(connectedFilesSocketsArr,"connectedFilesSocketsArr");
		myFree(filesSocketsArr,"filesSocketsArr");
		myFree(freePortsArr,"freePortsArr");
		freeFiles(files, numberOfFiles,"files");
	}
	closeSocket(masterSocket);

	puts("server close");
}

/********************************************************
 * main
 ********************************************************/
int main(int argc , char *argv[])
{
	server1();
	//server2();

	return 0;
}

