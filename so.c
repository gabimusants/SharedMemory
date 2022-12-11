#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem;                                                      // Declaração do semáforo

void *altVarThreadA(int *vars) {
    int i = 0, v0;
    int numSerie = 1;

    printf("Id da thread A: %ld\n\n", pthread_self());

    for (i; i < 100; i++) {
        sem_wait(&sem);                                         // Tenta entrar na região crítica
        v0 = vars[0];                                           // Copia o valor da 1ª variável da shm para uma variável local
        v0--;                                                   // Decrementa o valor da variável copiada
        sleep(0.4);                                             // Dorme um tempo aleatório
        vars[0] = v0;                                           // Armazena o valor decrementado na 1ª variável da memória compartilhada
        vars[1]++;
        
        // Exibe na tela informações relevantes
        printf("Numero de serie (thread A): %d\n", numSerie);
        numSerie++;
        printf("Variavel 1 = %d\n", vars[0]);
        printf("Variavel 2 = %d\n", vars[1]);
        printf("\n------------------------------\n\n");
        sem_post(&sem);                                         // Libera a região crítica
        sleep(1);                                               // Dorme um tempo aleatório
    }

    printf("Thread A terminou!\n");
    printf("\n++++++++++++++++++++++++++++++\n\n");
}

void *altVarThreadB(int *vars) {
    int i = 0, v0;
    int numSerie = 1;

    printf("Id da thread B: %ld\n\n", pthread_self());

    for (i; i < 100; i++) {
        sem_wait(&sem);                                         // Tenta entrar na região crítica
        v0 = vars[0];                                           // Copia o valor da 1ª variável da shm para uma variável local
        v0--;                                                   // Decrementa o valor da variável copiada
        sleep(0.6);                                             // Dorme um tempo aleatório
        vars[0] = v0;                                           // Armazena o valor decrementado na 1ª variável da memória compartilhada
        vars[1]++;

        // Exibe na tela informações relevantes
        printf("Numero de serie (thread B): %d\n", numSerie);
        numSerie++;
        printf("Variavel 1 = %d\n", vars[0]);
        printf("Variavel 2 = %d\n", vars[1]);
        printf("\n------------------------------\n\n");
        sem_post(&sem); // Libera a região crítica
        sleep(1);       // Dorme um tempo aleatório
    }

    printf("Thread B terminou!\n");
    printf("\n++++++++++++++++++++++++++++++\n\n");
}

int main() {
    int *vars;                                                  // Variável inicializada com valor 300

    sem_init(&sem, 0, 1);                                       // Inicialização do semáforo

    int shm_Id = shmget(IPC_PRIVATE,
                        sizeof(int) * 2,
                        SHM_R | SHM_W | IPC_CREAT);             // Cria a região de memória compartilhada

    if (shm_Id < 0) {
        printf("Ocorreu um erro ao criar a area de memoria compartilhada!\n");
        exit(0);
    }
    
    int id = fork();

    if(id > 0) {                                                // id > de 0 é o processo pai
        int numSerie = 1;

        printf("\n++++++++++++++++++++++++++++++\n\n");
        printf("Main - ID: %d\n", getpid());
        printf("Numero de serie: %d\n", numSerie);
        numSerie++;

        vars = (int *) shmat(shm_Id, NULL, 0);                  // Anexa a variável à região de memória

        vars[0] = 300;
        vars[1] = 0;
        
        printf("Variavel 1 = %d\n", vars[0]);
        printf("Variavel 2 = %d\n", vars[1]);
        printf("\n++++++++++++++++++++++++++++++\n\n");

        int id_2 = fork();

        if(id_2 == 0) {                                         // Filho 2 do main
            //sleep(5);
            int i, v0;
            int numSerie = 1;

            printf("Sou filho 2 - ID: %d\n", getpid());

            vars = (int *)shmat(shm_Id, NULL, 0);               // Anexa a variável à região de memória

            for (i; i < 100; i++) {
                sem_wait(&sem);                                 // Tenta entrar na região crítica
                v0 = vars[0];
                v0--;                                           // Decrementa o valor da variável copiada
                sleep(0.8);                                     // Dorme um tempo aleatório
                vars[0] = v0;                                   // Armazena o valor decrementado na 1ª variável da memória compartilhada
                vars[1]++;

                // Exibe na tela informações relevantes
                printf("Numero de serie (filho 2): %d\n", numSerie);
                numSerie++;
                printf("Variavel 1 = %d\n", vars[0]);
                printf("Variavel 2 = %d\n", vars[1]);
                printf("\n------------------------------\n\n");
                sem_post(&sem);                                 // Libera a região crítica
                sleep(1);                                       // Dorme um tempo aleatório
            }

            printf("Filho 2 terminou!\n"); 
            printf("\n++++++++++++++++++++++++++++++\n\n");

            shmdt(vars);                                        // Desanexa região de memória da variável 1

        }
        
        while((vars[0] != 0) && (vars[1] != 300));
        sleep(10);
        shmdt(vars);                                            // Desanexa região de memória da variável 1

        shmctl(shm_Id, IPC_RMID, NULL);                         // Destrói a região de memória compartilhada

    } else if (id == 0){                                        // id == 0 é o processo filho, filho 1 do main
        printf("Sou filho 1 - ID: %d\n", getpid());

        vars = (int *)shmat(shm_Id, NULL, 0);                   // Anexa a variável à região de memória

        pthread_t thrA, thrB;

        pthread_create(&thrA, NULL, (void *)altVarThreadA, vars); // Cria thread com id, inic, função e parâmetro da função
        pthread_create(&thrB, NULL, (void *)altVarThreadB, vars); // Cria thread com id, inic, função e parâmetro da função

        pthread_join(thrA, NULL);                               // Espera fim das threads
        pthread_join(thrB, NULL);                               // Espera fim das threads

        shmdt(vars);                                            // Desanexa região de memória da variável 1

    } else {
        printf("Fork falhou!\n");
    }
    return 0;
}

/*gcc so.c -o so -lpthread*/