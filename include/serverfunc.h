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
#include<sys/types.h>
#include<sys/stat.h>

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
	char sendBuf[1024];

	if(NULL == (pDIR = opendir(path))){

		perror("opendir");
	}

	chdir(path);

	while(memset(sendBuf, 0, sizeof(sendBuf)), (NULL != (dirInfo = readdir(pDIR)))){

		if(strcmp(dirInfo -> d_name, ",") == 0 || strcmp(dirInfo -> d_name, "..") == 0)
			continue;

		stat(dirInfo -> d_name, &fileStat);

		sprintf(sendBuf, "%s %lu %c", dirInfo -> d_name, fileStat.st_size, dirInfo -> d_type);

		send(sockFd, sendBuf, strlen(sendBuf), 0);
	}

	/* when the end of the dirent */

	memset(sendBuf, 0, sizeof(sendBuf));
	strcpy(sendBuf, "end");
	send(sockFd, sendBuf, strlen(sendBuf), 0);
}


/* handle command */

void handleCommand(char *command, char *para, int sockFd)
{
	if(strcmp(command, "ls") == 0){

		Ls(sockFd);

	}else if(strcmp(command, "pwd") == 0){

	//	Pwd(sockFd);

	}else if(strcmp(command, "cd") == 0){

	//	Cd(sockFd, para);

	}else if(strcmp(command, "gets") == 0){

	//	GetFiles(sockFd, para);

	}else if(strcmp(command, "puts") == 0){

	//	PutFiles(sockFd, para);

	}else if(strcmp(command, "remove") == 0){

	//	Remove(sockFd, para);
	}

}



#endif
