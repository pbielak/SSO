#include "common.h"

#define DEFAULT_SERVER_PORT 5000
#define MAX_CLIENTS 5

typedef struct {
    char host[128];
    struct sockaddr_in addr;
} client_t;

client_t clients[MAX_CLIENTS];

int server_sock;

// Return socket
int get_server_socket(int port);

void init_clients_list();

void handle_client_join(struct protocol_t packet, struct sockaddr_in client_addr);
void handle_client_message(struct protocol_t packet, struct sockaddr_in client_addr);
void handle_client_quit(struct sockaddr_in client_addr);
void broadcast_packet(struct protocol_t packet);

int main(int argc, char* argv[]) {
  int port;

  struct sockaddr_in client_addr;
  socklen_t clen;

  struct protocol_t packet;

  int res;

  if (argc > 1) {
    port = atoi(argv[1]);
  } else {
    port = DEFAULT_SERVER_PORT;
  }

  server_sock = get_server_socket(port);
  
  clen = sizeof(client_addr);

  init_clients_list();

  printf("[Server] Started. Waiting for connections...\n");

  while (1) {
    res = recvfrom(server_sock, &packet, sizeof(struct protocol_t), 0, (struct sockaddr*) &client_addr, &clen);  

    if (res == 0) {
      handle_client_quit(client_addr);
    }

    printf("[Server] Received packet: [TYPE: %s | USERNAME: %s | MSG: %s]\n", packet.type, packet.username, packet.msg);

    if(strcmp(packet.type, "CLIENT_JOIN") == 0) {
      handle_client_join(packet, client_addr);
    }
    else if(strcmp(packet.type, "CLIENT_MSG") == 0) {
      handle_client_message(packet, client_addr);
    }
    else if(strcmp(packet.type, "CLIENT_QUIT") == 0) {
      handle_client_quit(client_addr);
    }
    else {
      printf("[Server] Received unkown packet type: %s [USER: %s MSG: %s]\n", packet.type, packet.username, packet.msg);
    }
  }
}

int get_server_socket(int port) {
  int server_sock;
  struct sockaddr_in serv_addr;
  ssize_t res;

  server_sock = socket(AF_INET, SOCK_DGRAM, 0);
  may_die(server_sock, "socket");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);

  res = bind(server_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
  may_die(res, "bind");

  return server_sock;
}

void init_clients_list() {
  int i;

  for(i = 0; i < MAX_CLIENTS; i++) {
    strcpy(clients[i].host, "");
    memset(&(clients[i].addr), 0, sizeof(struct sockaddr_in));
  }
}

void handle_client_join(struct protocol_t packet, struct sockaddr_in client_addr) {
  char buf[128];
  int i;

  sprintf(buf, "%s:%d", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
  printf("[Server] New connection from %s\n", buf);

  for(i = 0; i < MAX_CLIENTS; i++) {
    if (strcmp(clients[i].host, "") == 0) {
      clients[i].addr = client_addr;
      strcpy(clients[i].host, buf);
      break;
    }
  }
}

void handle_client_message(struct protocol_t packet, struct sockaddr_in client_addr) {
  char buf[128];

  sprintf(buf, "%s:%d", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
  printf("[Server] Client (%s) sent message: [USERNAME: %s MSG: %s]\n", buf, packet.username, packet.msg);

  broadcast_packet(packet);
}

void broadcast_packet(struct protocol_t packet) {
  int i, res;
  struct sockaddr_in addr;

  for(i = 0; i < MAX_CLIENTS; i++) {
    if(strcmp(clients[i].host, "") != 0) {
      printf("[Server] Broadcasting message to client: %s\n", clients[i].host);
      addr = clients[i].addr;
      res = sendto(server_sock, &packet, sizeof(struct protocol_t), 0, (struct sockaddr*) &addr, sizeof(addr));
      may_die(res, "sendto");
    }
  }
}

void handle_client_quit(struct sockaddr_in client_addr) {
  int i;
  char buf[128];
  
  sprintf(buf, "%s:%d", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

  printf("[Server] Client (%s) closed connection.\n", buf);

  for (i = 0; i < MAX_CLIENTS; i++) {
    if (strcmp(clients[i].host, buf) == 0) {
      strcpy(clients[i].host, "");
      memset(&(clients[i].addr), 0, sizeof(struct sockaddr_in));
      break;
    }
  }
}

