#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_SERVER_PORT 5000

struct protocol_t {
  char cmd[64];
  int data_len;
  char data[4096];
};
  
void may_die(int res, char* cause);

int main(int argc, char* argv[]) {
    char* server_ip;
    int server_port;
    int res;
    int server_sock;
    struct sockaddr_in serv_addr;
    int option;
    char filename[64];
    int getfile_fd;
    struct protocol_t packet;

    if (argc >= 3) {
      server_ip = argv[1];  
      server_port = atoi(argv[2]);
    }
    else {
      server_ip = DEFAULT_SERVER_IP;
      server_port = DEFAULT_SERVER_PORT;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(server_ip);
    serv_addr.sin_port = htons(server_port);
    
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    may_die(server_sock, "socket");

    res = connect(server_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    may_die(res, "connect");


    while(1) {
      memset(&packet, 0, sizeof(struct protocol_t));
      printf("1) LISTDIR\n2) GETFILE\n3) QUIT\n");
      scanf("%d", &option);

      switch(option) {
        case 1:
          printf("[Client] Sending LISTDIR\n");
          strcpy(packet.cmd, "LISTDIR");
          res = write(server_sock, &packet, sizeof(struct protocol_t));
          may_die(res, "LISTDIR write");

          printf("[Client] Received:\n");
          do {
            memset(&packet, 0, sizeof(struct protocol_t));
            res = read(server_sock, &packet, sizeof(struct protocol_t));
            may_die(res, "LISTDIR read");

            if (res == 0 || strcmp(packet.cmd, "LISTDIR_DONE") == 0) break;

            printf("%s\n", packet.data);
          } while(res != 0);

          break;

        case 2:
          printf("Filename: ");
          scanf("%s", filename);

          printf("[Client] Sending GETFILE\n");
          strcpy(packet.cmd, "GETFILE");
          strcpy(packet.data, filename);

          res = write(server_sock, &packet, sizeof(struct protocol_t));
          may_die(res, "GETFILE write");

          getfile_fd = open(filename, O_CREAT | O_WRONLY, 0666);
          may_die(getfile_fd, "GETFILE open");

          do {
              res = read(server_sock, &packet, sizeof(struct protocol_t));
              may_die(res, "GETFILE read");

              if (res == 0 || strcmp(packet.cmd, "GETFILE_DONE") == 0) break;

              printf("[Client] Downloading file: %s...\n", filename);
              write(getfile_fd, &packet.data, packet.data_len);
          } while(res != 0);

          close(getfile_fd);
          printf("[Client] File downloaded\n");
          break;

        case 3:
          printf("[Client] Quitting...\n");
          
          strcpy(packet.cmd, "QUIT");

          res = write(server_sock, &packet, sizeof(struct protocol_t));
          may_die(res, "QUIT write");

          shutdown(server_sock, SHUT_WR);

          memset(&packet, 0, sizeof(struct protocol_t));
          res = read(server_sock, &packet, sizeof(struct protocol_t));
          may_die(res, "QUIT read");

          assert(strcmp(packet.cmd, "STATS") == 0);
          printf("Stats: %s\n", packet.data);

          shutdown(server_sock, SHUT_RD);
          close(server_sock);
          exit(0);
          break; 
        default:
          fprintf(stderr, "No such command: %d\n", option);
          break;
      }
    }
}

void may_die(int res, char* cause) {
    if (res < 0) {
        perror(cause);
        exit(-1);
    }
}
