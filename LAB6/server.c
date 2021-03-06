#include "common.h"

#define DEFAULT_SERVER_PORT 5000
#define MAX_WAITING_CLIENTS 5

// Return socket
int get_server_socket(int port);

void handle_client(int client_sock, char* client_name);
void handle_client_quit(int client_sock, int nb_commands);
void handle_client_listdir(int client_sock);
void handle_client_getfile(int client_sock, char* filename);

int main(int argc, char* argv[]) {

  int serv_sock, cli_sock;
  int pid, port;
  int status;
  socklen_t clen;
  struct sockaddr_in cl_addr;
  char buf[128];

  if (argc > 1) {
    port = atoi(argv[1]);
  } else {
    port = DEFAULT_SERVER_PORT;
  }

  serv_sock = get_server_socket(port);
  clen = sizeof(cl_addr);

  printf("[Server] Started. Waiting for connections...\n");

  while (1) {
    cli_sock = accept(serv_sock, (struct sockaddr*) &cl_addr, &clen);
    may_die(cli_sock, "accept");

    sprintf(buf, "%s:%d", inet_ntoa(cl_addr.sin_addr), cl_addr.sin_port);
    printf("[Server] New connection from %s\n", buf);

    switch (pid = fork()) {
      case 0:
        close(serv_sock);
        handle_client(cli_sock, buf);
        exit(0);
      case -1:
        perror("fork");
        exit(-1);
    }

    close(cli_sock);
    while (waitpid((pid_t) -1, &status, WNOHANG) != -1);
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

  res = listen(server_sock, MAX_WAITING_CLIENTS);
  may_die(res, "listen");

  return server_sock;
}

void handle_client(int client_sock, char* client_name) {
  struct protocol_t packet;
  int res;
  int nb_commands;

  nb_commands = 0;

  while (1) {
    packet = get_packet_from(client_sock, &res);
    printf("[Server] Client (%s) sent -- ", client_name);

    if (res == 0 || strcmp(packet.cmd, "QUIT") == 0) {
      printf("EOF / QUIT command... Shutting down connection!\n");
      handle_client_quit(client_sock, nb_commands);
      break;
    }

    nb_commands++;

    if (strcmp(packet.cmd, "LISTDIR") == 0) {
      printf("LISTDIR\n");
      handle_client_listdir(client_sock);
    } else if (strcmp(packet.cmd, "GETFILE") == 0) {
      printf("GETFILE (%s)\n", packet.data);
      handle_client_getfile(client_sock, packet.data);
    } else {
      printf("unknown command: %s\n", packet.cmd);
    }
  }
}

void handle_client_quit(int client_sock, int nb_commands) {
  struct protocol_t packet;

  memset(&packet, 0, sizeof(struct protocol_t));
  strcpy(packet.cmd, "STATS");
  sprintf(packet.data, "%d", nb_commands);

  send_packet_to(client_sock, packet);

  shutdown(client_sock, SHUT_RDWR);
  close(client_sock);
}

void handle_client_listdir(int client_sock) {
  int fd[2];
  int pid;
  int res;
  struct protocol_t packet;

  pipe(fd);
  switch (pid = fork()) {
    case -1:
      fprintf(stderr, "[Server][LISTDIR] Fork error\n");
      break;
    case 0: // Child
      close(fd[0]);
      res = dup2(fd[1], STDOUT_FILENO);
      may_die(res, "dup2");
      res = execlp("ls", "ls", "-l", "serv_dir/", NULL);
      may_die(res, "execlp");
      break;
    default: // Parent
      close(fd[1]);
      memset(&packet, 0, sizeof(struct protocol_t));

      while ((res = (int) read(fd[0], &packet.data, sizeof(packet.data))) != 0) {
        may_die(res, "read listdir");

        strcpy(packet.cmd, "LISTDIR_RES");
        packet.data_len = res;

        send_packet_to(client_sock, packet);
      }

      strcpy(packet.cmd, "LISTDIR_DONE");
      send_packet_to(client_sock, packet);
      break;
  }

}

void handle_client_getfile(int client_sock, char* filename) {
  char buf[64];
  int fd;
  struct protocol_t packet;
  int res, cnt;

  sprintf(buf, "serv_dir/%s", filename);
  fd = open(buf, O_RDONLY);
  may_die(fd, "GETFILE open");

  printf("[Server] Getfile (%s) -- opened file\n", filename);
  cnt = 0;

  while(1) {
    memset(&packet, 0, sizeof(struct protocol_t));
    strcpy(packet.cmd, "GETFILE_RES");
    res = (int) read(fd, &packet.data, sizeof(packet.data));
    may_die(res, "GETFILE read");

    if (res == 0) break;

    printf("[Server] Getfile (%s) -- [%d] read file chunk of size %d\n", filename, cnt++, res);
    packet.data_len = res;

    send_packet_to(client_sock, packet);
  }

  memset(&packet, 0, sizeof(struct protocol_t));
  strcpy(packet.cmd, "GETFILE_DONE");
  send_packet_to(client_sock, packet);
  close(fd);
  printf("[Server] Getfile (%s) -- closed file\n", filename);
}