#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <unistd.h>

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
    char buffer[256];
    
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

    // server = gethostbyname(argv[1]); // use hostname argument
    // if (server == NULL) {
    //     fprintf(stderr,"ERROR, no such host\n");
    //     exit(0);
    // }
    
    // set up sockaddr
    // bzero((char *) &serv_addr, sizeof(serv_addr));
    // serv_addr.sin_family = AF_INET;
    // bcopy((char *)server->h_addr_list[0], 
    //     (char *)&serv_addr.sin_addr.s_addr, 
    //     server->h_length);
    
    // set port number


    fprintf(stderr, "About to connect to %s:%s\n", argv[1], argv[2]);
    // connect to server
    if (connect(sockfd,(struct sockaddr *)&srv_addr,sizeof(srv_addr)) < 0) 
        error("ERROR connecting");
    
    // user inputs message
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    
    // write to socket
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

