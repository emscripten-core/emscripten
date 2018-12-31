#define _GNU_SOURCE
#include <grp.h>
#include <string.h>
#include <limits.h>

int getgrouplist(const char *user, gid_t gid, gid_t *groups, int *ngroups)
{
	size_t n, i;
	struct group *gr;
	if (*ngroups<1) return -1;
	n = *ngroups;
	*groups++ = gid;
	*ngroups = 1;

	setgrent();
	while ((gr = getgrent()) && *ngroups < INT_MAX) {
		for (i=0; gr->gr_mem[i] && strcmp(user, gr->gr_mem[i]); i++);
		if (!gr->gr_mem[i]) continue;
		if (++*ngroups <= n) *groups++ = gr->gr_gid;
	}
	endgrent();

	return *ngroups > n ? -1 : *ngroups;
}
