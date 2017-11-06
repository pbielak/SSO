#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char* argv[]) {
  int pid, status;
  int fd[2];
  int res;
  char in_buf[64], out_buf[64];

  if(argc != 2) { // Check for wrong number of args
    printf("Usage: %s msg\n", argv[0]);
    exit(-1);
  }

  res = pipe(fd); // Create pipe for process communication
  if(res == -1) {
    perror("pipe");
    exit(-1);
  }

  switch(pid = fork()) {
    case -1: // Die on error
      perror("fork");
      exit(-1);
      break;

    case 0: // Child --> reads message from pipe and writes to its STDOUT
      close(fd[1]);

      res = read(fd[0], out_buf, sizeof(out_buf));
      if(res == -1) {
        perror("read");
	exit(-1);
      }
      printf("Got: %s\n", out_buf);

      close(fd[0]);
      break;

    default: // Parent --> gets message from user, puts in buffer and writes to pipe
      close(fd[0]);

      strcpy(in_buf, argv[1]);
      printf("Sending: %s\n", in_buf);
      res = write(fd[1], in_buf, sizeof(in_buf));
      if(res == -1) {
        perror("write");
	exit(-1);
      }
      close(fd[1]);
      wait(&status);
  }

  return 0;
}
