#pragma once

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "hostport.h"

static inline void _closep(int* fd) {
  if(*fd != -1) {
    int error = close(*fd);
    if(error != 0) {
      perror("_closep(): Failed to close");
    }
  }
}

static inline void _freestrp(char** ptr) {
  if(*ptr != NULL) {
    free(*ptr);
  }
}

static inline void _freeaddrinfop(struct addrinfo** ai) {
  if(*ai != NULL) {
    freeaddrinfo(*ai);
  }
}

static inline void _freehostportp(struct hostport** hp) {
  if(*hp != NULL) {
    freehostport(*hp);
  }
}

#define _cleanup_close_ __attribute__((cleanup(_closep)))
#define _cleanup_freestr_ __attribute__((cleanup(_freestrp)))
#define _cleanup_freeaddrinfo_ __attribute__((cleanup(_freeaddrinfop)))
#define _cleanup_hostport_ __attribute__((cleanup(_freehostportp)))
