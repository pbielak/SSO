#include "common.h"

#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_SERVER_PORT 5000

// Returns socket
int get_connection(char* server_ip, int server_port);

void list_directory(int server_sock);

void download_file(int server_sock, char* filename);

void quit(int server_sock);

int main(int argc, char* argv[]) {
  char* server_ip;
  int server_port;
  int server_sock;
  int option;
  char filename[64];

  if (argc >= 3) {
    server_ip = argv[1];
    server_port = atoi(argv[2]);
  } else {
    server_ip = DEFAULT_SERVER_IP;
    server_port = DEFAULT_SERVER_PORT;
  }

  server_sock = get_connection(server_ip, server_port);

  while (1) {
    printf("1) LISTDIR\n2) GETFILE\n3) QUIT\n");
    scanf("%d", &option);

    switch (option) {
      case 1:
        list_directory(server_sock);
        break;

      case 2:
        printf("Filename: ");
        scanf("%s", filename);

        download_file(server_sock, filename);
        break;

      case 3:
        quit(server_sock);
        exit(0);

      default:
        fprintf(stderr, "No such command: %d\n", option);
        break;
    }
  }
}

int get_connection(char* server_ip, int server_port) {
  struct sockaddr_in serv_addr;
  int server_sock;
  int res;

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(server_ip);
  serv_addr.sin_port = htons(server_port);

  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  may_die(server_sock, "socket");

  res = connect(server_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
  may_die(res, "connect");

  return server_sock;
}

void list_directory(int server_sock) {
  struct protocol_t packet;
  int res;

  printf("[Client] Sending LISTDIR\n");
  strcpy(packet.cmd, "LISTDIR");

  send_packet_to(server_sock, packet);

  printf("[Client] Received:\n");
  while (1) {
    memset(&packet, 0, sizeof(struct protocol_t));
    packet = get_packet_from(server_sock, &res);

    if (res == 0 || strcmp(packet.cmd, "LISTDIR_DONE") == 0) break;

    printf("%s\n", packet.data);
  }
}

void download_file(int server_sock, char* filename) {
  struct protocol_t packet;
  int res;
  int fd;

  printf("[Client] Sending GETFILE\n");
  strcpy(packet.cmd, "GETFILE");
  strcpy(packet.data, filename);

  send_packet_to(server_sock, packet);

  fd = open(filename, O_CREAT | O_WRONLY, 0666);
  may_die(fd, "GETFILE open");

  while (1) {
    packet = get_packet_from(server_sock, &res);

    if (res == 0 || strcmp(packet.cmd, "GETFILE_DONE") == 0) break;

    printf("[Client] Downloading file: %s...\n", filename);
    write(fd, &packet.data, packet.data_len);
  }

  close(fd);
  printf("[Client] File downloaded\n");
}

void quit(int server_sock) {
  struct protocol_t packet;
  int res;

  printf("[Client] Quitting...\n");

  strcpy(packet.cmd, "QUIT");

  send_packet_to(server_sock, packet);

  shutdown(server_sock, SHUT_WR);

  memset(&packet, 0, sizeof(struct protocol_t));
  packet = get_packet_from(server_sock, &res);


  assert(strcmp(packet.cmd, "STATS") == 0);
  printf("Stats: %s\n", packet.data);

  shutdown(server_sock, SHUT_RD);
  close(server_sock);
}