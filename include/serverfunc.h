/*************************************************************************
  > File Name: /home/liruicheng/miniFTP/include/serverfunc.h
  > Author: Reacky
  > Mail:327763224@qq.com 
  > Created Time: Fri 03 Apr 2015 08:54:27 PM CST
 ************************************************************************/

#ifndef __SERVER_FUNC_H__
#define __SERVER_FUNC_H__

#include<dirent.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>

typedef struct _msg
{
	int _msgLen;
	char _msg[1024];
}Msg;

static Msg msgBuf;

/********************* File IO *******************************/

unsigned long memSendFile(int sockFd, int fileFd, unsigned long fileSize, Msg msgBuf)
{
	char *mapFile;

	mapFile = (char *)mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fileFd, 0);

	/* send package */

	unsigned long readFileSize;
	unsigned long totalSendFileSize = 0;

	while(memset(&msgBuf, 0, sizeof(msgBuf)), totalSendFileSize < fileSize){

		readFileSize = 0;
		while(readFileSize < 1024 && totalSendFileSize < fileSize){

			*(msgBuf._msg + readFileSize) = *(mapFile + totalSendFileSize);
			++totalSendFileSize;
			++readFileSize;
		}

		msgBuf._msgLen = readFileSize;//	if(ifdir)
		send(sockFd, &msgBuf, sizeof(msgBuf), 0);
	}

	munmap(mapFile, fileSize);

	return totalSendFileSize;
}

int sendEnd(int sockFd, Msg msgBuf)
{
	memset(&msgBuf, 0, sizeof(msgBuf));

	strcpy(msgBuf._msg, "end");
	msgBuf._msgLen = strlen(msgBuf._msg);

	send(sockFd, &msgBuf, sizeof(msgBuf), 0);

	return 0;
}

unsigned long recvFile(int sockFd, char* filePath, unsigned long fileSize, Msg msgBuf)
{
	unsigned long recvFileSize = 0;
	unsigned long writeFileSize;

	int cliFileFd = open(filePath, O_WRONLY | O_CREAT);

	while(recvFileSize < fileSize){

		memset(&msgBuf, 0, sizeof(msgBuf));
		recv(sockFd, &msgBuf, sizeof(msgBuf), 0);

		recvFileSize += msgBuf._msgLen;
		write(cliFileFd, msgBuf._msg, msgBuf._msgLen);

	}

	close(cliFileFd);
}

/********************* File IO *******************************/

/* server command */

void Ls(int sockFd)
{
	/* get paht */

	char path[128];
	memset(path, 0, sizeof(path));

	strcpy(path, getcwd(NULL, 0));

	/* go for all dirent */

	DIR *pDIR;
	struct dirent *dirInfo;
	struct stat fileStat;

	if(NULL == (pDIR = opendir(path))){

		perror("opendir");
	}

	chdir(path);

	while(memset(&msgBuf, 0, sizeof(msgBuf)), (NULL != (dirInfo = readdir(pDIR)))){

		if(strcmp(dirInfo -> d_name, ".") == 0 || strcmp(dirInfo -> d_name, "..") == 0)
			continue;

		stat(dirInfo -> d_name, &fileStat);

		sprintf(msgBuf._msg, "%s %lu %c", dirInfo -> d_name, fileStat.st_size, dirInfo -> d_type);
		msgBuf._msgLen = strlen(msgBuf._msg);

		send(sockFd, &msgBuf, sizeof(msgBuf), 0);
	}

	/* when the end of the dirent */

	sendEnd(sockFd, msgBuf);
}

void Pwd(int sockFd)
{
	memset(&msgBuf, 0, sizeof(msgBuf));
	strcpy(msgBuf._msg, getcwd(NULL, 0));
	msgBuf._msgLen = strlen(msgBuf._msg);

	send(sockFd, &msgBuf, sizeof(msgBuf), 0);

}

void Cd(int sockFd, char *para)
{
	if(strncmp(para, "..", 2) == 0)

		chdir("..");
	else

		chdir(para);
}

