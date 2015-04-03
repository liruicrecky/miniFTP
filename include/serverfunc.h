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

typedef struct _msg
{
	int _msgLen;
	char _msg[1024];
}Msg;

static Msg msgBuf;

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

	memset(&msgBuf, 0, sizeof(msgBuf));

	strcpy(msgBuf._msg, "end");
	msgBuf._msgLen = strlen(msgBuf._msg);

	send(sockFd, &msgBuf, sizeof(msgBuf), 0);
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

/* handle command */

void handleCommand(char *command, char *para, int sockFd)
{
	if(strcmp(command, "ls") == 0){

		Ls(sockFd);

	}else if(strcmp(command, "pwd") == 0){

		Pwd(sockFd);

	}else if(strcmp(command, "cd") == 0){

		Cd(sockFd, para);

	}else if(strcmp(command, "gets") == 0){

	//	GetFiles(sockFd, para);

	}else if(strcmp(command, "puts") == 0){

	//	PutFiles(sockFd, para);

	}else if(strcmp(command, "remove") == 0){

	//	Remove(sockFd, para);
	}

}



#endif
