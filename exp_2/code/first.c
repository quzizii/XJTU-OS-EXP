#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

int flag = 0;
void interrupt_handler(int signo){
	if (signo == SIGINT){
		printf("\nReceived SIGINT\n");
		flag = 1;
	}
	else if (signo == SIGQUIT){
		printf("\nReceived SIGQUIT\n");
		flag = 2;
	}
	else if (signo == SIGUSR1 || signo == SIGUSR2){
		printf("\nReceived SIGUSR\n");
		flag = 3;
	}
	else
		printf("\nReceived SIGALRM\n");
		flag = 4;
}

void waiting(){
	sleep(5);
}

int main(){
	pid_t pid1 = -1, pid2 = -1;
	// signal(SIGINT, interrupt_handler);
	// signal(SIGQUIT, interrupt_handler);
	// signal(SIGALRM, interrupt_handler);
	alarm(5);
	while (pid1 == -1)
		pid1 = fork();
	if (pid1 > 0){
		while(pid2 == -1) 
			pid2 = fork();
		if(pid2 > 0){
			signal(SIGINT, interrupt_handler);
			signal(SIGQUIT, interrupt_handler);
			signal(SIGALRM, interrupt_handler);
			waiting();
			printf("\nSending SIGUSR to child process\n");
			kill(pid1, SIGUSR1);
			kill(pid2, SIGUSR2);
			wait(NULL);
			wait(NULL);
			printf("\nParent process is killed\n");
		}
		else{
			signal(SIGUSR2, interrupt_handler);
			signal(SIGINT, interrupt_handler);
			signal(SIGQUIT, interrupt_handler);
			signal(SIGALRM, interrupt_handler);
			pause();
			if(flag == 3)
				printf("\nChild process 2 is killed by parent\n");
			else
				printf("\nChild process 2 is killed\n");
			exit(0);
		}
	}
	else{
		signal(SIGUSR1, interrupt_handler);
		signal(SIGINT, interrupt_handler);
		signal(SIGQUIT, interrupt_handler);
		signal(SIGALRM, interrupt_handler);
		pause();
		if (flag == 3)
			printf("\nChild process 1 is killed by parent\n");
		else
			printf("\nChild process 1 is killed\n");	
		exit(0);
	}
	return 0;
}
