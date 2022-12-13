#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <sys/types.h>
#include <semaphore.h>

#define TAM_MAX_MSG 256

sem_t *sem;                                                     // Declaração do &semáforo

struct message {                                                // Struct para envio de mensagens
    long type;
    char text[TAM_MAX_MSG];
};

struct message msg1;
struct message msg2;

int randomSleep() {                                             // Função utilizada para gerar um valor aleatório e passar de argumento para usleep
    double num = ((double)rand()) / ((double)RAND_MAX);
    num = num * 0.5 + 0.5;

    double micro = num * 1000000;
    return usleep(micro);
}

void *altVars(int *vars) {
    int v0;

    v0 = vars[0];                                               // Copia o valor da 1ª variável da shm para uma variável local
    v0--;                                                       // Decrementa o valor da variável copiada
    randomSleep();                                              // Dorme um tempo aleatório

    vars[0] = v0;                                               // Armazena o valor decrementado na 1ª variável da memória compartilhada
    vars[1]++;                                                  // Incrementa o valor da 2ª variável
}

void *ThreadA(int *vars) {                                      // Função utilizada para realização da thread A
    int i = 0;
    int numSerie = 1;

    for (i; i < 100; i++) {
        sem_wait(sem);                                          // Tenta entrar na região crítica
        altVars(vars);                                          // Chama a função que altera as variáveis

        // Exibe na tela informações relevantes
        printf("Thread A - ID: %ld\n\n", pthread_self());
        printf("Numero de serie (thread A): %d\n", numSerie);
        numSerie++;
        printf("Variavel 1 = %d\n", vars[0]);
        printf("Variavel 2 = %d\n", vars[1]);
        printf("\n-------------------------------------------\n\n");
        sem_post(sem);                                          // Libera a região crítica
        randomSleep();                                          // Dorme um tempo aleatório
    }

    printf("Thread A terminou!\n");
    printf("\n+++++++++++++++++++++++++++++++++++++++++++\n\n");
}

void *ThreadB(int *vars) {                                      // Função utilizada para realização da thread A
    int i = 0;
    int numSerie = 1;

    for (i; i < 100; i++) {
        sem_wait(sem);                                          // Tenta entrar na região crítica
        altVars(vars);

        // Exibe na tela informações relevantes
        printf("Thread B - ID: %ld\n\n", pthread_self());
        printf("Numero de serie (thread B): %d\n", numSerie);
        numSerie++;
        printf("Variavel 1 = %d\n", vars[0]);
        printf("Variavel 2 = %d\n", vars[1]);
        printf("\n-------------------------------------------\n\n");
        sem_post(sem);                                          // Libera a região crítica
        randomSleep();                                          // Dorme um tempo aleatório
    }

    printf("Thread B terminou!\n");
    printf("\n+++++++++++++++++++++++++++++++++++++++++++\n\n");
}

