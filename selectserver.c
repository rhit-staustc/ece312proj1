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

int main(void)
{
    fd_set master;
    fd_set read_fds; 
    int fdmax; // max file descriptor number

    int listener; // listening socket descriptor

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    listener = get_listener_socket(); // todo- implement

    // add listener to master set
    FD_SET(listerner, &master)
}