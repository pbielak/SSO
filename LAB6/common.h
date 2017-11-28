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


// Util function declarations
void may_die(int res, char* cause);
struct protocol_t get_packet_from(int sock, int* res_out);
void send_packet_to(int sock, struct protocol_t packet);


// Definitions
struct protocol_t get_packet_from(int sock, int* res_out) {
  struct protocol_t packet;
  ssize_t res;

  memset(&packet, 0, sizeof(struct protocol_t));
  res = read(sock, &packet, sizeof(struct protocol_t));
  may_die((int) res, "get_packet_from");

  *res_out = (int) res;
  return packet;
}

void send_packet_to(int sock, struct protocol_t packet) {
  ssize_t res;

  res = write(sock, &packet, sizeof(struct protocol_t));
  may_die((int) res, "send_packet_to");
}

void may_die(int res, char* cause) {
  if (res < 0) {
    perror(cause);
    exit(-1);
  }
}
#endif
