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
int time = 0;

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
	runningProgram = queue_pop(readyQueue);
	kill(runningProgram->pid, SIGCONT); // PROGRAMA EM EXECUÇÃO
	printf("\nO programa %s está sendo executado (SIGCONT) pid = %d tempo = %d\n",runningProgram->name, runningProgram->pid, time);
	runningProgram->status = 2;
	printf("Exibindo a fila de produção\n");
	queue_show(readyQueue);

}


//inicia escalonamento real-time
void startRealTime(){
	
	printf("Exibindo a fila de produção\n");
	queue_show(readyQueue);
	int qtd = readyQueue->lenght;
	int endRunning;
	int start;
	int duration;
	char * name;
	char * dependency;
	int pid;
	receivedSignal = 0;
	Program *aux;

	//enquanto houver programas
	while (qtd > 0){
	
		if (runningProgram == NULL){
 			runningProgram = readyQueue->first;
	
			while(runningProgram != NULL){

				start = runningProgram->runningTime;
				dependency = runningProgram ->dependentRunningTime;
				duration = runningProgram->duration;
				endRunning = start + duration;
				name = runningProgram->name;
				pid = runningProgram->pid;
				
				if (time == start){
					if (start + duration > 60){
						printf("O programa %s ultrapassa o tempo limite de 60 segundos\n", name);
						exit(0);
					}
					atualizaQueue();
					break;
				}
				
				time++;
				sleep(1);
				if (time > 60) time = 0;
				
			}
		}
		else{
		
			if (time == endRunning){
				
				runningProgram->multipleExecution++;
				printf("O programa %s , pid = %d vai parar, tempo = %d; Numero de execucao = %d\n", runningProgram->name, runningProgram->pid, time, runningProgram->multipleExecution);
				if (runningProgram->multipleExecution == 5)
				{
					printf("O programa %s terminou\n", runningProgram->name);
					kill(pid, SIGKILL);
					free(runningProgram);
					runningProgram = NULL;
					qtd--;
				}
				else{
					aux = program_copy(runningProgram);
					queue_insert(readyQueue, program_copy(aux));
					kill(pid, SIGSTOP);
				}
				runningProgram = NULL;
				printf("Exibindo a fila de pronto\n");
				if (!queue_isEmpty(loadQueue)){
				 aux = queue_pop(loadQueue);
				 queue_insert(readyQueue, program_copy(aux));
				}
				queue_show(readyQueue);
			}
			if (!queue_isEmpty(readyQueue)){
			if (runningProgram != NULL && readyQueue->first->runningTime == time){
				printf("O programa %s deve ser executado, mas outro  programa (%s) ainda está em execução\n",readyQueue->first->name, runningProgram->name);
				aux = queue_pop(readyQueue);
				queue_insert(loadQueue, program_copy(aux));
			}}
			
			time++;
			
			sleep(1);

			if (time > 60) time = 0;
		
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

	file = fopen("exec.txt", "r");
	if (file == NULL) {
		 printf("Erro abrindo o arquivo\n");
		 exit(1);    	 
	}
	 

	programas = (Info *) malloc (10*sizeof(Info));
	int j;

	// leitura do arquivo linha a linha
	while ((read = getline(&programa, &len, file)) != -1) {
		programa = strremove(programa,"Run <");
		programa = strremove(programa,"<");
		programa = strremove(programa,">");
		programa = strremove(programa,"I");
		programa = strremove(programa,"D");
		programa = strremove(programa,"=");
		programa = strremove(programa,"\n");
		
		char *ptr = strtok(programa," ");
		strcpy(programas[i].nome, ptr);
		ptr = strtok(NULL, " ");
		
		for (j = 0; j < i; j++) {
			if (strcmp(ptr,programas[j].nome) == 0) { 
				programas[i].runningTime = (programas[j].runningTime + programas[j].duration + 1);
				break;
			}
		}
		if (j == i) programas[i].runningTime = atoi(ptr);
		
		ptr = strtok(NULL, " ");
		
		programas[i].duration = atoi(ptr);
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
		p = program_create(programas[i].nome, pid, receivedSignal, programas[i].runningTime, programas[i].duration ); 
		queue_insert(readyQueue, p);
		i++;
	}
	
	printf("\nIniciando o escalonamento Real Time\n");
	
	startRealTime();
	
	free(programas);

	queue_free(readyQueue);
	queue_free(loadQueue);
}