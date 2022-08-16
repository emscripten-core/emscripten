#ifndef __TEST_SOCKETS_MSG_H__
#define __TEST_SOCKETS_MSG_H__

typedef struct {
  char *buffer;
  int length;
} msg_t;

int do_msg_read(int sockfd, msg_t *msg, int offset, int length, struct sockaddr *addr, socklen_t *addrlen) {
  int res;

  if (!msg->length) {
    // read the message length
    res = recvfrom(sockfd, &msg->length, sizeof(int), 0, addr, addrlen);
    if (res == -1) {
      assert(errno == EAGAIN);
      return res;
    } else if (res == 0) {
      return 0;
    }

    printf("do_msg_read: allocating %d bytes for message\n", msg->length);

    msg->buffer = malloc(msg->length);
  }

  // read the actual message
  int max = msg->length - offset;
  if (length && max > length) {
    max = length;
  }
  res = recvfrom(sockfd, msg->buffer + offset, max, 0, addr, addrlen);
  if (res == -1) {
    assert(errno == EAGAIN);
    return res;
  }

  printf("do_msg_read: read %d bytes\n", res);

  return res;
}

int do_msg_write(int sockfd, msg_t *msg, int offset, int length, struct sockaddr *addr, socklen_t addrlen) {
  int res;

  // send the message length first
  if (!offset) {
    if (addr) {
      res = sendto(sockfd, &msg->length, sizeof(int), 0, addr, addrlen);
    } else {
      res = send(sockfd, &msg->length, sizeof(int), 0);
    }
    if (res == -1) {
      assert(errno == EAGAIN);
      return res;
    }
    printf("do_msg_write: sending message header for %d bytes\n", msg->length);
    assert(res == sizeof(int));
  }

  // then the actual message
  int max = msg->length - offset;
  if (length && max > length) {
    max = length;
  }
  if (addr) {
    res = sendto(sockfd, msg->buffer + offset, max, 0, addr, addrlen);
  } else {
    res = send(sockfd, msg->buffer + offset, max, 0);
  }
  if (res == -1) {
    assert(errno == EAGAIN);
    return res;
  }

  printf("do_msg_write: wrote %d bytes %d\n", res, msg->length);

  return res;
}

#endif
