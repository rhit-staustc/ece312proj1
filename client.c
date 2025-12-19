#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <unistd.h>


const int MSG_LEN = 256;
const int USER_LEN = 20;
const int FORMAT_CHARS_LEN = 3; // len of <> and ' '
const int PREFIX_LEN = USER_LEN + FORMAT_CHARS_LEN;

void error(char *msg)
{
    perror(msg);
    exit(0);
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
    
    // user inputs message
    printf("<%s> ", username);
    char buffer[256];
    memset(buffer, 0, MSG_LEN);
    fgets(buffer, MSG_LEN-1, stdin);
    
    // prefix- '<[username]> '
    char prefix[PREFIX_LEN];
    snprintf(prefix, PREFIX_LEN-1, "<%s>",username);

    // write to socket
    // write username to socket
    // TODO include <> around username
    n = write(sockfd, prefix, PREFIX_LEN);
    if (n < 0) 
        error("ERROR writing to socket");
    // write message to socket
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
        error("ERROR writing to socket");
    
    // read response
    bzero(buffer,256); // clear buffer
    n = read(sockfd,buffer,255);
    if (n < 0) 
        error("ERROR reading from socket");
    printf("%s\n",buffer); // print response
    
    return 0;
}

