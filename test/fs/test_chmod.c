#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <emscripten/emscripten.h>

int main() {
    EM_ASM(
        FS.writeFile('testfile', 'a=1\nb=2\n');
        FS.mkdir('/folder');
    );

    struct stat fileStat;
    struct stat symlinkStat;
    struct stat folderStat;

    EM_ASM(
        FS.chmod("testfile", 0700);
    );

    stat("testfile", &fileStat);
    assert(fileStat.st_mode & (S_IRUSR | S_IWUSR | S_IXUSR));
    assert(!(fileStat.st_mode & (S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH)));

    EM_ASM(
        FS.chmod("testfile", 0444);
    );

    stat("testfile", &fileStat);
    assert(fileStat.st_mode & (S_IRUSR | S_IRGRP | S_IROTH));
    assert(!(fileStat.st_mode & (S_IWUSR | S_IXUSR | S_IWGRP | S_IXGRP | S_IWOTH | S_IXOTH)));

    EM_ASM(
        FS.chmod("testfile", 0);
    );

    stat("testfile", &fileStat);
    assert(!(fileStat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)));

    chmod("testfile", S_IRWXU);

#if WASMFS
    EM_ASM(
        var fd = FS.open('testfile', 'r');
        FS.fchmod(fd, 0444);
    );
#else
    EM_ASM(
        var stream = FS.open('testfile', 'r');
        FS.fchmod(stream.fd, 0444);
    );
#endif

    stat("testfile", &fileStat);
    assert(fileStat.st_mode & (S_IRUSR | S_IRGRP | S_IROTH));
    assert(!(fileStat.st_mode & (S_IWUSR | S_IXUSR | S_IWGRP | S_IXGRP | S_IWOTH | S_IXOTH)));

#if WASMFS
    EM_ASM(
        var fd = FS.open('testfile', 'r');
        FS.fchmod(fd, 0666);
    );
#else
    EM_ASM(
        var stream = FS.open('testfile', 'r');
        FS.fchmod(stream.fd, 0666);
    );
#endif

    stat("testfile", &fileStat);
    assert(fileStat.st_mode & (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
    assert(!(fileStat.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)));;

    EM_ASM(
        FS.symlink("testfile", "symlinkFile");
        FS.lchmod("symlinkFile", 0000);
    );

    lstat("symlinkFile", &symlinkStat);
    assert(S_ISLNK(symlinkStat.st_mode));
    assert(!(symlinkStat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXU)));
    
    stat("testfile", &fileStat);
    assert(fileStat.st_mode & (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
    assert(!(fileStat.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)));;

    EM_ASM(
        FS.chmod("/folder", 0700);
    );

    stat("/folder", &folderStat);
    assert(S_ISDIR(folderStat.st_mode));
    assert(folderStat.st_mode & S_IRWXU);
    assert(!(folderStat.st_mode & (S_IRWXG | S_IRWXO)));

#if WASMFS
    EM_ASM(
        var fd = FS.open("/folder");
        FS.fchmod(fd, 0000);
    );
#else
    EM_ASM(
        var stream = FS.open("/folder");
        FS.fchmod(stream.fd, 0000);
    );
#endif
    stat("/folder", &folderStat);
    assert(S_ISDIR(folderStat.st_mode));
    assert(!(folderStat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)));

    EM_ASM(
        FS.symlink("/folder", "/symlinkFolder");
        FS.lchmod("/symlinkFolder", 0444);
    );

    lstat("/symlinkFolder", &folderStat);
    assert(S_ISLNK(folderStat.st_mode));
    assert(folderStat.st_mode & (S_IRUSR | S_IRGRP | S_IROTH));
    assert(!(folderStat.st_mode & (S_IWUSR | S_IXUSR | S_IWGRP | S_IXGRP | S_IWOTH | S_IXOTH)));

    stat("/folder", &folderStat);
    assert(S_ISDIR(folderStat.st_mode));
    assert(!(folderStat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)));

    
    puts("success");
}