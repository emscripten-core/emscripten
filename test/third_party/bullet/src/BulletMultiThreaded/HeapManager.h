/*
   Copyright (C) 2009 Sony Computer Entertainment Inc.
   All rights reserved.

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

*/

#ifndef BT_HEAP_MANAGER_H__
#define BT_HEAP_MANAGER_H__

#ifdef __SPU__
	#define HEAP_STACK_SIZE 32
#else
	#define HEAP_STACK_SIZE 64
#endif

#define MIN_ALLOC_SIZE 16


class HeapManager
{
private:
	ATTRIBUTE_ALIGNED16(unsigned char *mHeap);
	ATTRIBUTE_ALIGNED16(unsigned int mHeapBytes);
	ATTRIBUTE_ALIGNED16(unsigned char *mPoolStack[HEAP_STACK_SIZE]);
	ATTRIBUTE_ALIGNED16(unsigned int mCurStack);
	
public:
	enum {ALIGN16,ALIGN128};

	HeapManager(unsigned char *buf,int bytes)
	{
		mHeap = buf;
		mHeapBytes = bytes;
		clear();
	}
	
	~HeapManager()
	{
	}
	
	int getAllocated()
	{
		return (int)(mPoolStack[mCurStack]-mHeap);
	}
	
	int getRest()
	{
		return mHeapBytes-getAllocated();
	}

	void *allocate(size_t bytes,int alignment = ALIGN16)
	{
		if(bytes <= 0) bytes = MIN_ALLOC_SIZE;
		btAssert(mCurStack < (HEAP_STACK_SIZE-1));

		
#if defined(_WIN64) || defined(__LP64__) || defined(__x86_64__)
		unsigned long long p = (unsigned long long )mPoolStack[mCurStack];
		if(alignment == ALIGN128) {
			p = ((p+127) & 0xffffffffffffff80);
			bytes = (bytes+127) & 0xffffffffffffff80;
		}
		else {
			bytes = (bytes+15) & 0xfffffffffffffff0;
		}

		btAssert(bytes <=(mHeapBytes-(p-(unsigned long long )mHeap)) );
		
#else
		unsigned long p = (unsigned long )mPoolStack[mCurStack];
		if(alignment == ALIGN128) {
			p = ((p+127) & 0xffffff80);
			bytes = (bytes+127) & 0xffffff80;
		}
		else {
			bytes = (bytes+15) & 0xfffffff0;
		}
		btAssert(bytes <=(mHeapBytes-(p-(unsigned long)mHeap)) );
#endif
		unsigned char * bla = (unsigned char *)(p + bytes);
		mPoolStack[++mCurStack] = bla;
		return (void*)p;
	}

	void deallocate(void *p)
	{
		(void) p;
		mCurStack--;
	}
	
	void clear()
	{
		mPoolStack[0] = mHeap;
		mCurStack = 0;
	}

//	void printStack()
//	{
//		for(unsigned int i=0;i<=mCurStack;i++) {
//			PRINTF("memStack %2d 0x%x\n",i,(uint32_t)mPoolStack[i]);
//		}
//	}

};

#endif //BT_HEAP_MANAGER_H__

