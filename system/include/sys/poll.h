
#ifndef _SYS_POLL_H
#define _SYS_POLL_H

#ifdef __cplusplus
extern "C" {
#endif

#define POLLIN   1
#define POLLOUT  2
#define POLLNVAL 4
#define POLLERR  8
#define POLLHUP 16
#define POLLPRI 32

struct pollfd {
  int fd;
  short events;
  short revents;
};

typedef unsigned int nfds_t;

int poll(struct pollfd *data, nfds_t num, int extra);

#ifdef __cplusplus
}
#endif

#endif

