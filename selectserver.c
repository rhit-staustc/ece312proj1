// program adapted from beej selectserver.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/select.h>

#define PORT "56789"   // port we are going to listen on

/*
convert socket to IP addr string.
addr: struct sockaddr_in or struct sockaddr_in6
*/
const char *inet_ntop2(void *addr, char *buf, size_t size) {
    struct sockaddr_storage *sas = addr;
    struct sockaddr_in *sa4;
    struct sockaddr_in6 *sa6;
    void *src;

    switch(sas->ss_family) {
        case AF_INET:
            sa4 = addr;
            src = &(sa4->sin_addr);
            break;
        case AF_INET6:
            sa6 = addr;
            src = &(sa6->sin6_addr);
            break;
        default:
            return NULL;
    }
    return inet_ntop(sas->ss_family, src, buf, size);
}

// returns a listening socket
int get_listener_socket(void) {
    struct addrinfo hints, *ai, *p;
    int yes=1;
    int rv;
    int listener;

    // gives us a socket and bind it 
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(listener < 0) {
            continue;
        }
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if(bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    //if we are here, didnt get bound
    if(p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); //done with this

    //listen
    if(listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }
    return listener;
}

/*
add new incoming connections to proper set
*/
void handle_new_connection(int listener, fd_set *master, int *fdmax) {
    socklen_t addrlen;
    int newfd; // newly accepted socket descriptor
    struct sockaddr_storage remoteaddr; // client addr
    char remoteIP[INET6_ADDRSTRLEN];
    addrlen = sizeof remoteaddr;
    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
    if(newfd == -1) {
        perror("accept");
    } else {
        FD_SET(newfd, master); // add to master set
        if(newfd > *fdmax) { // keep track of max
            *fdmax = newfd;
        }
        printf("selectserver: new connection from %s on socket %d\n", 
        inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP), newfd);
    }
}

/*
broadcast a message to all clients
*/
void broadcast(char *buf, int nbytes, int listener, int s, fd_set *master, int fdmax) {
    for (int j = 0; j <= fdmax; j++) { // send to everyone
        if(FD_ISSET(j, master)) { // except the listener and ourselves
            if(j != listener && j != s) {
                if(send(j, buf, nbytes, 0) == -1) {
                    perror("send");
                }
            }
        }
    }
}

/*
Handle client data and hangups
*/
void handle_client_data(int s, int listener, fd_set *master, int fdmax) {
    char buf[256]; // buffer for client data
    int nbytes;

    //handle data from a client
    if((nbytes = recv(s, buf, sizeof buf, 0)) <= 0) {
        //got error or connection closed by client
        if(nbytes = 0) {
            // connection closed
            printf("selectserver: socket %d hung up\n", s);
        } else {
            perror("recv");
        }
        close(s);
        FD_CLR(s, master); //remove from master set
    } else {
        // we got some data from a client
        broadcast(buf, nbytes, listener, s, master, fdmax);
    }
}

int main(void)
{
    fd_set master;
    fd_set read_fds; 
    int fdmax; // max file descriptor number

    int listener; // listening socket descriptor

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    listener = get_listener_socket(); 

    // add listener to master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor 
    fdmax = listener; // so far, its this one

    //main loop
    for(;;) {
        read_fds = master; // copy it
        if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        //run through existing connections looking for data to read
        for(int i = 0; i <= fdmax; i++) {
            if(FD_ISSET(i, &read_fds)) { // got one
                if(i == listener) 
                    handle_new_connection(i, &master, &fdmax); 
                else
                    handle_client_data(i, listener, &master, fdmax); 
            }
        }
    }
    return 0;
}   




