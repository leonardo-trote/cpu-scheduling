/*
Leonardo Trote Martins				1620572		
Ricardo Matheus de Oliveira Amaral		1621644	
*/

//Estruturas usadas
typedef struct program
{
    char* name;
    int pid;
    int waitingTime;
    int runningTime;
    struct program* next;
    int isIO; 
    int status; 
    
}Program;

typedef struct queue{
    Program* first;
    Program* last;
    int lenght;
}Queue;

typedef struct info {
    char nome[10]; /* Nome do programa */ 
}Info;

//Funções aplicadas na estrutura Program
Program* program_create(char* name, int pid, int isIO);
Program * program_copy(Program* p);
void program_free();

//Funções aplicadas na estrutura Queue
Queue* queue_create();

void queue_insert(Queue* q, Program* p);

int queue_isEmpty(Queue* q);

void queue_free(Queue* q);

Program* queue_pop(Queue* q);

void queue_show(Queue* p);

void queue_insertP(Queue* q, Program *p);