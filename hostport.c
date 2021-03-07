#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include "hostport.h"

int hostport(char* str, uint16_t dport, struct hostport** dst) {
  struct hostport* hp = malloc(sizeof(*hp));

  if(hp == NULL) {
    errno = ENOMEM;
    return -1;
  }

  hp->host = NULL;
  hp->port = dport;

  // check for an ipv6 address
  char* v6 = strchr(str, '[');
  char* remainder;

  if(v6 == NULL) {
    // host is a hostname or an ipv4 address.
    remainder = str;
  } else {
    char* close = strchr(v6, ']');

    if(close == NULL || close < v6) {
      freehostport(hp);
      errno = EINVAL;
      return -1;
    }

    // we've found an ipv6 address, pull it out into the struct.
    size_t v6len = close - v6 - 1;
    hp->host = malloc(v6len + 1);

    if(hp->host == NULL) {
      freehostport(hp);
      errno = ENOMEM;
      return -1;
    }

    strncpy(hp->host, v6 + 1, v6len);
    hp->host[v6len] = '\0';
    remainder = close + 1;
  }

  // try to parse port
  char* porta = strchr(remainder, ':');
  if(porta == NULL || porta < remainder) {
    // no port was supplied
    if(hp->host == NULL) {
      // we still don't know the host, assume it's the whole string
      hp->host = strdup(remainder);
    }

    // one might expect a fallback to the default port here, but the value of
    // hp->port is already dport.
  } else {
    // pull out port. on error, this *should* fail safely and
    // consequently fallback to dport.
    sscanf(porta + 1 /* skip past colon */, " %" SCNu16, &hp->port);

    if(hp->host == NULL) {
      // we still don't know the host, pull out everything before the colon.
      size_t hostlen = porta - remainder;
      hp->host = malloc(hostlen + 1);

      if(hp->host == NULL) {
        freehostport(hp);
        errno = ENOMEM;
        return -1;
      }

      strncpy(hp->host, remainder, hostlen);
      hp->host[hostlen] = '\0';
    }
  }

  // ensure host isn't all whitespace/empty
  int empty = 1;
  for(char* host = hp->host; empty && *host != '\0'; ++host) {
    empty = *host == ' '  ||
            *host == '\r' ||
            *host == '\n' ||
            *host == '\t' ||
            *host == '\f';
  }

  if(empty) {
    freehostport(hp);
    errno = EINVAL;
    return -1;
  }

  *dst = hp;
  return 0;
}

void freehostport(struct hostport* hp) {
  if(hp != NULL) {
    if(hp->host != NULL) {
      free(hp->host);
    }

    free(hp);
  }
}
