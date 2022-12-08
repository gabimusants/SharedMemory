## SharedMemory
<p> Implementação de um programa que utiliza a função fork para criação de 3 processos. <p> 
<p> Uma área de memória compartilhada é criada no processo pai de forma que armazene um vetor que contenha 2 inteiros, e o acesso a essa área é controlada através de semáforos. <p> 
<p> Os dois processos filhos acessam essa área de memória tanto diretamente quanto a partir de threads. Após a alteração dos valores do vetor os processos se comunicam através de troca de mensagens. <p> 
