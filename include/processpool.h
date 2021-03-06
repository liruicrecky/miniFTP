/*************************************************************************
	> File Name: /home/liruicheng/miniFTP/include/processpool.h
	> Author: Reacky
	> Mail:327763224@qq.com 
	> Created Time: Fri 03 Apr 2015 05:33:50 PM CST
 ************************************************************************/

#ifndef __PROCESS_POOL_H__
#define __PROCESS_POOL_H__

#include"serverfunc.h"

#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<wait.h>
#include<signal.h>
#include<sys/socket.h>
#include<sys/types.h>

#define FREE 0
#define BUSY 1

#define DONE 1
#define UNDONE 0

#define true 1
#define false 0

typedef struct Child
{
	int _socketFd;
	pid_t _childPid;
	int _stat;
	int _done;

}CHILD, *pCHILD;


void handleRequest(int sockFd);
void recvFd(int pairFd, int *fd);
void setNoneBlock(int sfd);
static void recvFork(int sigNum);


int makeChild(pCHILD pChild, int childNum)
{
	int cnt;
	int index = 0;
	int clientFd;

	for(cnt = 0;cnt != childNum;++cnt){

		int sockPair[2];
		if(-1 == socketpair(AF_LOCAL, SOCK_STREAM, 0, sockPair)){

			perror("sockpair");
			exit(EXIT_FAILURE);
		}

		pid_t childPid = fork();

		if(0 == childPid){

			close(sockPair[1]);
			while(1){

				recvFd(sockPair[0], &clientFd);
				handleRequest(clientFd);
				write(sockPair[0], "done", 4);
				printf("A client offline!!\n");

				/*
				 * return 0 means to exit the process and recv the sources of the
				 * process when task is done
				 * note the return 0 means to reuse the process when task is done
				 */

			//	return 0;
			}
		}
		
		close(sockPair[0]);
		setNoneBlock(sockPair[1]);
		pChild[index]._socketFd = sockPair[1];
		pChild[index]._childPid = childPid;
		pChild[index]._stat = FREE;
		pChild[index]._done = UNDONE;
		++index;
	}

	return 1;
}

void handleRequest(int sockFd)
{
	char buf[1024];

	memset(buf, 0, sizeof(buf));
	strcpy(buf, "---------------Welcome to the server!---------------");
	send(sockFd, buf, strlen(buf), 0);

	char command[10], para[128];

	while(memset(buf, 0, sizeof(buf)), 0 != read(sockFd, buf, 1024)){

		memset(command, 0, sizeof(command));
		memset(para, 0, sizeof(para));

		sscanf(buf, "%s %s", command, para);
		printf("%s %s\n", command, para);

		handleCommand(command, para, sockFd);

		printf("client :%s\n", buf);
	}

	close(sockFd);
}

void sendFd(int pairFd, int sockFd)
{
	struct msghdr _msg;

	memset(&_msg, 0, sizeof(_msg));

	_msg.msg_name = NULL;
	_msg.msg_namelen = 0;

	struct iovec _iove[1];

	char sendBuf[1024];
	memset(sendBuf, 0, sizeof(sendBuf));

	_iove[0].iov_base = &sendBuf;
	_iove[0].iov_len = sizeof(sendBuf);

	_msg.msg_iov = _iove;
	_msg.msg_iovlen = 1;
	_msg.msg_flags = 0;

	int cmsgLen = CMSG_LEN(sizeof(int));

	struct cmsghdr *pCmsg;

	pCmsg = (struct cmsghdr *)calloc(1, sizeof(struct cmsghdr));

	pCmsg -> cmsg_len = cmsgLen;

	pCmsg -> cmsg_level = SOL_SOCKET;
	pCmsg -> cmsg_type = SCM_RIGHTS;

	*(int *)CMSG_DATA(pCmsg) = sockFd;

	_msg.msg_control = pCmsg;
	_msg.msg_controllen = cmsgLen;

	sendmsg(pairFd, &_msg, 0);
}

void recvFd(int pairFd, int *fd)
{
	struct msghdr _msg;
	
	memset(&_msg, 0, sizeof(_msg));

	_msg.msg_name = NULL;
	_msg.msg_namelen = 0;

	struct iovec _iove[1];

	char recvBuf[1024];
	memset(recvBuf, 0, sizeof(recvBuf));

	_iove[0].iov_base = &recvBuf;
	_iove[0].iov_len = sizeof(recvBuf);

	_msg.msg_iov = _iove;
	_msg.msg_iovlen = 1;
	_msg.msg_flags = 0;

	int cmsgLen = CMSG_LEN(sizeof(int));

	struct cmsghdr _cmsg;
	memset(&_cmsg, 0, sizeof(_cmsg));

	_cmsg.cmsg_len = cmsgLen;
	_cmsg.cmsg_level = SOL_SOCKET;
	_cmsg.cmsg_type = SCM_RIGHTS;

	_msg.msg_control = &_cmsg;
	_msg.msg_controllen = cmsgLen;

	recvmsg(pairFd, &_msg, 0);
	
	*fd = *(int *)CMSG_DATA(&_cmsg);
}

static void recvFork(int sigNum)
{
	pid_t pid;
	pid = waitpid(-1, NULL, WNOHANG);

	printf("signal: %d, %d process recv success!\n", sigNum, pid);
}

void setNoneBlock(int sfd)
{
	int fileFl = fcntl(sfd, F_GETFL);
	fileFl = fileFl | O_NONBLOCK;
	fcntl(sfd, F_SETFL, fileFl);
}

#endif
