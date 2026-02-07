/*
 * utility functions
 * (c) 2020 D. Brian Larkins - larkinsb@rhodes.edu
 */
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "util.h"

/*
 * get_wctime() - returns wall clock time in seconds as double
 */
double get_wctime(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec + 1E-6 * tv.tv_usec);
} 

/**
 * get_in_addr - Helper funct to get ptr to right addr
 * @param sa - IPv4 or IPv6 sockaddr 
 * @Returns ptr to in_addr
 */
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *) sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}
