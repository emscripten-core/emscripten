/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <dirent.h>
#include <emscripten/emscripten.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

// TODO: check different WasmFS backends.
// TODO: check ignore case backend mounted under a different backend.

void write_file(const char* fname) {
  FILE* fp = fopen(fname, "wt");
  assert(fp);
  char data[] = "test";
  printf("Write '%s' to '%s'\n", data, fname);
  assert(fputs(data, fp) >= 0);
  assert(fclose(fp) == 0);
}

void read_file(const char* fname) {
  FILE* fp = fopen(fname, "rt");
  assert(fp);
  char buffer[10] = {};
  assert(fgets(buffer, sizeof(buffer), fp) != NULL);
  printf("Read '%s' from '%s'\n", buffer, fname);
  assert(strcmp(buffer, "test") == 0);
  assert(fclose(fp) == 0);
}

int exists(const char* fname) {
  struct stat st = {};
  return stat(fname, &st) == 0 ? 1 : 0;
}

std::vector<std::string> readdir(const char* dname) {
  static std::vector<std::string> skip(
    {".", "..", "dev", "tmp", "home", "proc"});
  printf("Files in '%s': ", dname);
  std::vector<std::string> files;
  DIR* d = opendir(dname);
  if (d) {
    struct dirent* dir;
    while ((dir = readdir(d)) != NULL) {
      if (dir->d_type & (DT_DIR | DT_REG)) {
        if (std::find(skip.begin(), skip.end(), dir->d_name) == skip.end()) {
          files.emplace_back(dir->d_name);
          printf("%s ", dir->d_name);
        }
      }
    }
    closedir(d);
  }
  printf("\n");
  return files;
}

int main() {
  // Create a file.
  write_file("test.txt");

  // Read and check the file.
  struct stat st;
  assert(stat("test.txt", &st) == 0);
  assert(st.st_size == 4);
  assert(exists("test.TXT"));
  assert(exists("Test.Txt"));
  read_file("Test.txt");

  // Rename the file.
  assert(rename("tesT.Txt", "test2.txt") == 0);
  assert(exists("test2.txt"));
  assert(exists("Test2.txt"));
  read_file("Test2.txt");

  // Create a directory.
  assert(mkdir("Subdir", S_IRWXUGO) == 0);
  assert(exists("Subdir"));
  assert(exists("subdir"));
  assert(mkdir("SUBDIR", S_IRWXUGO) != 0);
  assert(errno == EEXIST);

  // Check parent directory contents.
  auto contents = readdir(".");
  assert(contents.size() == 2);
  assert(std::find(contents.begin(), contents.end(), "test2.txt") !=
         contents.end());
  assert(std::find(contents.begin(), contents.end(), "Subdir") !=
         contents.end());

  // Create a file in the directory.
  write_file("SubDir/Test.txt");
  assert(exists("subdir/test.txt"));
  read_file("subdir/Test.txt");

  // Check child directory contents and entries name.
  contents = readdir("subdir");
  assert(contents.size() == 1);
  assert(std::find(contents.begin(), contents.end(), "Test.txt") !=
         contents.end());

  // Delete a file from a directory.
#ifdef WASMFS
  assert(unlink("SUBDIR/TEST.TXT") == 0);
#else
  // bug in FS.unlink()
  assert(unlink("subdir/Test.txt") == 0);
#endif
  assert(!exists("subdir/test.txt"));
  assert(readdir("subdir").size() == 0);

  // Check current directory name and case.
  assert(chdir("subdir") == 0);
  char buffer[256];
  printf("getcwd: %s\n", getcwd(buffer, sizeof(buffer)));
#ifdef WASMFS
  assert(std::string(buffer).ends_with("Subdir"));
#else
  // original case is not preserved
  assert(std::string(buffer).ends_with("subdir"));
#endif
  assert(chdir("..") == 0);

#ifdef WASMFS
  // Create a directory to be overwritten by the subsequent rename.
  assert(mkdir("SUBDIR2", S_IRWXUGO) == 0);
  int overwritten_dir = open("subdir2", O_RDONLY | O_DIRECTORY);
  assert(overwritten_dir != -1);
#endif

  // Rename a directory.
  assert(rename("subdir", "Subdir2") == 0);
  assert(!exists("subdir"));
  assert(exists("subdir2"));

#ifdef WASMFS
  // Check that the overwritten dir was properly unlinked.
  int cwd = open(".", O_DIRECTORY | O_RDONLY);
  assert(cwd != 0);
  assert(fchdir(overwritten_dir) == 0);
  assert(getcwd(buffer, sizeof(buffer)) == NULL);
  assert(errno == ENOENT);
  assert(fchdir(cwd) == 0);
#endif

#ifdef WASMFS
  // bug in legacy FS
  // Check that the parent of a moved directory is set correctly.
  assert(mkdir("subdir2/subsubdir", S_IRWXUGO) == 0);
  assert(rename("SubDir2/SubSubDir", "SUBSUBDIR") == 0);
  assert(!exists("SUBDIR2/subsubdir"));
  assert(exists("SubSubDir"));
  assert(chdir("subsubdir") == 0);
  assert(getcwd(buffer, sizeof(buffer)) == buffer);
  assert(strcmp(buffer, "/SUBSUBDIR") == 0);
  assert(fchdir(cwd) == 0);
  assert(rmdir("subsubdir") == 0);
#endif

  // Create a file symlink.
  assert(symlink("test2.txt", "test2.txt.lnk") == 0);
  assert(exists("test2.txt.LNK"));
  read_file("Test2.txt.lnk");

  // Create a directory symlink.
  assert(symlink("SUBDIR2", "SUBDIR2.LNK") == 0);
  assert(exists("subdir2.lnk"));

  // Check symlinks.
  contents = readdir(".");
  assert(contents.size() == 4);
  assert(std::find(contents.begin(), contents.end(), "test2.txt.lnk") !=
         contents.end());
  assert(std::find(contents.begin(), contents.end(), "SUBDIR2.LNK") !=
         contents.end());

  // Delete symlinks.
  assert(unlink("Test2.txt.lnk") == 0);
  assert(unlink("Subdir2.lnk") == 0);
  assert(!exists("test2.txt.lnk"));
  assert(!exists("subdir2.lnk"));

  // Delete a directory.
  assert(rmdir("SUBDIR2") == 0);
  assert(!exists("SUBDIR2"));
  assert(!exists("Subdir2"));

  // Delete a file.
#ifdef WASMFS
  assert(unlink("TEST2.txt") == 0);
#else
  // bug in FS.unlink()
  assert(unlink("test2.txt") == 0);
#endif
  assert(!exists("TEST2.txt"));
  assert(!exists("test2.txt"));

  contents = readdir(".");
  assert(contents.size() == 0);

  printf("ok\n");

  return 0;
}
