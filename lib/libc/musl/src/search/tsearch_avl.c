#include <stdlib.h>
#include <search.h>

/*
avl tree implementation using recursive functions
the height of an n node tree is less than 1.44*log2(n+2)-1
(so the max recursion depth in case of a tree with 2^32 nodes is 45)
*/

struct node {
	const void *key;
	struct node *left;
	struct node *right;
	int height;
};

static int delta(struct node *n) {
	return (n->left ? n->left->height:0) - (n->right ? n->right->height:0);
}

static void updateheight(struct node *n) {
	n->height = 0;
	if (n->left && n->left->height > n->height)
		n->height = n->left->height;
	if (n->right && n->right->height > n->height)
		n->height = n->right->height;
	n->height++;
}

static struct node *rotl(struct node *n) {
	struct node *r = n->right;
	n->right = r->left;
	r->left = n;
	updateheight(n);
	updateheight(r);
	return r;
}

static struct node *rotr(struct node *n) {
	struct node *l = n->left;
	n->left = l->right;
	l->right = n;
	updateheight(n);
	updateheight(l);
	return l;
}

static struct node *balance(struct node *n) {
	int d = delta(n);

	if (d < -1) {
		if (delta(n->right) > 0)
			n->right = rotr(n->right);
		return rotl(n);
	} else if (d > 1) {
		if (delta(n->left) < 0)
			n->left = rotl(n->left);
		return rotr(n);
	}
	updateheight(n);
	return n;
}

static struct node *find(struct node *n, const void *k,
	int (*cmp)(const void *, const void *))
{
	int c;

	if (!n)
		return 0;
	c = cmp(k, n->key);
	if (c == 0)
		return n;
	if (c < 0)
		return find(n->left, k, cmp);
	else
		return find(n->right, k, cmp);
}

static struct node *insert(struct node *n, const void *k,
	int (*cmp)(const void *, const void *), struct node **found)
{
	struct node *r;
	int c;

	if (!n) {
		n = malloc(sizeof *n);
		if (n) {
			n->key = k;
			n->left = n->right = 0;
			n->height = 1;
		}
		*found = n;
		return n;
	}
	c = cmp(k, n->key);
	if (c == 0) {
		*found = n;
		return 0;
	}
	r = insert(c < 0 ? n->left : n->right, k, cmp, found);
	if (r) {
		if (c < 0)
			n->left = r;
		else
			n->right = r;
		r = balance(n);
	}
	return r;
}

static struct node *remove_rightmost(struct node *n, struct node **rightmost)
{
	if (!n->right) {
		*rightmost = n;
		return n->left;
	}
	n->right = remove_rightmost(n->right, rightmost);
	return balance(n);
}

static struct node *remove(struct node **n, const void *k,
	int (*cmp)(const void *, const void *), struct node *parent)
{
	int c;

	if (!*n)
		return 0;
	c = cmp(k, (*n)->key);
	if (c == 0) {
		struct node *r = *n;
		if (r->left) {
			r->left = remove_rightmost(r->left, n);
			(*n)->left = r->left;
			(*n)->right = r->right;
			*n = balance(*n);
		} else
			*n = r->right;
		free(r);
		return parent;
	}
	if (c < 0)
		parent = remove(&(*n)->left, k, cmp, *n);
	else
		parent = remove(&(*n)->right, k, cmp, *n);
	if (parent)
		*n = balance(*n);
	return parent;
}

void *tdelete(const void *restrict key, void **restrict rootp,
	int(*compar)(const void *, const void *))
{
	if (!rootp)
		return 0;
	struct node *n = *rootp;
	struct node *ret;
	/* last argument is arbitrary non-null pointer
	   which is returned when the root node is deleted */
	ret = remove(&n, key, compar, n);
	*rootp = n;
	return ret;
}

void *tfind(const void *key, void *const *rootp,
	int(*compar)(const void *, const void *))
{
	if (!rootp)
		return 0;
	return find(*rootp, key, compar);
}

void *tsearch(const void *key, void **rootp,
	int (*compar)(const void *, const void *))
{
	struct node *update;
	struct node *ret;
	if (!rootp)
		return 0;
	update = insert(*rootp, key, compar, &ret);
	if (update)
		*rootp = update;
	return ret;
}

static void walk(const struct node *r, void (*action)(const void *, VISIT, int), int d)
{
	if (r == 0)
		return;
	if (r->left == 0 && r->right == 0)
		action(r, leaf, d);
	else {
		action(r, preorder, d);
		walk(r->left, action, d+1);
		action(r, postorder, d);
		walk(r->right, action, d+1);
		action(r, endorder, d);
	}
}

void twalk(const void *root, void (*action)(const void *, VISIT, int))
{
	walk(root, action, 0);
}
