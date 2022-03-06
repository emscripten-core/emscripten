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
            var blob = new File(["The quick brown fox jumps over the lazy dog"], "test.txt", {
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
    MAIN_THREAD_EM_ASM({
            var blob = new File(["The quick brown fox jumps over the lazy dog"], "test2.txt", {
                type: "text/plain",
                });
            NATIVEFS.worker.postMessage( {
                ops: "push_file",
                file: blob
                });
            // keep the record in the pthread
            NATIVEFS.files.push({
                fd: 4, // 0-1-2 are reserved
                fileName: blob.name,
                fileSize: blob.size
                // file: blob (for debug?)
                });            
        });
    MAIN_THREAD_EM_ASM({
            var blob = new File(["The quick brown fox jumps over the lazy dog"], "test3.txt", {
                    type: "text/plain",
                });
            NATIVEFS.worker.postMessage( {
                ops: "push_file",
                file: blob
                });
            // keep the record in the pthread
            NATIVEFS.files.push({
                fd: 5, // 0-1-2 are reserved
                fileName: blob.name,
                fileSize: blob.size
                // file: blob (for debug?)
                });            
        });
    MAIN_THREAD_EM_ASM({
            var blob = new File(["The quick brown fox jumps over the lazy dog"], "test4.txt", {
                    type: "text/plain",
                });
            NATIVEFS.worker.postMessage( {
                ops: "push_file",
                file: blob
                });
            // keep the record in the pthread
            NATIVEFS.files.push({
                fd: 6, // 0-1-2 are reserved
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
    printf("Starting nativefs_read tests...\n");

    ret = nativefs_read(3, NULL, 44);
    assert(ret == -1);
    printf("test_null_buffer: read returned:%zd bytes and read:%s\n", ret, buffer);

    ret = nativefs_read(-1, buffer, 44);
    assert(ret == -1);
    printf("test_bad_fd: read returned:%zd bytes and read:%s\n", ret, buffer);

    ret = nativefs_read(4242, buffer, 44);
    assert(ret == -1);
    printf("test_invalid_fd: read returned:%zd bytes and read:%s\n", ret, buffer);

    ret = nativefs_read(3, buffer, 0);
    assert(ret == 0);
    printf("test_zero_size: read returned:%zd bytes and read:%s\n", ret, buffer);
    memset(buffer, 0, 44);

    ret = nativefs_read(3, buffer, 99);
    assert(ret == 43);
    assert(strcmp(buffer, "The quick brown fox jumps over the lazy dog") == 0);
    printf("test_bigger_size: read returned:%zd bytes and read:%s\n", ret, buffer);
    memset(buffer, 0, 44);

    /*
      not sure what this test should do?
      char another_buffer[20];
      memset(another_buffer, 0, 20);
      ret = nativefs_read(4, another_buffer, 44);
      printf("test_smaller_buffer_size: read %zd bytes: %s\n", ret, another_buffer);
    */
    int i = 0;
    do {
        ret = nativefs_read(5, buffer + i, 1);
        i++;
    } while (ret);
    assert(strcmp(buffer, "The quick brown fox jumps over the lazy dog") == 0);
    printf("test_multiple_reads: buffer is :%s\n", buffer);
    memset(buffer, 0, 44);

    ret = nativefs_read(6, buffer, 44);
    assert(ret == 43);
    assert(strcmp(buffer, "The quick brown fox jumps over the lazy dog") == 0);
    printf("test_valid_read: read returned:%zd bytes and read:%s\n", ret, buffer);
    memset(buffer, 0, 44);

    ret = nativefs_read(6, buffer, 44);
    assert(ret == 0);
    printf("test_read_after_finish: read returned:%zd bytes and read:%s\n", ret, buffer);

    ret = nativefs_read(6, buffer, 44);
    assert(ret == 0);
    printf("test_read_another_after_finish: read returned:%zd bytes and read:%s\n", ret, buffer);

    flag = 0;
    printf("nativefs_read tests succeeded!\n");
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