int main() {
    int *vars;                                                  // Variável inicializada com valor 300

    sem = sem_open("/sem", O_CREAT, 0644, 1);                   // Inicialização do semáforo

    int shm_Id = shmget(IPC_PRIVATE,
                        sizeof(int) * 2,
                        SHM_R | SHM_W | IPC_CREAT);             // Cria a região de memória compartilhada

    if (shm_Id < 0) {
        printf("Ocorreu um erro ao criar a area de memoria compartilhada!\n");
        exit(0);
    }

    vars = (int *) shmat(shm_Id, NULL, 0);                      // Anexa a variável à região de memória

    int msgId_1 = msgget(IPC_PRIVATE, 0600 | IPC_CREAT);        // Função utilizada para criar uma nova fila de mensagem
    if (msgId_1 == -1) {
        printf("Erro ao criar a fila de mensagens!\n");
    }

    int msgId_2 = msgget(IPC_PRIVATE, 0600 | IPC_CREAT);        // Função utilizada para criar uma nova fila de mensagem
    if (msgId_2 == -1) {
        printf("Erro ao criar a fila de mensagens!\n");
    }

    int id = fork();

    if(id > 0) {                                                // id > de 0 é o processo pai
        int numSerie = 1;
        int type = 1;

        printf("\n+++++++++++++++++++++++++++++++++++++++++++\n");
        printf("\nMain - ID: %d\n", getpid());
        printf("Numero de serie: %d\n", numSerie);
        numSerie++;

        vars[0] = 300;
        vars[1] = 0;
        
        printf("Variavel 1 = %d\n", vars[0]);
        printf("Variavel 2 = %d\n", vars[1]);
        printf("\n+++++++++++++++++++++++++++++++++++++++++++\n");

        int id_2 = fork();

        if(id_2 == 0) {                                         // Filho 2 do main
            int i, v0;
            int numSerie = 1;

            for (i; i < 100; i++) {
                sem_wait(sem);                                  // Tenta entrar na região crítica
                altVars(vars);                                  // Chama a função que altera as variáveis

                // Exibe na tela informações relevantes
                printf("Filho 2 - ID: %d\n\n", getpid());
                printf("Numero de serie (filho 2): %d\n", numSerie);
                numSerie++;
                printf("Variavel 1 = %d\n", vars[0]);
                printf("Variavel 2 = %d\n", vars[1]);
                printf("\n-------------------------------------------\n\n");
                sem_post(sem);                                  // Libera a região crítica
                randomSleep();                                  // Dorme um tempo aleatório
            }

            msg2.type = 1;
            strcpy(msg2.text, "Processo pai ciente que filho 2 terminou!\n");
            if (msgsnd(msgId_2, &msg2, sizeof(msg2), 0) == -1) { // Faz a tentativa de envio da mensagem
                perror("Mensagem nao enviada!\n");
            }
        }
        
        if(id_2 > 0) {
            msgrcv(msgId_1, &msg1, sizeof(msg1), type, 0);      // Recebe primeira mensagem
            msgrcv(msgId_2, &msg2, sizeof(msg2), type, 0);      // Recebe segunda mensagem
            
            printf("%s\n", msg1.text);                          // Imprime a mensagem recebida
            printf("%s\n", msg2.text);                          // Imprime a mensagem recebida

            msgctl(msgId_1, IPC_RMID, NULL);                    // Destrói a fila de mensagem 1
            msgctl(msgId_2, IPC_RMID, NULL);                    // Destrói a fila de mensagem 2

            printf("+++++++++++++++++++++++++++++++++++++++++++\n\n");
            printf("O processo pai sera finalizado!\n");
            printf("Variavel 1 = %d\n", vars[0]);
            printf("Variavel 2 = %d\n", vars[1]);
            printf("\n+++++++++++++++++++++++++++++++++++++++++++\n");

            shmdt(vars);                                        // Desanexa região de memória da variável 1
            shmctl(shm_Id, IPC_RMID, NULL);                     // Destrói a região de memória compartilhada

            return 0;
        }

    } else if (id == 0) {                                        // id == 0 é o processo filho, filho 1 do main
        printf("\n+++++++++++++++++++++++++++++++++++++++++++\n");
        printf("\nFilho 1 - ID: %d\n", getpid());
        printf("\n+++++++++++++++++++++++++++++++++++++++++++\n");

        pthread_t thrA, thrB;

        pthread_create(&thrA, NULL, (void *)ThreadA, vars);     // Cria thread com id, inic, função e parâmetro da função
        pthread_create(&thrB, NULL, (void *)ThreadB, vars);     // Cria thread com id, inic, função e parâmetro da função

        pthread_join(thrA, NULL);                               // Espera fim das threads
        pthread_join(thrB, NULL);                               // Espera fim das threads

        msg1.type = 1;
        strcpy(msg1.text, "Processo pai ciente que filho 1 terminou!\n");
        if (msgsnd(msgId_1, &msg1, sizeof(msg1), 0) == -1) {    // Faz a tentativa de envio da mensagem
            perror("Mensagem nao enviada!\n");
        } 

    } else {
        printf("Fork falhou!\n");
    }

    return 0;
}

/*gcc so.c -o so -lpthread*/