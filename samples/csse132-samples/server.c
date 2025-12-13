#include <stdio.h>       // printf, fgets, etc.
#include <stdlib.h>      // malloc, etc.
#include <string.h>      // strlen, strcpy, etc.
#include <sys/socket.h>  // struct sockaddr, socket, etc.
#include <arpa/inet.h>   // htons, inet_ntoa, etc.
#include <unistd.h>      // POSIX standards, close

int main() {

  // 1. Create server (listening) socket
  int srv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (srv_sock < 0) {
    printf("Failed to create server socket\n");
    exit(1);
  }

  // 2. Set up server address
  struct sockaddr_in srv_addr;  // local address
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // accept on any interface
  srv_addr.sin_port = htons(8080);

  // 3. Bind server socket to address
  int res = bind(srv_sock, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr_in));
  if (res < 0) {
    printf("Failed to bind server to address and port\n");
    exit(1);
  }

  // 4. Set server socket as a listening socket
  int result = listen(srv_sock, 5); // enqueue at most 5 clients
  if (result < 0) {
    printf("Failed to start listening\n");
    exit(1);
  }

  int received_bytes = 0;
  char received_string[512];    // buffer for receiving data
  int sess_sock;                // session socket
  struct sockaddr_in cli_addr;  // client address

  // 5. LOOP to keep server running to accept more client connections!
  while (1) {
    // A. Accept a connection, obtain the session socket
    // WARNING: must initialize this length or accept() won't fill the address struct.
    unsigned int sock_addr_len = sizeof(struct sockaddr);
    sess_sock = accept(srv_sock,
                      (struct sockaddr *)&cli_addr,
                      &sock_addr_len);
    printf("Connected to client %s\n", inet_ntoa(cli_addr.sin_addr));

    // B. Protocol (send / recv)
    send(sess_sock, (const void *)"HELLO\n", 6, 0);
    printf(">>> Sent HELLO\n");

    received_bytes = recv(sess_sock, (void *)received_string, 512, 0);
    received_string[received_bytes] = '\0';
    printf("<<< Received: %s\n", received_string);

    // C. Close session socket
    shutdown(sess_sock, SHUT_RDWR); // close the socket connection
    close(sess_sock);               // close the file descriptor
  }

  // 6. Close listening socket
  close(srv_sock);
}