/*************************************************************************
	> File Name: ./include/fileoper.h
	> Author: Reacky
	> Mail:327763224@qq.com 
	> Created Time: Mon 06 Apr 2015 01:25:32 PM CST
 ************************************************************************/

#ifndef __FILE_OPER_H__
#define __FILE_OPER_H__

#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<sys/mman.h>

typedef struct _msg
{
	int _msgLen;
	char _msg[1024];
}Msg;

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

	int cliFileFd = open(filePath, O_WRONLY | O_CREAT);

	while(recvFileSize < fileSize){

		memset(&msgBuf, 0, sizeof(msgBuf));
		recv(sockFd, &msgBuf, sizeof(msgBuf), 0);

		recvFileSize += msgBuf._msgLen;
		write(cliFileFd, msgBuf._msg, msgBuf._msgLen);

	}

	close(cliFileFd);

	return recvFileSize;
}

#endif
