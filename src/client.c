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
	if(-1 == initConnect(cliSocket, initAddr(argv[1], atoi(argv[2])))){

		close(cliSocket);
		exit(EXIT_FAILURE);
	}

	memset(buf, 0, sizeof(buf));
	recv(cliSocket, buf, 1024, 0);
	printf("%s\n", buf);
	if(buf[0] == 'S'){

		close(cliSocket);
		exit(EXIT_FAILURE);
	}

	/* start */

	while(memset(buf, 0, sizeof(buf)), printf("#"), NULL != fgets(buf, 1024, stdin)){

		buf[strlen(buf) - 1] = '\0';
		/* handle command */

		/* command ls */
		if(strcmp(buf, "ls") == 0){

			send(cliSocket, buf, strlen(buf), 0);
			cliLs(cliSocket);
		}

		/* command pwd */
		else if(strcmp(buf, "pwd") == 0){

			send(cliSocket, buf, strlen(buf), 0);
			cliPwd(cliSocket);
		}

		/* command cd */
		else if(strncmp(buf, "cd", 2) == 0 && buf[2] == ' '){

			send(cliSocket, buf, strlen(buf), 0);
		}

		/* command getfiles */
		else if(strncmp(buf, "gets", 4) == 0 && buf[4] == ' '){

			send(cliSocket, buf, strlen(buf), 0);
			cliGetFiles(cliSocket, 1);
		}

		/* command putsfiles */
		else if(strncmp(buf, "puts", 4) == 0 && buf[4] == ' '){

			send(cliSocket, buf, strlen(buf), 0);
			cliPutFiles(cliSocket, buf, 1);
		}

		/* command remove */
		else if(strncmp(buf, "remove", 6) == 0 && buf[6] == ' '){

			send(cliSocket, buf, strlen(buf), 0);
			cliRemoveFiles(cliSocket);
		}

		/* others */
		else{

			printf("wrong command\n");
		}

	}

	close(cliSocket);

	return 0;
}
