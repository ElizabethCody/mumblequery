#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <endian.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <inttypes.h> // for PRIxxx constants.
#include "mumblequery.h"
#include "cleanup.h"

int mumble_snversion(char* buffer, size_t size, union mumble_version version) {
  return snprintf(buffer, size,
                  "%" PRIu8 ".%" PRIu8 ".%" PRIu8,
                  version.part[1], version.part[2], version.part[3]);
}

int mumble_query(struct sockaddr* addr, uint64_t id,
                 struct mumble_query_reply* reply) {
  errno = EIO;

  // open a UDP socket to the server
  _cleanup_close_ int sock = socket(addr->sa_family, SOCK_DGRAM, 0);

  if(sock == -1) {
    // socket() sets errno
    return -1;
  }

  struct timeval timeout = {
    .tv_sec  = 1, // seconds
    .tv_usec = 0  // microseconds
  };

  // set read timeout
  int status = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
                          &timeout, sizeof(timeout));
  if(status == -1) {
    // setsockopt() sets errno
    return -1;
  }

  // prepare query
  struct {
    uint32_t type;
    uint64_t id;
  } __attribute__((packed)) query = {
    .type = htobe32(0),
    .id   = htobe64(id)
  };

  // send query
  ssize_t sent = sendto(sock, &query, sizeof(query),
                        MSG_CONFIRM, addr, sizeof(*addr));
  if(sent == -1 || sent != sizeof(query)) {
    // sendto() *might have* set errno
    return -1;
  }

  // read reply
  socklen_t len = 0;
  ssize_t recv = recvfrom(sock, reply, sizeof(*reply),
                          MSG_WAITALL, addr, &len);
  if(recv == -1 || recv != sizeof(*reply)) {
    // recvfrom() *might have* set errno
    return -1;
  }

  // correct endianness
  reply->id        = be64toh(reply->id);
  reply->users     = be32toh(reply->users);
  reply->slots     = be32toh(reply->slots);
  reply->bandwidth = be32toh(reply->bandwidth);

  // ensure packet id matches
  if(reply->id != id) {
    return -1;
  }

  // success.
  errno = 0;
  return 0;
}
