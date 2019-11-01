#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h> 
#include <netinet/in.h>

#define PORT 7778

void print_ipv4(unsigned int ip) {
    printf("%d.%d.%d.%d", (ip & 0xff), ((ip & 0xff00) >> 8), ((ip & 0xff0000) >> 16), ((ip & 0xff000000) >> 24));
}
void print_port(unsigned short port) {
    printf("%d", (port >> 8) | ((port & 0xff) << 8));
}

typedef enum {
    CONN_NONE = 0,
    CONN_SYN,
    CONN_SYN_ACK,
    CONN_ACK,
} Connection_State;

int perform_handshake(int sockfd, struct sockaddr_in* cinfo) {
    char buffer[1024] = {0};
    int len = sizeof(struct sockaddr);

    Connection_State conn_state = CONN_NONE;

    // Try to exchange data using that information
    ssize_t sent = sendto(sockfd, "SYN", sizeof("SYN"), MSG_DONTWAIT, (struct sockaddr*)cinfo, len);
    printf("Sento to peer %ld bytes (SYN)\n", sent);

    while(1) {
        int n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *)cinfo, &len);
        if(n > 0) {
            printf("Received from peer message: %s at state %d\n", buffer, conn_state);

            if(strcmp("SYN", buffer) == 0 && conn_state == CONN_NONE) {
                conn_state = CONN_SYN;
                // received a SYN message
                ssize_t sent = sendto(sockfd, "SYN ACK", sizeof("SYN ACK"), MSG_DONTWAIT, (struct sockaddr*)cinfo, len);
                printf("Sento to peer %ld bytes (SYN ACK)\n", sent);
            } else if(strcmp("SYN ACK", buffer) == 0) {
                conn_state = CONN_SYN_ACK;
                // received a SYN ACK message
                ssize_t sent = sendto(sockfd, "ACK", sizeof("ACK"), MSG_DONTWAIT, (struct sockaddr*)cinfo, len);
                printf("Sento to peer %ld bytes (ACK)\n", sent);
                break;
            } else if (strcmp("ACK", buffer) == 0) {
                conn_state = CONN_ACK;
                break;
            }

        } else {
            printf("Error receiving data from peer: %s\n", strerror(errno));
            return -1;
        }
    }

    return 0;
}

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("usage: %s <server ip>\n", argv[0]);
        return 1;
    }

    int sockfd = 0;
    struct sockaddr_in servaddr = {0}; 
      
    // Creating socket file descriptor 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        printf("socket creation failed"); 
        return -1;
    }

    // Filling server information 
    servaddr.sin_family = AF_INET; // IPv4 
    servaddr.sin_port = htons(PORT); 
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr.s_addr); 
      
    char hello[1024] = "Hello World";
    int n, len; 
    sendto(sockfd, (const char *)hello, strlen(hello), 
        MSG_DONTWAIT, (const struct sockaddr *) &servaddr,  
        sizeof(servaddr)); 
    printf("Hello message sent.\n"); 

    // Receive peer information from the server if one is
    // already waiting for a connection.
    {
        char buffer[1024] = {0};
        n = recvfrom(sockfd, (char *)buffer, 1024,
            MSG_WAITALL, (struct sockaddr *) &servaddr, &len); 
        
        // Peer info
        struct sockaddr_in* cinfo = (struct sockaddr_in*)buffer;
        print_ipv4(cinfo->sin_addr.s_addr);
        printf(":");
        print_port(cinfo->sin_port);
        printf("\n");       

        perform_handshake(sockfd, cinfo);
        
        // Listen
        printf("Waiting for data...\n");
        while(1) {
            char buffer[1024] = {0};
            int n = recvfrom(sockfd, (char *)buffer, 1024,
                MSG_WAITALL, (struct sockaddr *)cinfo, &len);
            if(n > 0) {
                printf("Received from client message: %s\n", buffer);
            } else {
                printf("Error receiving data from client: %s\n", strerror(errno));
                break;
            }
        }
    }


    return 0;
}