#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void print_info();

int main(int argc, char* argv[]) {
  int status;
  int pid;

  switch(pid = fork()) {
    case -1:
      perror("fork");
      break;

    case 0:
      printf("From child process\n");
      print_info();
      break;

    default:
      printf("From parent process\n");
      print_info();
      wait(&status);
      break;
  }

  return 0;
}

void print_info() {
  printf("PID = %d\tPPID = %d\n", getpid(), getppid());
}
