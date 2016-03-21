/*
 ============================================================================
 Name        : client2.c
 Author      : dan
 Version     : 1
 Copyright   : Your copyright notice
 Description : client bla bla
 ============================================================================
 */

#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>    	   	//strlen
#include <sys/socket.h>    	//socket
#include <arpa/inet.h>    	//inet_addr
#include <unistd.h>			//close socket

#define CONTROL_PORT 50006
#define SERVER_IP "10.1.1.125"
#define FILE_NAME1 "files/file1.txt"
#define FILE_NAME2 "files/file2.txt"
#define FILE_NAME3 "files/file3.txt"

struct file{
	int fileNameSize;
	char *fileName;
	int fileContentSize;
	char *fileContent;
};

/********************************************************
 * myFree
 * do			(1)free var
 * 				(2)print var name
 ********************************************************/
void myFree(void* var, char* varName)
{
	printf("free %s\n",varName);
	free(var);
}

/********************************************************
 * intMalloc
 * do			(1)Allocates (size*int) memory to var
 * 				(2)print the name of the var
 ********************************************************/
void arrIntMalloc(int** var, int size, char* varName){
	printf("malloc %s\n",varName);
	*var =(int* )malloc(size*sizeof(int));
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
}

/********************************************************
 * mallocString
 ********************************************************/
void mallocString(char** fileName,int size,char* varName)
{
	printf("malloc %s\n",varName);
	*fileName =(char*)malloc(size*(sizeof(char)));
}

/********************************************************
 * freeFiles
 ********************************************************/
void freeFiles(struct file* files, int len ,char* varName)
{
	int i;

	printf("free files array:\n");
	for(i=0; i<len; i++ )
	{
		printf("(%d) file:\n",i);
		myFree(files[i].fileContent,"fileContent");
		//myFree(files[i].fileName,"fileName");
	}
	myFree(files,varName);
}

/********************************************************
 * getNumberFromUser
 * return:  number the user insert
 ********************************************************/
int getNumberFromUser()
{
	int number;
	puts("enter number of files");
	scanf("%d",&number);
	return number;
}

/********************************************************
 * connectToServer
 * return:  socket
 ********************************************************/
