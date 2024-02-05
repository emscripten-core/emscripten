/*
Copyright 2018 Embedded Microprocessor Benchmark Consortium (EEMBC)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Original Author: Shay Gal-on
*/

#include "coremark.h"
/*
Topic: Description
	Benchmark using a linked list.

	Linked list is a common data structure used in many applications.
	
	For our purposes, this will excercise the memory units of the processor.
	In particular, usage of the list pointers to find and alter data.
	
	We are not using Malloc since some platforms do not support this library.
	
	Instead, the memory block being passed in is used to create a list,
	and the benchmark takes care not to add more items then can be
	accomodated by the memory block. The porting layer will make sure
	that we have a valid memory block.
	
	All operations are done in place, without using any extra memory.
	
	The list itself contains list pointers and pointers to data items.
	Data items contain the following:
	
	idx - An index that captures the initial order of the list.
	data - Variable data initialized based on the input parameters. The 16b are divided as follows:
	o Upper 8b are backup of original data.
	o Bit 7 indicates if the lower 7 bits are to be used as is or calculated.
	o Bits 0-2 indicate type of operation to perform to get a 7b value.
	o Bits 3-6 provide input for the operation.
	
*/

/* local functions */

list_head *core_list_find(list_head *list,list_data *info);
list_head *core_list_reverse(list_head *list);
list_head *core_list_remove(list_head *item);
list_head *core_list_undo_remove(list_head *item_removed, list_head *item_modified);
list_head *core_list_insert_new(list_head *insert_point
	, list_data *info, list_head **memblock, list_data **datablock
	, list_head *memblock_end, list_data *datablock_end);
typedef ee_s32(*list_cmp)(list_data *a, list_data *b, core_results *res);
list_head *core_list_mergesort(list_head *list, list_cmp cmp, core_results *res);

ee_s16 calc_func(ee_s16 *pdata, core_results *res) {
	ee_s16 data=*pdata;
	ee_s16 retval;
	ee_u8 optype=(data>>7) & 1; /* bit 7 indicates if the function result has been cached */
	if (optype) /* if cached, use cache */
		return (data & 0x007f);
	else { /* otherwise calculate and cache the result */
		ee_s16 flag=data & 0x7; /* bits 0-2 is type of function to perform */
		ee_s16 dtype=((data>>3) & 0xf); /* bits 3-6 is specific data for the operation */
		dtype |= dtype << 4; /* replicate the lower 4 bits to get an 8b value */
		switch (flag) {
			case 0:
				if (dtype<0x22) /* set min period for bit corruption */
					dtype=0x22;
				retval=core_bench_state(res->size,res->memblock[3],res->seed1,res->seed2,dtype,res->crc);
				if (res->crcstate==0)
					res->crcstate=retval;
				break;
			case 1:
				retval=core_bench_matrix(&(res->mat),dtype,res->crc);
				if (res->crcmatrix==0)
					res->crcmatrix=retval;
				break;
			default:
				retval=data;
				break;
		}
		res->crc=crcu16(retval,res->crc);
		retval &= 0x007f; 
		*pdata = (data & 0xff00) | 0x0080 | retval; /* cache the result */
		return retval;
	}
}
/* Function: cmp_complex
	Compare the data item in a list cell.

	Can be used by mergesort.
*/
ee_s32 cmp_complex(list_data *a, list_data *b, core_results *res) {
	ee_s16 val1=calc_func(&(a->data16),res);
	ee_s16 val2=calc_func(&(b->data16),res);
	return val1 - val2;
}

/* Function: cmp_idx
	Compare the idx item in a list cell, and regen the data.

	Can be used by mergesort.
*/
ee_s32 cmp_idx(list_data *a, list_data *b, core_results *res) {
	if (res==NULL) {
		a->data16 = (a->data16 & 0xff00) | (0x00ff & (a->data16>>8));
		b->data16 = (b->data16 & 0xff00) | (0x00ff & (b->data16>>8));
	}
	return a->idx - b->idx;
}

void copy_info(list_data *to,list_data *from) {
	to->data16=from->data16;
	to->idx=from->idx;
}

