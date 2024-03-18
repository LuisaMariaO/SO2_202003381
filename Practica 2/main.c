#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>
#include<stdbool.h>
#include <regex.h>
#include<time.h>



#define THREADS_USERS 3
#define THREADS_OPERATIONS 4
#define MAX_LINE_LENGTH 1000
#define MAX_USERS 1000

pthread_mutex_t lock;
int usuariosLeidos;
int usuariosHilo1;
int usuariosHilo2;
int usuariosHilo3;

int operacionesLeidas;
int operacionesHilo1;
int operacionesHilo2;
int operacionesHilo3;
int operacionesHilo4;

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

char erroresUsuarios[10000];
char erroresOperaciones[10000];

char file_content[100000];
struct usuario usuarios[1000];
struct operacion operaciones[1000];


 /*   for(int i = 0; i < n; i++){
        printf("No. de cuenta: %d   Nombre: %s   Saldo: %10.2f\n", 
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

void estadoDeCuenta(){
    struct usuario* actual = cabezaUsuarios;
    FILE *reporte;
    reporte = fopen("Estado de cuenta.csv","w");
    if (reporte == NULL) {
        printf("Error al abrir el archivo.\n");
        return; // Salir del programa con un código de error
    }

    fprintf(reporte,"no_cuenta,nombre,saldo\n");
    while(actual != NULL){
        fprintf(reporte,"%d,%s,%10.2f\n",actual->no_cuenta,actual->nombre,actual->saldo);
        actual = actual->siguiente;
    }
    fclose(reporte);

}

struct usuario* getUsuario(int numeroCuenta){
    struct usuario* actual = cabezaUsuarios;

  while (actual != NULL) {
    if (actual->no_cuenta == numeroCuenta) {
      return actual;
    }

    actual = actual->siguiente;
  }

  return NULL;
}

void deposito(int cuenta, float monto){
    struct usuario* usuario = getUsuario(cuenta);
    usuario->saldo = usuario->saldo + monto;

}



void retiro(int cuenta, float monto, int linea){
    struct usuario* usuario = getUsuario(cuenta);
    char error[200];
    if(usuario->saldo - monto >= 0){//Si el saldo restante resultaría negativo, no se puede proceder
        usuario->saldo = usuario->saldo - monto;
        if(linea==0){
            printf("¡Retiro realizado!\n");
        }
    }else{
        if(linea>0){
        sprintf(error,"\t-Línea #%d - El saldo de [%d]: <%10.2f> es insuficiente para retirar <%10.2f> \n",linea,cuenta,usuario->saldo,monto);
        strcat(erroresOperaciones,error);
        }else{
            printf("Error: El saldo de [%d]: <%10.2f> es insuficiente para retirar <%10.2f> \n",cuenta,usuario->saldo,monto);
        }
    }
    
}
void transferencia(int cuenta1, int cuenta2, float monto, int linea){
    struct usuario* origen = getUsuario(cuenta1);
    struct usuario* destino = getUsuario(cuenta2);
    
    char error[200];
    char error2[200];
    if(destino!=NULL){

    if(origen->saldo - monto >= 0){//Si el saldo restante resultaría negativo, no se puede proceder
        origen->saldo = origen->saldo - monto; //Se resta al origen
        destino->saldo = destino->saldo + monto; //Se suma al destino
        if(linea==0){
            printf("¡Transferencia realizada!\n");
        }
    }else{
        if(linea>0){
        sprintf(error,"\t-Línea #%d - El saldo de [%d]: <%10.2f> es insuficiente para transferir <%10.2f> \n",linea,cuenta1,origen->saldo,monto);
        strcat(erroresOperaciones,error);
        }else{
            printf("Error: El saldo de [%d]: <%10.2f> es insuficiente para transferir <%10.2f> \n",cuenta1,origen->saldo,monto);
        }
    }
    }else{
        if(linea>0){
        sprintf(error,"\t-Línea #%d - La cuenta <%d> no existe\n",linea,cuenta2);
        strcat(erroresOperaciones,error);
        }else{
            printf("Error: La cuenta <%d> no existe\n",cuenta2);
        }
    }

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
    if (linea[bytes_leidos - 2] == '\r') {
        // Reemplazar el salto de línea con el carácter nulo
        linea[bytes_leidos - 2] = '\0';
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
  char error1[200];
  char error2[200];
  char error3[200];
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
                insertarUsuario(atoi(cuenta),nombre,atof(saldo));
            }else{
                /*
                 if(strcmp(token,"no_cuenta")!=0 && *(info_hilo->linea_leida_global)!=1){
                    printf("El valor no es un real positivo. Linea %d\n",*(info_hilo->linea_leida_global));
                    
                 }
                 */
                sprintf(error1,"\t-Línea #%d - El saldo <%s> no es un real positivo\n",*(info_hilo->linea_leida_global),saldo);
                strcat(erroresUsuarios,error1);
                
                
            }
        }else{
            sprintf(error2,"\t-Línea #%d - Cuenta <%s> repetida\n",*(info_hilo->linea_leida_global),cuenta);
           strcat(erroresUsuarios,error2);
        }


    }else{
        if(strcmp(cuenta,"no_cuenta")!=0 && *(info_hilo->linea_leida_global)!=1){
                    sprintf(error3,"\t-Línea #%d - El número de cuenta <%s> no es un entero positivo\n",*(info_hilo->linea_leida_global),cuenta);
                    strcat(erroresUsuarios,error3);
                    
                 }
        //printf("El número de cuenta no es un entero positivo. Linea %d\n",*(info_hilo->linea_leida_global));
    }
  

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

