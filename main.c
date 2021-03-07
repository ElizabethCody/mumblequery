#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <inttypes.h>
#include "mumblequery.h"
#include "hostport.h"
#include "cleanup.h"

#define MUMBLE_DEFAULT_PORT 64738

static inline uint64_t randu64() {
  return (uint64_t) rand() << UINT64_C(32) | (uint64_t) rand();
}

int main(int argc, char* argv[]) {
  if(argc < 2) {
    fprintf(stderr, "Usage: %s <host[:port]>...\n", argv[0]);
    return EXIT_FAILURE;
  }

  srand(time(NULL));

  for(size_t i = 1; i < argc; ++i) {
    struct hostport* hp = NULL;

    if(hostport(argv[i], MUMBLE_DEFAULT_PORT, &hp) != 0) {
      fprintf(stderr, "%s: failed to parse: %s.\n", argv[i], strerror(errno));
      continue;
    }

    struct sockaddr* addr = NULL;
    _cleanup_freeaddrinfo_ struct addrinfo* addrinfo = NULL;

    // lookup hostname
    if(getaddrinfo(hp->host, NULL, NULL, &addrinfo) != 0) {
      fprintf(stderr, "%s: failed to lookup: %s.\n", argv[i], strerror(errno));
      continue;
    }

    // find first IP address and set the port number
    for(struct addrinfo* cur = addrinfo; cur != NULL; cur = cur->ai_next) {
      switch(cur->ai_family) {
        case AF_INET: {
          struct sockaddr_in* in = (struct sockaddr_in*) cur->ai_addr;
          in->sin_port = htons(hp->port);
          addr = (struct sockaddr*) in;
          break;
        }

        case AF_INET6: {
          struct sockaddr_in6* in6 = (struct sockaddr_in6*) cur->ai_addr;
          in6->sin6_port = htons(hp->port);
          addr = (struct sockaddr*) in6;
          break;
        }

        default: continue;
      }

      break;
    }

    if(addr == NULL) {
      fprintf(stderr, "%s: could not be resolved.\n", hp->host);
      continue;
    }

    struct mumble_query_reply reply;
    uint64_t id = randu64();

    // send query
    if(mumble_query(addr, id, &reply) != 0) {
      fprintf(stderr, "%s: failed to query: %s.\n", argv[i], strerror(errno));
      continue;
    }

    char version[16] = { 0 };
    mumble_snversion(version, sizeof(version), reply.version);

    printf("%s:%" PRIu16 ", %s, %" PRIu32 ", %" PRIu32 ", %" PRIu32 " bps\n",
           argv[i], hp->port, version,
           reply.users, reply.slots, reply.bandwidth);
  }

  return EXIT_SUCCESS;
}
