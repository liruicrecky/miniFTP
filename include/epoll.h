/*************************************************************************
	> File Name: /home/liruicheng/miniFTP/include/epoll.h
	> Author: Reacky
	> Mail:327763224@qq.com 
	> Created Time: Fri 03 Apr 2015 09:41:16 PM CST
 ************************************************************************/

#ifndef __EPOLL_H__
#define __EPOLL_H__

#include<sys/epoll.h>

#include"processpool.h"


int initEpoll(int num)
{
	int _epoll = epoll_create(num);

	if(-1 == _epoll){

		perror("epoll create");
		return -1;
	}

	return _epoll;
}

int epollAdd(int epollFd, int fd, int mode)
{
	struct epoll_event epollEvent;	
	memset(&epollEvent, 0, sizeof(epollEvent));

	epollEvent.events = mode;
	epollEvent.data.fd = fd;

	if(-1 == epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &epollEvent)){

		perror("epoll add");
		return -1;
	}
	
	return 0;
}

void epollLoop(int epollFd, int fd, int processNum, pCHILD pChild)
{
	int readable, cnt, index;
	struct epoll_event readableEpollEvents[1024];
	struct sockaddr_in cliAddr;
	int cliAccept, i;
	socklen_t cliAddrLen = sizeof(struct sockaddr_in);

	memset(readableEpollEvents, 0, sizeof(readableEpollEvents));
	readable = epoll_wait(epollFd, readableEpollEvents, 1024, -1);

	if(-1 == readable)
		return;

	for(cnt = 0;cnt != readable;++cnt){

		if(readableEpollEvents[cnt].data.fd == fd){

			memset(&cliAddr, 0, sizeof(cliAddr));
			cliAccept = accept(readableEpollEvents[cnt].data.fd, (struct sockaddr *)&cliAddr, &cliAddrLen);
			printf("A new client %s:%d online\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));

			for(index = 0;index != processNum;++index){

				if(pChild[index]._stat == FREE)
					break;
			}
			if(index == processNum){

				char msg[50];
				strcpy(msg, "Server is full! try again later!");
				printf("full client!\n");
				write(cliAccept, msg, sizeof(msg));
				continue;
			}

			sendFd(pChild[index]._socketFd, cliAccept);
			pChild[index]._stat = BUSY;

		}else{

			/*
			 * when the task is done then reuse the process
			 * or want to exit the child process, to node these code below
			 */

			for(i = 0;i != processNum;++i){

				if(pChild[i]._socketFd == readableEpollEvents[cnt].data.fd)
					break;
			}

			pChild[i]._stat = FREE;
		}
	}
}

#endif
