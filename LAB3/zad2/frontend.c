#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ncurses.h>

#define PAUSE_CMD "p"
#define MUTE_CMD "m"
#define SEEK_FORWARD_CMD "seek 15"
#define SEEK_BACKWARD_CMD "seek -15"
#define EXIT_CMD "q"

void die(char* cause);
void print_menu();
void write_to_fifo(int fd, char* command);


int main(int argc, char* argv[]) {
  int fd;
  char cmd;

  if(argc < 2) {
    fprintf(stderr, "Usage: %s fifo_name\n", argv[0]);
    exit(-1);
  }

  fd = open(argv[1], O_WRONLY);
  if (fd < 0) { die("open"); }

  initscr();
  noecho();
  timeout(-1);

  print_menu();

  while(1) {
    cmd = getch();

    if (cmd == '\n') {
      continue;
    }

    mvprintw(7, 0, "Chosen command: %c\n", cmd);

    switch(cmd) {
      case 'p':
        write_to_fifo(fd, PAUSE_CMD);
        break;
      case 'm':
	write_to_fifo(fd, MUTE_CMD);
	break;
      case 'f':
	write_to_fifo(fd, SEEK_FORWARD_CMD);
	break;
      case 'b':
	write_to_fifo(fd, SEEK_BACKWARD_CMD);
	break;
      case 'e':
	write_to_fifo(fd, EXIT_CMD);
	endwin();
	close(fd);
	exit(0);
      default:
        mvprintw(7, 0, "Unknown command: %c\n", cmd);
        break;
    }
  }

  return 0;
}


void die(char* cause) {
  perror(cause);
  exit(-1);
}

void print_menu() {
  mvprintw(0, 0, "------ MPLAYERCNTL ---\n");
  mvprintw(1, 0, "Choose action:\n");
  mvprintw(2, 0, "p] Pause\n");
  mvprintw(3, 0, "m] Mute\n");
  mvprintw(4, 0, "f] Seek forward 15\n");
  mvprintw(5, 0, "b] Seek backward 15\n");
  mvprintw(6, 0, "e] Exit\n");
}

void write_to_fifo(int fd, char* command) {
  int r;
  char buf[64];

  r = sprintf(buf, "%s\n", command);
  r = write(fd, buf, r);
  if (r < 0) { die("write"); }
}
