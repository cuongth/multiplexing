#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <pthread.h>

#define PORT 8080
#define NUM_CONNECTIONS 80 //1000
#define MESSAGES_PER_CONNECTION 100//100

void* client_thread(void* arg) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char *message = "Hello Server";
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return NULL;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return NULL;
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return NULL;
    }
    
    for(int i = 0; i < MESSAGES_PER_CONNECTION; i++) {
        send(sock, message, strlen(message), 0);
        read(sock, buffer, 1024);
    }
    
    close(sock);
    return NULL;
}

int main() {
    pthread_t threads[NUM_CONNECTIONS];
    struct timeval start, end;
    
    gettimeofday(&start, NULL);
    
    // Create multiple client connections
    for(int i = 0; i < NUM_CONNECTIONS; i++) {
        pthread_create(&threads[i], NULL, client_thread, NULL);
    }
    
    // Wait for all threads to complete
    for(int i = 0; i < NUM_CONNECTIONS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    gettimeofday(&end, NULL);
    double time_taken = (end.tv_sec - start.tv_sec) * 1e6;
    time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;
    printf("Total time taken: %f seconds\n", time_taken);
    
    return 0;
}
