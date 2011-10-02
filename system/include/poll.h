
#ifdef __cplusplus
extern "C" {
#endif

#define POLLIN   1
#define POLLOUT  2
#define POLLNVAL 4
#define POLLERR  8

struct pollfd {
  int fd;
  short events;
  short revents;
};

int poll(pollfd *data, int num, int extra);

#ifdef __cplusplus
}
#endif

