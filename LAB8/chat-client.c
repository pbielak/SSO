#include "common.h"
#include <curses.h>

#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_SERVER_PORT 5000
#define DEFAULT_USER_NAME "guest"

// Global connection variables
int server_sock;
char* username;

// Global ncurses variables
WINDOW *history_window, *input_window;

// Returns socket
int get_connection(char* server_ip, int server_port);
void handle_user_input();
void handle_server_request();

void handle_quit(int param);

void init_gui();
void refresh_gui();

int main(int argc, char* argv[]) {
  char* server_ip;
  int server_port;

  fd_set readfds;
  int max_fd, res;

  if (argc >= 4) {
    server_ip = argv[1];
    server_port = atoi(argv[2]);
    username = argv[3];
  } else {
    server_ip = DEFAULT_SERVER_IP;
    server_port = DEFAULT_SERVER_PORT;
    username = DEFAULT_USER_NAME;
  }

  initscr();
  signal(SIGINT, handle_quit);

  init_gui();

  server_sock = get_connection(server_ip, server_port);
  max_fd = server_sock;

  while (1) {
    refresh_gui();

    FD_ZERO(&readfds);
    FD_SET(server_sock, &readfds);
    FD_SET(STDIN_FILENO, &readfds);

    res = select(max_fd + 1, &readfds, NULL, NULL, NULL);
    may_die(res, "select");

    if(FD_ISSET(STDIN_FILENO, &readfds)) {
      handle_user_input();
    }

    if(FD_ISSET(server_sock, &readfds)) {
      handle_server_request();
    }
  }

  return 0;
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

void handle_user_input() {
  struct protocol_t packet;
  char buf[128];

  memset(&packet, 0, sizeof(struct protocol_t));

  wgetstr(input_window, buf);
  mvwprintw(input_window, 0, 0, "%s", buf);

  strcpy(packet.msg, buf);
  strcpy(packet.username, username);

  send_packet_to(server_sock, packet);
}

void handle_server_request() {
  struct protocol_t packet;
  int res;

  packet = get_packet_from(server_sock, &res);

  wprintw(history_window, " %s> %s\n", packet.username, packet.msg);
}

void handle_quit(int param) {
  endwin();
  printf("Application quit\n");
  shutdown(server_sock, SHUT_RDWR);
  close(server_sock);
  exit(0);
}

void init_gui() {
  int height, width;

  getmaxyx(stdscr, height, width);
  history_window = newwin(height - 1, width, 0, 0);
  input_window = newwin(1, width, height - 1, 0);

  scrollok(history_window, TRUE);
  wmove(history_window, 1, 0);
}

void refresh_gui() {
  box(history_window, '|', '-');
  wrefresh(history_window);

  wclear(input_window);
  mvwprintw(input_window, 0, 0, "%s> ", username);
  wrefresh(input_window);
}