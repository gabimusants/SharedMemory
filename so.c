#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/shm.h>
#include <pthread.h>

void *altVar(int *vars) {
    int i = 0, v0;
    int numSerie = 1;
    v0 = vars[0];                                              // Copia o valor da 1ª variável da shm para uma variável local

    printf("Id da thread: %ld\n\n", pthread_self());

    for (i; i < 5; i++) {
        v0--;                                                   // Decrementa o valor da variável copiada
        sleep(1);                                               // Dorme um tempo aleatório
        vars[0] = v0;                                          // Armazena o valor decrementado na 1ª variável da memória compartilhada
        vars[1]++;
        
        // Exibe na tela informações relevantes
        printf("Numero de serie: %d\n", numSerie);
        numSerie++;
        printf("Variavel 1 = %d\n", vars[0]);
        printf("Variavel 2 = %d\n", vars[1]);
        printf("\n-----------------------------\n\n");
        sleep(0.1);                                               // Dorme um tempo aleatório
    }
}


int main() {
    int *vars;                                          // Variável inicializada com valor 300
    
    int shm_Id = shmget(IPC_PRIVATE,
                        sizeof(int) * 2,
                        SHM_R | SHM_W | IPC_CREAT);     // Cria a região de memória compartilhada dentro do processo pai

    if (shm_Id < 0) {
        printf("Ocorreu um erro ao criar a area de memoria compartilhada!\n");
        exit(0);
    }
    
    int id = fork();

    if(id > 0) {                                           // id > de 0 é o processo pai
        int numSerie = 1;

        printf("\n+++++++++++++++++++++++++++++\n\n");
        printf("Main - ID: %d\n", getpid());
        printf("Numero de serie: %d\n", numSerie);
        numSerie++;

        vars = (int *) shmat(shm_Id, NULL, 0);           // Anexa a variável à região de memória

        vars[0] = 300;
        vars[1] = 0;
        
        printf("Variavel 1 = %d\n", vars[0]);
        printf("Variavel 2 = %d\n", vars[1]);
        printf("\n+++++++++++++++++++++++++++++\n\n");

        int id_2 = fork();

        if(id_2 == 0) {                                 // Filho 2 do main
            sleep(5);
            int i, v0;
            int numSerie = 1;

            printf("Sou filho 2 - ID: %d\n", getpid());
            //printf("Numero de serie: %d\n", numSerie);
            //numSerie++;

            vars = (int *)shmat(shm_Id, NULL, 0);       // Anexa a variável à região de memória
            v0 = vars[0];
            //vars[0] = 200;
            //vars[1] = 10;

            for (i; i < 5; i++) {
                v0--;         // Decrementa o valor da variável copiada
                sleep(1);     // Dorme um tempo aleatório
                vars[0] = v0; // Armazena o valor decrementado na 1ª variável da memória compartilhada
                vars[1]++;

                // Exibe na tela informações relevantes
                printf("Numero de serie: %d\n", numSerie);
                numSerie++;
                printf("Variavel 1 = %d\n", vars[0]);
                printf("Variavel 2 = %d\n", vars[1]);
                printf("\n-----------------------------\n\n");
                sleep(0.1); // Dorme um tempo aleatório
            }

            //printf("Var 1 = %d\n", vars[0]);
            //printf("Var 2 = %d\n", vars[1]);

            // APAGAR ESTE PRINT DEPOIS QUE TIVER FEITO A TROCA DE MENSAGEM
            printf("Filho 2 terminou!\n"); // APAGAR ESTE PRINT DEPOIS QUE TIVER FEITO A TROCA DE MENSAGEM
            printf("\n+++++++++++++++++++++++++++++\n\n");

            shmdt(vars);                                // Desanexa região de memória da variável 1

        }
        
        //sleep(20);
        while((vars[0] != 285) && (vars[1] != 15));
        //while(*var2 != 300);

        shmdt(vars);                                    // Desanexa região de memória da variável 1
        //shmdt(var2);                                    // Desanexa região de memória da variável 2

        shmctl(shm_Id, IPC_RMID, NULL);                 // Destrói a região de memória compartilhada

    } else if (id == 0){                                // id == 0 é o processo filho, filho 1 do main
        sleep(12);

        printf("Sou filho 1 - ID: %d\n\n", getpid());

        vars = (int *)shmat(shm_Id, NULL, 0);                 // Anexa a variável à região de memória
        //var2 = (int *)shmat(shm_Id, NULL, 0);               // Anexa a variável à região de memória
        //vars[0] = 200;
        //vars[1] = 20;

        pthread_t thrA, thrB;

        pthread_create(&thrA, NULL, (void *)altVar, vars); // Cria thread com id, inic, função e parâmetro da função
        pthread_join(thrA, NULL);                           // Espera fim das threads
        printf("Thread A terminou!\n");
        printf("\n+++++++++++++++++++++++++++++\n\n");

        pthread_create(&thrB, NULL, (void *)altVar, vars); // Cria thread com id, inic, função e parâmetro da função
        pthread_join(thrB, NULL);                           // Espera fim das threads
        printf("Thread B terminou!\n");
        printf("\n+++++++++++++++++++++++++++++\n\n");

        shmdt(vars);                                    // Desanexa região de memória da variável 1

    } else {
        printf("Fork falhou!\n");
    }
    return 0;
}

/*gcc so.c -o so -lpthread*/