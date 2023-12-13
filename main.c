#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include "job.h"

#define PORT 8080
#define MAX_CLIENTS 5

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

        struct job buffer;
        memset(&buffer, 0, sizeof(buffer));


        while(1) {
            int bytes_received = recv(new_socket, &buffer, sizeof(struct job), 0);
            if (bytes_received < 0) {
                if(errno == EBADF) {
                    perror("Socket is not valid");
                } else {
                    perror("Échec de la réception des données");
                }
                exit(EXIT_FAILURE);
            }

            if(bytes_received == 0) {
                break; // Connection closed by client
            }

            FILE *printerFile;
            int ppm = 60;
            char name[20];
             sprintf(name, "imprimante%d.log", buffer.printerId);
             printf("%s",name);
            if(buffer.printerId == 1 ){
                printerFile = fopen( name, "a" );
                if ( printerFile == NULL ) {
                    perror( "Cannot open file\n" );
                    exit(3);
                }

                fprintf( printerFile, "Imprime %d pages : ", buffer.pages);
                fflush(printerFile);

                //nb_pages = atoi(nb_pages_str) ;
                sleep(buffer.pages * 60/ppm) ;
                fprintf( printerFile, "Fait en %d secondes\n", buffer.pages *60/ppm );
                fflush(printerFile);
                printf("Fin impression %d \n", buffer.pages) ;
                fflush(stdout) ;
            }else{
                printerFile = fopen( "resultat_imprimante1.txt", "a" );
                if ( printerFile == NULL ) {
                    perror( "Cannot open file\n" );
                    exit(3);
                }
            }
            printf("Received: Printer ID: %d, Pages: %d, Priority: %d\n", buffer.printerId, buffer.pages, buffer.priority);
        }
    }

    return 0;
}
