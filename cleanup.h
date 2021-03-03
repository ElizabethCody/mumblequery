#pragma once

#include <stdio.h>
#include <unistd.h>

static inline void _closep(int* fd) {
  if(*fd != -1) {
    int error = close(fd);
    if(error != 0) {
      perror("_closep(): Failed to close");
    }
  }
}

#define _cleanup_close_ __attribute__((cleanup(_closep)))
