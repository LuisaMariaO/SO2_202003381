#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NOMBRE_ARCHIVO "data.csv"

typedef struct hilo_info {
  int id;                      // Identificador del hilo
  FILE *archivo;                // Puntero al archivo CSV
  pthread_mutex_t mutex;       // Mutex para sincronización
  int *linea_leida_global;     // Puntero a la variable compartida
} HiloInfo;

int usuariosLeidos;

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

void *hilo_funcion(void *arg) {
  HiloInfo *info_hilo = (HiloInfo *)arg;
  char *linea;

  // Bloquear el mutex antes de acceder a las variables compartidas
  pthread_mutex_lock(&(info_hilo->mutex));

  while ((linea = leer_linea(info_hilo->archivo)) != NULL) {
    // Acceder y modificar la variable compartida usando el puntero pasado
    *(info_hilo->linea_leida_global) = *(info_hilo->linea_leida_global) + 1;
    usuariosLeidos++;

    // Imprimir la línea, el número de hilo y el número de línea
    printf("Hilo %d: Línea %d: %s\n", info_hilo->id, *(info_hilo->linea_leida_global), linea);

    // Liberar la memoria de la línea leída
    free(linea);

    // Desbloquear el mutex antes de salir del bucle
    pthread_mutex_unlock(&(info_hilo->mutex));
  }

  // Cancelar el hilo
  pthread_exit(NULL);

  return NULL;
}

int main() {
  // Abrir el archivo CSV
  FILE *archivo = fopen("/home/luisa/Descargas/prueba_usuarios.csv", "r");
  if (archivo == NULL) {
    printf("Error al abrir el archivo CSV\n");
    return 1;
  }

  // Declarar e inicializar la variable compartida
  int linea_leida = 0;
  int *linea_leida_global = &linea_leida;
    usuariosLeidos=0;
  // Crear 3 hilos
  HiloInfo hilos[3];
  pthread_t tid_hilos[3];

  for (int i = 0; i < 3; i++) {
    hilos[i].id = i + 1;
    hilos[i].archivo = archivo;
    pthread_mutex_init(&(hilos[i].mutex), NULL);
    hilos[i].linea_leida_global = linea_leida_global;  // Pasar el puntero

    // Crear el hilo
    pthread_create(&(tid_hilos[i]), NULL, hilo_funcion, &(hilos[i]));
  }

  // Esperar a que finalicen los hilos
  for (int i = 0; i < 3; i++) {
    pthread_join(tid_hilos[i], NULL);
  }

  // Cerrar el archivo
  fclose(archivo);

  return 0;
}
