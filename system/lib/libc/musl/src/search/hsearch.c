#include <stdlib.h>
#include <string.h>
#include <search.h>

/*
open addressing hash table with 2^n table size
quadratic probing is used in case of hash collision
tab indices and hash are size_t
after resize fails with ENOMEM the state of tab is still usable

with the posix api items cannot be iterated and length cannot be queried
*/

#define MINSIZE 8
#define MAXSIZE ((size_t)-1/2 + 1)

struct elem {
	ENTRY item;
	size_t hash;
};

static size_t mask;
static size_t used;
static struct elem *tab;

static size_t keyhash(char *k)
{
	unsigned char *p = (void *)k;
	size_t h = 0;

	while (*p)
		h = 31*h + *p++;
	return h;
}

static int resize(size_t nel)
{
	size_t newsize;
	size_t i, j;
	struct elem *e, *newe;
	struct elem *oldtab = tab;
	struct elem *oldend = tab + mask + 1;

	if (nel > MAXSIZE)
		nel = MAXSIZE;
	for (newsize = MINSIZE; newsize < nel; newsize *= 2);
	tab = calloc(newsize, sizeof *tab);
	if (!tab) {
		tab = oldtab;
		return 0;
	}
	mask = newsize - 1;
	if (!oldtab)
		return 1;
	for (e = oldtab; e < oldend; e++)
		if (e->item.key) {
			for (i=e->hash,j=1; ; i+=j++) {
				newe = tab + (i & mask);
				if (!newe->item.key)
					break;
			}
			*newe = *e;
		}
	free(oldtab);
	return 1;
}

int hcreate(size_t nel)
{
	mask = 0;
	used = 0;
	tab = 0;
	return resize(nel);
}

void hdestroy(void)
{
	free(tab);
	tab = 0;
	mask = 0;
	used = 0;
}

static struct elem *lookup(char *key, size_t hash)
{
	size_t i, j;
	struct elem *e;

	for (i=hash,j=1; ; i+=j++) {
		e = tab + (i & mask);
		if (!e->item.key ||
		    (e->hash==hash && strcmp(e->item.key, key)==0))
			break;
	}
	return e;
}

ENTRY *hsearch(ENTRY item, ACTION action)
{
	size_t hash = keyhash(item.key);
	struct elem *e = lookup(item.key, hash);

	if (e->item.key)
		return &e->item;
	if (action == FIND)
		return 0;
	e->item = item;
	e->hash = hash;
	if (++used > mask - mask/4) {
		if (!resize(2*used)) {
			used--;
			e->item.key = 0;
			return 0;
		}
		e = lookup(item.key, hash);
	}
	return &e->item;
}
