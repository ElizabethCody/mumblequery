#pragma once

#include <stdint.h>

struct hostport {
  char* host;
  uint16_t port;
};

int hostport(char*, uint16_t, struct hostport**);
void freehostport(struct hostport*);
