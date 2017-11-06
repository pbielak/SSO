#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define SEPARATOR "+"

int find_separator(int argc, char* argv[]);

int main(int argc, char* argv[]) {
  int pid;
  int res;
  int separator_pos;
  char* program_name;
  int fd[2];

  separator_pos = find_separator(argc, argv); // Look for separator
  if(separator_pos == -1) { // If no separator found, just execute the program given in arguments
	                    // example invocation: ./zad4 tac --> program name: tac, arguments: None
			    // The name of the program is in the current argument vector at pos 1
			    // and the arguments are the current argument vector "shifted" one position to the right
    execvp(argv[1], argv+1);
  }
  else {
    program_name = argv[0]; // If separator found, remember the program name (zad4) and put a NULL terminator
                            // in the place of the separator
    argv[separator_pos] = NULL;
  }

  res = pipe(fd); // Create a pipe for process communication
  if(res == -1) {
    perror("pipe");
    exit(-1);
  }

  switch(pid = fork()) {
    case -1: // Die on fork error
      perror("fork");
      exit(-1);

    case 0: // Child --> will not use pipe output, but will write its STDOUT to pipe input
            // "exec" function used in the same way as it was for the situation where no separator was found
      close(fd[0]);
      dup2(fd[1], STDOUT_FILENO);
      execvp(argv[1], argv+1);
      break;

    default: // Parent --> will not use pipe input, but converts its STDIN to pipe output
             // Here comes the recursive step of the algorithm: a new argument vector is created (by shifting 
	     // the old vector begin to the separator position and replacing the value on this position (NULL) to 
	     // the program name (zad4). Then it converts the process into itself (with the modified argv) 
	     // by using the "exec" function --> recursive call.
      close(fd[1]);
      dup2(fd[0], STDIN_FILENO);
      argv = argv + separator_pos;
      argv[0] = program_name;
      execvp(program_name, argv);
      break;
  }

  return 0;
}

int find_separator(int argc, char* argv[]) {
  int i;
  for(i = 0; i < argc; i++) {
    if(strcmp(argv[i], SEPARATOR) == 0) {
     return i;
    }
  }

  return -1;
}