/* Benchmark for linked list:
	- Try to find multiple data items.
	- List sort
	- Operate on data from list (crc)
	- Single remove/reinsert
	* At the end of this function, the list is back to original state
*/
ee_u16 core_bench_list(core_results *res, ee_s16 finder_idx) {
	ee_u16 retval=0;
	ee_u16 found=0,missed=0;
	list_head *list=res->list;
	ee_s16 find_num=res->seed3;
	list_head *this_find;
	list_head *finder, *remover;
	list_data info;
	ee_s16 i;

	info.idx=finder_idx;
	/* find <find_num> values in the list, and change the list each time (reverse and cache if value found) */
	for (i=0; i<find_num; i++) {
		info.data16= (i & 0xff) ;
		this_find=core_list_find(list,&info);
		list=core_list_reverse(list);
		if (this_find==NULL) {
			missed++;
			retval+=(list->next->info->data16 >> 8) & 1;
		}
		else {
			found++;
			if (this_find->info->data16 & 0x1) /* use found value */
				retval+=(this_find->info->data16 >> 9) & 1;
			/* and cache next item at the head of the list (if any) */
			if (this_find->next != NULL) {
				finder = this_find->next;
				this_find->next = finder->next;
				finder->next=list->next;
				list->next=finder;
			}
		}
		if (info.idx>=0)
			info.idx++;
#if CORE_DEBUG
	ee_printf("List find %d: [%d,%d,%d]\n",i,retval,missed,found);
#endif
	}
	retval+=found*4-missed;
	/* sort the list by data content and remove one item*/
	if (finder_idx>0)
		list=core_list_mergesort(list,cmp_complex,res);
	remover=core_list_remove(list->next);
	/* CRC data content of list from location of index N forward, and then undo remove */
	finder=core_list_find(list,&info);
	if (!finder)
		finder=list->next;
	while (finder) {
		retval=crc16(list->info->data16,retval);
		finder=finder->next;
	}
#if CORE_DEBUG
	ee_printf("List sort 1: %04x\n",retval);
#endif
	remover=core_list_undo_remove(remover,list->next);
	/* sort the list by index, in effect returning the list to original state */
	list=core_list_mergesort(list,cmp_idx,NULL);
	/* CRC data content of list */
	finder=list->next;
	while (finder) {
		retval=crc16(list->info->data16,retval);
		finder=finder->next;
	}
#if CORE_DEBUG
	ee_printf("List sort 2: %04x\n",retval);
#endif
	return retval;
}
/* Function: core_list_init
	Initialize list with data.

	Parameters:
	blksize - Size of memory to be initialized.
	memblock - Pointer to memory block.
	seed - 	Actual values chosen depend on the seed parameter.
		The seed parameter MUST be supplied from a source that cannot be determined at compile time

	Returns:
	Pointer to the head of the list.

*/
list_head *core_list_init(ee_u32 blksize, list_head *memblock, ee_s16 seed) {
	/* calculated pointers for the list */
	ee_u32 per_item=16+sizeof(struct list_data_s);
	ee_u32 size=(blksize/per_item)-2; /* to accomodate systems with 64b pointers, and make sure same code is executed, set max list elements */
	list_head *memblock_end=memblock+size;
	list_data *datablock=(list_data *)(memblock_end);
	list_data *datablock_end=datablock+size;
	/* some useful variables */
	ee_u32 i;
	list_head *finder,*list=memblock;
	list_data info;

	/* create a fake items for the list head and tail */
	list->next=NULL;
	list->info=datablock;
	list->info->idx=0x0000;
	list->info->data16=(ee_s16)0x8080;
	memblock++;
	datablock++;
	info.idx=0x7fff;
	info.data16=(ee_s16)0xffff;
	core_list_insert_new(list,&info,&memblock,&datablock,memblock_end,datablock_end);
	
	/* then insert size items */
	for (i=0; i<size; i++) {
		ee_u16 datpat=((ee_u16)(seed^i) & 0xf);
		ee_u16 dat=(datpat<<3) | (i&0x7); /* alternate between algorithms */
		info.data16=(dat<<8) | dat;		/* fill the data with actual data and upper bits with rebuild value */
		core_list_insert_new(list,&info,&memblock,&datablock,memblock_end,datablock_end);
	}
	/* and now index the list so we know initial seed order of the list */
	finder=list->next;
	i=1;
	while (finder->next!=NULL) {
		if (i<size/5) /* first 20% of the list in order */
			finder->info->idx=i++;
		else { 
			ee_u16 pat=(ee_u16)(i++ ^ seed); /* get a pseudo random number */
			finder->info->idx=0x3fff & (((i & 0x07) << 8) | pat); /* make sure the mixed items end up after the ones in sequence */
		}
		finder=finder->next;
	}
	list = core_list_mergesort(list,cmp_idx,NULL);
#if CORE_DEBUG
	ee_printf("Initialized list:\n");
	finder=list;
	while (finder) {
		ee_printf("[%04x,%04x]",finder->info->idx,(ee_u16)finder->info->data16);
		finder=finder->next;
	}
	ee_printf("\n");
#endif
	return list;
}

/* Function: core_list_insert
	Insert an item to the list

	Parameters:
	insert_point - where to insert the item.
	info - data for the cell.
	memblock - pointer for the list header
	datablock - pointer for the list data
	memblock_end - end of region for list headers
	datablock_end - end of region for list data

	Returns:
	Pointer to new item.
*/
list_head *core_list_insert_new(list_head *insert_point, list_data *info, list_head **memblock, list_data **datablock
	, list_head *memblock_end, list_data *datablock_end) {
	list_head *newitem;
	
	if ((*memblock+1) >= memblock_end)
		return NULL;
	if ((*datablock+1) >= datablock_end)
		return NULL;
		
	newitem=*memblock;
	(*memblock)++;
	newitem->next=insert_point->next;
	insert_point->next=newitem;
	
	newitem->info=*datablock;
	(*datablock)++;
	copy_info(newitem->info,info);
	
	return newitem;
}