int connectToServer(char* serverIP ,int port)
{

	int new_sock;
	struct sockaddr_in server;

	server.sin_addr.s_addr = inet_addr(serverIP);
	server.sin_family = AF_INET;
	server.sin_port = htons( port );

	//Create socket
	if ((new_sock = socket(AF_INET , SOCK_STREAM , 0)) == -1)
	{
		perror("socket");
		return -1;
	}

	//Connect to remote server
	if (connect(new_sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect");
		return -1;
	}

	printf("connect to server socketId-%d, port-%d\n",new_sock, port);

	return new_sock;
}

/********************************************************
 * closeSocket
 ********************************************************/
void closeSocket(int sd)
{
	close(sd);
	printf("close socket-%d\n",sd);
}

/********************************************************
 * sendToServer
 ********************************************************/
void sendIntToServer(int sd, int msg,char* varName)
{
	int len = sizeof (int);
	printf("send %s=%d \n",varName,msg);
	//Send some data
	if( send(sd , &msg , len , 0) < 0)
	{
		perror("Send");
	}
}

/********************************************************
 * sendStringToServer
 ********************************************************/
void sendStringToServer(int sd,char* fileName,char* varName){
	int len = strlen (fileName)
			;
	printf("send %s=%s \n",varName,fileName);
	//Send some data
	if( send(sd , fileName , len , 0) < 0)
	{
		perror("Send");
	}
}

/********************************************************
 * sendToServer
 * return:		success - ports number
 * 				failed	- null
 ********************************************************/
int* getIntArray(int sd, int numOfFiles,  char* varName)
{
	int i, *msg;

	arrIntMalloc(&msg,numOfFiles,varName);
	//int  = (int*)(malloc(numOfFiles*sizeof(int)));

	printf("read int array from socket-%d\n",sd);

	//Receive a reply from the server
	if( read(sd , msg , sizeof(int)*numOfFiles) < 0)
	{
		perror("read failed");
		return NULL;
	}

	printf("%s:\n",varName);
	for (i=0; i<numOfFiles; i++)
		printf("(%d) %d \n",i,msg[i]);

	return msg;
}

/********************************************************
 * closeSockets
 * do:		get array of socketId and call closeSocket with each socekt in the array
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
 * MultiConnectionToServer
 ********************************************************/
int* multiConnectionToServer(char* serverIP ,int *ports, int len,char* varName)
{
	int *sdArr, i;

	arrIntMalloc(&sdArr, len, "sdArr");

	puts("multi connection:");
	for(i=0; i<len; i++)
	{
		printf("(%d) ",i);
		sdArr[i] = connectToServer(serverIP, ports[i]);
	}

	return sdArr;
}

/********************************************************
 * getFile
 ********************************************************/
void getFile(struct file* file, char* fileName)
{
	int fileNameSize, fileContentSize;
	char *fileContent;
	FILE* fp;

	fileNameSize=strlen(fileName);
	//mallocString(&fileName,fileNameSize,"fileName");

	//open file
	if ( (fp = fopen(fileName,"r")) == NULL ){
		perror("open");
		return;
	}
	printf("open file: %s \n", fileName);

	//get file length and file content
	fseek(fp,0,SEEK_END);
	fileContentSize = ftell(fp);

	//get file content
	mallocString(&fileContent,fileContentSize,"fileContent");
	fseek(fp, 0,SEEK_SET);
	fread(fileContent,1,fileContentSize, fp);
	fileContent[fileContentSize]='\0';

	//close file
	fclose(fp);
	printf("close file: %s \n", fileName);

	//set file
	file->fileNameSize = fileNameSize;
	file->fileName = fileName;
	file->fileContentSize = fileContentSize;
	file->fileContent = fileContent;


}

/********************************************************
 * getFiles
 ********************************************************/
struct file* getFiles(numOfFiles)
{
	int i;
	struct file* files;

	mallocFilesArr(&files, numOfFiles, "files");

	puts("get files:");

	//proxy !!!!
	switch(numOfFiles){

		case 1:
			i=0;
			getFile(&(files[i]),FILE_NAME1);
			printf("(%d) fileNameSize=%d, fileName=%s, fileContentSize=%d fileContent=%s, \n",i, files[i].fileNameSize, files[i].fileName, files[i].fileContentSize, files[i].fileContent);
			break;
		case 2:
			i=0;
			getFile(&(files[i]),FILE_NAME1);
			printf("(%d) fileNameSize=%d, fileName=%s, fileContentSize=%d fileContent=%s, \n",i, files[i].fileNameSize, files[i].fileName, files[i].fileContentSize, files[i].fileContent);
			i=1;
			getFile(&(files[i]),FILE_NAME2);
			printf("(%d) fileNameSize=%d, fileName=%s, fileContentSize=%d fileContent=%s, \n",i, files[i].fileNameSize, files[i].fileName, files[i].fileContentSize, files[i].fileContent);
			break;
		case 3:
			i=0;
			getFile(&(files[i]),FILE_NAME1);
			printf("(%d) fileNameSize=%d, fileName=%s, fileContentSize=%d fileContent=%s, \n",i, files[i].fileNameSize, files[i].fileName, files[i].fileContentSize, files[i].fileContent);
			i=1;
			getFile(&(files[i]),FILE_NAME2);
			printf("(%d) fileNameSize=%d, fileName=%s, fileContentSize=%d fileContent=%s, \n",i, files[i].fileNameSize, files[i].fileName, files[i].fileContentSize, files[i].fileContent);
			i=2;
			getFile(&(files[i]),FILE_NAME3);
			printf("(%d) fileNameSize=%d, fileName=%s, fileContentSize=%d fileContent=%s, \n",i, files[i].fileNameSize, files[i].fileName, files[i].fileContentSize, files[i].fileContent);
			break;
		default:
			for(i=0; i<numOfFiles; i++)
			{
				getFile(&(files[i]),FILE_NAME1);
				printf("(%d) fileNameSize=%d, fileName=%s, fileContentSize=%d fileContent=%s, \n",i, files[i].fileNameSize, files[i].fileName, files[i].fileContentSize, files[i].fileContent);
			}

	};


	return files;
}

/********************************************************
 * sendFiles
 ********************************************************/
void sendFiles(int* sdArr,int len, struct file* files)
{
	int i;

	puts("send files:");
	for(i=0; i<len; i++)
	{
		printf("(%d) file",i);
		sendIntToServer(sdArr[i],files[i].fileNameSize,"fileNameSize");
		sendStringToServer(sdArr[i],files[i].fileName,"fileName");
		sendIntToServer(sdArr[i],files[i].fileContentSize,"fileContentSize");
		sendStringToServer(sdArr[i],files[i].fileContent,"fileContent");
	}

}

/********************************************************
 * main
 * do:		connect to server
 ********************************************************/
int main(void)
{
	int control_socket, numOfFiles, *FreePorts,* sdArr;

	struct file* files;

	//(1)get number from the user
	numOfFiles = getNumberFromUser();
	puts("-------------------------");

	//(2)connect to server on SERVER_IP CONTROL_PORT
	control_socket = connectToServer(SERVER_IP, CONTROL_PORT);
	puts("-------------------------");

	//if connectToServer succeed
	if(control_socket!= -1)
	{
		//(3)send numOfFiles to server
		sendIntToServer(control_socket,numOfFiles,"numOfFiles");
		puts("-------------------------");

		//(4)read free ports form server - (error = return null)
		FreePorts = getIntArray(control_socket,numOfFiles, "FreePorts");
		puts("-------------------------");

		//(5)for each port in freePorts connect to server
		sdArr = multiConnectionToServer(SERVER_IP,FreePorts,numOfFiles,"sdArr");
		puts("-------------------------");

		//(6)get files
		files = getFiles(numOfFiles);
		puts("-------------------------");

		//(7)send files
		sendFiles(sdArr,numOfFiles, files);
		puts("-------------------------");
	}

	//(8)close control_socket free ports
	closeSocket(control_socket);
	closeSockets(sdArr, numOfFiles, "sdArr");
	puts("-------------------------");

	//(9)free: FreePorts
	myFree(FreePorts,"FreePorts");
	freeFiles(files,numOfFiles,"files");
	puts("-------------------------");

	puts("end client");
	return 0;
}

