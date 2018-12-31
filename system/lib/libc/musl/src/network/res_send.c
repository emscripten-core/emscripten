#include <resolv.h>
#include "libc.h"

int __res_msend(int, const unsigned char *const *, const int *, unsigned char *const *, int *, int);

int __res_send(const unsigned char *msg, int msglen, unsigned char *answer, int anslen)
{
	int r = __res_msend(1, &msg, &msglen, &answer, &anslen, anslen);
	return r<0 ? r : anslen;
}

weak_alias(__res_send, res_send);
