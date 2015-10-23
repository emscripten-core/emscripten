#define _GNU_SOURCE
#include <stdlib.h>
#include <search.h>

struct node {
	void *key;
	struct node *left;
	struct node *right;
};

void tdestroy(void *root, void (*freekey)(void *))
{
	struct node *r = root;

	if (r == 0)
		return;
	tdestroy(r->left, freekey);
	tdestroy(r->right, freekey);
	if (freekey) freekey(r->key);
	free(r);
}
