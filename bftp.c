#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8889
#define BUFFER_SIZE 1024

void *handle_client(void *client_socket);
void *server_thread(void *arg);
void *client_thread(void *arg);

int main() {
    pthread_t server_tid, client_tid;

    // Crear un hilo para el servidor
    if (pthread_create(&server_tid, NULL, server_thread, NULL) != 0) {
        perror("Error al crear el hilo del servidor");
        exit(EXIT_FAILURE);
    }

    // Crear un hilo para el cliente
    if (pthread_create(&client_tid, NULL, client_thread, NULL) != 0) {
        perror("Error al crear el hilo del cliente");
        exit(EXIT_FAILURE);
    }

    // Esperar a que los hilos terminen
    pthread_join(server_tid, NULL);
    pthread_join(client_tid, NULL);

    return 0;
}

void *server_thread(void *arg) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t thread_id;

    // Crear el socket del servidor
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Enlazar el socket del servidor a la dirección y puerto especificados
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al enlazar el socket del servidor");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_socket, 5) < 0) {
        perror("Error al escuchar en el socket del servidor");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en el puerto %d...\n", PORT);

    // Bucle principal para aceptar conexiones entrantes
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Error al aceptar la conexión del cliente");
            continue;
        }

        printf("Cliente conectado\n");

        // Crear un nuevo hilo para manejar la conexión del cliente
        if (pthread_create(&thread_id, NULL, handle_client, (void *)&client_socket) != 0) {
            perror("Error al crear el hilo para el cliente");
            close(client_socket);
        }
    }

    // Cerrar el socket del servidor
    close(server_socket);
    return NULL;
}

void *client_thread(void *arg) {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    // Crear el socket del cliente
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error al crear el socket del cliente");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Solicitar la dirección IP del servidor
    printf("Ingrese la dirección IP del servidor: ");
    scanf("%s", buffer);
    if (inet_pton(AF_INET, buffer, &server_addr.sin_addr) <= 0) {
        perror("Dirección IP inválida");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Conectar al servidor
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar al servidor");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Conectado al servidor %s\n", buffer);

    // Bucle para enviar comandos al servidor
    while (1) {
        printf("ftp> ");
        fgets(command, BUFFER_SIZE, stdin);
        command[strcspn(command, "\n")] = 0; // Eliminar el salto de línea

        // Enviar el comando al servidor
        send(client_socket, command, strlen(command), 0);

        // Recibir la respuesta del servidor
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("%s\n", buffer);
        }
    }

    // Cerrar el socket del cliente
    close(client_socket);
    return NULL;
}

void *handle_client(void *client_socket) {
    int socket = *(int *)client_socket;
    char buffer[BUFFER_SIZE];
    int bytes_read;

    // Bucle para recibir y procesar comandos del cliente
    while ((bytes_read = recv(socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_read] = '\0';
        printf("Comando recibido: %s\n", buffer);

        // Aquí se procesarán los comandos recibidos del cliente
        // ...

        // Enviar una respuesta al cliente
        char *response = "200 OK\n";
        send(socket, response, strlen(response), 0);
    }

    // Cerrar el socket del cliente
    close(socket);
    printf("Cliente desconectado\n");
    return NULL;
}
