#ifndef COMMON_H
#define COMMON_H

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

struct protocol_t {
    char cmd[64];
    int data_len;
    char data[4096];
};


// Util functions
void may_die(int res, char* cause) {
  if (res < 0) {
    perror(cause);
    exit(-1);
  }
}



#endif
