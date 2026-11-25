# Memory Arenas and Concurrent Bitmaps

- [`arena.c`](arena.c): Arena creation, reservation, traversal, and destruction.
- [`arena-alloc.c`](arena-alloc.c): Allocation, freeing, commitment, and purging of arena slices.
- [`arena-page.c`](arena-page.c): Per-heap arena page bitmaps, and finding, abandoning, and reclaiming abandoned pages.
- [`arena-page-alloc.c`](arena-page-alloc.c): Page allocation (`_mi_arenas_page_alloc`): reuse an abandoned page or allocate and initialize a fresh one; and freeing pages.
- [`arena-heap.c`](arena-heap.c): Visiting, collecting, and moving or destroying all pages of a heap.
- [`arena.h`](arena.h): Private arena declarations, traversal callbacks, and layout helpers.
- [`arena-page.h`](arena-page.h): Private declarations and helpers shared by the arena page modules (guard pages, arena page bitmaps).
- [`bitmap.c`](bitmap.c): Concurrent bitmap searches and range operations; these bitmaps are used to manage the slices in an arena.
- [`bitmap-chunk.c`](bitmap-chunk.c): Bitmaps consist of 512-bit chunks; this provides the low-level atomic bit-field and bitmap-chunk operations.
- [`bitmap.h`](bitmap.h): Bitmap types, constants, and operation declarations.
- [`bitmap-chunk.h`](bitmap-chunk.h): Private declarations and iteration helpers for bitmap fields and chunks.
