#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h> // booleans


const int MSG_LEN = 256;
const int USER_LEN = 20;
const int FORMAT_CHARS_LEN = 3; // len of <> and ' '
const int PREFIX_LEN = USER_LEN + FORMAT_CHARS_LEN;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

void print_prompt(const char *username) {
    printf("<%s> ", username);
    fflush(stdout);
}


// takes ip and portnumber as arguments
int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct hostent *server;
    
    if (argc < 3) {
        fprintf(stderr,"usage %s ip port\n", argv[0]);
        exit(0);
    }
    
    
    // open socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // SOCK_STREAM- provides sequenced, reliable, two-way, connection-based byte streams.
    if (sockfd < 0) 
    error("ERROR opening socket");
    
    // retrieve server ip
    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    // TODO: take ip as input from user (or hardcode)
    srv_addr.sin_addr.s_addr = inet_addr(argv[1]); 
    portno = atoi(argv[2]);
    srv_addr.sin_port = htons(portno);
    
    // username input
    printf("Provide user name: ");
    char username[20];
    fgets(username, 19, stdin);
    username[strcspn(username, "\n")] = 0;

    // connect to server
    // fprintf(stderr, "About to connect to %s:%s\n", argv[1], argv[2]);
    if (connect(sockfd,(struct sockaddr *)&srv_addr,sizeof(srv_addr)) < 0) 
        error("ERROR connecting");
    // send username to server immediately
    write(sockfd, username, strlen(username) + 1);

    printf("Waiting for connection . . .\n");
    
    fd_set read_fds;
    // print_prompt(username);
    char buf[512];
    int ser_msg_count = 0;
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sockfd, &read_fds);

        int maxfd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

        if (select(maxfd + 1, &read_fds, NULL, NULL, NULL) < 0)
            error("select");

        // user typed something
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char msg[256];
            fgets(msg, sizeof msg, stdin);

            // quit command
            if (strncmp(msg, "/quit", 5) == 0) {
                printf("Disconnecting...\n");
                close(sockfd);
                exit(0);
            }

            char out[512];
            snprintf(out, sizeof out, "<%s> %s", username, msg);
            write(sockfd, out, strlen(out));
            print_prompt(username);
        }

        // server sent something
        if (FD_ISSET(sockfd, &read_fds)) {
            int n = recv(sockfd, buf, sizeof buf - 1, 0);
            if (n <= 0) {
                printf("Server closed connection\n");
                break;
            }
            buf[n] = '\0';
            printf("\n%s", buf);
                print_prompt(username);
        }
    } 
}

