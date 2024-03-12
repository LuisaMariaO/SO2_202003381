#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

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
struct usuario usuarios[1000];
struct operacion operaciones[1000];


void cargarUsuarios(){
FILE *fp;
    fp = fopen("/home/luisa/Descargas/prueba_usuarios.csv", "r");

    char row[1000];
    char *token;

    fgets(row, 1000, fp);

    int n = 0;

    while (feof(fp) != true)
    {
        fgets(row, 1000, fp);

        token = strtok(row, ",");
        usuarios[n].no_cuenta = atoi(token);

        token = strtok(NULL, ",");
        sprintf(usuarios[n].nombre, "%s", token);

        token = strtok(NULL, ",");
        usuarios[n].saldo = atof(token);

        n++;
    }

    for(int i = 0; i < n; i++){
        printf("No. de cuenta: %d   Nombre: %s   Saldo: %5.2f\n", 
            usuarios[i].no_cuenta, usuarios[i].nombre, usuarios[i].saldo);
    }

    
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
    int op;
    printf("Practica 2 - Sistemas Operativos 2\n");
    printf("Luisa Maria Ortiz Romero - 202003381\n");
    while(true){
        printf("|-----------------------MENU-----------------------|\n");
        printf("|1.Cargar usuarios                                 |\n");
        printf("|2.Operaciones individuales                        |\n");
        printf("|3.Cargar operaciones                              |\n");
        printf("|4. Salir                                          |\n");
        printf("|--------------------------------------------------|\n");
        scanf("%d",&op);

         switch(op){
        case 1:
            cargarUsuarios();
            break;
        case 2:
            
            break;
        case 3:
            cargarOperaciones();
            break;
        case 4:
            exit(0);
    }

    }
   
    
}