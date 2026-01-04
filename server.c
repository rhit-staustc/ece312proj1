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
#include <arpa/inet.h>

#define PORT 56789
#define MAXMSG 512
#define USER_LEN 20
#define MAXCLIENTS 2
#define QUIT_CMD "/quit"
#define QUIT_MSG "__QUIT__"

int client_fds[MAXCLIENTS];
char client_names[MAXCLIENTS][USER_LEN];
char client_ips[MAXCLIENTS][INET_ADDRSTRLEN];
int client_count = 0;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(void)
{
    int listener, newfd;
    struct sockaddr_in srv_addr, cli_addr;
    socklen_t addrlen;

    fd_set master, read_fds;
    int fdmax;
    char buf[MAXMSG];

    //create listener
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0) error("socket");

    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    memset(&srv_addr, 0, sizeof srv_addr);
    srv_addr.sin_family = AF_INET; 
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr*)&srv_addr, sizeof srv_addr) < 0)
        error("bind");

    if (listen(listener, 10) < 0)
        error("listen");

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(listener, &master);
    fdmax = listener;

    printf("Chat server listening on port %d\n", PORT);


    while (1) {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) < 0)
            error("select");

        for (int i = 0; i <= fdmax; i++) {
            if (!FD_ISSET(i, &read_fds)) continue;

            // new connection
            if (i == listener) {
                addrlen = sizeof cli_addr;
                newfd = accept(listener,
                    (struct sockaddr*)&cli_addr, &addrlen);

                if (newfd < 0) {
                    perror("accept");
                    continue;
                }

                FD_SET(newfd, &master);
                if (newfd > fdmax) fdmax = newfd;

                // get client IP
                inet_ntop(AF_INET, &cli_addr.sin_addr,
                        client_ips[client_count],
                        INET_ADDRSTRLEN);

                // read username
                recv(newfd,
                    client_names[client_count],
                    USER_LEN,
                    0);

                client_fds[client_count] = newfd;

                printf("Client connected from %s (%s)\n",
                    client_ips[client_count],
                    client_names[client_count]);

                client_count++;
                if (client_count == 2) {
                    char msg[256];

                    // notify client 0
                    snprintf(msg, sizeof msg,
                        "Connection established with %s (%s)\n",
                        client_ips[1], client_names[1]);
                    send(client_fds[0], msg, strlen(msg), 0);

                    // notify client 1
                    snprintf(msg, sizeof msg,
                        "Connection established with %s (%s)\n",
                        client_ips[0], client_names[0]);
                    send(client_fds[1], msg, strlen(msg), 0);
                }
                char *ip = inet_ntoa(cli_addr.sin_addr);
                printf("New connection: IP %s\n", ip);
            }
            // client message
            else {
                int n = recv(i, buf, sizeof buf, 0);
                buf[n] = '\0';

                if (strncmp(buf, QUIT_CMD, 4) == 0) {
                    printf("Connection terminated\n");

                    // notify both clients
                    for (int k = 0; k < client_count; k++) {
                        send(client_fds[k], QUIT_MSG, strlen(QUIT_MSG), 0);
                        close(client_fds[k]);
                        FD_CLR(client_fds[k], &master);
                    }

                    client_count = 0;
                    continue;
                }


                if (n <= 0) {
                    close(i);
                    FD_CLR(i, &master);
                    printf("Client %d disconnected\n", i);
                } else {
                    // broadcast
                    for (int j = 0; j <= fdmax; j++) {
                        if (FD_ISSET(j, &master)) {
                            if (j != listener && j != i) {
                                send(j, buf, n, 0);
                            }
                        }
                    }
                }
            }
        }
    }
}