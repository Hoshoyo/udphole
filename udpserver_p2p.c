#include <fcntl.h>
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

#define PORT 7778

void print_ipv4(unsigned int ip) {
    printf("%d.%d.%d.%d", (ip & 0xff), ((ip & 0xff00) >> 8), ((ip & 0xff0000) >> 16), ((ip & 0xff000000) >> 24));
}
void print_port(unsigned short port) {
    printf("%d", (port >> 8) | ((port & 0xff) << 8));
}

typedef struct {
    struct sockaddr_in addr;
} Peer_Info;

#define MAXLINE 1024

int main(int argc, char** argv) {
    int sockfd; 
    struct sockaddr_in servaddr = {0}, cliaddr = {0}; 
      
    // Creating socket file descriptor 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        printf("socket creation failed"); 
        return -1;
    } 
      
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
      
    // Bind the socket with the server address 
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        printf("bind failed");
        return -1;
    }

    struct sockaddr_in info[2] = {0};

    int even = 0;
    while(1) {
        char buffer1[1024] = {0};
        int len = sizeof(struct sockaddr);
        int n1 = recvfrom(sockfd, (char *)buffer1, 1024,
            MSG_WAITALL, ( struct sockaddr *) &info[even],
            &len);

        print_ipv4(info[even].sin_addr.s_addr);
        printf(":");
        print_port(info[even].sin_port);
        printf("\n");

        even++;

        if(even == 2) {
            even = 0;
            // Send to both clients information about each other
            ssize_t sent = sendto(sockfd, &info[1], sizeof(struct sockaddr_in), MSG_CONFIRM, (struct sockaddr*)&info[0], len);
            printf("Sent info to client 0 (%ld bytes)\n", sent);

            sent = sendto(sockfd, &info[0], sizeof(struct sockaddr_in), MSG_CONFIRM, (struct sockaddr*)&info[1], len);
            printf("Sent info to client 1 (%ld bytes)\n", sent);
        }
    }

    return 0;
}