#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 8080
#define MAX_REQUEST_SIZE 1024

void serve_client(int client_socket) {
    char response[1024] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    strcat(response, "<html><body><h1>Click to Open Application</h1>");
    strcat(response, "<a href=\"/open\">Open Application</a><br>");
    strcat(response, "<a href=\"/download\">Download Application</a>");
    strcat(response, "</body></html>");

    send(client_socket, response, strlen(response), 0);
    close(client_socket);
}

void open_application() {
    // Code to open the application goes here
    printf("Opening application...\n");
}

void *handle_client(void *arg) {
    int client_socket = *((int *)arg);

    char buffer[MAX_REQUEST_SIZE] = {0};
    if (read(client_socket, buffer, MAX_REQUEST_SIZE) <= 0) {
        perror("read");
        close(client_socket);
        return NULL;
    }

    // Check if the request is for opening the application
    if (strstr(buffer, "GET /open") != NULL) {
        // Open the application only if the request is for "/open"
        open_application();
    } else if (strstr(buffer, "GET /download") != NULL) {
        // Send a download link if the request is for "/download"
        char download_response[1024] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        strcat(download_response, "<html><body><h1>Download Application</h1>");
        strcat(download_response, "<a href=\"/path/to/your/application\">Download Here</a>");
        strcat(download_response, "</body></html>");

        send(client_socket, download_response, strlen(download_response), 0);
    } else {
        // Serve the client with the web page
        serve_client(client_socket);
    }

    close(client_socket);
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Accept incoming connections
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Create a new thread to handle the client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)&new_socket) < 0) {
            perror("pthread_create");
            close(new_socket);
        }
    }

    return 0;
}
