#ifndef CMD_H
#define CMD_H

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>

char* help_command();
char* current_directory();
char* list_directories(const char *restrict buffer);
void send_file(int zombie_fd, const char *restrict buffer);

#endif // CMD_H