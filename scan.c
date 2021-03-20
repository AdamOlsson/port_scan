#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <assert.h>

static struct addrinfo*
addrinfo_create(char* ip, char* port)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    struct addrinfo* addrs;
    assert(getaddrinfo(ip, port, &hints, &addrs) == 0);

    return addrs;
}

static void
addrinfo_destroy(struct addrinfo* addr)
{
    freeaddrinfo(addr);
}

static void
probe()
{}

static void
scan(char* ip, char* port_range[])
{
    printf("%s:[%s, %s]\n", ip, port_range[0], port_range[1]);

    struct addrinfo* serv_addr = addrinfo_create(ip, port_range[0]);

    struct addrinfo* addr;
    for (addr = serv_addr; addr != NULL; addr = addr->ai_next) {
        int sfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sfd == -1) {
            continue;
        }

        if (connect(sfd, addr->ai_addr, addr->ai_addrlen) == 0) {
            printf("Port %s is open.\n", port_range[0]);
        } else {
            printf("Port %s NOT is open.\n", port_range[0]);

        }

        close(sfd);
    }

    addrinfo_destroy(serv_addr);
}


int
main(int argc, char *argv[])
{   
    if (argc < 4) {
        fprintf(stderr, "Please provide an ipv4 address and port range.\nExample: ./scan 192.168.1.1 100 200\n");
        exit(EXIT_FAILURE);
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