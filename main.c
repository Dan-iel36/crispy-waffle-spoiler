#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "job.h"

#define PORT 8080
#define MAX_CLIENTS 5
#define PRINTERPORT 8088
#define PRINTERHOST "127.0.0.1"

void* jobForwarder(struct job* myjob){
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Création du socket client
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Échec de la création du socket client");
        exit(EXIT_FAILURE);
    }

    //Sockets et TCP/IP
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PRINTERPORT);

    // Convertir l'adresse IPv4 et affecter au socket client
    if (inet_pton(AF_INET, PRINTERHOST,
                  &serv_addr.sin_addr) <= 0) {
        perror("Adresse invalide / Adresse non supportée");
        exit(EXIT_FAILURE);
    }

    // Connexion au serveur
    if (connect(sock, (struct sockaddr *) &serv_addr,
                sizeof(serv_addr)) < 0) {
        perror("Connexion échouée");
        exit(EXIT_FAILURE);
    }



    //while(1){
        send(sock, myjob, sizeof(struct job), 0);
        printf("%d, %d, %d, %s here for send\n", sock, myjob->priority, myjob->pages, myjob->printerId);
        fflush(stdout);
        sleep(rand() % 20 + 1);

        //close(sock);
        //breakout of loop

    //}
close(sock);
    return NULL;
}

void* treatJobWithAThread(void* arg){
    struct job* buffer = (struct job*) arg;
    FILE *printerFile;
    int ppm = 60;
    char name[20];
    sprintf(name, "imprimante_%s.log", buffer->printerId);
    printf("%s \n",name);

    printerFile = fopen( name, "a" );
    if ( printerFile == NULL ) {
        perror( "Cannot open file\n" );
        exit(3);
    }

    fprintf( printerFile, "Imprime %d pages : ", buffer->pages);
    fflush(printerFile);

    sleep(buffer->pages * 60/ppm) ;
    fprintf( printerFile, "Fait en %d secondes\n", buffer->pages *60/ppm );
    fflush(printerFile);
    printf("Fin impression %d \n", buffer->pages) ;
    fflush(stdout) ;
    jobForwarder(buffer);

    free(buffer);
    //close(sock);
    return NULL;
}



int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Échec de la création du socket serveur");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Échec de la liaison du socket serveur");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Échec de l'écoute du socket serveur");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("Échec de l'acceptation de la connexion");
            exit(EXIT_FAILURE);
        }

        struct job* buffer = (struct job*) malloc(sizeof(struct job));
        memset(buffer, 0, sizeof(struct job));

        while(1) {
            int bytes_received = recv(new_socket, buffer, sizeof(struct job), 0);
            if (bytes_received < 0) {
                if(errno == EBADF) {
                    perror("Socket is not valid");
                } else {
                    perror("Échec de la réception des données");
                }
                exit(EXIT_FAILURE);
            }

            if(bytes_received == 0) {
                printf("connection closed by client");
                break; // Connection closed by client
            }

            pthread_t thread_id;
            struct job* buffer_copy = (struct job*) malloc(sizeof(struct job));
            memcpy(buffer_copy, buffer, sizeof(struct job));

            pthread_create(&thread_id, NULL, treatJobWithAThread, buffer_copy);
            //pthread_join(thread_id, NULL);

            printf("Received: Printer ID: %s, Pages: %d, Priority: %d, ThreadInCharge: %lu \n", buffer->printerId, buffer->pages, buffer->priority,  (unsigned long int) thread_id);

        }

        free(buffer);
    }

    return 0;
}
