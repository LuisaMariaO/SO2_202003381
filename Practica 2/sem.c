#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NOMBRE_ARCHIVO "data.csv" // Reemplazar con la ruta del archivo CSV

typedef struct {
  int numeroHilo;
  FILE *archivo;
  int *numeroLinea;
  pthread_mutex_t mutex;
} ParametroHilo;
pthread_mutex_t lock;
void *leerLinea(void *arg);

int main() {
  pthread_t hilos[3];
  ParametroHilo parametros[3];

  // Inicializar el mutex
  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, NULL);

  // Abrir el archivo CSV
  FILE *archivo = fopen("/home/luisa/Descargas/prueba_usuarios.csv", "r");
  if (archivo == NULL) {
    printf("Error al abrir el archivo CSV.\n");
    return 1;
  }

  // Crear y lanzar 3 hilos
  for (int i = 0; i < 3; i++) {
    parametros[i].numeroHilo = i + 1;
    parametros[i].archivo = archivo;
    parametros[i].numeroLinea = (int *)malloc(sizeof(int)); // Asignar memoria para el número de línea
    parametros[i].mutex = mutex;

    pthread_create(&hilos[i], NULL, leerLinea, &parametros[i]);
  }

  // Esperar a que finalicen los hilos
  for (int i = 0; i < 3; i++) {
    pthread_join(hilos[i], NULL);
  }

  // Cerrar el archivo
  fclose(archivo);

  // Destruir el mutex
  pthread_mutex_destroy(&mutex);

  return 0;
}

void *leerLinea(void *arg) {
    //wait 
    pthread_mutex_lock(&lock); //Bloquear
    printf("Inicio - %s\n", (char*) arg); 
    ParametroHilo *parametro = (ParametroHilo *)arg;
    int numeroHilo = parametro->numeroHilo;
    FILE *archivo = parametro->archivo;
    int *numeroLinea = parametro->numeroLinea;
    pthread_mutex_t mutex = parametro->mutex;
    char linea[1024];

    while (fgets(linea, sizeof(linea), archivo) != NULL) {
    printf("Hilo %d: %s (Línea %d)\n", numeroHilo, linea, *numeroLinea);

  
    //signal 
    printf("Termino - %s\n", (char*) arg); 
    pthread_mutex_unlock(&lock); 
  
  // Leer líneas del archivo
 

    (*parametro->numeroLinea)++;
    
    // Desbloquear el mutex después de acceder al archivo
    pthread_mutex_unlock(&mutex);
  }

  return NULL;
}
