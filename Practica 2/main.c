#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>
#include<stdbool.h>

#define THREADS_USERS 3
#define THREADS_OPERATIONS 4
#define MAX_LINE_LENGTH 1000
#define MAX_USERS 1000

pthread_mutex_t lock;
int usuariosLeidos;
int usuariosHilo1;
int usuariosHilo2;
int usuariosHilo3;
struct usuario {
    int no_cuenta;
    char nombre[100];
    float saldo;
};
struct operacion {
    int operacion;
    int cuenta1;
    int cuenta2;
    float monto;
};

char file_content[100000];
struct usuario usuarios[1000];
struct operacion operaciones[1000];


 /*   for(int i = 0; i < n; i++){
        printf("No. de cuenta: %d   Nombre: %s   Saldo: %5.2f\n", 
            usuarios[i].no_cuenta, usuarios[i].nombre, usuarios[i].saldo);
    }

    
*/
typedef struct hilo_info {
  int id;                      // Identificador del hilo
  FILE *archivo;                // Puntero al archivo CSV
  pthread_mutex_t mutex;       // Mutex para sincronización
  int *linea_leida_global;     // Puntero a la variable compartida
} HiloInfo;

bool existeCuenta(int cuenta){
    
    for(int i=0; i<MAX_USERS;i++){
       // printf("%d %d",cuenta,usuarios[i].no_cuenta);
        if(cuenta == usuarios[i].no_cuenta){
            return true;
        }
    }
    return false;
}

void inicializarUsuarios(){
    for(int i=0;i<MAX_USERS;i++){
        usuarios[i].no_cuenta=0;
        usuarios[i].saldo=0;
    }
}




char *leer_linea(FILE *archivo) {
  char *linea = NULL;
  size_t tam_linea = 0;
  ssize_t bytes_leidos;

  // Leer la línea del archivo
  bytes_leidos = getline(&linea, &tam_linea, archivo);

  if (bytes_leidos <= 0) {
    // Si no se pudo leer la línea, liberar la memoria y retornar NULL
    free(linea);
    linea = NULL;
  }

  return linea;
}




void* cargarUsuarios(void* arg) {
 HiloInfo *info_hilo = (HiloInfo *)arg;
  char *linea;
  char *token;
  int cuenta;
  char nombre[100];
  float saldo;

  // Bloquear el mutex antes de acceder a las variables compartidas
  pthread_mutex_lock(&(info_hilo->mutex));

  while ((linea = leer_linea(info_hilo->archivo)) != NULL) {
    // Acceder y modificar la variable compartida usando el puntero pasado
    *(info_hilo->linea_leida_global) = *(info_hilo->linea_leida_global) + 1;
    usuariosLeidos++;
    if(info_hilo->id==1){
        usuariosHilo1++;
    }else if(info_hilo->id==2){
        usuariosHilo2++;
    }else{
        usuariosHilo3++;
    }
    // Analizando la información, si es correcta se almacena, de lo contrario se guarda un error
    token = strtok(linea, ",");
    cuenta = atoi(token);

   
    //operaciones[n].cuenta1 = atoi(token);

    
    //operaciones[n].cuenta2 = atoi(token);

    if(cuenta==atoi(token) && cuenta>0){
        //Continuar
         token = strtok(NULL, ",");
        strcpy(nombre,token);
         if(existeCuenta(cuenta)){
        //Continuar
        token = strtok(NULL, ",");
        saldo = atof(token);
        if(saldo==atof(token) && saldo>0){
            //Guardar
        }else{
            printf("El valor no es un real positivo. Linea %d\n",*(info_hilo->linea_leida_global));
        }
        }else{
            printf("Error: Ya existe la cuenta. Línea %d\n",*(info_hilo->linea_leida_global));
        }

    }else{
        printf("El valor no es un entero positivo. Linea %d\n",*(info_hilo->linea_leida_global));
    }
   
    


    //printf("Hilo %d: Línea %d: %s\n", info_hilo->id, *(info_hilo->linea_leida_global), linea);

    // Liberar la memoria de la línea leída
    free(linea);

    // Desbloquear el mutex antes de salir del bucle
    pthread_mutex_unlock(&(info_hilo->mutex));
    sleep(0.5);
  }

  // Cancelar el hilo
  pthread_exit(NULL);

  return NULL;   
}

