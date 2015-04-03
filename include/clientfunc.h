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

void cliLs(int sockFd)
{
	char buf[1024];
	char fileName[128];
	unsigned long fileSize;
	unsigned char fileType;

	while(memset(buf, 0, sizeof(buf)), 0 != recv(sockFd, buf, 1024, 0)){

		if(0 == strncmp(buf, "end", 3))
			break;

		sscanf(buf, "%s %lu %c", fileName, &fileSize, &fileType);

		if(fileType & DT_DIR)
			printf("  \033[34m%-10s\033[0m	%-10u\n", fileName, fileSize);
		else if(fileType & DT_FIFO) 
			printf("  \033[33m%-10s\033[0m	%-10u\n", fileName, fileSize);
		else
			printf("  %-10s	%-10u\n", fileName, fileSize);
	}
}


#endif
