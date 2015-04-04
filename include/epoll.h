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

int epollAdd(int epollFd, int fd)
{
	struct epoll_event epollEvent;	
	memset(&epollEvent, 0, sizeof(epollEvent));

	epollEvent.events = EPOLLIN;
	epollEvent.data.fd = fd;

	if(-1 == epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &epollEvent)){

		perror("epoll add");
		return -1;
	}
	
	return 0;
}

int epollRemove(int epollFd, int fd)
{


}

void epollLoop(int epollFd, int fd, int processNum, pCHILD pChild)
{
	int readable, cnt, index;
	struct epoll_event epollEvent, readableEpollEvents[1024];
	int cliAccept;

	memset(readableEpollEvents, 0, sizeof(readableEpollEvents));
	readable = epoll_wait(epollFd, readableEpollEvents, 1024, -1);

	if(-1 == readable)
		return;

	printf("%d\n", readable);

	for(cnt = 0;cnt != readable;++cnt){

		if(readableEpollEvents[cnt].data.fd == fd){

			cliAccept = accept(readableEpollEvents[cnt].data.fd, NULL, NULL);
			printf("A new client online\n");

			for(index = 0;index != processNum;++index){

				if(pChild[index]._stat == FREE)
					break;
			}
			if(index == processNum){

				printf("full client!\n");
				continue;
			}

			sendFd(pChild[index]._socketFd, cliAccept);
			pChild[index]._stat = BUSY;
		}
	}
}

#endif