/* Function: core_list_remove
	Remove an item from the list.

	Operation:
	For a singly linked list, remove by copying the data from the next item 
	over to the current cell, and unlinking the next item.

	Note: 
	since there is always a fake item at the end of the list, no need to check for NULL.

	Returns:
	Removed item.
*/
list_head *core_list_remove(list_head *item) {
	list_data *tmp;
	list_head *ret=item->next;
	/* swap data pointers */
	tmp=item->info;
	item->info=ret->info;
	ret->info=tmp;
	/* and eliminate item */
	item->next=item->next->next;
	ret->next=NULL;
	return ret;
}

/* Function: core_list_undo_remove
	Undo a remove operation.

	Operation:
	Since we want each iteration of the benchmark to be exactly the same,
	we need to be able to undo a remove. 
	Link the removed item back into the list, and switch the info items.

	Parameters:
	item_removed - Return value from the <core_list_remove>
	item_modified - List item that was modified during <core_list_remove>

	Returns:
	The item that was linked back to the list.
	
*/
list_head *core_list_undo_remove(list_head *item_removed, list_head *item_modified) {
	list_data *tmp;
	/* swap data pointers */
	tmp=item_removed->info;
	item_removed->info=item_modified->info;
	item_modified->info=tmp;
	/* and insert item */
	item_removed->next=item_modified->next;
	item_modified->next=item_removed;
	return item_removed;
}

/* Function: core_list_find
	Find an item in the list

	Operation:
	Find an item by idx (if not 0) or specific data value

	Parameters:
	list - list head
	info - idx or data to find

	Returns:
	Found item, or NULL if not found.
*/
list_head *core_list_find(list_head *list,list_data *info) {
	if (info->idx>=0) {
		while (list && (list->info->idx != info->idx))
			list=list->next;
		return list;
	} else {
		while (list && ((list->info->data16 & 0xff) != info->data16))
			list=list->next;
		return list;
	}
}
/* Function: core_list_reverse
	Reverse a list

	Operation:
	Rearrange the pointers so the list is reversed.

	Parameters:
	list - list head
	info - idx or data to find

	Returns:
	Found item, or NULL if not found.
*/

list_head *core_list_reverse(list_head *list) {
	list_head *next=NULL, *tmp;
	while (list) {
		tmp=list->next;
		list->next=next;
		next=list;
		list=tmp;
	}
	return next;
}
/* Function: core_list_mergesort
	Sort the list in place without recursion.

	Description:
	Use mergesort, as for linked list this is a realistic solution. 
	Also, since this is aimed at embedded, care was taken to use iterative rather then recursive algorithm.
	The sort can either return the list to original order (by idx) ,
	or use the data item to invoke other other algorithms and change the order of the list.

	Parameters:
	list - list to be sorted.
	cmp - cmp function to use

	Returns:
	New head of the list.

	Note: 
	We have a special header for the list that will always be first,
	but the algorithm could theoretically modify where the list starts.

 */
list_head *core_list_mergesort(list_head *list, list_cmp cmp, core_results *res) {
    list_head *p, *q, *e, *tail;
    ee_s32 insize, nmerges, psize, qsize, i;

    insize = 1;

    while (1) {
        p = list;
        list = NULL;
        tail = NULL;

        nmerges = 0;  /* count number of merges we do in this pass */

        while (p) {
            nmerges++;  /* there exists a merge to be done */
            /* step `insize' places along from p */
            q = p;
            psize = 0;
            for (i = 0; i < insize; i++) {
                psize++;
			    q = q->next;
                if (!q) break;
            }

            /* if q hasn't fallen off end, we have two lists to merge */
            qsize = insize;

            /* now we have two lists; merge them */
            while (psize > 0 || (qsize > 0 && q)) {

				/* decide whether next element of merge comes from p or q */
				if (psize == 0) {
				    /* p is empty; e must come from q. */
				    e = q; q = q->next; qsize--;
				} else if (qsize == 0 || !q) {
				    /* q is empty; e must come from p. */
				    e = p; p = p->next; psize--;
				} else if (cmp(p->info,q->info,res) <= 0) {
				    /* First element of p is lower (or same); e must come from p. */
				    e = p; p = p->next; psize--;
				} else {
				    /* First element of q is lower; e must come from q. */
				    e = q; q = q->next; qsize--;
				}

		        /* add the next element to the merged list */
				if (tail) {
				    tail->next = e;
				} else {
				    list = e;
				}
				tail = e;
	        }

			/* now p has stepped `insize' places along, and q has too */
			p = q;
        }
		
	    tail->next = NULL;

        /* If we have done only one merge, we're finished. */
        if (nmerges <= 1)   /* allow for nmerges==0, the empty list case */
            return list;

        /* Otherwise repeat, merging lists twice the size */
        insize *= 2;
    }
#if COMPILER_REQUIRES_SORT_RETURN
	return list;
#endif
}
