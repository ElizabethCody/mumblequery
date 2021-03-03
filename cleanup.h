#pragma once

#include <stdio.h>
#include <unistd.h>

static inline _closep(int* sock) {
  if(*sock != -1) {
    int error = close(sock);
    if(error != 0) {
      perror("_closep(): Failed to close");
    }
  }
}

#define _cleanup_close_ __attribute__((cleanup(_closep)))
