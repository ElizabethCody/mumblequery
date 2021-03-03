#pragma once

#include <stdint.h>
#include <stddef.h>
#include <netinet/in.h>

union mumble_version {
  uint8_t part[4];
};

struct mumble_query_reply {
  union mumble_version version;
  uint64_t id;
  uint32_t users;
  uint32_t slots;
  uint32_t bandwidth;
} __attribute__((packed));

int mumble_query(struct sockaddr*, uint64_t, struct mumble_query_reply*);
int mumble_snversion(char*, size_t, union mumble_version);
