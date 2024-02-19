# Manual técnico
Luisa María Ortíz Romero - 202003381 

## Proceso Padre
Se realizó un archivo llamado *parent.c* en el que se ejecutan las acciones necesarias para crear y monitorear el trabajo de los procesos hijo.
### Librerías importadas
~~~
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
~~~
**stdio.h:** Esta librería proporciona funciones para entrada y salida estándar, como printf() y scanf(), así como manipulación de archivos con funciones como fopen() y fclose().

**string.h:** Ofrece funciones para manipular cadenas de caracteres, como strlen() para obtener la longitud de una cadena y strcpy() para copiar cadenas.

**stdlib.h:** Contiene funciones para la gestión de memoria dinámica, conversión de tipos de datos y otras utilidades generales, como malloc() y free() para la gestión de la memoria dinámica y atoi() para convertir cadenas a enteros.

**unistd.h:** Proporciona acceso a numerosas funciones del sistema operativo POSIX relacionadas con la gestión de procesos, directorios y archivos, así como para la gestión de recursos del sistema, como fork() para crear procesos y exec() para ejecutar programas.

**errno.h:** Esta librería define la variable errno, que se utiliza para informar sobre errores durante las llamadas a funciones del sistema y proporciona macros para identificar los diferentes códigos de error.

**sys/wait.h:** Contiene funciones y macros relacionadas con la espera y el control de procesos, como wait() y waitpid() para esperar a que un proceso hijo termine.

**signal.h:** Permite la manipulación de señales del sistema, como SIGINT para la señal de interrupción (Ctrl+C) y SIGTERM para la señal de terminación.

**fcntl.h:** Esta librería proporciona funciones para el control de descriptores de archivos, como open() para abrir archivos y fcntl() para realizar operaciones en descriptores de archivos.


### Manejador de la señal CTRL + C
~~~
volatile sig_atomic_t sigint_received = 0; // Variable compartida para indicar si se recibió la señal SIGINT

void ctrlc_handler(int signal) {
    sigint_received = 1; // Establecer la bandera al recibir la señal SIGINT
}
~~~
### Método que se invoca al terminar la ejecución del programa
~~~
void stat(int pid1, int pid2, int monitor, int fd) {
    printf("\n");
    printf("Finalizando ejecución...\n");
    char buff[1024];
    char *action;
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buff, sizeof(buff))) > 0) {
        // Busca las acciones en el buffer
        char *ptr = buff;
        while ((action = strtok(ptr, "\n")) != NULL) {
            ptr = NULL; // Siguiente llamada a strtok debe recibir NULL
            // Incrementa el contador según la acción
            if (strstr(action, "read") != NULL) {
                c_read++;
            } else if (strstr(action, "lseek") != NULL) {
                c_seek++;
            } else if (strstr(action, "write") != NULL) {
                c_write++;
            }
        
    }
    }
    calls = c_read + c_seek + c_write;

    printf("-------------------------------------------------\n");
    printf("Llamadas al sistema de los procesos hijo: %d      \n",calls);
    printf("Llamadas write: %d                                \n",c_write);
    printf("Llamadas read: %d                                 \n",c_read);
    printf("Llamadas seek %d                                  \n",c_seek);
    printf("-------------------------------------------------\n");
    close(fd);
    kill(pid1,SIGKILL);
    kill(pid2,SIGKILL);
    kill(monitor, SIGKILL);
    exit(0);
    
}
~~~
### Método para monitorear el systemTap:
~~~
void monitor_syscalls(int pid1, int pid2) {
    
    char command[100];
    sprintf(command, "%s %d %d %s", "sudo stap trace.stp ", pid1, pid2, " > syscalls.log 2>&1");
    system(command);
}
~~~
### Método principal en el que se crean los distintos procesos hijo
~~~
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

            // Crear proceso hijo para monitorear las llamadas al sistema y poder terminarlo con CTRL + C
            pid_t monitor = fork();
            if(monitor==0){
                monitor_syscalls(pid1,pid2);
            }
          

            /*Se espera a que los procesos hijos terminen*/
            int status;
            waitpid(pid2, &status, 0);
            waitpid(pid2, &status, 0);
            waitpid(monitor, &status, 0);
            stat(pid1,pid2, monitor, fd);
            
        }
    }

    return 0;
}
~~~

## Procesos hijo
Se creó un archivo *child.c* que se encargará de realizar las distintas llamadas el sistema para que puedan ser monitoreadas, solo se crea un archivo y se ejecuta 2 veces desde el padre.
### Librerías importadas que no están en parent.c
~~~
#include <time.h>
~~~
**time.h:** Esta librería proporciona funciones y tipos de datos para trabajar con el tiempo y la fecha en programas en C. Algunas de las funciones más comunes incluyen time(), que devuelve el tiempo actual en segundos desde el 1 de enero de 1970 (también conocido como el tiempo UNIX), y ctime(), que convierte una marca de tiempo en una cadena de caracteres legible. Además, time.h incluye tipos de datos como time_t para representar tiempos y struct tm para representar fechas y horas desglosadas.

### Variables globales
~~~
volatile sig_atomic_t sigint_received = 0; // Variable compartida para indicar si se recibió la señal SIGINT

int fdch; //Para manejar practica1.txt
~~~
### Manejador de la señal CTRL + C
~~~
void ctrlc_handler(int signal) {
    sigint_received = 1; // Establecer la bandera al recibir la señal SIGINT
    close(fdch); // Cerrar el archivo fdch
}
~~~

### Generador de caracteres alfanuméricos aleatorios
~~~
char random_char() {
    const char charset[] = "0123456789"
                           "abcdefghijklmnopqrstuvwxyz"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    const int charset_size = sizeof(charset) - 1;
    return charset[rand() % charset_size];
}

~~~

### Método principal en que se generan las llamadas al sistema
~~~
int main(int argc, char *argv[]) {
    srand(time(NULL) + getpid()); //seed único para lograr números aleatorios
    printf("\nHola, soy tu hijo\n");
    fdch = open("practica1.txt", O_RDWR, 0777); //Abro el archivo en modo lectura y escritura
    //Manejador de señales para SIGINT (Ctrl+C)
    signal(SIGINT, ctrlc_handler);
    
  
    while (!sigint_received) {
        // Realizar llamadas del sistema hasta que se reciba la señal SIGINT
        int time = rand() % 3 + 1; // Tiempo de espera entre cada operación
        int op = rand() % 3 + 1; // Tipo de operación a realizar: 1-> write, 2-> Read, 3-> Seek

        if(op==1){
            //Write
            char random_string[8]; // 8 caracteres alfanuméricos
            for (int i = 0; i < 8; ++i) {
                random_string[i] = random_char();
            }
            write(fdch, random_string, 8); 
        }else if(op==2){
            //Read
            char buff[8];
            read(fdch,buff,8);
        }else{
            //Seek
             lseek(fdch,0,0);
        }

        sleep(time); // Esperar el tiempo especificado
    }

    return 0;
}
~~~
