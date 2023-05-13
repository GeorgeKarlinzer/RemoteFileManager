#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <regex.h>
#include "operations.c"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void handle_connection(int server_fd, int addrlen);
int send_protocols(int fd);
int send_options(int fd);
void get_argument(char *input, int index, char *output);

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
        recv(client_fd, buffer, BUFFER_SIZE, 0);
        printf("Received command from client %s:%d: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

        char *first_arg;
        get_argument(buffer, 0, first_arg);
        printf("%s", first_arg);

        if (strcmp(first_arg, "help\n") == 0)
        {
            int size = get_help(buffer);
            send(client_fd, buffer, size, 0);
        }
        else if (strcmp(first_arg, "ls\n") == 0)
        {
            int size = get_ls(buffer);
            send(client_fd, buffer, size, 0);
        }
        else if (strcmp(first_arg, "delete") == 0)
        {
            char *file;
            get_argument(buffer, 1, file);
            int size = delete_file(file, buffer);
            send(client_fd, buffer, size, 0);
        }
        else
        {
            int size = get_unrecognized(buffer);
            send(client_fd, buffer, size, 0);
        }
    }

    close(client_fd);
}

int send_protocols(int fd)
{
    char msg[] = "Select protocol:\n"
                 "For UDP send udp\n"
                 "For TCP send tcp\n"
                 "For SCTP send sctp\n";
    return send(fd, msg, sizeof(msg), 0);
}

int send_options(int fd)
{
    char options[] = "Select ";
    return send(fd, "test", sizeof("test"), 0);
}

void get_argument(char *input, int index, char *output)
{
    printf("inside");
    char *first_word = strtok(input, " ");
    printf("arg: %s", first_word);

    for (int i = 0; i < index; i++)
    {
        first_word[0] = ' ';
        first_word = strtok(NULL, " ");
        printf("arg: %s", first_word);
    }

    strcpy(output, first_word);
}