void* cargarOperaciones(void *arg){
HiloInfo *info_hilo = (HiloInfo *)arg;
  char *linea;
  char *token;
  char operacion[100];
  char cuenta1[100];
  char cuenta2[100];
  char monto[100];
  char error1[200];
  char error2[200];
  char error3[200];
  char error4[200];
  char error5[200];
  char error6[200];
  // Bloquear el mutex antes de acceder a las variables compartidas
  pthread_mutex_lock(&(info_hilo->mutex));

  while ((linea = leer_linea(info_hilo->archivo)) != NULL) {
    // Acceder y modificar la variable compartida usando el puntero pasado
    *(info_hilo->linea_leida_global) = *(info_hilo->linea_leida_global) + 1;

    operacionesLeidas++;
    if(info_hilo->id==1){
        operacionesHilo1++;
    }else if(info_hilo->id==2){
        operacionesHilo2++;
    }else if(info_hilo->id==3){
        operacionesHilo3++;
    }else{
        operacionesHilo4++;
    }
    // Analizando la información, si es correcta se almacena, de lo contrario se guarda un error
    
  //printf("Línea %d->%s\n",*(info_hilo->linea_leida_global),linea);
  //Guardo la operacion
  token = strtok(linea,",");
  if (token != NULL) {
   
    strcpy(operacion, token); // Copiar el token a cuenta si no es NULL
    }
    //Guardo la cuenta1
    token = strtok(NULL,",");
    if(token!=NULL){
    strcpy(cuenta1,token);
    }
    //Guardo la cuenta2
    token = strtok(NULL,",");
    if(token!=NULL){
    strcpy(cuenta2,token);
    }
    //Guardo el monto
    token = strtok(NULL,",");
    if(token!=NULL){
    strcpy(monto,token);
    
    }
    regex_t regex;
    int reti;
    reti = regcomp(&regex, "^(1|2|3)$", REG_EXTENDED);
    reti = regexec(&regex, operacion, 0, NULL, 0);

    int reti2;
    reti2 = regcomp(&regex, "^([0-9]+)$", REG_EXTENDED);
    reti2 = regexec(&regex,cuenta1,0,NULL,0);

    int reti3;
    reti3 = regcomp(&regex, "^([0-9]+)$", REG_EXTENDED);
    reti3 = regexec(&regex,cuenta2,0,NULL,0);

    int reti4;
    reti4 = regcomp(&regex, "^[0-9]+([.][0-9]+)?$", REG_EXTENDED);
    reti4 = regexec(&regex, monto, 0, NULL, 0);

    if(reti==0 ){
        //Se continua con el resto de datos
        if(reti2==0){
            //Continuar
            if(existeCuenta(atoi(cuenta1))){
                //Continuar
                if(reti3==0){
                    //Continnuar
                        if(reti4==0){
                            //Proceder a la operacion
                            if(strcmp(operacion,"1")==0){
                                deposito(atoi(cuenta1),atof(monto));
                            }else if(strcmp(operacion,"2")==0){
                                retiro(atoi(cuenta1),atof(monto),*(info_hilo->linea_leida_global));
                            }else{
                                transferencia(atoi(cuenta1),atoi(cuenta2),atof(monto),*(info_hilo->linea_leida_global));
                            }

                        }else{
                            sprintf(error6,"\t-Línea #%d - El monto <%s> no es un real positivo\n",*(info_hilo->linea_leida_global),monto);
                            strcat(erroresOperaciones,error6);    
                        }
                   
                }else{
                    sprintf(error4,"\t-Línea #%d - El número de cuenta <%s> no es un entero positivo\n",*(info_hilo->linea_leida_global),cuenta2);
                    strcat(erroresOperaciones,error4);
                }
            }else{
                sprintf(error3,"\t-Línea #%d - El número de cuenta <%s> no existe\n",*(info_hilo->linea_leida_global),cuenta1);
                strcat(erroresOperaciones,error3);
            }
        }else{
            sprintf(error2,"\t-Línea #%d - El número de cuenta <%s> no es un entero positivo\n",*(info_hilo->linea_leida_global),cuenta1);
            strcat(erroresOperaciones,error2);
        }
    }else{
        if(strcmp(operacion,"operacion")!=0 && *(info_hilo->linea_leida_global)!=1){
        sprintf(error1,"\t-Línea #%d - La operacion <%s> no es válida\n",*(info_hilo->linea_leida_global),operacion);
        strcat(erroresOperaciones,error1);
        }
    }

    

    // Liberar la memoria de la línea leída
    free(linea);
    sleep(0.6);
    // Desbloquear el mutex antes de salir del bucle
    pthread_mutex_unlock(&(info_hilo->mutex));
    
  }

  // Cancelar el hilo
  pthread_exit(NULL);

  return NULL;   
}

