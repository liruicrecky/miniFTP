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

		buf[strlen(buf)] = 0;
		/* handle command */
		send(cliSocket, buf, strlen(buf) - 1, 0);

		printf("strlen: %li, sizeof: %zi\n", strlen(buf), sizeof(buf));

		/* command ls */
		if(strncmp(buf, "ls", 2) == 0){

			cliLs(cliSocket);
		}

		/* command pwd */
		else if(strncmp(buf, "pwd", 3) == 0){

			cliPwd(cliSocket);
		}

		/* command cd */
		else if(strncmp(buf, "cd", 2) == 0){

		}

		/* command getfiles */
		else if(strncmp(buf, "gets", 4) == 0){

			cliGetFiles(cliSocket, 1);
		}

		/* command putsfiles */
		else if(strncmp(buf, "puts", 4) == 0){

			cliPutFiles(cliSocket, buf, 1);
		}

		/* others */
		else{

			printf("wrong command\n");
		}
	}

	close(cliSocket);

	return 0;
}
