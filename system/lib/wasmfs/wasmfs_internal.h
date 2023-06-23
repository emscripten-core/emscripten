// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#ifdef __cplusplus
extern "C" {
#endif

// These helper functions are defined in library_wasmfs.js.

int _wasmfs_get_num_preloaded_files();
int _wasmfs_get_num_preloaded_dirs();
int _wasmfs_get_preloaded_file_mode(int index);
void _wasmfs_get_preloaded_parent_path(int index, char* parentPath);
void _wasmfs_get_preloaded_path_name(int index, char* fileName);
void _wasmfs_get_preloaded_child_path(int index, char* childName);
size_t _wasmfs_get_preloaded_file_size(uint32_t index);
void _wasmfs_copy_preloaded_file_data(uint32_t index, uint8_t* data);

// Returns the next character from stdin, or -1 on EOF.
int _wasmfs_stdin_get_char(void);

#ifdef __cplusplus
}
#endif
