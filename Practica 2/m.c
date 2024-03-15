#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_USERS 1000
#define MAX_OPERATIONS 1000
#define MAX_ERROR_MESSAGES 100

// Estructuras de datos
typedef struct {
  int account_number;
  char name[100];
  double balance;
} User;

typedef struct {
  int operation_type; // 1: deposit, 2: withdraw, 3: transfer
  int account_from;
  int account_to;
  double amount;
} Operation;

User users[MAX_USERS];
Operation operations[MAX_OPERATIONS];
char error_messages[MAX_ERROR_MESSAGES][100];
int error_count = 0;

pthread_mutex_t users_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t operations_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t error_mutex = PTHREAD_MUTEX_INITIALIZER;

// Funciones auxiliares
void add_error_message(const char* message) {
  pthread_mutex_lock(&error_mutex);
  if (error_count < MAX_ERROR_MESSAGES) {
    strcpy(error_messages[error_count], message);
    error_count++;
  }
  pthread_mutex_unlock(&error_mutex);
}

User* find_user_by_account_number(int account_number) {
  for (int i = 0; i < MAX_USERS; i++) {
    if (users[i].account_number == account_number) {
      return &users[i];
    }
  }
  return NULL;
}

// Función para validar un registro de usuario
bool validate_user_record(const char* line) {
  int account_number;
  char name[100];
  double balance;
  return sscanf(line, "%d,%99[^,],%lf", &account_number, name, &balance) == 3;
}

// Función para validar un registro de operación
bool validate_operation_record(const char* line) {
  int operation_type;
  int account_from;
  int account_to;
  double amount;
  return sscanf(line, "%d,%d,%d,%lf", &operation_type, &account_from, &account_to, &amount) == 4 && (operation_type == 1 || operation_type == 2 || operation_type == 3); // Validar tipo de operación
}

void* load_users(void* arg) {
    const char* filename = (const char*)arg;
    char full_path[256];
    sprintf(full_path, "Entradas/%s", filename); // Construye la ruta completa

    FILE* file = fopen(full_path, "r");
    if (file == NULL) {
        add_error_message("Failed to open users file");
        return NULL;
    }

    char line[256];
    int successful_loads = 0;
    while (fgets(line, sizeof(line), file)) {
        if (!validate_user_record(line)) {
        add_error_message("Invalid user record");
        continue;
        }

        pthread_mutex_lock(&users_mutex);
        User* user = find_user_by_account_number(atoi(strtok(strdup(line), ","))); // Extrae el numero de cuenta
        if (user == NULL) {
        for (int i = 0; i < MAX_USERS; i++) {
            if (users[i].account_number == 0) {
            sscanf(line, "%d,%99[^,],%lf", &users[i].account_number, users[i].name, &users[i].balance);
            successful_loads++;
            break;
            }
        }
        } else {
        add_error_message("Duplicate account number");
        }
        pthread_mutex_unlock(&users_mutex);
    }

    fclose(file);

    // Mostrar mensaje de éxito con la cantidad de usuarios cargados
    if (successful_loads > 0) {
        printf("Usuarios cargados exitosamente: %d\n", successful_loads);
    }

    return NULL;
}

void* load_operations(void* arg) {
    const char* filename = (const char*)arg;
    char full_path[256];
    sprintf(full_path, "Entradas/%s", filename); // Construye la ruta completa

    FILE* file = fopen(full_path, "r");
    if (file == NULL) {
        add_error_message("Failed to open operations file");
        return NULL;
    }

    char line[256];
    int successful_loads = 0;
    while (fgets(line, sizeof(line), file)) {
        if (!validate_operation_record(line)) {
        add_error_message("Invalid operation record");
        continue;
        }

        pthread_mutex_lock(&operations_mutex);
        Operation operation;
        sscanf(line, "%d,%d,%d,%lf", &operation.operation_type, &operation.account_from, &operation.account_to, &operation.amount);
        for (int i = 0; i < MAX_OPERATIONS; i++) {
        if (operations[i].operation_type == 0) {
            operations[i] = operation;
            successful_loads++;
            break;
        }
        }
        pthread_mutex_unlock(&operations_mutex);
    }

    fclose(file);

    // Mostrar mensaje de éxito con la cantidad de operaciones cargadas
    if (successful_loads > 0) {
        printf("Operaciones cargadas exitosamente: %d\n", successful_loads);
    }

    return NULL;
}

