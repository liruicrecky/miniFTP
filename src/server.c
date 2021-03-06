/*************************************************************************
  > File Name: server.c
  > Author: Reacky
  > Mail:327763224@qq.com 
  > Created Time: Fri 03 Apr 2015 05:01:20 PM CST
 ************************************************************************/

#include"socket.h"
#include"epoll.h"
#include"processpool.h"

#include<signal.h>
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char **argv)
{
	/* recv child process */


	signal(SIGCHLD, recvFork);

	/* check argv */

	if(argc < 3){

		printf("argv error!\n");
		printf("try (*.o ip port processnum) to init\n");
		exit(EXIT_FAILURE);
	}

	char ip[32];
	int port, processNum;

	strcpy(ip, argv[1]);
	port = atoi(argv[2]);
	processNum = atoi(argv[3]);

	/* init server */

	int serSocket = initSocket();

	if(-1 == serSocket)
		exit(EXIT_FAILURE);

	if(-1 == initBind(serSocket,initAddr(ip, port))){

		close(serSocket);
		exit(EXIT_FAILURE);
	}

	if(-1 == initListen(serSocket, 10)){

		close(serSocket);
		exit(EXIT_FAILURE);
	}

	/* init epoll */

	int serEpoll = initEpoll(1024);

	if(-1 == serEpoll){

		close(serSocket);
		exit(EXIT_FAILURE);
	}

	/* add server socket to epoll */

	if(-1 == epollAdd(serEpoll, serSocket, EPOLLIN)){

		close(serSocket);
		close(serEpoll);
		exit(EXIT_FAILURE);
	}

	/* init process pool */

	int pid, i;
	pCHILD pChild = (pCHILD)calloc(1, sizeof(CHILD));
	pid = makeChild(pChild, processNum);

	/*
	 * add the local sock of the child process to the
	 * epoll in order to konw if the task is done
	 * or want to exit child process when task is done
	 * note the code below
	 *
	 */

	for(i = 0;i != processNum;++i){

		epollAdd(serEpoll, pChild[i]._socketFd, EPOLLIN | EPOLLET);
	}


	/* start listen to epoll */

	if(pid){

		while(true){

			epollLoop(serEpoll, serSocket, processNum, pChild);
		}

		close(serEpoll);
		close(serSocket);
	}


	return 0;
}

