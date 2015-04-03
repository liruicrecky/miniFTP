/*************************************************************************
  > File Name: /home/liruicheng/miniFTP/include/clientfunc.h
  > Author: Reacky
  > Mail:327763224@qq.com 
  > Created Time: Fri 03 Apr 2015 09:23:31 PM CST
 ************************************************************************/

#ifndef __CLIENT_FUNC_H__
#define __CLIENT_FUNC_H__

#include<stdio.h>
#include<dirent.h>

typedef struct _msg
{
	int _msgLen;
	char _msg[1024];
}Msg;

static char buf[1024];
static char fileName[128];
static unsigned long fileSize;
static unsigned char fileType;
static Msg msgBuf;

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


#endif