void* process_operations(void* arg) {
    int thread_id = *((int*)arg);
    int start_index = thread_id * (MAX_OPERATIONS / 4);
    int end_index = (thread_id + 1) * (MAX_OPERATIONS / 4);

    for (int i = start_index; i < end_index; i++) {
        pthread_mutex_lock(&operations_mutex);
        Operation* operation = &operations[i];
        pthread_mutex_unlock(&operations_mutex);

        if (operation->operation_type != 0) {
        pthread_mutex_lock(&users_mutex);
        User* user_from = find_user_by_account_number(operation->account_from);
        User* user_to = find_user_by_account_number(operation->account_to);
        bool success = true;

        if (user_from == NULL || user_to == NULL) {
            add_error_message("Invalid account number in operation");
            success = false;
        } else {
            if (operation->operation_type == 1) {
            // Deposit
            user_to->balance += operation->amount;
            } else if (operation->operation_type == 2) {
            // Withdraw
            if (user_from->balance >= operation->amount) {
                user_from->balance -= operation->amount;
            } else {
                add_error_message("Insufficient balance for withdrawal");
                success = false;
            }
            } else if (operation->operation_type == 3) {
            // Transfer
            if (user_from->balance >= operation->amount) {
                user_from->balance -= operation->amount;
                user_to->balance += operation->amount;
            } else {
                add_error_message("Insufficient balance for transfer");
                success = false;
            }
            }
        }

        // Mostrar mensaje de éxito o error para la operación
        if (success) {
            printf("Operación realizada exitosamente: %d\n", i);
        } else {
            printf("Operación fallida: %d\n", i);
        }

        pthread_mutex_unlock(&users_mutex);
        }
    }

    return NULL;
}

int main() {
    pthread_t user_threads[3];
    pthread_t operation_threads[4];

    pthread_create(&user_threads[0], NULL, load_users, (void*)"/home/luisa/Descargas/prueba_usuarios.csv");
    pthread_create(&user_threads[1], NULL, load_users, (void*)"/home/luisa/Descargas/prueba_usuarios.csv");
    pthread_create(&user_threads[2], NULL, load_users, (void*)"/home/luisa/Descargas/prueba_usuarios.csv");

    pthread_create(&operation_threads[0], NULL, load_operations, (void*)"/home/luisa/Descargas/prueba_transacciones.csv");
    pthread_create(&operation_threads[1], NULL, load_operations, (void*)"/home/luisa/Descargas/prueba_transacciones.csv");
    pthread_create(&operation_threads[2], NULL, load_operations, (void*)"/home/luisa/Descargas/prueba_transacciones.csv");
    pthread_create(&operation_threads[3], NULL, load_operations, (void*)"/home/luisa/Descargas/prueba_transacciones.csv");

    for (int i = 0; i < 3; i++) {
        pthread_join(user_threads[i], NULL);
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(operation_threads[i], NULL);
    }

    // Procesamiento de operaciones

    int thread_ids[4] = {0, 1, 2, 3};
    for (int i = 0; i < 4; i++) {
        pthread_create(&operation_threads[i], NULL, process_operations, &thread_ids[i]);
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(operation_threads[i], NULL);
    }

    // Generación de reportes
    FILE* user_report_file = fopen("user_report.csv", "w");
    if (user_report_file != NULL) {
        fprintf(user_report_file, "no_cuenta,nombre,saldo\n");
        for (int i = 0; i < MAX_USERS; i++) {
            if (users[i].account_number != 0) {
                fprintf(user_report_file, "%d,%s,%.2f\n", users[i].account_number, users[i].name, users[i].balance);
            }
        }
        fclose(user_report_file);
    }

    FILE* error_report_file = fopen("error_report.csv", "w");
    if (error_report_file != NULL) {
        fprintf(error_report_file, "Errores:\n");
        for (int i = 0; i < error_count; i++) {
            fprintf(error_report_file, "- %s\n", error_messages[i]);
        }
        fclose(error_report_file);
    }

    return 0;
}
