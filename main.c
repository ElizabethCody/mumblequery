#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include "mumblequery.h"
#include "cleanup.h"

#define MUMBLE_DEFAULT_PORT 64738
#define CHECKED_MALLOC(dst, size) do { \
  dst = malloc(size); \
  if(dst == NULL) { \
    perror("malloc() failed"); \
    return EXIT_FAILURE; \
  } \
} while(0)

int main(int argc, char* argv[]) {
  if(argc < 2) {
    fprintf(stderr, "Usage: %s <host[:port]>...\n", argv[0]);
    return EXIT_FAILURE;
  }

  srand(time(NULL));

  for(size_t i = 1; i < argc; ++i) {
    _cleanup_freestr_ char* host = NULL;
    uint16_t port = MUMBLE_DEFAULT_PORT;

    // check for an ipv6 address
    char* v6 = strchr(argv[i], '[');
    char* remainder;
    if(v6 == NULL) {
      // host is a hostname or v4 address.
      remainder = argv[i];
    } else {
      // host is a v6 address
      char* close = strchr(v6, ']');
      if(close == NULL || close < v6) {
        fprintf(stderr, "%s: invalid host\n", host);
        continue;
      }
      size_t v6len = close - v6 - 1;
      CHECKED_MALLOC(host, v6len + 1);
      strncpy(host, v6 + 1, v6len);
      host[v6len] = '\0';
      remainder = close + 1;
    }

    // try to parse port
    char* porta = strchr(remainder, ':');
    if(porta == NULL || porta < remainder) {
      if(host == NULL) {
        // there is no port, the remainder is the host.
        host = strdup(remainder);
      } else {
        // nothing to do, we already know the host and a port was not supplied.
      }
    } else {
      // pull out port. on error, this *should* fail safely and
      // consequently fallback to the default port.
      sscanf(porta + 1 /* skip past colon */, " %" SCNu16, &port);

      if(host == NULL) {
        // pull out host
        size_t hostlen = porta - remainder;
        CHECKED_MALLOC(host, hostlen + 1);
        strncpy(host, argv[i], hostlen);
        host[hostlen] = '\0';
      }
    }

    if(strlen(host) < 1) {
      fprintf(stderr, "%s: invalid host\n", argv[i]);
      continue;
    }

    struct sockaddr* addr = NULL;
    _cleanup_freeaddrinfo_ struct addrinfo* addrinfo = NULL;

    // lookup hostname
    if(getaddrinfo(host, NULL, NULL, &addrinfo) != 0) {
      fprintf(stderr, "%s: ", host);
      perror("getaddrinfo() failed");
      continue;
    }

    // find first IP address and set the port number
    for(struct addrinfo* cur = addrinfo; cur != NULL; cur = cur->ai_next) {
      switch(cur->ai_family) {
        case AF_INET: {
          struct sockaddr_in* in = (struct sockaddr_in*) cur->ai_addr;
          in->sin_port = htons(port);
          addr = (struct sockaddr*) in;
          break;
        }

        case AF_INET6: {
          struct sockaddr_in6* in6 = (struct sockaddr_in6*) cur->ai_addr;
          in6->sin6_port = htons(port);
          addr = (struct sockaddr*) in6;
          break;
        }

        default: continue;
      }

      break;
    }

    if(addr == NULL) {
      fprintf(stderr, "%s: could not be resolved.\n", host);
      continue;
    }

    struct mumble_query_reply reply;
    uint64_t id = (uint64_t) rand() << UINT64_C(32) | (uint64_t) rand();

    // send query
    if(mumble_query(addr, id, &reply) != 0) {
      perror("mumble_query() failed");
      return EXIT_FAILURE;
    }

    char version[16] = { 0 };
    mumble_snversion(version, sizeof(version), reply.version);

    printf("%s:%" PRIu16 ", %s, %" PRIu32 ", %" PRIu32 ", %" PRIu32 " bps\n",
           host, port, version, reply.users, reply.slots, reply.bandwidth);
  }

  return EXIT_SUCCESS;
}
