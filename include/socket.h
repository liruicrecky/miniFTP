/*************************************************************************
	> File Name: /home/liruicheng/miniFTP/include/socket.h
	> Author: Reacky
	> Mail:327763224@qq.com 
	> Created Time: Fri 03 Apr 2015 05:02:29 PM CST
 ************************************************************************/

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int initSocket()
{
	/*
	 *  init socket 
	 */

	int _socket = socket(AF_INET, SOCK_STREAM, 0);
	
	if(-1 == _socket){

		perror("socket");
		return -1;
	}

	printf("-----Init Socket success\n");

	return _socket;
}

struct sockaddr_in initAddr(char *ip, int port)
{
	/*
	 *  init sockaddr
	 */

	struct sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(port);
	sockAddr.sin_addr.s_addr = inet_addr(ip);

	return sockAddr;
}

int initBind(int socket, struct sockaddr_in sockAddr)
{
	/*
	 *  init bind
	 */

	if(-1 == bind(socket, (struct sockaddr *)&sockAddr, sizeof(struct sockaddr))){

		perror("bind");
		return -1;
	}

	printf("-----Init Bind success\n");
	return 0;
}

int initListen(int socket, int num)
{
	/*
	 *  init listen
	 */

	if(-1 == listen(socket, num)){
			
		perror("listen");
		return -1;
	}

	printf("-----Init Listen sucess\n");
	return 0;
}

int initConnect(int socket, struct sockaddr_in sockAddr)
{
	/*
	 *  init client connect
	 */

	if(-1 == connect(socket, (struct sockaddr *)&sockAddr, sizeof(struct sockaddr))){

		perror("connect");
		return -1;
	}

	printf("-----Init Connect sucess\n");
	return 0;
}



#endif


