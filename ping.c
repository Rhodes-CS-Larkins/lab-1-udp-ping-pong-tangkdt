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
#define INITIAL_VAL ((unsigned char) 200)

int client_setup(char *host, char *pongport);

int client_setup(char *host, char *pongport) {
  struct addrinfo hints, *servinfo, *p;
  int sockfd, rv;

  // Set criteria for returned socket addrs 
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;      // IPv4
  hints.ai_socktype = SOCK_DGRAM; // UDP
  hints.ai_protocol = 0;          // Autoselect protocol -> IPPROTO_UDP in this case
                         
  rv = getaddrinfo(host, pongport, &hints, &servinfo);
  if (rv != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return -1;
  }

  // Attempt to connect to avail addrs
  for (p = servinfo; p != NULL; p = p->ai_next) {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) {continue;}

    // Success!
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) != -1) {break;}
    close(sockfd);
  }

  freeaddrinfo(servinfo);

  // No addr succeeded
  if (!p) {
    fprintf(stderr, "Could not connect\n");
    return -1;
  }
  
  return sockfd;
}

int main(int argc, char **argv) {
  double start, end, RTT, total_RTT = 0.0, avg_RTT;
  int ch, sockfd, errors = 0;
  ssize_t nwritten, nread;
  int nping = 1;                        // Default packet count
  char *ponghost = strdup("localhost"); // Default host
  char *pongport = strdup(PORTNO);      // Default port
  int arraysize = 100;                  // Default packet size
  unsigned char *send_buf, *recv_buf;
  unsigned char expected_val;
                                        
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
  send_buf = (unsigned char *) malloc(arraysize);
  recv_buf = (unsigned char *) malloc(arraysize);

  if (!send_buf || !recv_buf) {
    perror("Malloc failed!");
    free(send_buf); free(recv_buf);
    free(ponghost); free(pongport);
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < arraysize; i++) {send_buf[i] = INITIAL_VAL;}
  expected_val = (INITIAL_VAL + 1) % 255; 

  // Create UDP client socket
  sockfd = client_setup(ponghost, pongport);
  if (sockfd == -1) {
    fprintf(stderr, "Failed to create UDP client socket");
    free(send_buf); free(recv_buf);
    free(ponghost); free(pongport);
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < nping; i++) {
    start = get_wctime();

    // Send arr to server
    nwritten = write(sockfd, send_buf, (size_t) arraysize);
    if (nwritten != arraysize) {
      errors += 1;
      fprintf(stderr, "Partial/failed write\n");
    }

    // Wait and recv modified arr from server
    nread = read(sockfd, recv_buf, (size_t) arraysize);
    if (nread != arraysize) {
      errors += 1;
      fprintf(stderr, "Partial/failed read\n");
    }

    end = get_wctime();

    // Validate results from server
    for (int j = 0; j < arraysize; j++) {
      if (recv_buf[j] != expected_val) {
        fprintf(stderr, "Server did not modify arr correctly\n");
        errors += 1;
      }
    }

    // Output round-trip times 
    RTT = (end - start) * 1000; // Secs -> ms
    printf("ping[%d] : round-trip time: %.3lf ms\n", i, RTT);
    total_RTT += RTT;
  }

  avg_RTT = total_RTT / nping;
  if (errors == 0) {printf("no errors detected\n");}
  printf("time to send %d packets of %d bytes %.3lf ms (%.3lf avg per packet)\n", nping, arraysize, total_RTT, avg_RTT);

  // Cleanup
  close(sockfd);
  free(send_buf); free(recv_buf);
  free(ponghost); free(pongport);

  return 0;
}
