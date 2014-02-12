#include <unistd.h>
#include <wchar.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include "libc.h"

char *optarg;
int optind=1, opterr=1, optopt, __optpos, __optreset=0;

#define optpos __optpos
weak_alias(__optreset, optreset);

int getopt(int argc, char * const argv[], const char *optstring)
{
	int i;
	wchar_t c, d;
	int k, l;
	char *optchar;

	if (!optind || __optreset) {
		__optreset = 0;
		__optpos = 0;
		optind = 1;
	}

	if (optind >= argc || !argv[optind] || argv[optind][0] != '-' || !argv[optind][1])
		return -1;
	if (argv[optind][1] == '-' && !argv[optind][2])
		return optind++, -1;

	if (!optpos) optpos++;
	if ((k = mbtowc(&c, argv[optind]+optpos, MB_LEN_MAX)) < 0) {
		k = 1;
		c = 0xfffd; /* replacement char */
	}
	optchar = argv[optind]+optpos;
	optopt = c;
	optpos += k;

	if (!argv[optind][optpos]) {
		optind++;
		optpos = 0;
	}

	for (i=0; (l = mbtowc(&d, optstring+i, MB_LEN_MAX)) && d!=c; i+=l>0?l:1);

	if (d != c) {
		if (optstring[0] != ':' && opterr) {
			write(2, argv[0], strlen(argv[0]));
			write(2, ": illegal option: ", 18);
			write(2, optchar, k);
			write(2, "\n", 1);
		}
		return '?';
	}
	if (optstring[i+1] == ':') {
		if (optind >= argc) {
			if (optstring[0] == ':') return ':';
			if (opterr) {
				write(2, argv[0], strlen(argv[0]));
				write(2, ": option requires an argument: ", 31);
				write(2, optchar, k);
				write(2, "\n", 1);
			}
			return '?';
		}
		optarg = argv[optind++] + optpos;
		optpos = 0;
	}
	return c;
}

weak_alias(getopt, __posix_getopt);
