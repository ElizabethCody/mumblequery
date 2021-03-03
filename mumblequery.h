#pragma once

#include <stdint.h>
#include <stddef.h>
#include <netent/in.h>

union mumble_version {
  uint8_t version[4];
};

struct mumble_query_reply {
  union mumble_version version;
  uint32_t users;
  uint32_t slots;
  uint32_t bandwidth;
} __attribute__(packed);

int mumble_query(const struct sockaddr_in*, uint64_t, struct mumble_query_reply*);
int mumble_snversion(char*, size_t, union mumble_version);
