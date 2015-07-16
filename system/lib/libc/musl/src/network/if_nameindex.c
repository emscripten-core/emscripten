#define _GNU_SOURCE
#include <net/if.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "syscall.h"

static void *do_nameindex(int s, size_t n)
{
	size_t i, len, k;
	struct ifconf conf;
	struct if_nameindex *idx;

	idx = malloc(n * (sizeof(struct if_nameindex)+sizeof(struct ifreq)));
	if (!idx) return 0;

	conf.ifc_buf = (void *)&idx[n];
	conf.ifc_len = len = n * sizeof(struct ifreq);
	if (ioctl(s, SIOCGIFCONF, &conf) < 0) {
		free(idx);
		return 0;
	}
	if (conf.ifc_len == len) {
		free(idx);
		return (void *)-1;
	}

	n = conf.ifc_len / sizeof(struct ifreq);
	for (i=k=0; i<n; i++) {
		if (ioctl(s, SIOCGIFINDEX, &conf.ifc_req[i]) < 0) {
			k++;
			continue;
		}
		idx[i-k].if_index = conf.ifc_req[i].ifr_ifindex;
		idx[i-k].if_name = conf.ifc_req[i].ifr_name;
	}
	idx[i-k].if_name = 0;
	idx[i-k].if_index = 0;

	return idx;
}

struct if_nameindex *if_nameindex()
{
	size_t n;
	void *p = 0;
	int s = socket(AF_UNIX, SOCK_DGRAM|SOCK_CLOEXEC, 0);
	if (s>=0) {
		for (n=0; (p=do_nameindex(s, n)) == (void *)-1; n++);
		__syscall(SYS_close, s);
	}
	errno = ENOBUFS;
	return p;
}
