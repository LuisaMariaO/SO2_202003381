#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h> 

int calls = 0;
int c_write = 0;
int c_read = 0;
int c_seek = 0;

pthread_t monitor_thread_id;

volatile sig_atomic_t sigint_received = 0; // Variable compartida para indicar si se recibió la señal SIGINT

void ctrlc_handler(int signal) {
    sigint_received = 1; // Establecer la bandera al recibir la señal SIGINT
}

//Después de recibir el ctrl+c
void stat() {
    printf("\n");
    printf("Finalizando ejecución...\n");
    printf("|-------------------------------------------------|\n");
    printf("|Llamadas al sistema de los procesos hijo: %d      |\n",calls);
    printf("|Llamadas write: %d                                |\n",c_write);
    printf("|Llamadas read: %d                                 |\n",c_read);
    printf("|Llamadas seek %d                                  |\n",c_seek);
    printf("|-------------------------------------------------|\n");
    exit(0);
    
}

void monitor_syscalls(int pid1, int pid2) {
  
    char command[100];
    sprintf(command, "%s %d %d %s", "sudo stap trace.stp ", pid1, pid2, " > syscalls.log 2>&1");
    system(command);
}

void *monitor_thread(void *args) {
    

    
    int pid1 = *((int *)args);
    int pid2 = *((int *)args + 1);
    monitor_syscalls(pid1, pid2);
    pthread_exit(NULL);
    
}

int main() {
    signal(SIGINT, ctrlc_handler);
    //Abro el archivo syscalls.log, lo si no existe, y se existe, se vacía
    int fd = open("syscalls.log", O_RDWR | O_CREAT | O_TRUNC, 0777);
    //De igual manera se abre practica1.txt que será utilizado por los hijos pero se cierra acá
    int fdch = open("practica1.txt",  O_CREAT | O_TRUNC, 0777);
    close(fdch);

    pid_t pid1 = fork(); //Se realiza el fork 1
    
    if(pid1 == -1){
        perror("fork");
        exit(1);
    }
    if(pid1 == 0){

        /*Se escriben los argumentos para el proceso hijo*/
        char *arg_Ptr[4];
        arg_Ptr[0] = " child.c";
        arg_Ptr[1] = " Hola! ";
        arg_Ptr[2] = " Soy el proceso hijo! ";
        arg_Ptr[3] = NULL; // El ultimo indice de argv siempre debe de ser NULL

        /*Se ejecuta el ejecutable del proceso hijo*/
        execv("/home/luisa/Escritorio/Sopes2/SO2_202003381/Practica 1/child.bin", arg_Ptr);
    
     }else{
        pid_t pid2 = fork(); //Se realiza el fork 2

        if(pid2 == -1){
            perror("fork");
            exit(1);
        }
        
        if(pid2 == 0){
            // Código para el segundo proceso hijo
            printf("Soy el proceso hijo 2\n");

            /*Se escriben los argumentos para el proceso hijo*/
            char *arg_Ptr[4];
            arg_Ptr[0] = " child.c";
            arg_Ptr[1] = " Hola! ";
            arg_Ptr[2] = " Soy el proceso hijo! ";
            arg_Ptr[3] = NULL; // El ultimo indice de argv siempre debe de ser NULL

            /*Se ejecuta el ejecutable del proceso hijo*/
            execv("/home/luisa/Escritorio/Sopes2/SO2_202003381/Practica 1/child.bin", arg_Ptr);
        }else{
            printf("Soy el proceso padre\n");

            // Crear un hilo para monitorear las llamadas al sistema
            pthread_t thread;
            int args[2] = {pid1, pid2};
            pthread_create(&thread, NULL, monitor_thread, (void *)args);

            /*Se espera a que los procesos hijos terminen*/
            int status;
            waitpid(pid2, &status, 0);
            waitpid(pid2, &status, 0);

            /*Se imprime el codigo de salida de los procesos hijos*/
            if(WIFEXITED(status)){

                stat();
            } else {
                printf("Ocurrio un error al terminar los procesos hijos");
            }
            
            // Esperar a que el hilo de monitoreo termine
            pthread_join(thread, NULL);
            printf("\nYa se acabó el hilo\n");
            
        }
    }

    return 0;
}