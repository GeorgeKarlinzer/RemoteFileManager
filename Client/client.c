#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <libgen.h>
#include "../Core/strhelper.c"

#define BUFFER_SIZE 1024

int download_file(int sock, char *file_name)
{
    FILE *file;
    char buffer[BUFFER_SIZE];
    int file_size, bytes_rcv, total_rcv = 0;

    recv(sock, buffer, BUFFER_SIZE, 0);
    file_size = atoi(buffer);

    file = fopen(file_name, "w");

    while (total_rcv < file_size)
    {
        bytes_rcv = recv(sock, buffer, BUFFER_SIZE, 0);
        fwrite(buffer, 1, bytes_rcv, file);
        total_rcv += bytes_rcv;
    }
    send(sock, "ok", 2, 0);

    fclose(file);
    recv(sock, buffer, BUFFER_SIZE, 0);
    printf("%s\n", buffer);
}

int upload_file(int sock, char *path)
{
    FILE *file;
    char *file_name;
    char buffer[BUFFER_SIZE];
    int file_size, total_sent, bytes_sent;

    file_name = basename(path);

    file = fopen(path, "rb");
    if (file == NULL)
    {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0L, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    sprintf(buffer, "upload %s %d", file_name, file_size);

    send(sock, buffer, BUFFER_SIZE, 0);
    total_sent = 0;
    bytes_sent = 0;

    while (total_sent < file_size)
    {
        int bytes_read = fread(buffer, 1, BUFFER_SIZE, file);
        bytes_sent = send(sock, buffer, bytes_read, 0);
        total_sent += bytes_sent;
    }
    fclose(file);

    recv(sock, buffer, BUFFER_SIZE, 0);
    printf("%s\n", buffer);
}

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char *file_name;
    int bytes_received, bytes_sent, file_size, total_sent = 0;
    FILE *file;

    // create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct timeval timeout;
    timeout.tv_sec = 10;  // Timeout of 10 seconds
    timeout.tv_usec = 0;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    // convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address or address not supported");
        exit(EXIT_FAILURE);
    }

    // connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Successfully connected to server!\n");

    while (1)
    {
        int is_end = 0;
        bzero(buffer, BUFFER_SIZE);
        printf("client: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        if (buffer[strlen(buffer) - 1] = '\n')
            buffer[strlen(buffer) - 1] = '\0';

        char **my_argv;
        int my_argc;
        parse_args(buffer, &my_argv, &my_argc);

        if (strcmp(my_argv[0], "upload") == 0)
        {
            upload_file(sock, my_argv[1]);
        }
        else if (strcmp(my_argv[0], "download") == 0)
        {
            send(sock, buffer, BUFFER_SIZE, 0);
            download_file(sock, my_argv[1]);
        }
        else
        {
            send(sock, buffer, BUFFER_SIZE, 0);
            bzero(buffer, BUFFER_SIZE);
            bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
            printf("%s\n", buffer);
            if(strcmp(my_argv[0], "exit") == 0) is_end = 1;
        }
        for (int i = 0; i < my_argc; i++)
        {
            free(my_argv[i]);
        }
        free(my_argv);

        if(is_end == 1) break; 
    }

    return 0;
}
