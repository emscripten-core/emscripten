#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <emscripten.h>
#include <emscripten/nativefs.h>
#include <assert.h> 

int flag = 1;

void generate_test_files() {
    /*
      Test routine to allow testing files without ui
     */
    MAIN_THREAD_EM_ASM({
            var blob = new File(["The lazy brown dog jumps over the quick fox"], "test.txt", {
                    type: "text/plain",
                });
            NATIVEFS.worker.postMessage( {
                ops: "push_file",
                file: blob
                });
            // keep the record in the pthread
            NATIVEFS.files.push({
                fd: 3, // 0-1-2 are reserved
                fileName: blob.name,
                fileSize: blob.size
                // file: blob (for debug?)
                });            
        });
}

void *run(void *args) {
    /* 
       doing this here, because before pthread is initialized, nativefs
       may not be ready
    */
    generate_test_files();
    char buffer[44];
    ssize_t ret = 0;
    memset(buffer, 0, 44);
    printf("Starting nativefs_seek tests...\n");

    ret = nativefs_seek(-1, 42, NATIVEFS_SEEK_SET);
    assert(ret == -1);
    printf("test_bad_fd: seek returned %zd\n", ret);

    ret = nativefs_seek(4242, 42, NATIVEFS_SEEK_SET);
    assert(ret == -1);
    printf("test_invalid_fd: seek returned %zd\n", ret);

    ret = nativefs_seek(3, 44, NATIVEFS_SEEK_SET);
    assert(ret == -1);
    printf("test_invalid_offset_cur_high: seek returned %zd\n", ret);

    ret = nativefs_seek(3, -1, NATIVEFS_SEEK_SET);
    assert(ret == -1);
    printf("test_invalid_offset_cur_low: seek returned %zd\n", ret);

    ret = nativefs_seek(3, 44, NATIVEFS_SEEK_CUR);
    assert(ret == -1);
    printf("test_invalid_offset_begin_high: seek returned %zd\n", ret);
    ret = nativefs_seek(3, -44, NATIVEFS_SEEK_CUR);
    assert(ret == -1);
    printf("test_invalid_offset_begin_low: seek returned %zd\n", ret);

    ret = nativefs_seek(3, -44, NATIVEFS_SEEK_END);
    assert(ret == -1);
    printf("test_invalid_offset_end_high: seek returned %zd\n", ret);

    ret = nativefs_seek(3, 1, NATIVEFS_SEEK_END);
    assert(ret == -1);
    printf("test_invalid_offset_end_low: seek returned %zd\n", ret);
    
    ret = nativefs_seek(3, 42, 42);
    assert(ret == -1);
    printf("test_invalid_whence: seek returned %zd\n", ret);
    
    ssize_t read_ret = 0;
    read_ret = nativefs_read(3, buffer, 4);
    ret = nativefs_seek(3, -9, NATIVEFS_SEEK_END);
    assert(ret == 34);
    read_ret += nativefs_read(3, buffer + read_ret, 5);
    ret = nativefs_seek(3, 8, NATIVEFS_SEEK_SET);
    assert(ret == 8);
    read_ret += nativefs_read(3, buffer + read_ret, 7);
    ret = nativefs_seek(3, -3, NATIVEFS_SEEK_END);
    assert(ret == 40);
    read_ret += nativefs_read(3, buffer + read_ret, 6);
    ret = nativefs_seek(3, 0, NATIVEFS_SEEK_SET);
    assert(ret == 0);
    ret = nativefs_seek(3, 18, NATIVEFS_SEEK_CUR);
    assert(ret == 18);
    read_ret += nativefs_read(3, buffer + read_ret, 15);
    ret = nativefs_seek(3, -30, NATIVEFS_SEEK_CUR);
    assert(ret == 3);
    read_ret += nativefs_read(3, buffer + read_ret, 5);
    ret = nativefs_seek(3, 14, NATIVEFS_SEEK_SET);
    assert(ret == 14);
    read_ret += nativefs_read(3, buffer + read_ret, 4);
    printf("test_valid_multiple_seek: %s\n", buffer);
    assert (strcmp(buffer, "The quick brown fox jumps over the lazy dog") == 0);

    char another_buffer[5];
    memset(another_buffer, 0, 5);
    ret = nativefs_seek(3, 43, NATIVEFS_SEEK_SET);
    assert(ret == 43);
    ret = nativefs_read(3, another_buffer, 4);
    assert(ret == 0);
    ret = nativefs_read(3, another_buffer, 4);
    assert(ret == 0);
    printf("test_read_after_seek_to_end: second read returned %zd\n", ret);
    
    flag = 0;

    printf("nativefs_read seek succeeded!\n");
    return NULL;
}

void iter(void) {
    if (flag == 0) {
        flag = 1;
        emscripten_cancel_main_loop();
        emscripten_force_exit(0);
    }
}

int main() {
    pthread_t tid;
    nativefs_init(NULL);
    pthread_create(&tid, NULL, run, NULL);
    // avoid early return (that will beat the purpose of the test)
    emscripten_set_main_loop(iter, 0, 1);
    return 0;
}
