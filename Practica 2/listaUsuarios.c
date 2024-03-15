#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <string.h>
// Definición de la estructura de un nodo
struct Usuario {
    int no_cuenta;
    char nombre[100];
    float saldo;
    struct Usuario* siguiente;
};

// Función para crear un nuevo nodo
struct Usuario* crearNodo(int no_cueta, char nombre[100], float saldo) {
    struct Usuario* nuevoNodo = (struct Usuario*)malloc(sizeof(struct Usuario));
    if (nuevoNodo == NULL) {
        printf("Error: No se pudo asignar memoria para el nuevo nodo\n");
        exit(EXIT_FAILURE);
    }
    nuevoNodo->no_cuenta = no_cueta;
    strcpy(nuevoNodo->nombre,nombre);
    nuevoNodo->saldo = saldo;
    nuevoNodo->siguiente = NULL;
    return nuevoNodo;
}

// Función para insertar un nodo al final de la lista
void insertarAlFinal(struct Usuario** cabeza, int no_cueta, char nombre[100], float saldo) {
    struct Usuario* nuevoNodo = crearNodo(no_cueta, nombre, saldo);
    if (*cabeza == NULL) {
        *cabeza = nuevoNodo;
    } else {
        struct Usuario* actual = *cabeza;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
        actual->siguiente = nuevoNodo;
    }
}

// Función para imprimir la lista
void imprimirLista(struct Usuario* cabeza) {
    struct Usuario* actual = cabeza;
    while (actual != NULL) {
        printf("%d, %s, %5.2f\n", actual->no_cuenta, actual->nombre, actual->saldo);
        actual = actual->siguiente;
    }
    printf("NULL\n");
}

//Función para buscar números de cuenta repetidos
bool existeCuenta(struct Usuario* cabeza, int cuenta) {
    struct Usuario* actual = cabeza;
    while (actual != NULL) {
        if(actual->no_cuenta==cuenta){
            return true;
        }
        actual = actual->siguiente;
    }
    return false;
}