/*************************************************************************
  > File Name: /home/liruicheng/miniFTP/include/clientfunc.h
  > Author: Reacky
  > Mail:327763224@qq.com 
  > Created Time: Fri 03 Apr 2015 09:23:31 PM CST
 ************************************************************************/

#ifndef __CLIENT_FUNC_H__
#define __CLIENT_FUNC_H__

#include<stdio.h>
#include<unistd.h>
#include<dirent.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/mman.h>

typedef struct _msg
{
	int _msgLen;
	char _msg[1024];

}Msg;

static char buf[1024];
static char fileName[128];
static char filePath[128];
static unsigned long fileSize;
static unsigned char fileType;
static Msg msgBuf;

/***************Client IO********************************/

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

/***************Client IO********************************/

/* client command */

void cliLs(int sockFd)
{
	while(memset(&msgBuf, 0, sizeof(msgBuf)), 0 != recv(sockFd, &msgBuf, sizeof(msgBuf), 0)){

		if(0 == strncmp(msgBuf._msg, "end", 3))
			break;

		sscanf(msgBuf._msg, "%s %lu %c", fileName, &fileSize, &fileType);

		if(fileType & DT_DIR)
			printf("  \033[34m%-10s\033[0m	%-10u\n", fileName, fileSize);
		else if(fileType & DT_FIFO) 
			printf("  \033[33m%-10s\033[0m	%-10u\n", fileName, fileSize);
		else
			printf("  %-10s	%-10u\n", fileName, fileSize);
	}
}

void cliPwd(int sockFd)
{
	memset(&msgBuf, 0, sizeof(msgBuf));
	recv(sockFd, &msgBuf, sizeof(msgBuf), 0);
	printf("server path: %s\n", msgBuf._msg);
}

void cliGetFiles(int sockFd, int firstDownFlag)
{

	/* first download and get file path or default */

	if(firstDownFlag){

		printf("Download path(use absolute path)(input 0 for current directory):");
		scanf("%s", filePath);

		if(filePath[0] != '0'){

			chdir(filePath);

		}else{

			memset(filePath, 0, sizeof(filePath));
			sprintf(filePath, "%s", getcwd(NULL, 0));
			printf("%s\n", filePath);

		}
	}

	/* if file is a dirent */
	int isDir;

	if(firstDownFlag){

		recv(sockFd, &msgBuf, sizeof(msgBuf), 0);
		sscanf(msgBuf._msg, "%s %lu %d", fileName, &fileSize, &isDir);
		sprintf(filePath, "%s/%s", getcwd(NULL, 0), fileName);

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
			cliGetFiles(sockFd, 0);
			chdir("..");

		}else{

			recvFile(sockFd, filePath, fileSize, msgBuf);

		}
	}
}

int cliPutFiles(int sockFd, char *buf, int flag)
{
	char filePath[128];
	char fileName[128];

	/* get file name */

	memset(fileName, 0, sizeof(fileName));
	sscanf(buf, "%*s %s",fileName);

	/* get file path */
	
	memset(filePath, 0, sizeof(filePath));

	sprintf(filePath, "%s/%s", getcwd(NULL, 0), fileName);
	printf("%s\n", filePath);

	/* if is a dirent */

	struct stat fileStat;
	unsigned long FileSize;
	int FileFd;

	stat(filePath, &fileStat);

	if(!S_ISDIR(fileStat.st_mode) && flag){

		/* send file information*/

		sprintf(msgBuf._msg, "%s %lu %d", fileName, fileStat.st_size, 0);
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
		
		sprintf(msgBuf._msg, "%s %lu %d", fileName, fileStat.st_size, 1);
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

			cliPutFiles(sockFd, dirInfo -> d_name, 0);
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


#endif