int main(){

    printf("Practica 2 - Sistemas Operativos 2\n");
    int linea_leida;
    int *linea_leida_global;
    FILE *archivo;
   
    printf("Luisa Maria Ortiz Romero - 202003381\n");
    while(true){
        char op;
        char rutaUsuarios[2000];
        char rutaOperaciones[2000];
        printf("|-----------------------MENU-----------------------|\n");
        printf("|1.Cargar usuarios                                 |\n");
        printf("|2.Cargar operaciones                              |\n");
        printf("|3.Operaciones individuales                        |\n");
        printf("|4.Generar estados de cuenta                       |\n");
        printf("|5.Salir                                           |\n");
        printf("|--------------------------------------------------|\n");
        scanf("%c",&op);
        getchar(); // Consumir el carácter de nueva línea residual
        
         switch(op){
        case '1':
            printf("Ingrese la ruta del archivo de usuarios: ");
            scanf("%s",rutaUsuarios);
            getchar(); // Consumir el carácter de nueva línea residual
            archivo = fopen(rutaUsuarios, "r");
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
               
                // Cerrar el archivo
                fclose(archivo);

                //Generando reporte de carga de usuarios
                char nombre[100];
                char fecha[100];
                time_t tiempo_actual;
                struct tm *info_tiempo;
                time(&tiempo_actual);
                info_tiempo = localtime(&tiempo_actual);
                strftime(nombre,sizeof(nombre),"carga_%Y_%m_%d-%H_%M_%S.log",info_tiempo);
                strftime(fecha,sizeof(fecha),"%Y-%m-%d %H:%M:%S",info_tiempo);
                FILE *reporte;
                reporte = fopen(nombre, "w"); // Abrir el archivo en modo de escritura ("w")

                if (reporte == NULL) {
                    printf("Error al generar el archivo de reportes de usuario.\n");
                    return 1; // Salir del programa con código de error
                }
                fprintf(reporte,"------------------------------------------CARGA DE USUARIOS------------------------------------------\n");
                fprintf(reporte,"Fecha: %s\n",fecha);
                fprintf(reporte, "Usuarios cargados:\n"); // Escribir en el archivo
                fprintf(reporte,"Hilo #1 : %d\n",usuariosHilo1-1);
                fprintf(reporte,"Hilo #2 : %d\n",usuariosHilo2);
                fprintf(reporte,"Hilo #3 : %d\n",usuariosHilo3);
                fprintf(reporte,"Total : %d\n",usuariosLeidos-1);
                fprintf(reporte,"Errores:\n");
                fprintf(reporte,"%s",erroresUsuarios);
                fclose(reporte); // Cerrar el archivo
                printf("¡Usuarios cargados!\n");
               

            break;
        case '2':
            printf("Ingrese la ruta del archivo de operaciones: ");
            scanf("%s",rutaOperaciones);
            getchar(); // Consumir el carácter de nueva línea residual
            archivo = fopen(rutaOperaciones, "r");
                if (archivo == NULL) {
                    printf("Error al abrir el archivo CSV\n");
                    return 1;
                }

                // Declarar e inicializar la variable compartida
               linea_leida = 0;
                linea_leida_global = &linea_leida;
                    operacionesLeidas=0;
                    operacionesHilo1=0;
                    operacionesHilo2=0;
                    operacionesHilo3=0;
                    operacionesHilo4=0;
                // Crear 3 hilos
                HiloInfo hilosO[4];
                pthread_t tid_hilos_o[4];

                for (int i = 0; i < 4; i++) {
                    hilosO[i].id = i + 1;
                    hilosO[i].archivo = archivo;
                    pthread_mutex_init(&(hilosO[i].mutex), NULL);
                    hilosO[i].linea_leida_global = linea_leida_global;  // Pasar el puntero

                    // Crear el hilo
                    pthread_create(&(tid_hilos_o[i]), NULL, cargarOperaciones, &(hilosO[i]));
                }

                // Esperar a que finalicen los hilos
                for (int i = 0; i < 4; i++) {
                    pthread_join(tid_hilos_o[i], NULL);
                }
               
                // Cerrar el archivo
                fclose(archivo);

                //Generando reporte de carga de usuarios
                char nombre_o[100];
                char fecha_o[100];
                time_t tiempo_actual_o;
                struct tm *info_tiempo_o;
                time(&tiempo_actual_o);
                info_tiempo_o = localtime(&tiempo_actual_o);
                strftime(nombre_o,sizeof(nombre_o),"operaciones_%Y_%m_%d-%H_%M_%S.log",info_tiempo_o);
                strftime(fecha_o,sizeof(fecha_o),"%Y-%m-%d %H:%M:%S",info_tiempo_o);
                FILE *reporte_o;
                reporte_o = fopen(nombre_o, "w"); // Abrir el archivo en modo de escritura ("w")

                if (reporte_o == NULL) {
                    printf("Error al generar el archivo de reporte de operaciones.\n");
                    return 1; // Salir del programa con código de error
                }
                fprintf(reporte_o,"------------------------------------------CARGA DE OPERACIONES------------------------------------------\n");
                fprintf(reporte_o,"Fecha: %s\n",fecha_o);
                fprintf(reporte_o, "Operaciones cargadas:\n"); // Escribir en el archivo
                fprintf(reporte_o,"Hilo #1 : %d\n",operacionesHilo1-1);
                fprintf(reporte_o,"Hilo #2 : %d\n",operacionesHilo2);
                fprintf(reporte_o,"Hilo #3 : %d\n",operacionesHilo3);
                fprintf(reporte_o,"Hilo #4 : %d\n",operacionesHilo4);
                fprintf(reporte_o,"Total : %d\n",operacionesLeidas-1);
                fprintf(reporte_o,"Errores:\n");
                fprintf(reporte_o,"%s",erroresOperaciones);
                fclose(reporte_o); // Cerrar el archivo
                printf("¡Operaciones cargadas!\n");
            break;
        case '3':
                char cuenta1[100];
                char cuenta2[100];
                char monto [100];
            //Operaciones individuales
                printf("|--------------OPERACIONES INDIVIDUALES--------------|\n");
                printf("|1.Depósito                                          |\n");
                printf("|2.Retiro                                            |\n");
                printf("|3.Transferencia                                     |\n");
                printf("|4.Consultar cuenta                                  |\n");
                printf("|5.Regresar                                          |\n");
                printf("|----------------------------------------------------|\n");
                scanf("%c",&op);
                getchar(); // Consumir el carácter de nueva línea residual

                switch (op)
                {
                case '1':
                    regex_t regex;
                    int reti;
                    printf("Ingrese el número de cuenta: ");
                    scanf("%s",cuenta1);
                    getchar(); // Consumir el carácter de nueva línea residual
                    if (existeCuenta(atoi(cuenta1))){
                    printf("Ingrese el monto a depositar: ");
                    scanf("%s", monto);
                    getchar(); // Consumir el carácter de nueva línea residual
                    reti = regcomp(&regex, "^[0-9]+([.][0-9]+)?$", REG_EXTENDED);
                    reti = regexec(&regex, monto, 0, NULL, 0);

                if(reti==0){
                    deposito(atoi(cuenta1),atof(monto));
                    printf("¡Depósito realizado!\n");
                }else{
                    printf("Error: El monto <%s> no es un real positivo\n",monto);
                }

                    
                    }else{
                        printf("Error: No se encontró la cuenta <%s>\n",cuenta1);
                    }
                    break;
                case '2':
                    regex_t regex2;
                    int reti2;
                    printf("Ingrese el número de cuenta: ");
                    scanf("%s",cuenta1);
                    getchar(); // Consumir el carácter de nueva línea residual
                    if (existeCuenta(atoi(cuenta1))){
                    printf("Ingrese el monto a retirar: ");
                    scanf("%s", monto);
                    getchar(); // Consumir el carácter de nueva línea residual
                    reti2 = regcomp(&regex, "^[0-9]+([.][0-9]+)?$", REG_EXTENDED);
                    reti2 = regexec(&regex, monto, 0, NULL, 0);

                if(reti2==0){
                    retiro(atoi(cuenta1),atof(monto),0);
                }else{
                    printf("Error: El monto <%s> no es un real positivo\n",monto);
                }

                    
                    }else{
                        printf("Error: No se encontró la cuenta <%s>\n",cuenta1);
                    }
               
                break;

                case '3':
                    regex_t regex3;
                    int reti3;
                    printf("Ingrese el número de cuenta origen: ");
                    scanf("%s",cuenta1);
                    getchar(); // Consumir el carácter de nueva línea residual
                    if (existeCuenta(atoi(cuenta1))){
                        printf("Ingrese el número de cuenta destino: ");
                    scanf("%s",cuenta2);
                    getchar(); // Consumir el carácter de nueva línea residual
                    printf("Ingrese el monto a transferir: ");
                    scanf("%s", monto);
                    getchar(); // Consumir el carácter de nueva línea residual
                    reti3 = regcomp(&regex, "^[0-9]+([.][0-9]+)?$", REG_EXTENDED);
                    reti3 = regexec(&regex, monto, 0, NULL, 0);

                if(reti3==0){
                    transferencia(atoi(cuenta1),atoi(cuenta2),atof(monto),0);
                }else{
                    printf("Error: El monto <%s> no es un real positivo",monto);
                }

                    
                    }else{
                        printf("Error: No se encontró la cuenta <%s>\n",cuenta1);
                    }
                    break;
                case '4':
                    printf("Ingrese el número de cuenta: ");
                    scanf("%s",cuenta1);
                    getchar(); // Consumir el carácter de nueva línea residual
                    struct usuario* usuario;
                    usuario = getUsuario(atoi(cuenta1));
                    if(usuario!=NULL){
                        printf("\n-Número de cuenta: %d\n",usuario->no_cuenta);
                        printf("-Nombre: %s\n",usuario->nombre);
                        printf("-Saldo: %10.2f\n",usuario->saldo);
                    }else{
                        printf("No se encontró la cuenta\n");
                    }
                break;
                case '5':
                break;
                default:
                    printf("Operación inválida\n");
                    break;
                }
                    break;
        case '4':
            //Estados de cuenta
            estadoDeCuenta();
            break;
        case '5':
            printf("¡Ejecución finalizada!\n");
            exit(0);
            break;
        default:
            printf("Opción inválida\n");
            break;
    }

    }
   
    
}
