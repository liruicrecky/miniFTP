/*************************************************************************
  > File Name: /home/liruicheng/miniFTP/include/serverfunc.h
  > Author: Reacky
  > Mail:327763224@qq.com 
  > Created Time: Fri 03 Apr 2015 08:54:27 PM CST
 ************************************************************************/

#ifndef __SERVER_FUNC_H__
#define __SERVER_FUNC_H__

#include"fileoper.h"

#include<dirent.h>

static Msg msgBuf;
static struct stat fileInfo;

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

void Cd(char *para)
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

	int FileFd;

	/* init file information */

	if(-1 == stat(filePath, &fileInfo)){

		printf("init file information failed\n");
		return 0;
	}

	/* if is a dirent */

	if(!S_ISDIR(fileInfo.st_mode) && flag){

		/* send file information*/

		sprintf(msgBuf._msg, "%s %lu %d", para, fileInfo.st_size, 0);
		msgBuf._msgLen = strlen(msgBuf._msg);
		send(sockFd, &msgBuf, sizeof(msgBuf), 0);

		/* recv from client if the file already exist */

		unsigned long fileOffset;
		recv(sockFd, &msgBuf, sizeof(msgBuf), 0);
		if(msgBuf._msgLen != 0){

			fileOffset = msgBuf._msgLen;
			printf("%lu\n", fileOffset);

		}else{

			fileOffset = 0;
		}

		/* send file */

		FileFd = open(filePath, O_RDONLY);
		memSendFile(sockFd, FileFd, fileOffset, fileInfo.st_size, msgBuf);
		close(FileFd);

		sendEnd(sockFd, msgBuf);

		return 0;

	}else if(flag){
		
		sprintf(msgBuf._msg, "%s %lu %d", para, fileInfo.st_size, 1);
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

		memset(filePath, 0, sizeof(filePath));
		sprintf(filePath, "%s/%s", getcwd(NULL, 0), dirInfo -> d_name);

		stat(dirInfo -> d_name, &fileInfo);

		if(S_ISDIR(fileInfo.st_mode)){

			sprintf(msgBuf._msg, "%s %lu %d", dirInfo -> d_name, fileInfo.st_size, 1);

			msgBuf._msgLen = strlen(msgBuf._msg);
			send(sockFd, &msgBuf, sizeof(msgBuf), 0);

			GetFiles(sockFd, dirInfo -> d_name, 0);
			chdir("..");
			
		}else{

			sprintf(msgBuf._msg, "%s %lu %d", dirInfo -> d_name, fileInfo.st_size, 0);
			send(sockFd, &msgBuf, sizeof(msgBuf), 0);

			FileFd = open(filePath, O_RDONLY);

			memSendFile(sockFd, FileFd, 0, fileInfo.st_size, msgBuf);
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
	
			recvFile(sockFd, filePath, fileSize, msgBuf, 0);
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

			recvFile(sockFd, filePath, fileSize, msgBuf, 0);

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

		Cd(para);

	}else if(strcmp(command, "gets") == 0){

		GetFiles(sockFd, para, 1);

	}else if(strcmp(command, "puts") == 0){

		PutFiles(sockFd, 1);

	}else if(strcmp(command, "remove") == 0){

		//	Remove(sockFd, para);
	}

}



#endif
