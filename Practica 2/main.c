#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>
#include<stdbool.h>
#include <regex.h>



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
    struct usuario* siguiente;
};
struct usuario* cabezaUsuarios = NULL;
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



void inicializarUsuarios(){
    for(int i=0;i<MAX_USERS;i++){
        usuarios[i].no_cuenta=0;
        usuarios[i].saldo=0;
    }
}
void insertarUsuario(int numeroCuenta, char nombre[100], float saldo) {
  struct usuario* nuevoNodo = (struct usuario*)malloc(sizeof(struct usuario));
  nuevoNodo->no_cuenta = numeroCuenta;
  strcpy(nuevoNodo->nombre, nombre);
  nuevoNodo->saldo = saldo;
  nuevoNodo->siguiente = cabezaUsuarios;
  cabezaUsuarios = nuevoNodo;
}

void eliminarUsuario(int numeroCuenta) {
  struct usuario* actual = cabezaUsuarios;
  struct usuario* anterior;

  while (actual != NULL && actual->no_cuenta != numeroCuenta) {
    anterior = actual;
    actual = actual->siguiente;
  }

  if (actual != NULL) {
    if (actual == cabezaUsuarios) {
      cabezaUsuarios = actual->siguiente;
    } else {
      anterior->siguiente = actual->siguiente;
    }

    free(actual);
  }
}

bool existeCuenta(int numeroCuenta) {
  struct usuario* actual = cabezaUsuarios;

  while (actual != NULL) {
    if (actual->no_cuenta == numeroCuenta) {
      return true;
    }

    actual = actual->siguiente;
  }

  return false;
}




char *leer_linea(FILE *archivo) {
  char *linea = NULL;
  size_t tam_linea = 0;
  ssize_t bytes_leidos;

  // Leer la línea del archivo
  bytes_leidos = getline(&linea, &tam_linea, archivo);
  if (bytes_leidos != -1) {
    // Verificar si el último carácter es un salto de línea
    if (linea[bytes_leidos - 1] == '\n') {
        // Reemplazar el salto de línea con el carácter nulo
        linea[bytes_leidos - 1] = '\0';
    }
    // Ahora la cadena no tiene el carácter de salto de línea al final
}

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
  char cuenta[100];
  char nombre[100];
  char saldo[100];

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
    
  //printf("Línea %d->%s\n",*(info_hilo->linea_leida_global),linea);
  //Guardo el número de cuenta
  token = strtok(linea,",");
  if (token != NULL) {
    strcpy(cuenta, token); // Copiar el token a cuenta si no es NULL
    }
    //Guardo el nombre
    token = strtok(NULL,",");
    if(token!=NULL){
    strcpy(nombre,token);
    }
    //Guardo el saldo
    token = strtok(NULL,",");
    if(token!=NULL){
    strcpy(saldo,token);
    }
    regex_t regex;
    int reti;
    reti = regcomp(&regex, "^([0-9]+)$", REG_EXTENDED);
    reti = regexec(&regex, cuenta, 0, NULL, 0);

    int reti2;
reti2 = regcomp(&regex, "^[0-9]+([.][0-9]+)?$", REG_EXTENDED);
reti2 = regexec(&regex, saldo, 0, NULL, 0);

    if(reti==0){
        //Continuar con el procesamiento de datos
        if(!existeCuenta(atoi(cuenta))){
            //Continuando el procesamiento
            if(reti2==0){
                //Continuando a almacenar el usuario
                printf("%s\n",saldo);
            }else{
                printf("El valor no es un real positivo. Linea %d\n",*(info_hilo->linea_leida_global));
            }
        }else{
            printf("Error: Ya existe la cuenta. Línea %d\n",*(info_hilo->linea_leida_global));
        }


    }else{
        printf("El número de cuenta no es un entero positivo. Linea %d\n",*(info_hilo->linea_leida_global));
    }
  
/*
   token = strtok(linea, ",");
    printf("%s ",token);
    //operaciones[n].cuenta1 = atoi(token); 
    
    
    //operaciones[n].cuenta2 = atoi(token);
    regex_t regex;
    int reti;
    reti = regcomp(&regex, "^([0-9]+)$", REG_EXTENDED);
    reti = regexec(&regex, token, 0, NULL, 0);
    if(reti==0){
        //Continuar
        cuenta = atoi(token);
         token = strtok(NULL, ",");
         printf("%s ",token);
         if(token!=NULL){
        strcpy(nombre,token);
         }
         if(!existeCuenta(cuenta)){
           
        //Continuar
        token = strtok(NULL, ",");
        printf("%s\n",token);
        reti = regcomp(&regex, "^[0-9]+(.[0-9]*)$", REG_EXTENDED);
        reti = regexec(&regex, token, 0, NULL, 0);
        if(reti==0){
             saldo = atof(token);
            //Guardar
            //printf("%5.2f",saldo);
           // printf("%5.2f",atof(token));
            
        }else{
            printf("El valor no es un real positivo. Linea %d\n",*(info_hilo->linea_leida_global));
        }
        }else{
            printf("Error: Ya existe la cuenta. Línea %d\n",*(info_hilo->linea_leida_global));
        }

    }else{
        printf("El valor no es un entero positivo. Linea %d\n",*(info_hilo->linea_leida_global));
    }
   
    
    */

    //printf("Hilo %d: Línea %d: %s\n", info_hilo->id, *(info_hilo->linea_leida_global), linea);

    // Liberar la memoria de la línea leída
    free(linea);
    sleep(0.5);
    // Desbloquear el mutex antes de salir del bucle
    pthread_mutex_unlock(&(info_hilo->mutex));
    
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
                printf("Hilo3: %d\n",usuariosHilo3);
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
