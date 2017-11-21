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
  char data[2048];
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

          memset(&packet, 0, sizeof(struct protocol_t));
          res = read(server_sock, &packet, sizeof(struct protocol_t));

          assert(strcmp(packet.cmd, "LISTDIR_RES") == 0);
          printf("[Client] Received:\n");
          printf("%s\n", packet.data);
          break;

        case 2:
          
          printf("[Client] Sending GETFILE\n");
          break;

        case 3:
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
/*
void handle_client(int cli_sock) {
  struct protocol_t packet;
  int res, pid;
  int nb_commands;
  int fd[2];
  int getfile_fd;
  char buf[64];

  nb_commands = 0;

  while(1) {
    memset(&packet, 0, sizeof(struct protocol_t));
    res = read(cli_sock, &packet, sizeof(struct protocol_t));
    
    if (res == 0) { // Connection quit by client
      printf("[Server] Got EOF from client... Shutting down connection!\n");
      
      memset(&packet, 0, sizeof(struct protocol_t));
      strcpy(packet.cmd, "STATS");
      sprintf(packet.data, "%d", nb_commands); 

      write(cli_sock, &packet, sizeof(struct protocol_t));
      shutdown(cli_sock, SHUT_RDWR);
      close(cli_sock);
      break;
    }

    nb_commands++;

    if(strcmp(packet.cmd, "QUIT_SERV\n") == 0) { // Quit server
    
    }
    else if(strcmp(packet.cmd, "LISTDIR\n") == 0) { // List directory
       pipe(fd);
       switch(pid = fork()) {
        case -1:
          fprintf(stderr, "[Server][LISTDIR] Fork error\n");
          break;
        case 0: // Child
          close(fd[0]);
          res = dup2(fd[1], STDOUT_FILENO); may_die(res, "dup2"); 
          res = execlp("ls", "ls", "-l", "serv_dir/", NULL); may_die(res, "execlp");
          break;
        default: // Parent
          close(fd[1]);
          memset(&packet, 0, sizeof(struct protocol_t));
          strcpy(packet.cmd, "LISTDIR_RES");
          res = read(fd[0], packet.data, sizeof(packet.data));
          may_die(res, "read listdir");
          packet.data_len = res;
          write(cli_sock, &packet, sizeof(struct protocol_t));
          break;
       }
    }
    else if(strcmp(packet.cmd, "GETFILE\n") == 0) { // Get file with name in args
      sprintf(buf, "serv_dir/%s", packet.data);
      memset(&packet, 0, sizeof(struct protocol_t));
      getfile_fd = open(buf, O_RDONLY); may_die(getfile_fd, "GETFILE open");
      
      strcpy(packet.cmd, "GETFILE_RES");
      res = read(getfile_fd, &packet.data, sizeof(packet.data)); may_die(res, "GETFILE read");
      packet.data_len = res;

      write(cli_sock, &packet, sizeof(struct protocol_t));
    }
    else { // Unknown command
      printf("[Server] Received unknown command: %s\n", packet.cmd);
    }
  }
}*/
