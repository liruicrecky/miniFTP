/*************************************************************************
	> File Name: client.c
	> Author: Reacky
	> Mail:327763224@qq.com 
	> Created Time: Fri 03 Apr 2015 09:17:45 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"socket.h"
#include"clientfunc.h"

int main(int argc, char **argv)
{
	if(argc < 2){

		printf("argv error!\n");
		printf("try (*.o ip port) again!\n");
		exit(EXIT_FAILURE);
	}

	/* conntec to the server */

	char buf[1024];

	int cliSocket = initSocket();
	initConnect(cliSocket, initAddr(argv[1], atoi(argv[2])));

	memset(buf, 0, sizeof(buf));
	recv(cliSocket, buf, 1024, 0);
	printf("%s\n", buf);

	/* start */

	while(memset(buf, 0, sizeof(buf)), printf("#"), NULL != fgets(buf, 1024, stdin)){

		/* handle command */
		if(strncmp(buf, "ls", 2) == 0){

			send(cliSocket, buf, strlen(buf), 0);
			cliLs(cliSocket);
		}

	}

	close(cliSocket);

	return 0;
}
