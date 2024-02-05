// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>
#include <memory.h>

// A custom tiny malloc & free impl
uintptr_t arena_end = 0;

struct mem_node
{
	uint32_t size;
	mem_node *next;
};

// List of reclaimed free memory nodes
mem_node free_memory_root = {};

void *mymalloc(size_t size)
{
	// Static init at program startup:
	if (!arena_end)
	{
		arena_end = (uintptr_t)sbrk(0);
		assert(arena_end != (uintptr_t)-1);
	}

	// Find if there is an existing node we can reuse.
	mem_node *prev = &free_memory_root;
	mem_node *n = prev->next;
	while(n)
	{
		if (n->size >= size)
		{
			prev->next = n->next; // Splice this node off from the free list.
			return (void*)((uintptr_t)n + sizeof(mem_node));
		}
		prev = n;
		n = n->next;
	}

	// If not, allocate new node from empty area
	size_t allocated_size = sizeof(mem_node) + size;

#if TEST_BRK // test brk()
	uintptr_t new_brk = arena_end + allocated_size;
	int failed = brk((void*)new_brk);
	if (failed) return 0;

	mem_node *node = (mem_node*)arena_end;
	arena_end = (uintptr_t)sbrk(0);
	assert(arena_end == new_brk);
#else // test sbrk()
	mem_node *node = (mem_node*)sbrk(allocated_size);
	if ((uintptr_t)node == (uintptr_t)-1)
		return 0;
#endif
	node->size = size;
	return (void*)((uintptr_t)node + sizeof(mem_node));
}

void myfree(void *ptr)
{
	mem_node *freed_node = (mem_node*)((uintptr_t)ptr - sizeof(mem_node));
	freed_node->next = free_memory_root.next;
	free_memory_root.next = freed_node;
}

int main()
{
#define N 3000 // Arbitrary amount that fits within the default 16MB heap

	uint8_t *data[N];
	for(int i = 0; i < N; ++i)
	{
		int count = i&~15U;
		uint8_t *memory = (uint8_t*)mymalloc(count);
		assert(memory);
		uint8_t *memory2 = (uint8_t*)mymalloc(count);
		assert(memory2);

		myfree(memory);
		data[i] = memory2;
		memset(memory2, (uint8_t)i, count);
	}
	uintptr_t dynamicTop = (uintptr_t)sbrk(0);
	for(int i = 0; i < N; ++i)
	{
		int count = i&~15U;
		assert((uintptr_t)data[i] + count <= dynamicTop);
		for(int j = 0; j < count; ++j)
			assert(data[i][j] == (uint8_t)i);
	}
	printf("OK. brk at end: %p. \n", sbrk(0));
}
