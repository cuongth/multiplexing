#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define MAX_CLIENTS 1000
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_socket[MAX_CLIENTS], max_clients = MAX_CLIENTS;
    int activity, i, sd, max_sd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    fd_set readfds;
    struct timeval start, end;
    
    // Initialize all client_socket[] to 0
    for (i = 0; i < max_clients; i++) {
        client_socket[i] = 0;
    }
    
    // Create master socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, addrlen) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    while(1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;
        
        // Add child sockets to set
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if(sd > 0)
                FD_SET(sd, &readfds);
            if(sd > max_sd)
                max_sd = sd;
        }
        
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        
        if (activity < 0) {
            printf("select error");
        }
        
        // it's an incoming connection
        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            flags = fcntl(new_socket, F_GETFL, 0);
            fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);
            
            // Add new socket to array of sockets
            for (i = 0; i < max_clients; i++) {
                if(client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    break;
                }
            }
        }
        
        // Check for I/O operation on client sockets
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            
            if (FD_ISSET(sd, &readfds)) {
                int valread = read(sd, buffer, BUFFER_SIZE);
                if (valread == 0) {
                    close(sd);
                    client_socket[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }
    }
    
    return 0;
}
