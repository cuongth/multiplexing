#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_EVENTS 1000    // Maximum number of events to handle at once
#define PORT 8080         // Server port number
#define BUFFER_SIZE 1024  // Buffer size for reading/writing data

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    struct epoll_event ev, events[MAX_EVENTS];

    // Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket to non-blocking mode
    // This allows us to handle multiple connections efficiently
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

    // Create epoll instance
    // The argument to epoll_create1(0) is ignored, but must be greater than zero
    int epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // Add server socket to epoll
    ev.events = EPOLLIN;              // Monitor for incoming connections
    ev.data.fd = server_fd;           // Store the server socket fd
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("epoll_ctl: server_fd");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Wait for events on any registered file descriptor
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            break;
        }

        // Process all events received
        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == server_fd) {
                // Handle new connection(s)
                int new_socket;
                
                // Accept all pending connections
                while ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                       &addrlen)) > 0) {
                    
                    // Set new socket to non-blocking
                    flags = fcntl(new_socket, F_GETFL, 0);
                    fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);

                    // Add new socket to epoll
                    ev.events = EPOLLIN;          // Monitor for incoming data
                    ev.data.fd = new_socket;      // Store the new socket fd
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, new_socket, &ev) == -1) {
                        perror("epoll_ctl: new_socket");
                        close(new_socket);
                    }
                }
                // Check if accept failed for a reason other than no more connections
                if (new_socket < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("accept failed");
                }
            } else {
                // Handle client data
                int fd = events[n].data.fd;
                int valread = read(fd, buffer, BUFFER_SIZE);
                
                if (valread <= 0) {
                    // Connection closed or error
                    // Remove from epoll and close socket
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                } else {
                    // Echo received data back to client
                    send(fd, buffer, valread, 0);
                }
            }
        }
    }

    return 0;
}
