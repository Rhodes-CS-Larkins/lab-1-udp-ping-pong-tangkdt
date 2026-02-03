/*
 * ping.c - UDP ping/pong client code
 *          author: Kerry Tang
 */
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "util.h"

#define PORTNO "1266"

int client_setup(char *host, char *pongport);

int client_setup(char *host, char *pongport) {
  int sockfd, rv;
  struct addrinfo hints, *servinfo, *p;

  // Set criteria for returned socket addrs 
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;      // IPv4
  hints.ai_socktype = SOCK_DGRAM; // UDP
  hints.ai_protocol = 0;          // Autoselect protocol -> IPPROTO_UDP in this case
                         
  rv = getaddrinfo(host, pongport, &hints, &servinfo);
  if (rv != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(EXIT_FAILURE);
  }

  // Attempt to connect to avail addrs
  for (p = servinfo; p != NULL; p = p->ai_next) {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) {continue;}

    // Success!
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) != -1) {break;}
    close(sockfd);
  }

  // No addr succeeded
  if (!p) {
    fprintf(stderr, "Could not connect\n");
    exit(EXIT_FAILURE);
  }
  
  return sockfd;
}

int main(int argc, char **argv) {
  double start, end, RTT, total_RTT = 0.0, avg_RTT;
  int ch, errors = 0;
  int nping = 1;                        // Default packet count
  char *ponghost = strdup("localhost"); // Default host
  char *pongport = strdup(PORTNO);      // Default port
  int arraysize = 100;                  // Default packet size
  int sockfd;
  size_t bufsize;
  ssize_t nwritten, nread;
                                        
  while ((ch = getopt(argc, argv, "h:n:p:s:")) != -1) {
    switch (ch) {
    case 'h':
      ponghost = strdup(optarg);
      break;
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    case 's':
      arraysize = atoi(optarg);
      break;
    default:
      fprintf(stderr, "usage: ping [-h host] [-n #pings] [-p port] [-s size]\n");
    }
  }

  // Init arr
  bufsize = arraysize * sizeof(int);
  int *buf = malloc(bufsize);
  if (!buf) {
    perror("Malloc failed!");
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < arraysize; i++) {buf[i] = 200;}

  sockfd = client_setup(ponghost, pongport);

  for (int i = 0; i < nping; i++) {
    start = get_wctime();

    // Send arr to server
    nwritten = write(sockfd, buf, bufsize);
    if (nwritten != arraysize) {
      errors += 1;
      fprintf(stderr, "Partial/failed write\n");
    }

    // Wait and recv modified arr from server
    nread = read(sockfd, buf, bufsize);
    if (nread == -1) {
      perror("read");
      errors += 1;
    }

    end = get_wctime();

    // Validate results from pong server

    // Print round-trip times 
    RTT = (end - start) * 1000; // Secs -> ms
    printf("ping[%d] : round-trip time: %lf ms\n", i, RTT);
    total_RTT += RTT;
  }

  avg_RTT = total_RTT / nping;
  if (errors < 0) {printf("no errors detected");}
  printf("time to send %d packets of %d bytes %lf (%lf avg per packet)\n", nping, arraysize, total_RTT, avg_RTT);

  free(buf);

  return 0;
}
