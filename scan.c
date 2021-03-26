#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>

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
probe(struct addrinfo* addr)
{
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    int sfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

    if (sfd == -1) {
        return;
    }

    int flags = 0;        
    if((flags = fcntl(sfd, F_GETFL, NULL)) < 0) { 
        fprintf(stderr, "Error getting socket flags.\n"); 
        exit(0); 
    }
    /* Set non-blocking socket */
    flags |= O_NONBLOCK; 
    if(fcntl(sfd, F_SETFL, flags) < 0) { 
        fprintf(stderr, "Error setting socket flags.\n"); 
        exit(0);
    } 

    int res = connect(sfd, addr->ai_addr, addr->ai_addrlen);
    if (res == -1) {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)addr->ai_addr;
        uint16_t port = ntohs(addr_in->sin_port);
        char* addr_str = inet_ntoa((struct in_addr)addr_in->sin_addr);

        if (errno == EINPROGRESS) {
            fd_set setp;
            FD_ZERO(&setp); 
            FD_SET(sfd, &setp);
            res = select(sfd+1, NULL, &setp, NULL, &timeout);
            
            if (res < 0 && errno != EINTR) { 
                fprintf(stderr, "ERROR %d - %s\n", errno, strerror(errno)); 
                exit(0); 
            } else if(res > 0) {
                /* Connected */
                fprintf(stderr, "Found open port %s:%u\n", addr_str, port);
            } else {
                /* Timeout */
                // fprintf(stderr, "NOT OPEN.\n");
            }
        } else {
            fprintf(stderr, "Error connecting to %s:%u - Reason:\n%d - %s", addr_str, port, errno, strerror(errno));
        }
    }

    // Set to blocking mode again... 
    if((flags = fcntl(sfd, F_GETFL, NULL)) < 0) { 
        fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
        exit(0); 
    }

    flags &= (~O_NONBLOCK); 
    if(fcntl(sfd, F_SETFL, flags) < 0) { 
        fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
        exit(0); 
    } 

    close(sfd);
}

static void
scan(char* ip, char* port_range[])
{
    fprintf(stderr, "%s:[%s, %s]\n", ip, port_range[0], port_range[1]);

    /* TODO:
        Make efficient on large amount of ports
        Allow for IPv6
        Perform partial TCP connection
    */
    
    char port_str[16];
    for(int port = atoi(port_range[0]); port <= atoi(port_range[1]); port++){
        
        sprintf(port_str, "%d", port);

        struct addrinfo* serv_addr = addrinfo_create(ip, port_str);
        
        struct addrinfo* addr;
        for (addr = serv_addr; addr != NULL; addr = addr->ai_next) {
            probe(addr);
        }
        addrinfo_destroy(serv_addr);
    }
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