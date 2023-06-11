#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <regex.h>

#include "operations.c"
#include "../Core/strhelper.c"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void handle_connection(int server_fd, int addrlen);

int main(int argc, char *argv[])
{
    int server_fd, client_fd, opt = 1;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    int addrlen = sizeof(server_addr);
    int bytes_received, bytes_sent, file_size, total_sent = 0;
    FILE *file;

    // create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // set server address
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));
    // bind socket to address

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\n", atoi(argv[1]));

    while (1)
    {
        handle_connection(server_fd, addrlen);
    }

    return 0;
}

void handle_connection(int server_fd, int addrlen)
{
    struct sockaddr_in client_addr;
    int client_fd, bytes_rcv;
    char buffer[BUFFER_SIZE];

    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen)) < 0)
    {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    if (fork() != 0)
        return;

    printf("New client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    while (1)
    {
        int is_break = 0;
        recv(client_fd, buffer, BUFFER_SIZE, 0);
        printf("Received command from client %s:%d: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

        char **argv;
        int argc;
        parse_args(buffer, &argv, &argc);

        bzero(buffer, BUFFER_SIZE);
        if (strcmp(argv[0], "help") == 0) get_help(buffer);
        else if (strcmp(argv[0], "ls") == 0) get_ls(buffer);
        else if (strcmp(argv[0], "delete") == 0) delete_file(argv[1], buffer);
        else if (strcmp(argv[0], "rename") == 0) rename_file(argv[1], argv[2], buffer);
        else if (strcmp(argv[0], "upload") == 0) upload_file(argv[1], atoi(argv[2]), client_fd, buffer);
        else if (strcmp(argv[0], "download") == 0) download_file(argv[1], client_fd, buffer);
        else if (strcmp(argv[0], "exit") == 0) { get_exit(buffer); is_break = 1; }
        else get_unrecognized(buffer);

        for (int i = 0; i < argc; i++)
        {
            free(argv[i]);
        }

        free(argv);

        send(client_fd, buffer, BUFFER_SIZE, 0);
        if (is_break == 1)
            break;
    }

    close(client_fd);
}