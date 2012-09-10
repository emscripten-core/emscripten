#ifndef _SELECT_H
#define _SELECT_H

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

#endif
