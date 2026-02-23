/* TERMS OF USE
 * This source code is subject to the terms of the MIT License.
 * Copyright(c) 2026 Vladimir Vasilich Tregub
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

#include "cc-wavelet.h"

int bytes_received = 0;

#define PORT 8080
#define SOCKET_ERROR -1

#define CHUNKSIZE 0x800

void build_http_response_with_file(int client_socket, char* file_name) {

    char response[1024];

    char fullpath[255];
    FILE* file = NULL;
    int res = 0;

    if (strlen(file_name) == 0) {
        strcpy(fullpath, "index.html");
    }
    else {
        file = fopen(file_name, "rb");
        if (file == NULL) {
            memcpy(fullpath, file_name, strlen(file_name));
            if (file_name[strlen(file_name)] == 0x2F)
                memcpy(fullpath + strlen(file_name), "index.html", 11);
            else
                memcpy(fullpath + strlen(file_name), "/index.html", 12);
        }
        else {
            memcpy(fullpath, file_name, strlen(file_name) + 1);
            fclose(file);
        }
    }
    file = fopen(fullpath, "rb");
    if (file == NULL) {
        // File not found, send 404 response
        sprintf(response, "HTTP/1.1 404 Not Found\r\n\r\n");
        send(client_socket, response, (int)strlen(response), 0);
    }
    else {
        // File found, send 200 OK response
        sprintf(response, "HTTP/1.1 200 OK\r\n\r\n");
        send(client_socket, response, (int)strlen(response), 0);
        // Send the file content
        char file_buffer[1024];
        int bytes_read = 0;
        int total_sent = 0;
        do {
            bytes_read = (int)fread(file_buffer, sizeof(char), 1024, file);
            int sent = send(client_socket, file_buffer, bytes_read, 0);
            total_sent += sent;
        } while (bytes_read > 0);

        fclose(file);
    }
}

int width = 0;
int height = 0;
int horLevels = 0;
int vertLevels = 0;
char* received = NULL;
void handle_client(int client_socket) {
    char request[CHUNKSIZE] = { 0 };
    bytes_received = 0;
    bytes_received = recv(client_socket, request, CHUNKSIZE, 0);
    if (bytes_received > 0)
    {
        printf("Bytes received: %d\n", bytes_received);
        char* query_method = strtok(request, " ");
        if (!strcmp(query_method, "GET")) {
            char* file_name;
            char* query_string = strtok(NULL, " ");
            if (query_string[1] == 0x3F)
            {
                char response[256];
                sprintf(response, "HTTP/1.1 200 OK\r\n\r\n");
                send(client_socket, response, (int)strlen(response), 0);
                int sent = send(client_socket, query_string, strlen(query_string), 0);
                width = atoi(query_string + 8);
                char* ptr_height = strstr(query_string, "height=") + 7;
                height = atoi(ptr_height);
                char* ptr_horLevels = strstr(query_string, "horLevels=") + 10;
                horLevels = atoi(ptr_horLevels);
                char* ptr_vertLevels = strstr(query_string, "vertLevels=") + 11;
                vertLevels = atoi(ptr_vertLevels);
                return;
            }
            file_name = strtok(query_string, " ");
            build_http_response_with_file(client_socket, file_name + 1);
        }
        if (!strcmp(query_method, "POST")) {
            char* query_route = strtok(NULL, " ");
            if (!strcmp(query_route, "/DWT")) {
                char* ptr_contentlength = strstr(request + 10, "Content-Length: ");
                if (ptr_contentlength == 0) {
                    printf("No Content-Length header found, exit\n");
                    exit(-1);
                }
                int contentlength = atoi(ptr_contentlength + strlen("Content-Length: "));
                int total_received = 0;
                char* received = calloc(contentlength, 1);
                if ((contentlength + bytes_received) <= CHUNKSIZE) {
                    memcpy(received, request + bytes_received - contentlength, contentlength);
                    total_received = contentlength;
                }
                else
                {
                    int ixb = 0; // body origin index
                    for (; ixb <= bytes_received - 4; ++ixb) {
                        if (*(request + ixb + 0) == 0x0d && *(request + ixb + 1) == 0x0a &&
                            *(request + ixb + 2) == 0x0d && *(request + ixb + 3) == 0x0a)
                            break;
                    }
                    if (ixb > bytes_received - 4) {
                        printf("DWT xhr seems to never end with (CR-LF-CR-LF), exit\n");
                        exit(-1);
                    }
                    else
                    {
                        memcpy(received, request + ixb + 4, bytes_received - ixb - 4);
                        total_received = bytes_received - ixb - 4;
                        do {
                            bytes_received = recv(client_socket, received + total_received, CHUNKSIZE, 0);
                            if (bytes_received == SOCKET_ERROR) {
                                printf("recv error in DWT xhr: %d\n", errno);
                                break;
                            }
                            total_received += bytes_received;
                        } while (total_received < contentlength);
                    }
                }
                printf("...%d of %d\n", total_received, contentlength);
                char response[256];
                sprintf(response, "HTTP/1.1 200 OK\r\n\r\n");
                int ret = send(client_socket, response, (int)strlen(response), 0);
                if (ret == SOCKET_ERROR) {
                    printf("'send'1 (when handling POST/DWT): %d", errno);
                    exit(-1);
                }

                int execTime = forward_transform(received, width, height, horLevels, vertLevels);

                ret = send(client_socket, received, contentlength, 0);
                if (ret == SOCKET_ERROR) {
                    printf("'send'2 (when handling POST): %d", errno);
                    exit(-1);
                }

                double execTime_ms = 1000 * (double)(execTime) / CLOCKS_PER_SEC;
                printf("dwt execution time: %f ms\n", execTime_ms);

                free(received);
                received = NULL;
            }
            else if (!strcmp(query_route, "/iDWT")) {
                char* ptr_contentlength = strstr(request + 11, "Content-Length: ");
                if (ptr_contentlength == 0) {
                    printf("No Content-Length header found, exit\n");
                    exit(-1);
                }
                int contentlength = atoi(ptr_contentlength + strlen("Content-Length: "));
                int total_received = 0;
                char* received = calloc(contentlength, 1);
                if ((contentlength + bytes_received) <= CHUNKSIZE) {
                    memcpy(received, request + bytes_received - contentlength, contentlength);
                    total_received = contentlength;
                }
                else
                {
                    int ixb = 0; // body origin index
                    for (; ixb <= bytes_received - 4; ++ixb) {
                        if (*(request + ixb + 0) == 0x0d && *(request + ixb + 1) == 0x0a &&
                            *(request + ixb + 2) == 0x0d && *(request + ixb + 3) == 0x0a)
                            break;
                    }
                    if (ixb > bytes_received - 4) {
                        printf("iDWT xhr seems to never end with (CR-LF-CR-LF), exit\n");
                        exit(-1);
                    }
                    else
                    {
                        memcpy(received, request + ixb + 4, bytes_received - ixb - 4);
                        total_received = bytes_received - ixb - 4;
                        do {
                            bytes_received = recv(client_socket, received + total_received, CHUNKSIZE, 0);
                            if (bytes_received == SOCKET_ERROR) {
                                printf("recv error in iDWT xhr: %d\n", errno);
                                break;
                            }
                            total_received += bytes_received;
                        } while (total_received < contentlength);
                    }
                }
                printf("...%d of %d\n", total_received, contentlength);
                char response[256];
                sprintf(response, "HTTP/1.1 200 OK\r\n\r\n");
                int ret = send(client_socket, response, (int)strlen(response), 0);
                if (ret == SOCKET_ERROR) {
                    printf("'send'1 (when handling POST/DWT): %d", errno);
                    exit(-1);
                }

                int execTime = inverse_transform(received, width, height, horLevels, vertLevels);

                ret = send(client_socket, received, contentlength, 0);
                if (ret == SOCKET_ERROR) {
                    printf("'send'2 (when handling POST): %d", errno);
                    exit(-1);
                }

                double execTime_ms = 1000 * (double)(execTime) / CLOCKS_PER_SEC;
                printf("dwt execution time: %f ms\n", execTime_ms);

                free(received);
                received = NULL;
            }
        }
    }
    else if (bytes_received == 0) {
        printf("Connection closed\n");
    }
    else
        printf("recv failed: %d\n", errno);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // Binding the socket to the port 8080
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    // Listening for incoming connections
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        // Handle the client's request in a separate function
        handle_client(new_socket);
        close(new_socket);
    }
    printf("Server is listening on port %d\n", PORT);
    return 0;
}
