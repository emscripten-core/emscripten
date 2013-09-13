#include <errno.h>
#include <unistd.h>
#include <sys/uio.h>

ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{
    int i;
    ssize_t total_readed = 0;

    if (iov == NULL || iovcnt < 0) {
        errno = EINVAL;
        return -1;
    }

    for (i = 0; i < iovcnt; i++) {
        int readed;

        readed = read(fd, iov[i].iov_base, iov[i].iov_len);
        if (readed < 0) {
            return -1;
        } else if (readed == 0) {
            break;
        } else {
            total_readed += readed;
            if (readed < iov[i].iov_len) {
                break;
            }
        }
    }

    return total_readed;
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
    int i;
    ssize_t total_written = 0;

    if (iov == NULL || iovcnt < 0) {
        errno = EINVAL;
        return -1;
    }

    for (i = 0; i < iovcnt; i++) {
        int written;

        written = write(fd, iov[i].iov_base, iov[i].iov_len);
        if (written < 0) {
            return -1;
        } else if (written == 0) {
            break;
        } else {
            total_written += written;
            if (written < iov[i].iov_len) {
                break;
            }
        }
    }

    return total_written;
}
