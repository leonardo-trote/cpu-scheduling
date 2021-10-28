/*
Leonardo Trote Martins				1620572		
Ricardo Matheus de Oliveira Amaral		1621644	
*/

#include<stdio.h>
#include<stdlib.h>
#include"fila.h"

//Funções aplicadas na estrutura Program
Program* program_create(char* name, int pid, int isIO, int start, int duration){
    Program* p = (Program*)malloc(sizeof(Program));
    p->name = name;
    p->pid = pid;
    p->runningTime = start;
    p->duration = duration;
    p->waitingTime = 0;
    p->next = NULL;
    p->isIO = isIO; 
    p->status = 0;
    p->multipleExecution = 0;
    return p;
}
void program_free(Program* p){
    free(p);
}
Program * program_copy(Program* p){
    Program* new = program_create(p->name,p->pid,p->isIO, p->runningTime, p->duration);
    new->next = NULL;
    new->status = p->status;
    new->multipleExecution = p->multipleExecution;
    return new;
}

//Funções aplicadas na estrutura Queue
Queue* queue_create(){
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->first = q->last = NULL;
    q->lenght = 0;
    return q;
}
void queue_insert(Queue* q, Program* p){
    p->next = NULL;
    if (q->last == NULL){ //Caso a fila esteja vazia.
        q->first = p;
    }
    else{
        q->last->next = p;
    }
    q->last = p;
    q->lenght++;
}

void queue_insertP(Queue* q, Program *p){
    Program* aux, * ant;
    aux = q->first;
    if (!queue_isEmpty(q)){
        
        if (aux->status > p->status){
            q->first = p;
            p->next = aux;
            return;
        }
    }
    else if (q->lenght == 1){
        aux->next = p;
        return;
    }
        ant = aux;
        aux = aux->next;    
        while (aux != NULL){
            if (aux->status > p->status){
                ant->next = p;
                p->next = aux;
                break;
            }
            if (aux->next == NULL){
                aux->next = p;
                break;
            }
            ant = aux;
            aux = aux->next;
        }
    
    q->lenght++;
}
int queue_isEmpty(Queue* q){
    return q->lenght == 0;
}
void queue_free(Queue* q){
    Program* p;
    while (!queue_isEmpty(q)){
        p = q->first;
        q->first = q->first->next;
        program_free(p);
        q->lenght--;
    }
    free(q);
}
Program* queue_pop(Queue* q){
    Program* p = q->first;

    if (q->lenght == 1){
        q->first = NULL;
        q->last = NULL;
    }
    else{
    q->first = q->first->next;
    p->next = NULL;
    }
    q->lenght--;
    return p;
}
void queue_show(Queue* p){
    if (p->lenght > 0){
    Program* aux = p->first;
    printf("Fila: | ");
    while (aux->next != NULL){
        printf("%s | ",aux->name);
        aux = aux->next;
    }
    printf("%s| \n", aux->name);
    }
    else printf("Fila Vazia\n");
}

