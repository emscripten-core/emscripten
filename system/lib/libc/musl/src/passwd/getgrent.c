#include "pwf.h"

static FILE *f;

void setgrent()
{
	if (f) fclose(f);
	f = 0;
}

weak_alias(setgrent, endgrent);

struct group *getgrent()
{
	static char *line, **mem;
	static struct group gr;
	size_t size=0, nmem=0;
	if (!f) f = fopen("/etc/group", "rbe");
	if (!f) return 0;
	return __getgrent_a(f, &gr, &line, &size, &mem, &nmem);
}

struct group *getgrgid(gid_t gid)
{
	struct group *gr;
	int errno_saved;
	setgrent();
	while ((gr=getgrent()) && gr->gr_gid != gid);
	errno_saved = errno;
	endgrent();
	errno = errno_saved;
	return gr;
}

struct group *getgrnam(const char *name)
{
	struct group *gr;
	int errno_saved;
	setgrent();
	while ((gr=getgrent()) && strcmp(gr->gr_name, name));
	errno_saved = errno;
	endgrent();
	errno = errno_saved;
	return gr;
}
