#include "common.h"

#define DEFAULT_SERVER_PORT 5000
#define MAX_CLIENTS 5

typedef struct {
    char host[128];
    int socket;
} client_t;

client_t clients[MAX_CLIENTS];

// Return socket
int get_server_socket(int port);

void init_readfds(int server_sock, fd_set* readfds, int* max_fd);
void accept_new_client(int server_sock);
void handle_client(int client_index);


int main(int argc, char* argv[]) {
  int server_sock;
  int port;

  fd_set readfds;
  int max_fd;
  int i, res;

  if (argc > 1) {
    port = atoi(argv[1]);
  } else {
    port = DEFAULT_SERVER_PORT;
  }

  server_sock = get_server_socket(port);

  printf("[Server] Started. Waiting for connections...\n");

  while (1) {
    init_readfds(server_sock, &readfds, &max_fd);

    res = select(max_fd + 1, &readfds, NULL, NULL, NULL);
    may_die(res, "select");

    if (FD_ISSET(server_sock, &readfds)) {
      accept_new_client(server_sock);
    }

    for(i = 0; i < MAX_CLIENTS; i++) {
      if (FD_ISSET(clients[i].socket, &readfds)) {
       handle_client(i);
      }
    }
  }
}

int get_server_socket(int port) {
  int server_sock;
  struct sockaddr_in serv_addr;
  ssize_t res;

  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  may_die(server_sock, "socket");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  res = bind(server_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
  may_die(res, "bind");

  res = listen(server_sock, MAX_CLIENTS);
  may_die(res, "listen");

  return server_sock;
}

void init_readfds(int server_sock, fd_set* readfds, int* max_fd) {
  int i, s;

  FD_ZERO(readfds);

  FD_SET(server_sock, readfds);
  *max_fd = server_sock;

  for(i = 0; i < MAX_CLIENTS; i++) {
    s = clients[i].socket;

    if (s > 0) {
      FD_SET(s, readfds);
    }

    if (s > *max_fd) {
      *max_fd = s;
    }
  }
}

void accept_new_client(int server_sock) {
  int client_sock;
  socklen_t clen;
  struct sockaddr_in client_addr;
  char buf[128];
  int i;

  clen = sizeof(client_addr);

  client_sock = accept(server_sock, (struct sockaddr*) &client_addr, &clen);
  may_die(client_sock, "accept");

  sprintf(buf, "%s:%d", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
  printf("[Server] New connection from %s\n", buf);

  for(i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].socket == 0) {
      clients[i].socket = client_sock;
      strcpy(clients[i].host, buf);
      break;
    }
  }
}

void handle_client(int client_index) {
  struct protocol_t packet;
  int res, i;
  int cs;
  int client_sock;

  client_sock = clients[client_index].socket;
  packet = get_packet_from(client_sock, &res);

  if (res == 0) {
    printf("[Server] Client (%s) closed connection.\n", clients[client_index].host);
    close(client_sock);
    clients[client_index].socket = 0;
    strcpy(clients[client_index].host, "");
    return;
  }

  // Broadcast received message
  for(i = 0; i < MAX_CLIENTS; i++) {
    cs = clients[i].socket;

    if (cs > 0) {
      send_packet_to(cs, packet);
    }
  }
}