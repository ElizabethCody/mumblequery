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

int mumble_query(const struct sockaddr_in* address, uint64_t id,
                 struct mumble_query_reply* reply) {
  // open a UDP socket to the server
  _cleanup_close_ int sock = socket(address->sin_family, SOCK_DGRAM, 0);

  if(sock == -1) {
    return errno;
  }

  struct timeval timeout = {
    .tv_sec  = 1, // seconds
    .tv_usec = 0  // microseconds
  };

  // set read timeout
  if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
    return errno;
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
  ssize_t sent = sendto(sock, &query, sizeof(query), MSG_CONFIRM,
                        (const struct sockaddr*) address, sizeof(query));

  if(sent != sizeof(query)) {
    return EIO;
  }

  // read reply
  socklen_t len = 0;
  ssize_t recv = recvfrom(sock, reply, sizeof(*reply), MSG_WAITALL,
                          (struct sockaddr*) address, &len);

  if(recv != sizeof(*reply)) {
    return EIO;
  }

  // correct endianness
  reply->id        = be64toh(reply->id);
  reply->users     = be32toh(reply->users);
  reply->slots     = be32toh(reply->slots);
  reply->bandwidth = be32toh(reply->bandwidth);

  if(reply->id != id) {
    return EIO;
  }

  // return zero for success.
  return 0;
}
