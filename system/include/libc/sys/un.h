#ifndef	_SYS_UN_H
#define	_SYS_UN_H

#define __NEED_sa_family_t
#include <bits/alltypes.h>

struct sockaddr_un
{
	sa_family_t sun_family;
	char sun_path[108];
};

#endif
