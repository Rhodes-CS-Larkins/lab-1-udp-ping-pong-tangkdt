/*
 * pong.c - UDP ping/pong server code
 *          author: Kerry Tang
 */
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "util.h"

#define PORTNO "1266"
#define MAX_ARR 1024

int server_setup(char *port);

int server_setup(char *port) {
  int sockfd, rv;
  struct addrinfo hints, *servinfo, *p;
  
  // Set criteria for returned sock addrs
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // IPV4
  hints.ai_socktype = SOCK_DGRAM; // UDP
  hints.ai_flags = AI_PASSIVE; // Bind to wildcard addr
  hints.ai_protocol = 0; // Autoselect protocol -> IPPROTO_UDP in this case
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  rv = getaddrinfo(NULL, port, &hints, &servinfo);
  if (rv != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(EXIT_FAILURE);
  }

  // Attempt to bind to avail socket addr
  for (p = servinfo; p != NULL; p = p->ai_next) {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) {continue;} 
    
    // Success!
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == 0) {break;}
    close(sockfd);
  }

  // No addr succeeded 
  if (!p) {
    fprintf(stderr, "Could not bind\n");
    exit(EXIT_FAILURE);
  }
 
  return sockfd;
}

int main(int argc, char **argv) {
  int ch, sockfd;
  int nping = 1;                    // Default packet count
  char *pongport = strdup(PORTNO);  // Default port
  struct sockaddr_storage peer_addr; 
  socklen_t peer_addr_len;
  ssize_t nread, nwritten; 
  char ip[INET_ADDRSTRLEN];
  unsigned char buf[MAX_ARR];

  while ((ch = getopt(argc, argv, "h:n:p:")) != -1) {
    switch (ch) {
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    default:
      fprintf(stderr, "usage: pong [-n #pings] [-p port]\n");
    }
  }

  // Create UDP server socket
  sockfd = server_setup(pongport);

  for (int i = 0; i < nping; i++) {
    peer_addr_len = sizeof(peer_addr); // Reset

    // Recv arr from client
    nread = recvfrom(sockfd, buf, MAX_ARR, 0, (struct sockaddr *) &peer_addr, &peer_addr_len);
    if (nread == -1) {
      fprintf(stderr, "Error reading\n");
      continue;
    }

    // Retrieve IP 
    inet_ntop(peer_addr.ss_family, get_in_addr((struct sockaddr *) &peer_addr), ip, sizeof(ip));
    printf("pong[%d]: received packet from %s\n", i, ip);

    // Add one to every element in arr
    for (int i = 0; i < nread; i++) {buf[i] ++;}

    // Send arr back to client
    nwritten = sendto(sockfd, buf, nread, 0, (struct sockaddr *) &peer_addr, peer_addr_len);

    // Result must match length sent
    if (nwritten != nread) {
      fprintf(stderr, "Error sending response\n");
    }
  }

  // Cleanup
  close(sockfd);
  free(pongport);

  return 0;
}

