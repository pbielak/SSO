#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FIFO_NAME "/tmp/test_fifo"

void die(char* cause);

int main(int argc, char* argv[]) {
  int i;
  int fd;
  int res;
  char c;

  if (argc < 2) {
    fprintf(stderr, "Usage %s mode\n", argv[0]);
    exit(-1);
  }


  if (strcmp(argv[1], "reader") == 0) {

    if (argc < 3) {
      fprintf(stderr, "Usage %s reader number_of_steps\n", argv[0]);
      exit(-1);
    }

    fd = open(FIFO_NAME, O_RDONLY);

    sleep(5);
    for(i = 0; i < atoi(argv[2]); i++) {
      res = read(fd, &c, 1);
      if (res < 0) { die("read"); }

      printf("[Child][%d] Received: %c\n", i, c);
      sleep(1);
    }

    close(fd);
  }
  else if (strcmp(argv[1], "writer") == 0) {

    if (argc < 3) {
      fprintf(stderr, "Usage %s writer number_of_steps\n", argv[0]);
      exit(-1);
    }

    fd = open(FIFO_NAME, O_WRONLY);

    for(i = 0; i < atoi(argv[2]); i++) {
      c = 'A' + (i % ('Z' - 'A' + 1));
      res = write(fd, &c, 1);
      if (res < 0) { die("write"); }

      printf("[Parent][%d] Sent: %c\n", i, c);
    }

    close(fd);
  }
  else if(strcmp(argv[1], "create") == 0) {
    res = mkfifo(FIFO_NAME, 0666);
    if (res < 0) { die("mkfifo"); }
    printf("Created fifo %s\n", FIFO_NAME);
  }
  else {
    fprintf(stderr, "No such mode like: %s\n", argv[1]);
  }

  return 0;
}

void die(char* cause) {
  perror(cause);
  exit(-1);
}
