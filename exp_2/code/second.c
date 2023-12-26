#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

pid_t pid1, pid2;

int main(){
	int fd[2];
	char inpipe[10000];
	char c1 = '1', c2 = '2';
	pipe(fd);

	while((pid1 = fork()) == -1);
	if(pid1 == 0){
		close(fd[0]);
		lockf(fd[1], 1, 0);
		for(int i = 0;i < 200; i++){
			write(fd[1], &c1, 1);
		}
		sleep(5);
		lockf(fd[1], 0, 0);
		close(fd[1]);
		exit(0);
	}
	else{
		while((pid2 = fork()) == -1);
		if(pid2 == 0){
			close(fd[0]);
			lockf(fd[1], 1, 0);
			for(int i = 0; i < 200; i++){
				write(fd[1], &c2, 1);
			}
			sleep(5);
			lockf(fd[1], 0, 0);
			close(fd[1]);
			exit(0);
		}
		else{
			close(fd[1]);
			wait(0);
			wait(0);
			ssize_t bytesRead = read(fd[0], inpipe, 400);
			inpipe[bytesRead] = '\0';
			printf("%s\n", inpipe);
			close(fd[0]);
			exit(0);
		}
	}
	
	return 0;
}
