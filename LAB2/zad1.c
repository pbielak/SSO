#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void die(char* cause);

int main(int argc, char* argv[]) {
  int pid, status;
  int i;
  int fd[2];
  int res;
  char c;

  if (argc < 2) {
    fprintf(stderr, "Usage %s number_of_steps\n", argv[0]);
    exit(-1);
  }

  res = pipe(fd);
  if (res == -1) { die("pipe"); }

  switch(pid = fork()) {
    case -1:
      die("fork");

    case 0: // Child
      close(fd[1]);

      sleep(5);
      for(i = 0; i < atoi(argv[1]); i++) {
        res = read(fd[0], &c, 1);
	if (res < 0) { die("read"); }

	printf("[Child][%d] Received: %c\n", i, c);
	sleep(1);
      }
      close(fd[0]);
      break;

   default:
      close(fd[0]);

      for(i = 0; i < atoi(argv[1]); i++) {
	c = 'A' + (i % ('Z' - 'A' + 1));
        res = write(fd[1], &c, 1);
	if (res < 0) { die("write"); }

	printf("[Parent][%d] Sent: %c\n", i, c);
      }

      close(fd[1]);
      wait(&status);
      break;
  }

  return 0;
}

void die(char* cause) {
  perror(cause);
  exit(-1);
}
