#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

volatile sig_atomic_t sigint_received = 0; // Variable compartida para indicar si se recibió la señal SIGINT

int fdch; //Para manejar practica1.txt

void ctrlc_handler(int signal) {
    sigint_received = 1; // Establecer la bandera al recibir la señal SIGINT
    close(fdch); // Cerrar el archivo fdch
}

char random_char() {
    const char charset[] = "0123456789"
                           "abcdefghijklmnopqrstuvwxyz"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    const int charset_size = sizeof(charset) - 1;
    return charset[rand() % charset_size];
}



int main(int argc, char *argv[]) {
    srand(time(NULL) + getpid()); //seed único para lograr números aleatorios
    //printf("\nHola, soy tu hijo\n");
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
 