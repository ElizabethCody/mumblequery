#include <stdio.h>
#include <inttypes.h> // for PRIxxx constants.
#include "mumblequery.h"

int mumble_snversion(char* buffer, size_t size, union mumble_version version) {
  return snprintf(buffer, size,
                  "%" PRIu8 ".%" PRIu8 ".%" PRIu8,
                  version.part[1], version.part[2], version.part[3]);
}
