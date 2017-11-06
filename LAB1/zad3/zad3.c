#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char* argv[]) {
  int pid, status;
  int res;
  int fd[2];

  if(argc != 2) { // Check for wrong number of arguments
    printf("Usage: %s command_input\n", argv[0]);
    exit(-1);
  }

  res = pipe(fd); // Create pipe for process communication
  if(res == -1) {
    perror("pipe");
    exit(-1);
  }

  switch(pid = fork()) {
    case -1: // Die on fork error
      perror("fork");
      exit(-1);

    case 0: // Child --> convert pipe output to process STDIN (dup2), then convert process into another one (cat)
      close(fd[1]);
      res = dup2(fd[0], STDIN_FILENO);
      if(res == -1) {
        perror("dup2");
	exit(-1);
      }
      res = execlp("cat", "cat",  "-n", NULL);
      if(res == -1) {
        perror("execlp");
	exit(-1);
      }
      break;

    default: // Parent --> get input from user and write it to pipe input
      close(fd[0]);
      printf("Sending: %s\n", argv[1]);
      res = write(fd[1], argv[1], strlen(argv[1]) + 1);
      if(res == -1) {
        perror("write");
	exit(-1);
      }
      close(fd[1]);
      wait(&status);
  }

  return 0;
}
