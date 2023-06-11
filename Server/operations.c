#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <libgen.h>

#define C_RD "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define C_YLW "\x1b[33m"
#define C_BL "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define C_RST "\x1b[0m"

#define DIRECTORY "/home/kali/Desktop/Output/"

#define CHECKFILE(file) (strchr(file, '\\') == NULL && strchr(file, '/') == NULL)

#define BUFFER_SIZE 1024

int upload_file(char *file_name, int file_size, int client_fd, char *output)
{
    char tmp_file[20], tmp_path[255], file_path[255];
    FILE *file;
    char buffer[BUFFER_SIZE];
    int total_rcv = 0;
    int bytes_rcv;

    tmpnam(tmp_file);
    sprintf(tmp_path, DIRECTORY "%s", tmp_file);
    sprintf(file_path, DIRECTORY "%s", basename(file_name));
    file = fopen(tmp_path, "w");

    while (total_rcv < file_size)
    {
        bytes_rcv = recv(client_fd, buffer, BUFFER_SIZE, 0);
        fwrite(buffer, 1, bytes_rcv, file);
        total_rcv += bytes_rcv;
    }

    fclose(file);

    if (rename(tmp_path, file_path) != 0)
    {
        remove(tmp_path);
        sprintf(output, C_RD "\tCannot upload file.\n" C_RST);
        return strlen(output);
    }

    strcpy(output, C_BL "\tFile was uploaded successfully\n" C_RST);
    return strlen(output);
}

int download_file(char *file_name, int client_fd, char *output)
{
    if (CHECKFILE(file_name))
    {
        FILE *file;
        char file_path[255];
        char buffer[BUFFER_SIZE];
        int file_size, total_sent, bytes_sent;

        sprintf(file_path, DIRECTORY "%s", file_name);

        file = fopen(file_path, "rb");
        if (file == NULL)
        {
            strcpy(output, C_RD "\tCannot download file\n" C_RST);
            return strlen(output);
        }

        fseek(file, 0L, SEEK_END);
        file_size = ftell(file);
        rewind(file);

        sprintf(buffer, "%d", file_size);

        send(client_fd, buffer, BUFFER_SIZE, 0);
        total_sent = 0;
        bytes_sent = 0;

        while (total_sent < file_size)
        {
            int bytes_read = fread(buffer, 1, BUFFER_SIZE, file);
            bytes_sent = send(client_fd, buffer, bytes_read, 0);
            total_sent += bytes_sent;
            printf("Sent %d/%d\n", total_sent, file_size);
        }
        fclose(file);
        recv(client_fd, buffer, BUFFER_SIZE, 0);
        strcpy(output, C_BL "\tFile was downloaded successfully\n" C_RST);
        return strlen(output);
    }

    strcpy(output, C_RD "\tCannot download file\n" C_RST);
    return strlen(output);
}

int delete_file(char *file, char *output)
{
    if (CHECKFILE(file))
    {
        char path[100];
        strcpy(path, DIRECTORY "/");
        strcat(path, file);
        if (remove(path) == 0)
        {
            sprintf(output, C_BL "\tFile '%s' deleted successfully.\n" C_RST, file);
            return strlen(output);
        }
    }
    sprintf(output, C_RD "\tError deleting file '%s'.\n" C_RST, file);

    return strlen(output);
}

int rename_file(char *file, char *new_name, char *output)
{
    if (CHECKFILE(file) && CHECKFILE(new_name))
    {
        char old_path[255], new_path[255];
        strcpy(old_path, DIRECTORY);
        strcat(old_path, file);
        strcpy(new_path, DIRECTORY);
        strcat(new_path, new_name);

        if (rename(old_path, new_path) == 0)
        {
            sprintf(output, C_BL "\tFile renamed successfully.\n" C_RST);
            return strlen(output);
        }
    }

    sprintf(output, C_RD "\tError renaming file.\n" C_RST);
    return strlen(output);
}

void get_ls(char *output)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(DIRECTORY);
    if (dir == NULL)
    {
        strcpy(output, C_RD "\tWorking directory does not exist!\n" C_RST);
        return;
    }

    bzero(output, sizeof(output));
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            char *tmp = malloc(strlen(C_BL "\t\n" C_RST "\0") + strlen(entry->d_name) + 1);
            sprintf(tmp, C_BL "\t%s\n" C_RST "\0", entry->d_name);
            if (strlen(tmp) + strlen(output) + 1 > BUFFER_SIZE)
            {
                free(tmp);
                break;
            }
            sprintf(output + strlen(output), tmp);
            free(tmp);
        }
    }

    closedir(dir);

    if (strlen(output) == 0)
        sprintf(output, C_YLW "\tWorking directory is empty\n" C_RST);
}

int get_help(char *output)
{
    char msg[] =
        C_BL "\thelp\n" C_RST
            C_BL "\tls " C_RST "-- List all files\n" C_BL "\tdownload " C_RD "[file]\n" C_RST
                C_BL "\tupload " C_RD "[file]\n" C_RST
                    C_BL "\tdelete " C_RD "[file]\n" C_RST
                        C_BL "\trename " C_RD "[file] [new name]\n" C_RST
                            C_BL "\texit\n" C_RST;

    strcpy(output, msg);

    return strlen(msg);
}

int get_exit(char *output)
{
    char msg[] = C_BL "\tConnection was closed!\n" C_RST;
    strcpy(output, msg);
    return strlen(output);
}

int get_unrecognized(char *output)
{
    char msg[] = "Command does not exist!\n";
    strcpy(output, msg);

    return strlen(msg);
}