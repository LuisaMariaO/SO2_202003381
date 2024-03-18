# Manual técnico - Práctica 2
## Laboratorio de Sistemas Operativos 2
Luisa María Ortíz Romero - 202003381 

## Estrategia de sincronización de hilos
Para lograr la lectura de archivos con la cantidad de hilos solicitados, se utilizó un semáforo con mutex, el struct del hilo almacena la siguiente información:
~~~
typedef struct hilo_info {
  int id;                      // Identificador del hilo
  FILE *archivo;                // Puntero al archivo CSV
  pthread_mutex_t mutex;       // Mutex para sincronización
  int *linea_leida_global;     // Puntero a la variable compartida
} HiloInfo;
~~~
Se utilzaron el archivo y el contador de líneas como apuntadores para que todos los hilos accedieran al mismo espacio de memoria y asegurar la concurrencia.
## Almacenamiento de usuarios
Para almacenar los usuarios se utilizó una lista enlazada simple de structs que tenían la siguiente estructura:
~~~
struct usuario {
    int no_cuenta;
    char nombre[100];
    float saldo;
    struct usuario* siguiente; //Apuntados al siguiente en la lista
};
~~~
En *main.c* se tiene una variable global a la cabeza de la lista enlazada para podr acceder a los datos:
~~~
struct usuario* cabezaUsuarios = NULL;
~~~
### Métodos que acceden a la lista enlazada de usuarios
~~~
void insertarUsuario(int numeroCuenta, char nombre[100], float saldo) {
  struct usuario* nuevoNodo = (struct usuario*)malloc(sizeof(struct usuario));
  nuevoNodo->no_cuenta = numeroCuenta;
  strcpy(nuevoNodo->nombre, nombre);
  nuevoNodo->saldo = saldo;
  nuevoNodo->siguiente = cabezaUsuarios;
  cabezaUsuarios = nuevoNodo;
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
~~~
## Código de las transacciones
~~~
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
~~~
Los métodos que reciben como parámetro *línea* reciben 0 si se está realizando una operación individual, de esta manera el programa sabe si imprimir los errores en consola o se almacena para generar reportesa de carga.

## Utilización de mutex
La forma de utilización aplica para la carga de usuarios y de transacciones. Se utiliza un sleep de 0.5 en la carga de usuarios y de 0.6 en la carga de transacciones, esto para evitar que un solo hilo haga todo el trabajo y ayudar a la mejora de la concurrencia.
~~~
void* cargarOperaciones(void *arg){
HiloInfo *info_hilo = (HiloInfo *)arg;
 
  // Bloquear el mutex antes de acceder a las variables compartidas
  pthread_mutex_lock(&(info_hilo->mutex));

  
    

    ...
    sleep(0.6);
    // Desbloquear el mutex antes de salir del bucle
    pthread_mutex_unlock(&(info_hilo->mutex));
    
  }
~~~