#include <stdio.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/sendfile.h>

static void test_without_sendfile_offset(void)
{
    int i, fd1, fd2;
    char buf[5000];
    ssize_t written, readed;
    off_t offset;

    for (i = 0; i < 5000; i++) {
        buf[i] = (char) i;
    }

    fd1 = open("/testfile1", O_WRONLY | O_CREAT, 0666);
    assert(fd1 >= 0);

    written = write(fd1, buf, 5000);
    assert(written == 5000);

    close(fd1);

    fd1 = open("/testfile1", O_RDONLY, 0666);
    assert(fd1 >= 0);

    fd2 = open("/testfile2", O_WRONLY | O_CREAT, 0666);
    assert(fd2 >= 0);

    written = sendfile(fd2, fd1, NULL, 5000);
    printf("written = %d\n", (int) written);
    assert(written == 5000);

    offset = lseek(fd1, 0, SEEK_CUR);
    printf("offset = %d\n", (int) offset);
    assert(offset == 5000);

    close(fd2);

    fd2 = open("/testfile1", O_RDONLY, 0666);
    assert(fd2 >= 0);

    readed = read(fd2, buf, 5000);
    printf("readed = %d\n", (int) readed);
    assert(readed == 5000);

    for (i = 0; i < 5000; i++) {
        assert(buf[i] == (char) i);
    }

    close(fd1);
    close(fd2);
    unlink("/testfile1");
    unlink("/testfile2");
}

static void test_with_sendfile_offset(void)
{
    int i, fd1, fd2;
    char buf[5000];
    ssize_t written, readed;
    off_t offset, read_offset;

    for (i = 0; i < 5000; i++) {
        buf[i] = (char) i;
    }

    fd1 = open("/testfile1", O_WRONLY | O_CREAT, 0666);
    assert(fd1 >= 0);

    written = write(fd1, buf, 5000);
    assert(written == 5000);

    close(fd1);

    fd1 = open("/testfile1", O_RDONLY, 0666);
    assert(fd1 >= 0);

    fd2 = open("/testfile2", O_WRONLY | O_CREAT, 0666);
    assert(fd2 >= 0);

    read_offset = 0;
    written = sendfile(fd2, fd1, &read_offset, 5000);
    printf("written = %d\n", (int) written);
    printf("read_offset = %d\n", (int) read_offset);
    assert(written == 5000);
    assert(read_offset == 5000);

    offset = lseek(fd1, 0, SEEK_CUR);
    printf("offset = %d\n", (int) offset);
    assert(offset == 0);

    close(fd2);

    fd2 = open("/testfile1", O_RDONLY, 0666);
    assert(fd2 >= 0);

    readed = read(fd2, buf, 5000);
    printf("readed = %d\n", (int) readed);
    assert(readed == 5000);

    for (i = 0; i < 5000; i++) {
        assert(buf[i] == (char) i);
    }

    close(fd1);
    close(fd2);
    unlink("/testfile1");
    unlink("/testfile2");
}

int main(void)
{
    test_without_sendfile_offset();
    test_with_sendfile_offset();
    return 0;
}
