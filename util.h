/*
 * utility functions
 * (c) 2020 D. Brian Larkins - larkinsb@rhodes.edu
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

double get_wctime(void);
void *get_in_addr(struct sockaddr *sa);
