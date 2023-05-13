#include <string.h>
#include <stdio.h>
#include <dirent.h>

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

int upload_file(char *file_name, int file_size, int (*get_bytes)(char *, int), char *output)
{
    FILE *file = fopen(file_name, "w");
    char buffer[BUFFER_SIZE];
    int total_rcv = 0;
    int bytes_rcv;

    while (total_rcv < file_size)
    {
        bytes_rcv = (*get_bytes)(buffer, BUFFER_SIZE);
        fwrite(buffer, 1, bytes_rcv, file);
        total_rcv += bytes_rcv;
    }

    strcpy(output, "\tFile was uploaded successfully\n\0");
    return strlen(output);
}

int download_file(char *file_name, char *buffer, int buffer_size)
{
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

int get_ls(char *output)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(DIRECTORY);
    if (dir == NULL)
    {
        return 0;
    }

    bzero(output, sizeof(output));
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            sprintf(output + strlen(output), C_BL "\t%s\n" C_RST, entry->d_name);
        }
    }

    closedir(dir);

    if (strlen(output) == 0)
        sprintf(output, C_YLW "\tWorking directory is empty\n" C_RST);

    return strlen(output);
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

int get_unrecognized(char *output)
{
    char msg[] = "Command does not exist!\n";
    strcpy(output, msg);

    return strlen(msg);
}