int GetFiles(int sockFd, char *para, int flag)
{
	/* get file path */

	char filePath[128];
	memset(filePath, 0, sizeof(filePath));

	sprintf(filePath, "%s/%s", getcwd(NULL, 0), para);
	printf("%s\n", filePath);

	/* if is a dirent */

	struct stat fileStat;
	unsigned long FileSize;
	int FileFd;

	stat(filePath, &fileStat);

	if(!S_ISDIR(fileStat.st_mode) && flag){

		/* send file information*/

		sprintf(msgBuf._msg, "%s %lu %d", para, fileStat.st_size, 0);
		msgBuf._msgLen = strlen(msgBuf._msg);
		send(sockFd, &msgBuf, sizeof(msgBuf), 0);
		
		/* send file */

		FileSize = fileStat.st_size;
		FileFd = open(filePath, O_RDONLY);
		memSendFile(sockFd, FileFd, FileSize, msgBuf);
		close(FileFd);

		sendEnd(sockFd, msgBuf);

		return 0;

	}else if(flag){
		
		sprintf(msgBuf._msg, "%s %lu %d", para, fileStat.st_size, 1);
		msgBuf._msgLen = strlen(msgBuf._msg);
		send(sockFd, &msgBuf, sizeof(msgBuf), 0);
	}

	/* open dirent and send all dirent */

	DIR *pDIR;
	struct dirent *dirInfo;

	if(NULL == (pDIR = opendir(filePath))){

		perror("open dir");
		return -1;
	}

	chdir(filePath);

	while(memset(&msgBuf, 0, sizeof(msgBuf)), (NULL != (dirInfo = readdir(pDIR)))){

		if(strcmp(dirInfo -> d_name, ".") == 0 || strcmp(dirInfo -> d_name, "..") == 0)
			continue;

		stat(dirInfo -> d_name, &fileStat);

		memset(filePath, 0, sizeof(filePath));
		sprintf(filePath, "%s/%s", getcwd(NULL, 0), dirInfo -> d_name);

		if(dirInfo -> d_type & DT_DIR){

			sprintf(msgBuf._msg, "%s %lu %d", dirInfo -> d_name, fileStat.st_size, 1);

			msgBuf._msgLen = strlen(msgBuf._msg);
			send(sockFd, &msgBuf, sizeof(msgBuf), 0);

			GetFiles(sockFd, dirInfo -> d_name, 0);
			chdir("..");
			
		}else{

			sprintf(msgBuf._msg, "%s %lu %d", dirInfo -> d_name, fileStat.st_size, 0);
			send(sockFd, &msgBuf, sizeof(msgBuf), 0);

			FileFd = open(filePath, O_RDONLY);
			FileSize = fileStat.st_size;

			memSendFile(sockFd, FileFd, FileSize, msgBuf);
			close(FileFd);
		}
	}

	if(flag)
		chdir("..");

	/* when the end of the dirent */

	sendEnd(sockFd, msgBuf);

	return 0;
}

void PutFiles(int sockFd, int firstDownFlag)
{
	/* first download and get file path or default */

	char filePath[128];
	char fileName[128];
	unsigned long fileSize;

	memset(filePath, 0, sizeof(filePath));

	/* if file is a dirent */
	int isDir;

	if(firstDownFlag){

		recv(sockFd, &msgBuf, sizeof(msgBuf), 0);
		sscanf(msgBuf._msg, "%s %lu %d", fileName, &fileSize, &isDir);
		sprintf(filePath, "%s/%s", getcwd(NULL, 0), fileName);
		printf("name: %s, size: %lu\n", fileName, fileSize);

		if(!isDir){
	
			recvFile(sockFd, filePath, fileSize, msgBuf);
			return;

		}else{

			mkdir(filePath, 0766);
			chdir(filePath);
		}
	}

	/* if a dirent */

	while(memset(&msgBuf, 0, sizeof(msgBuf)),recv(sockFd, &msgBuf, sizeof(msgBuf), 0)){

		if(0 == strncmp(msgBuf._msg, "end", 3))
			break;

		sscanf(msgBuf._msg, "%s %lu %d", fileName, &fileSize, &isDir);

		sprintf(filePath, "%s/%s", getcwd(NULL, 0), fileName);

		printf("path: %s\n", filePath);

		if(isDir){

			mkdir(filePath, 0766);
			chdir(filePath);
			PutFiles(sockFd, 0);
			chdir("..");

		}else{

			recvFile(sockFd, filePath, fileSize, msgBuf);

		}
	}

}

/*********************** handle command **************************/

void handleCommand(char *command, char *para, int sockFd)
{
	if(strcmp(command, "ls") == 0){

		Ls(sockFd);

	}else if(strcmp(command, "pwd") == 0){

		Pwd(sockFd);

	}else if(strcmp(command, "cd") == 0){

		Cd(sockFd, para);

	}else if(strcmp(command, "gets") == 0){

		GetFiles(sockFd, para, 1);

	}else if(strcmp(command, "puts") == 0){

		PutFiles(sockFd, 1);

	}else if(strcmp(command, "remove") == 0){

		//	Remove(sockFd, para);
	}

}



#endif
