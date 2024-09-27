#include <assert.h>
#include <stdio.h>
#include <sys/statvfs.h>
#include <emscripten.h>

void test_statfs(const char *path) {
    printf("Testing statfs for path: %s\n", path);
    struct statvfs st;
    int result = statvfs(path, &st);

    assert(result == 0 && "statvfs should succeed");

    // Basic sanity checks
    assert(st.f_bsize > 0 && "Block size should be positive");
    assert(st.f_blocks > 0 && "Total blocks should be positive");
    assert(st.f_bfree <= st.f_blocks && "Free blocks should not exceed total blocks");
    assert(st.f_bavail <= st.f_bfree && "Available blocks should not exceed free blocks");
    assert(st.f_files > 0 && "Total inodes should be positive");
    assert(st.f_ffree <= st.f_files && "Free inodes should not exceed total inodes");
}

void setup() {
	EM_ASM(
		FS.mkdir('/working');
		FS.mount(NODEFS, { root: '.' }, '/working');
	);
}

int main() {
    // Test the root filesystem (which should be MEMFS by default)

    test_statfs("/");

	setup();
    test_statfs("/working");

    puts("success");
}
