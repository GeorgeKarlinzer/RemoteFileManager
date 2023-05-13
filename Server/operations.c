#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>

#define C_RD "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define C_BL "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define C_RST "\x1b[0m"

#define DIRECTORY "/home/kali/Desktop/Output"

bool upload_file()
{
}

bool download_file()
{
}

int delete_file(char *file, char *output)
{
    int status = remove(file);
    if (status == 0) {
        sprintf(output, "File '%s' deleted successfully.\n", file);
    } else {
        sprintf(output, "Error deleting file '%s'.\n", file);
    }

    return strlen(output);
}

bool rename_file()
{
}

int get_ls(char *buffer)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(DIRECTORY);
    if (dir == NULL)
    {
        return 0;
    }

    bzero(buffer, sizeof(buffer));
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            sprintf(buffer + strlen(buffer), C_BL"\t%s\n"C_RST, entry->d_name);
        }
    }

    closedir(dir);

    return strlen(buffer);
}

int get_help(char *buffer)
{
    char msg[] =
    C_BL "\thelp\n" C_RST
    C_BL "\tls " C_RST "-- List all files\n" C_BL "\tdownload " C_RD "[file]\n" C_RST
    C_BL "\tupload " C_RD "[file]\n" C_RST
    C_BL "\tdelete " C_RD "[file]\n" C_RST
    C_BL "\trename " C_RD "[file] [new name]\n" C_RST
    C_BL "\texit\n" C_RST;

    strcpy(buffer, msg);

    return strlen(msg);
}

int get_unrecognized(char *buffer)
{
    char msg[] = "Command does not exist!\n";
    strcpy(buffer, msg);

    return strlen(msg);
}