#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_CLIENTS 1000    // Maximum number of clients the server can handle
#define PORT 8080          // Server port number
#define BUFFER_SIZE 1024   // Buffer size for reading/writing data

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    struct pollfd fds[MAX_CLIENTS + 1];  // Array of file descriptors to monitor (+1 for server socket)
    int nfds = 1;                        // Number of file descriptors being monitored

    // Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This allows us to handle multiple connections without blocking
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    // Configure server address
    address.sin_family = AF_INET;            // IPv4
    address.sin_addr.s_addr = INADDR_ANY;    // Accept connections on any interface
    address.sin_port = htons(PORT);          // Convert port to network byte order

    // Bind socket to address and port
    if (bind(server_fd, (struct sockaddr *)&address, addrlen) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // Initialize poll structure
    // First slot is always for the server socket
    memset(fds, 0, sizeof(fds));
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;    // Monitor for incoming connections

    while (1) {
        // Wait for events on any socket (timeout = -1 means wait indefinitely)
        int activity = poll(fds, nfds, -1);
        
        if (activity < 0) {
            perror("poll error");
            break;
        }

        // if there's a new connection on the server socket
        if (fds[0].revents & POLLIN) {
            int new_socket;
            
            // Accept all pending connections
            while ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                   &addrlen)) > 0) {
                
                // Set new socket to non-blocking
                flags = fcntl(new_socket, F_GETFL, 0);
                fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);

                // Add new socket to poll array if there's space
                if (nfds < MAX_CLIENTS + 1) {
                    fds[nfds].fd = new_socket;
                    fds[nfds].events = POLLIN;    // Monitor for incoming data
                    nfds++;
                } else {
                    // If maximum clients reached, reject connection
                    close(new_socket);
                }
            }
            // if accept failed for a reason other than no more connections
            if (new_socket < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("accept failed");
            }
        }

        // Check all client sockets for data
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                int valread = read(fds[i].fd, buffer, BUFFER_SIZE);
                
                if (valread <= 0) {
                    // Connection closed or error
                    close(fds[i].fd);
                    
                    // Remove from poll array by shifting remaining entries
                    for (int j = i; j < nfds - 1; j++) {
                        fds[j] = fds[j + 1];
                    }
                    nfds--;
                    i--;    // Adjust loop counter as we removed an entry
                } else {
                    // Echo received data back to client
                    send(fds[i].fd, buffer, valread, 0);
                }
            }
        }
    }

    return 0;
}
