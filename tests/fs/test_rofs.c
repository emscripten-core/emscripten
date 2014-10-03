#include <stdio.h>
#include <emscripten.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

// struct used when a directory entry is a file (not another directory)
// and so has ro_dir_ent.is_dir = 0.  In this case, ro_dir_ent.ptr.file
// points to one of these
struct ro_file
{
  const char*  file_data;    // ptr to the data of the file
  size_t       file_data_sz; // size of the data pointed to by file_data
};

// struct used when a directory entry is another directory (not a file)
// and so has the owning ro_dir_ent.is_dir = 1.  In this case
// ro_dir_ent.ptr.dir points to one of these.
struct ro_dir {
  size_t                    num_ents;   // number of entries (files or other directories) in this directory
  const struct ro_dir_ent*  ents;       // pointer to the list of entries in this directory
};

// struct used to represent an entry in a directory.  Each entry
// can either represent a file or another directory.  If is_dir != 0
// then it is a directory, otherwise it is a file
struct ro_dir_ent
{
  const char* d_name;               // the name of the file or directory
  union {
    const struct ro_file*   file;   // if this is a file, then the pointer to the ro_file
    const struct ro_dir*    dir;    // otherwise if this a directory, the pointer to that ro_dir
  } ptr;
  int is_dir;                       // != 0 if this ro_dir_ent is a directory, otherwise == 0 (if it is a file)
};


#define file1_contents "contents of file1"
#define file2_contents "contents of file2"
#define file3_contents "yes, the contents of file3"

// static initialization of the data structures representing the three files
// with a directory structure looking like:
//
//  root
//      file1           -> contains file1_contents
//      file2           -> contains file2_contents
//      subdir
//          file3       -> contains file3_contents

struct ro_file file1 = { file1_contents, sizeof(file1_contents) - 1 }; // we'll not include the null byte
struct ro_file file2 = { file2_contents, sizeof(file2_contents) - 1 };
struct ro_file file3 = { file3_contents, sizeof(file3_contents) - 1 };

struct ro_dir_ent  subdir_ents[] = {
  {"file3",{&file3},0}
};
struct ro_dir subdir = { 1, &subdir_ents[0] };

struct ro_dir_ent  dir_ents[] = {
  {"file1",{&file1},0},
  {"file2",{&file2},0},
  {"subdir",{(const struct ro_file*)&subdir},1}
};
struct ro_dir root_dir = { 3, &dir_ents[0] };

// implemented in library_rofs.js, which must be included
// in the list of libraries compiled/linked by emcc.  See
// test_core.py, test_fs_rofs for an example of how to do this.
void rofs_mount(const char* path, const struct ro_dir* root);

int main() {

  int fd;

  mkdir("/readonly", 0777);
  rofs_mount("/readonly", &root_dir);
  
  struct stat st;
  if (stat("/readonly", &st) != 0) {
    printf("stat(\"/readonly\", &st) failed with errno: %d\n", errno);
    return 0;
  }

  if (st.st_mode != (S_IFDIR | S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH)) {
    printf("stat(\"/readonly\", &st) incorrectly reported st_mode as 0%o\n", st.st_mode);
    return 0;
  }
  
  fd = open("/readonly/file1", O_RDONLY);
  if (fd == -1) {
    printf("open(\"/readonly/file1\", O_RDONLY) failed with errno: %d\n", errno);
    return 0;
  }
  close(fd);
  
  fd = open("/readonly/file1", O_RDWR);
  if (fd != -1 || errno != EROFS) {
    printf("open(\"/readonly/file1\", O_RDWR) returned %d, errno: %d\n", fd, errno);
    return 0;
  }
  
  fd = open("/readonly/does_not_exist", O_RDONLY | O_CREAT, 0666);
  if (fd != -1 || errno != EROFS) {
    printf("open(\"/readonly/does_not_exist\", O_RDONLY | O_CREAT, 0666) returned %d, errno: %d\n", fd, errno);
    return 0;
  }
  
  fd = open("/readonly/subdir/file3", O_RDONLY);
  if (fd == -1) {
    printf("open(\"/readonly/subdir/file3\", O_RDONLY) failed with errno: %d\n", errno);
    return 0;
  }
  close(fd);
  
  if (stat("/readonly/subdir/file3", &st) != 0) {
    printf("stat(\"/readonly/subdir/file3\", &st) failed with errno: %d\n", errno);
    return 0;
  }
  
  if (st.st_size != sizeof(file3_contents) - 1) {
    printf("stat(\"/readonly/subdir/file3\", &st) reported file size as %d instead of %d\n", (int)(st.st_size), (int)(sizeof(file3_contents) - 1));
    return 0;
  }

  printf("success\n");
  return 0;
}
