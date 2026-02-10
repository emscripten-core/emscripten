#include <vector>

#include <emscripten/proxying.h>

#include "backend.h"

using namespace wasmfs;

extern "C" {

// Ensure that the root OPFS directory is initialized with ID 0.
void _wasmfs_opfs_init_root_directory(em_proxying_ctx* ctx);

// Look up the child under `parent` with `name`. Write 1 to `child_type` if it's
// a regular file or 2 if it's a directory. Write the child's file or directory
// ID to `child_id`, or -1 if the child does not exist, or -2 if the child
// exists but cannot be opened.
void _wasmfs_opfs_get_child(em_proxying_ctx* ctx,
                            int parent,
                            const char* name,
                            int* child_type,
                            int* child_id);

// Create a file under `parent` with `name` and store its ID in `child_id`.
void _wasmfs_opfs_insert_file(em_proxying_ctx* ctx,
                              int parent,
                              const char* name,
                              int* child_id);

// Create a directory under `parent` with `name` and store its ID in `child_id`.
void _wasmfs_opfs_insert_directory(em_proxying_ctx* ctx,
                                   int parent,
                                   const char* name,
                                   int* child_id);

void _wasmfs_opfs_move_file(em_proxying_ctx* ctx,
                            int file_id,
                            int new_parent_id,
                            const char* name,
                            int* err);

void _wasmfs_opfs_remove_child(em_proxying_ctx* ctx,
                               int dir_id,
                               const char* name,
                               int* err);

void _wasmfs_opfs_get_entries(em_proxying_ctx* ctx,
                              int dirID,
                              std::vector<Directory::Entry>* entries,
                              int* err);

void _wasmfs_opfs_open_access(em_proxying_ctx* ctx,
                              int file_id,
                              int* access_id);

void _wasmfs_opfs_open_blob(em_proxying_ctx* ctx, int file_id, int* blob_id);

void _wasmfs_opfs_close_access(em_proxying_ctx* ctx, int access_id, int* err);

void _wasmfs_opfs_close_blob(int blob_id);

void _wasmfs_opfs_free_file(int file_id);

void _wasmfs_opfs_free_directory(int dir_id);

// Synchronous read. Return the number of bytes read.
int _wasmfs_opfs_read_access(int access_id,
                             uint8_t* buf,
                             uint32_t len,
                             off_t pos);

int _wasmfs_opfs_read_blob(em_proxying_ctx* ctx,
                           int blob_id,
                           uint8_t* buf,
                           uint32_t len,
                           off_t pos,
                           int32_t* nread);

// Synchronous write. Return the number of bytes written.
int _wasmfs_opfs_write_access(int access_id,
                              const uint8_t* buf,
                              uint32_t len,
                              off_t pos);

// Get the size via an AccessHandle.
void _wasmfs_opfs_get_size_access(em_proxying_ctx* ctx,
                                  int access_id,
                                  off_t* size);

off_t _wasmfs_opfs_get_size_blob(int blob_id);

// Get the size of a file handle via a File Blob.
void _wasmfs_opfs_get_size_file(em_proxying_ctx* ctx, int file_id, off_t* size);

void _wasmfs_opfs_set_size_access(em_proxying_ctx* ctx,
                                  int access_id,
                                  off_t size,
                                  int* err);

void _wasmfs_opfs_set_size_file(em_proxying_ctx* ctx,
                                int file_id,
                                off_t size,
                                int* err);

void _wasmfs_opfs_flush_access(em_proxying_ctx* ctx, int access_id, int* err);

} // extern "C"
