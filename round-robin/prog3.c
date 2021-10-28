/*
Leonardo Trote Martins				1620572		
Ricardo Matheus de Oliveira Amaral		1621644	
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[]) {
	int i = 0, pidParent;
	pidParent = atoi(argv[0]);
    
	while(1) {
	
		kill(pidParent, SIGUSR1);

		i++;
		sleep(1);
	}

}
