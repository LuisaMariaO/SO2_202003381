#include <stdio.h>
#include <stdlib.h>

// Definición de la estructura de un nodo
struct Error {
    char error[200];
    struct Error* siguiente;
};

// Función para crear un nuevo nodo
struct Error* crearNodo(char error[200]) {
    struct Error* nuevoNodo = (struct Error*)malloc(sizeof(struct Error));
    if (nuevoNodo == NULL) {
        printf("Error: No se pudo asignar memoria para el nuevo nodo\n");
        exit(EXIT_FAILURE);
    }

    strcopy(nuevoNodo->error,error);

    nuevoNodo->siguiente = NULL;
    return nuevoNodo;
}

// Función para insertar un nodo al final de la lista
void insertarAlFinal(struct Error** cabeza, char error[200]) {
    struct Error* nuevoNodo = crearNodo(error);
    if (*cabeza == NULL) {
        *cabeza = nuevoNodo;
    } else {
        struct Error* actual = *cabeza;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
        actual->siguiente = nuevoNodo;
    }
}

// Función para imprimir la lista
void imprimirLista(struct Error* cabeza) {
    struct Error* actual = cabeza;
    while (actual != NULL) {
        printf("%d, %s, %5.2f\n", actual->error);
        actual = actual->siguiente;
    }
    printf("NULL\n");
}