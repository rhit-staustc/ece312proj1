#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
  // 1. Create the socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    printf("Could not create socket for connection to server.");
    return -1;
  }

  // 2. Set the server address
  struct sockaddr_in srv_addr;
  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_addr.s_addr = inet_addr("146.190.62.39");
  srv_addr.sin_port = htons(80);

  // 3. Connect to the server and run the loop.
  int res = connect(sock, (struct sockaddr*)&srv_addr, sizeof(struct sockaddr_in));
  if (res < 0) {
    printf("Could not connect to server.");
    return -1;
  }

  // 4. Send request -- you should really check return values.
  char* msg0 = "GET / HTTP/1.1\n";
  send(sock, msg0, strlen(msg0), 0);
  char* msg1 = "Host: www.httpforever.com\n\n";
  send(sock, msg1, strlen(msg1), 0);

  // 5. Read response
  char buf[128];
  int len = recv(sock, buf, 127, 0);
  while (len == 127) {
    buf[len] = '\0';
    printf("%s", buf);
    len = recv(sock, buf, 127, 0);
  }
  buf[len] = '\0';
  printf("%s",buf);

  // 6. Clean up
  close(sock);
  return 0;
}
