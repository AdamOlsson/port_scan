#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

static void
scan(char* ip, char* port_range[])
{
    printf("%s:[%s, %s]\n", ip, port_range[0], port_range[1]);

    struct addrinfo hints;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    int res;
    struct addrinfo** serv_addr;
    if((res = getaddrinfo(ip, port_range[0], &hints, serv_addr)) != 0) {
        printf("Error: Server address creation failed with exit code %d.\n", res);
    }

}


int
main(int argc, char *argv[])
{   
    if (argc < 4) {
        printf("Please provide an ipv4 address and port range.\n");
        printf("Example: ./scan 192.168.1.1 100 200\n");
        exit(1);
    }

    char* target_ip = strdup(argv[1]);
    char* port_range[] = { strdup(argv[2]), strdup(argv[3]) };

    /* TODO: Validate input. */
    /* Valid ipv4 address. */
    /* Ports are valid Integers */
    /* Start port < End port */

    scan(target_ip, port_range);

    return 0;
}