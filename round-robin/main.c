/*
Leonardo Trote Martins				1620572		
Ricardo Matheus de Oliveira Amaral		1621644	
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include "fila.h"

// variáveis globais
#define TIME_MAX 5
int receivedSignal;
Queue* readyQueue;
Queue* loadQueue;
int freeCPU = 1;
Program* runningProgram;

//funções auxiliares
char * int2str(int number){
	char * str = (char*) malloc (6 * sizeof(char));
	sprintf(str, "%d", number);
	return str;
}

//remove parte da string
char *strremove(char *str, const char *sub) {
	char *p, *q, *r;
	if ((q = r = strstr(str, sub)) != NULL) {
    	size_t len = strlen(sub);
    	while ((r = strstr(p = r + len, sub)) != NULL) {
        	while (p < r)
            	*q++ = *p++;
    	}
    	while ((*q++ = *p++) != '\0')
        	continue;
	}
	return str;
}

//função auxiliar para verificar se o programa enviou sinal de I/O.
void handler(int signal){
	if (signal == SIGUSR1)
		receivedSignal = 1;
	
}
//criando o programa
int newProgram(char* ProgramName){
    int pid, pidParent;
	char path[50] = "./";
	receivedSignal = 0;
    pidParent = getpid();
    pid = fork();
	
    if (pid < 0){
		printf("Erro ao criar o programa (Filho)\n");
		exit(1);
	}
	else if (pid == 0){
		char* arg[2];
		arg[0] = int2str(pidParent);
		arg[1] = NULL;
		strcat(path,ProgramName);
		if(execv(path, arg) < 0) // Tenta executar o programa
    			printf("Erro ao executar %s.\n", ProgramName);
		
	}
	else{
		signal(SIGUSR1, handler);
		sleep(1);
		kill(pid, SIGSTOP); // PAUSA O PROGRAMA	
	}
    return pid;
   }


void atualizaLoadQueue(){
	Program *p = loadQueue->first;
	while (p != NULL){
		p->waitingTime++;
		p = p->next;
	}
	if (loadQueue->first->waitingTime == 3){
		loadQueue->first->status = 1;
		printf("O programa %s (- Pid = %d) entrou na fila de pronto pois passou o tempo de I/O (respeitando a prioridade) \n", loadQueue->first->name, loadQueue->first->pid);
		queue_insertP(readyQueue,program_copy(queue_pop(loadQueue)));
	}
}

void atualizaQueue(){
	if (!queue_isEmpty(readyQueue)){
		runningProgram = queue_pop(readyQueue);
		kill(runningProgram->pid, SIGCONT); // PROGRAMA EM EXECUÇÃO
		printf("\nO programa %s está sendo executado (SIGCONT) pid = %d\n",runningProgram->name, runningProgram->pid);
		runningProgram->status = 2;
		
	}
	printf("Exibindo a fila de produção\n");
	queue_show(readyQueue);

}

//inicia round-robin
void startRoundRobin(){
	printf("Exibindo a fila de produção\n");
	queue_show(readyQueue);
	int qtd = readyQueue->lenght;
	atualizaQueue();
	receivedSignal = 0;
	 
	//enquanto houver programa para executar
	while (qtd > 0){
		sleep(1);
		
		
		if (loadQueue->lenght > 0) atualizaLoadQueue(); // Atualizando a fila de espera
		if (runningProgram != NULL){
			runningProgram->runningTime++;
			if (runningProgram->runningTime >= TIME_MAX){ // Verifica se o tempo máximo de um programa foi atendido
				printf("O prog %s terminou\n",runningProgram->name);
				kill(runningProgram->pid, SIGKILL);
				qtd--;
				free(runningProgram);
				runningProgram = NULL;
			}
			else{

				kill(runningProgram->pid, SIGSTOP);
				printf("O programa %s foi pausado (SIGSTOP) Pid = %d\n", runningProgram->name, runningProgram->pid);
				
				if (runningProgram->isIO){ // Verifica se o programa tem I/O
					printf("O programa %s (- Pid = %d) saiu da fila de pronto pois recebeu um sinal de I/O \n", runningProgram->name, runningProgram->pid);
					//runningProgram->isIO = 0; 
					queue_insert(loadQueue, program_copy(runningProgram));
				}
				else { // Programa sem I/O
					queue_insert(readyQueue,program_copy(runningProgram));
				}
				printf("Exibindo a fila de produção\n");
				queue_show(readyQueue);
			}
		}
			
		if (qtd > 0){ //Verifica se ainda existe programa para serem executados
			atualizaQueue();
		}
		
	}

}

int main(void){

	///interpretador
	FILE *file;
	size_t len = 0;
	ssize_t read;
	char *programa;
	int i = 0;
	Info * programas;

	//abre arquivo
	file = fopen("exec.txt", "r");
	if (file == NULL) {
		 printf("Erro abrindo o arquivo\n");
		 exit(1);    	 
	}
	 

	programas = (Info *) malloc (10*sizeof(Info));

	// leitura do arquivo linha a linha
	while ((read = getline(&programa, &len, file)) != -1) {
		programa = strremove(programa,"Run <");
		programa = strremove(programa,">");
		programa = strremove(programa,"\n");
		
		strcpy(programas[i].nome,programa);
		i++;
		
	sleep(1);
	}
	fclose(file);

	//teste
	i=0;
	readyQueue = queue_create();
	loadQueue = queue_create();
	Program* p;
	int pid;
	

	printf("Nome dos programas que serão executados\n");
	while (strlen(programas[i].nome) != 0) {
		pid = newProgram(programas[i].nome);
		printf("Nome do programa: %s - pid: %d\n",programas[i].nome, pid);
		p = program_create(programas[i].nome, pid, receivedSignal); 
		queue_insert(readyQueue, p);
		i++;
	}
	
	printf("\nIniciando o escalonamento Round Robin\n");
	startRoundRobin();
	free(programas);
	queue_free(readyQueue);
	queue_free(loadQueue);
}
