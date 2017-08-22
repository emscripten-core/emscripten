#include <cassert>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
    const auto* const dirName = u8"абвг";

    auto ret = mkdir(dirName, 0777);
    assert(ret == 0);
    ret = chdir(dirName);
    assert(ret == 0);

    char buf[255];
    auto ret2 = getcwd(buf, sizeof(buf));
    assert(ret2 != 0);
    printf("ret2 = %s\n", ret2);
}