void cargarOperaciones(){
FILE *fp;
    fp = fopen("/home/luisa/Descargas/prueba_transacciones.csv", "r");

    char row[1000];
    char *token;

    fgets(row, 1000, fp);

    int n = 0;

    while (feof(fp) != true)
    {
        fgets(row, 1000, fp);

        token = strtok(row, ",");
        operaciones[n].operacion = atoi(token);

        token = strtok(NULL, ",");
        operaciones[n].cuenta1 = atoi(token);

        token = strtok(NULL, ",");
        operaciones[n].cuenta2 = atoi(token);

        token = strtok(NULL, ",");
        operaciones[n].monto = atof(token);

        n++;
    }

    for(int i = 0; i < n; i++){
        printf("Operacion: %d   Cuenta1: %d Cuenta2:%d   Monto: %5.2f\n", 
            operaciones[i].operacion, operaciones[i].cuenta1, operaciones[i].cuenta2, operaciones[i].monto);
    }

    
}
int main(){

    printf("Practica 2 - Sistemas Operativos 2\n");int linea_leida;
    int *linea_leida_global;
    FILE *archivo;
    printf("Luisa Maria Ortiz Romero - 202003381\n");
    while(true){
        char op;
        printf("|-----------------------MENU-----------------------|\n");
        printf("|1.Cargar usuarios                                 |\n");
        printf("|2.Operaciones individuales                        |\n");
        printf("|3.Cargar operaciones                              |\n");
        printf("|4. Salir                                          |\n");
        printf("|--------------------------------------------------|\n");
        scanf("%c",&op);
        getchar(); // Consumir el carácter de nueva línea residual
        
         switch(op){
        case '1':
            inicializarUsuarios();
            archivo = fopen("/home/luisa/Descargas/prueba_usuarios.csv", "r");
                if (archivo == NULL) {
                    printf("Error al abrir el archivo CSV\n");
                    return 1;
                }

                // Declarar e inicializar la variable compartida
               linea_leida = 0;
                linea_leida_global = &linea_leida;
                    usuariosLeidos=0;
                    usuariosHilo1=0;
                    usuariosHilo2=0;
                    usuariosHilo3=0;
                // Crear 3 hilos
                HiloInfo hilos[3];
                pthread_t tid_hilos[3];

                for (int i = 0; i < 3; i++) {
                    hilos[i].id = i + 1;
                    hilos[i].archivo = archivo;
                    pthread_mutex_init(&(hilos[i].mutex), NULL);
                    hilos[i].linea_leida_global = linea_leida_global;  // Pasar el puntero

                    // Crear el hilo
                    pthread_create(&(tid_hilos[i]), NULL, cargarUsuarios, &(hilos[i]));
                }

                // Esperar a que finalicen los hilos
                for (int i = 0; i < 3; i++) {
                    pthread_join(tid_hilos[i], NULL);
                }

                
                printf("Leídos: %d\n",usuariosLeidos);
                printf("Hilo1: %d\n",usuariosHilo1);
                printf("Hilo2: %d\n",usuariosHilo2);
                printf("Hilo: %d\n",usuariosHilo3);
                // Cerrar el archivo
                fclose(archivo);

            break;
        case '2':
            
            break;
        case '3':
            cargarOperaciones();
            break;
        case '4':
            printf("¡Ejecución finalizada!\n");
            exit(0);
            break;;
        default:
            printf("Opción inválida\n");
            break;
    }

    }
   
    
}
