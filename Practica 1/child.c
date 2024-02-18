#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>


int main(char *argv[]) {
    srand(time(NULL) + getpid());
    printf("\nHola, soy tu hijo\n");
   // while (1) {
        // Realizar llamadas de sistema sobre "practica1.txt"
        // Escribir, Leer, Seek
        // Controlar intervalos de tiempo aleatorios
   // }

    return 0;
}
 