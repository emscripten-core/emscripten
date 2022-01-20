#ifndef NATIVEFS_H
#define NATIVEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>

#define NATIVEFS_SEEK_SET 0
#define NATIVEFS_SEEK_CUR 1
#define NATIVEFS_SEEK_END 2

/*
  [main thread]
  @function nativefs_init

  @param element - The html element that will hold the
  event handler that will select the file(s).

  @discussion This function will set click handler callback on element. 
  If called with NULL, it will default to "NATIVEFS_button", else, we
  need to provide an element id prefixed with "#"
  defined in src/shell.html

  @result 0 if no issue is detected, (does not mean the push_file message 
  was received). -1 if it was called from a pthread.
*/
int nativefs_init(const char *element);

/*
  [main thread]
  @function nativefs_get_fd_from_filename

  @param name - The name of the file (encoded in utf8)
  @discussion Lookup the files array in the main thread until we find
  a match for the name of the file we want. This function will of course fail
  if we call it before the user selected a file.

  @result valid file descriptor, -1 if the file was not found
*/
int nativefs_get_fd_from_filename(const char *name);

/*
  [pthread]
  @function nativefs_read

  @param fd - File descriptor of a file previously selected in the file picker
  @param buffer - allocated destination in linear memory to write the file content to
  @param size - size of the buffer to read

  @discussion Synchronous function that will read size bytes into buffer for the file (represented by fd)

  @result -1 if there was an error, or the number of bytes read
*/
ssize_t nativefs_read(int fd, void *buffer, size_t size);

/*
  [pthread]
  @function nativefs_seek

  @param fd - File descriptor of a file previously selected in the file picker
  @param offset - new value of the offset
  @param whence - specifies how to apply the offset (NATIVEFS_SEEK_SET, NATIVEFS_SEEK_CUR, NATIVEFS_SEEK_END)

  @discussion Synchronous function that will set the file offset.
  NATIVEFS_SEEK_SET: absolute
  NATIVEFS_SEEK_CUR: relative to current offset
  NATIVEFS_SEEK_END: relative to the end of the file

  @result -1 if there was an error, or the new offset of the file.
*/
ssize_t nativefs_seek(int fd, size_t offset, int whence);

/*
  [pthread]
  @function nativefs_close_file

  @param fd - File descriptor of a file previously selected in the file picker
  @discussion This function will lookup the file and mark it for gc.
  @result -1 if the fd is invalid, or a valid file descriptor.
*/
int nativefs_close_file(int fd);

/*
  [main_thread]
  @function nativefs_cleanup

  @discussion This function is called by default if emscripten is set to exit the runtime, it is exposed
  in case the user would want to do it without exiting the runtime.

  @result 0 if it succeeded, -1 if there was an error.
*/
int nativefs_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif NATIVEFS_H
