#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

int calls = 0;
int c_write = 0;
int c_read = 0;
int c_seek = 0;

//Para manejar la señal SIGINT (Ctrl+C)
void ctrlc_handler(int signal) {
    printf("\nFinalizando ejecución...\n");
    printf("Llamadas al sistema de los procesos hijo: %d\n",calls);
    printf("Llamadas write: %d\n",c_write);
    printf("Llamadas read: %d\n",c_read);
    printf("Llamadas seek %d\n",c_seek);
    exit(0);
    
}

void monitor_syscalls() {
    // Implementar la lógica para monitorear las llamadas al sistema y registrarlas en "syscalls.log"
}

int main() {
    //Creo el archivo si no existe, de lo contrario, se vacía
     int fd = open("syscalls.log", O_CREAT | O_TRUNC, 0777);
    //Manejador de señales para SIGINT (Ctrl+C)
    signal(SIGINT, ctrlc_handler);

   //Proceso hijo 1
    pid_t pid1 = fork();
    if (pid1 == 0) {
        char *argv[] = {"/home/luisa/Escritorio/Sopes2/SO2_202003381/Practica 1/child.bin", NULL};
        execv(argv[0], argv);
        perror("execv"); // En caso de que execv falle
        exit(1);
    } else if (pid1 > 0) {
        // Código del proceso padre
        pid_t pid2 = fork();
        if (pid2 == 0) {
            // Código del segundo proceso hijo
            char *argv[] = {"/home/luisa/Escritorio/Sopes2/SO2_202003381/Practica 1/child.bin", NULL};
            execv(argv[0], argv);
            perror("execv"); // En caso de que execv falle
            exit(1);
        } else if (pid2 < 0) {
            perror("fork");
            exit(1);
        }
    } else {
        perror("fork");
        exit(1);
    }


    // Monitorear las llamadas al sistema de los procesos hijo
    monitor_syscalls();

    return 0;
}