/* TERMS OF USE
 * This source code is subject to the terms of the MIT License.
 * Copyright(c) 2026 Vladimir Vasilich Tregub
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <stdint.h>

#include "cc-wavelet.h"

#pragma comment(lib, "Ws2_32.lib")

int byte_received = 0;

#define PORT 8080

#define CHUNKSIZE 0x800

void build_http_response_with_file(int client_socket, char* file_name) {

    char response[1024];

    char fullpath[255];
    FILE* file = NULL;
    int res = 0;

    if (strlen(file_name) == 0) {
        strcpy_s(fullpath, 11, "index.html");
    }
    else {
        res = fopen_s(&file, file_name, "rb");
        if (res != 0) {
            memcpy(fullpath, file_name, strlen(file_name));
            if (file_name[strlen(file_name)] == 0x2F)
                memcpy(fullpath + strlen(file_name), "index.html", 11);
            else
                memcpy(fullpath + strlen(file_name), "/index.html", 12);
        }
        else
            memcpy(fullpath, file_name, strlen(file_name) + 1);
    }
    res = fopen_s(&file, fullpath, "rb");
    if (res != 0) {
        // File not found, send 404 response
        sprintf_s(response, 27, "HTTP/1.1 404 Not Found\r\n\r\n");
        send(client_socket, response, (int)strlen(response), 0);
    }
    else {
        // File found, send 200 OK response
        sprintf_s(response, 20, "HTTP/1.1 200 OK\r\n\r\n");
        send(client_socket, response, (int)strlen(response), 0);
        // Send the file content
        char file_buffer[1024];
        int bytes_read = 0;
        int total_sent = 0;
        do {
            bytes_read = (int)fread_s(file_buffer, 1024, sizeof(char), 1024, file);
            int sent = send(client_socket, file_buffer, bytes_read, 0);
            total_sent += sent;
        } while (bytes_read == 1024);

        fclose(file);
    }
}

int width = 0;
int height = 0;
void handle_client(int client_socket) {
    char request[CHUNKSIZE] = { 0 };
    byte_received = 0;
    byte_received = recv(client_socket, request, CHUNKSIZE, 0);
    if (byte_received > 0)
    {
        printf("Bytes received: %d\n", byte_received);
        char* query_method = strtok(request, " ");
        if (!strcmp(query_method, "GET")) {
            char* file_name;
            char* query_string = strtok(NULL, " ");
            if (query_string[1] == 0x3F)
            {
                char response[256];
                sprintf_s(response, 20, "HTTP/1.1 200 OK\r\n\r\n");
                send(client_socket, response, (int)strlen(response), 0);
                int sent = send(client_socket, query_string, strlen(query_string), 0);
                width = atoi(query_string + 8);
                char* ptr_height = strstr(query_string, "height=") + 7;
                height = atoi(ptr_height);
                return NULL;
            }
            file_name = strtok(query_string, " ");
            build_http_response_with_file(client_socket, file_name + 1);
        }
        if (!strcmp(query_method, "POST")) {
            char* ptr_contentlength = strstr(request + 5, "Content-Length: ");
            if (ptr_contentlength == 0) {
                printf("No Content-Length header found, exit\n");
                exit(-1);
            }
            int contentlength = atoi(ptr_contentlength + strlen("Content-Length: "));
            int total_received = 0;
            char* received = calloc(contentlength, 1);
            if ((contentlength + byte_received) <= CHUNKSIZE) {
                memcpy(received, request + byte_received - contentlength, contentlength);
                total_received = contentlength;
            }
            else
            {
                do {
                    byte_received = recv(client_socket, received + total_received, CHUNKSIZE, 0);
                    if (byte_received == SOCKET_ERROR) {
                        printf("recv error in POST xhr: %d\n", WSAGetLastError());
                        break;
                    }
                    total_received += byte_received;
                    //printf("%d,", byte_received);
                } while (total_received < contentlength);
            }
            printf("...%d of %d\n", total_received, contentlength);
            char response[256];
            sprintf_s(response, 20, "HTTP/1.1 200 OK\r\n\r\n");
            int ret = send(client_socket, response, (int)strlen(response), 0);
            if (ret == SOCKET_ERROR) {
                printf("'send'1 (when handling POST): %d", WSAGetLastError());
                exit(-1);
            }

            clock_t loopstarted = clock();
            forward_transform(received, width, height, 1, 1);

            char* sendback = calloc(contentlength * 2, 1);
            for (int i = 0; i < contentlength; ++i) {
                sendback[2 * i] = received[i] & 0x7F;
                sendback[2 * i + 1] = ((received[i] & 0x80) ? 1 : 0);
            }
            clock_t loopexited = clock();

            ret = send(client_socket, sendback, contentlength * 2, 0);
            if (ret == SOCKET_ERROR) {
                printf("'send'2 (when handling POST): %d", WSAGetLastError());
                exit(-1);
            }

            double execTime = 1000 * (double)(loopexited - loopstarted) / CLOCKS_PER_SEC;
            printf("dwt execution time: %f ms\n", execTime);


            free(received);
            received = NULL;
            return "";// parse_POST(query_method, client_socket);
        }
    }
    else if (byte_received == 0) {
        printf("Connection closed\n");
    }
    else
        printf("recv failed: %d\n", WSAGetLastError());
}

int main() {

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    WSADATA wsaData;
    int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (err != 0)
        return 1;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // Binding the socket to the port 8080
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");// INADDR_LOOPBACK;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        perror("bind failed");
        closesocket(server_fd);
        exit(EXIT_FAILURE);
    }
    // Listening for incoming connections
    if (listen(server_fd, 10) == SOCKET_ERROR) {
        perror("listen");
        closesocket(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port %d\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, NULL/*(struct sockaddr*)&address*/, NULL/*(socklen_t*)&addrlen*/)) < 0) {
            perror("accept");
            closesocket(server_fd);
            exit(EXIT_FAILURE);
        }
        // Handle the client's request in a separate function
        handle_client(new_socket);
        closesocket(new_socket);
    }

    WSACleanup();

    //free(TAPE);

    return 0;
}
