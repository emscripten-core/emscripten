#include <errno.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/sendfile.h>

static ssize_t write_all(int out_fd, char* buf, size_t count)
{
    ssize_t written;
    size_t left = count, pos = 0;

    while (left > 0) {
        written = write(out_fd, buf + pos, left);
        if (written < 0) {
            return -1;
        } else {
            left -= written;
            pos += written;
        }
    }

    return (ssize_t) pos;
}

ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
    char buf[4096];
    size_t left = count;
    ssize_t total_written = 0, readed, written;
    off_t initial_in_offset;

    if (count > SSIZE_MAX) {
        errno = EINVAL;
        return -1;
    }

    if (offset != NULL) {
        initial_in_offset = lseek(in_fd, 0, SEEK_CUR);
        if (initial_in_offset == -1) {
            return -1;
        }
    }

    while (left > 0) {
        readed = read(in_fd, buf, MIN(left, 4096));
        if (readed < 0) {
            return -1;
        } else if (readed == 0) {
            break;
        } else {
            written = write_all(out_fd, buf, (size_t) readed);
            if (written < 0) {
                return -1;
            } else {
                total_written += readed;
                left -= total_written;
            }
        }
    }

    if (offset != NULL) {
        lseek(in_fd, initial_in_offset, SEEK_SET);
        *offset = initial_in_offset + total_written;
    }

    return total_written;
}
