/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
   gcc server2.c 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h> // booleans
#include <unistd.h> // fork

const int MSG_LEN = 256;
const int USER_LEN = 20;
const int FORMAT_CHARS_LEN = 3; // len of <> and ' '
const int MAX_LEN = MSG_LEN + USER_LEN + FORMAT_CHARS_LEN;

void dostuff(int); /* function prototype */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen, pid;
    struct sockaddr_in srv_addr, cli_addr;

    if (argc < 2) {
       fprintf(stderr,"ERROR, no port provided\n");
       exit(1);
    }

    // open socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    error("ERROR opening socket");
    
    memset(&srv_addr, 0, sizeof(srv_addr));
    portno = atoi(argv[1]);
    srv_addr.sin_family = AF_INET; // specified IPv4
    srv_addr.sin_addr.s_addr = INADDR_ANY; // tells socket to bind to any available network interface on the machine- this is where the ip address comes from
    srv_addr.sin_port = htons(portno); // converts port number
    
    if (bind(sockfd, (struct sockaddr *) &srv_addr,
            sizeof(srv_addr)) < 0) 
            error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    
    
    while (1) {
    // accept- 
    // 1. extracts first connection req. on the queue of pending connections for the listening socket, sockfd,
    // 2. creates a new connected socket, and
    // 3. returns a new file descripter referring to that socket
        newsockfd = accept(sockfd, 
            (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0)  {
            close(sockfd);
            dostuff(newsockfd);
            exit(0);
        }
        else close(newsockfd); 
    } 
    return 0; /* we never get here */
}

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void dostuff (int sock)
{
    while (true) {
        int n;
        char buffer[MAX_LEN];
            
        // read message from socket
        memset(buffer, 0, MAX_LEN);
        n = read(sock, buffer, MAX_LEN-1);
        if (n < 0) error("ERROR reading from socket");
    
        // prints the message on the serverside
        printf("%s\n", buffer);
    
        // write response to client
        n = write(sock,"I got your message",18);
        if (n < 0) error("ERROR writing to socket");

    }
}
