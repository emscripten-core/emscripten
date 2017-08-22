#include <fstream>
#include <cassert>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

void touch(const char* fileName)
{
    std::ofstream f(fileName, std::ios_base::out);
    assert(f.good());
}

int main()
{
    auto ret = mkdir("test_dir", 0777);
    assert(ret == 0);

    touch(u8"test_dir/абвг");

    auto fd = open("test_dir", O_RDONLY | O_DIRECTORY);
    assert(fd > 0);

    for (;;)
    {
        dirent d;
        auto nread = getdents(fd, &d, sizeof(d));
        assert(nread != -1);
        if (nread == 0)
        {
            break;
        }

        if (strcmp(d.d_name, ".") == 0 || strcmp(d.d_name, "..") == 0)
        {
            continue;
        }

        printf("d.d_name = %s\n", d.d_name);
    }
}
