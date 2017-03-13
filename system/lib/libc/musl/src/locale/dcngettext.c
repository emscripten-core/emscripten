#include <libintl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <ctype.h>
#include "locale_impl.h"
#include "libc.h"
#include "atomic.h"

struct binding {
	struct binding *next;
	int dirlen;
	volatile int active;
	char *domainname;
	char *dirname;
	char buf[];
};

static void *volatile bindings;

static char *gettextdir(const char *domainname, size_t *dirlen)
{
	struct binding *p;
	for (p=bindings; p; p=p->next) {
		if (!strcmp(p->domainname, domainname) && p->active) {
			*dirlen = p->dirlen;
			return (char *)p->dirname;
		}
	}
	return 0;
}

char *bindtextdomain(const char *domainname, const char *dirname)
{
	static volatile int lock[2];
	struct binding *p, *q;

	if (!domainname) return 0;
	if (!dirname) return gettextdir(domainname, &(size_t){0});

	size_t domlen = strlen(domainname);
	size_t dirlen = strlen(dirname);
	if (domlen > NAME_MAX || dirlen >= PATH_MAX) {
		errno = EINVAL;
		return 0;
	}

	LOCK(lock);

	for (p=bindings; p; p=p->next) {
		if (!strcmp(p->domainname, domainname) &&
		    !strcmp(p->dirname, dirname)) {
			break;
		}
	}

	if (!p) {
		p = malloc(sizeof *p + domlen + dirlen + 2);
		if (!p) {
			UNLOCK(lock);
			return 0;
		}
		p->next = bindings;
		p->dirlen = dirlen;
		p->domainname = p->buf;
		p->dirname = p->buf + domlen + 1;
		memcpy(p->domainname, domainname, domlen+1);
		memcpy(p->dirname, dirname, dirlen+1);
		a_cas_p(&bindings, bindings, p);
	}

	a_store(&p->active, 1);

	for (q=bindings; q; q=q->next) {
		if (!strcmp(p->domainname, domainname) && q != p)
			a_store(&q->active, 0);
	}

	UNLOCK(lock);
	
	return (char *)p->dirname;
}

static const char catnames[][12] = {
	"LC_CTYPE",
	"LC_NUMERIC",
	"LC_TIME",
	"LC_COLLATE",
	"LC_MONETARY",
	"LC_MESSAGES",
};

static const char catlens[] = { 8, 10, 7, 10, 11, 11 };

struct msgcat {
	struct msgcat *next;
	const void *map;
	size_t map_size;
	void *volatile plural_rule;
	volatile int nplurals;
	char name[];
};

static char *dummy_gettextdomain()
{
	return "messages";
}

weak_alias(dummy_gettextdomain, __gettextdomain);

const unsigned char *__map_file(const char *, size_t *);
int __munmap(void *, size_t);
unsigned long __pleval(const char *, unsigned long);

char *dcngettext(const char *domainname, const char *msgid1, const char *msgid2, unsigned long int n, int category)
{
	static struct msgcat *volatile cats;
	struct msgcat *p;
	struct __locale_struct *loc = CURRENT_LOCALE;
	const struct __locale_map *lm;
	const char *dirname, *locname, *catname;
	size_t dirlen, loclen, catlen, domlen;

	if ((unsigned)category >= LC_ALL) goto notrans;

	if (!domainname) domainname = __gettextdomain();

	domlen = strlen(domainname);
	if (domlen > NAME_MAX) goto notrans;

	dirname = gettextdir(domainname, &dirlen);
	if (!dirname) goto notrans;

	lm = loc->cat[category];
	if (!lm) {
notrans:
		return (char *) ((n == 1) ? msgid1 : msgid2);
	}
	locname = lm->name;

	catname = catnames[category];
	catlen = catlens[category];
	loclen = strlen(locname);

	size_t namelen = dirlen+1 + loclen+1 + catlen+1 + domlen+3;
	char name[namelen+1], *s = name;

	memcpy(s, dirname, dirlen);
	s[dirlen] = '/';
	s += dirlen + 1;
	memcpy(s, locname, loclen);
	s[loclen] = '/';
	s += loclen + 1;
	memcpy(s, catname, catlen);
	s[catlen] = '/';
	s += catlen + 1;
	memcpy(s, domainname, domlen);
	s[domlen] = '.';
	s[domlen+1] = 'm';
	s[domlen+2] = 'o';
	s[domlen+3] = 0;

	for (p=cats; p; p=p->next)
		if (!strcmp(p->name, name))
			break;

	if (!p) {
		void *old_cats;
		size_t map_size;
		const void *map = __map_file(name, &map_size);
		if (!map) goto notrans;
		p = malloc(sizeof *p + namelen + 1);
		if (!p) {
			__munmap((void *)map, map_size);
			goto notrans;
		}
		p->map = map;
		p->map_size = map_size;
		memcpy(p->name, name, namelen+1);
		do {
			old_cats = cats;
			p->next = old_cats;
		} while (a_cas_p(&cats, old_cats, p) != old_cats);
	}

	const char *trans = __mo_lookup(p->map, p->map_size, msgid1);
	if (!trans) goto notrans;

	/* Non-plural-processing gettext forms pass a null pointer as
	 * msgid2 to request that dcngettext suppress plural processing. */
	if (!msgid2) return (char *)trans;

	if (!p->plural_rule) {
		const char *rule = "n!=1;";
		unsigned long np = 2;
		const char *r = __mo_lookup(p->map, p->map_size, "");
		char *z;
		while (r && strncmp(r, "Plural-Forms:", 13)) {
			z = strchr(r, '\n');
			r = z ? z+1 : 0;
		}
		if (r) {
			r += 13;
			while (isspace(*r)) r++;
			if (!strncmp(r, "nplurals=", 9)) {
				np = strtoul(r+9, &z, 10);
				r = z;
			}
			while (*r && *r != ';') r++;
			if (*r) {
				r++;
				while (isspace(*r)) r++;
				if (!strncmp(r, "plural=", 7))
					rule = r+7;
			}
		}
		a_store(&p->nplurals, np);
		a_cas_p(&p->plural_rule, 0, (void *)rule);
	}
	if (p->nplurals) {
		unsigned long plural = __pleval(p->plural_rule, n);
		if (plural > p->nplurals) goto notrans;
		while (plural--) {
			size_t rem = p->map_size - (trans - (char *)p->map);
			size_t l = strnlen(trans, rem);
			if (l+1 >= rem)
				goto notrans;
			trans += l+1;
		}
	}
	return (char *)trans;
}

char *dcgettext(const char *domainname, const char *msgid, int category)
{
	return dcngettext(domainname, msgid, 0, 1, category);
}

char *dngettext(const char *domainname, const char *msgid1, const char *msgid2, unsigned long int n)
{
	return dcngettext(domainname, msgid1, msgid2, n, LC_MESSAGES);
}

char *dgettext(const char *domainname, const char *msgid)
{
	return dcngettext(domainname, msgid, 0, 1, LC_MESSAGES);
}
