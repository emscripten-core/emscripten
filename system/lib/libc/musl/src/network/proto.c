#include <netdb.h>
#include <string.h>

/* do we really need all these?? */

static int idx;
static const unsigned char protos[][8] = {
	"\000ip",
	"\001icmp",
	"\002igmp",
	"\003ggp",
	"\006tcp",
	"\014pup",
	"\021udp",
	"\026idp",
	"\051ipv6",
	"\072icmpv6",
	"\377raw",
	"\0\0"
};

void endprotoent(void)
{
	idx = 0;
}

void setprotoent(int stayopen)
{
	idx = 0;
}

struct protoent *getprotoent(void)
{
	static struct protoent p;
	static const char *aliases;
	if (!protos[idx][1]) return NULL;
	p.p_proto = protos[idx][0];
	p.p_name = (char *)protos[idx++]+1;
	p.p_aliases = (char **)&aliases;
	return &p;
}

struct protoent *getprotobyname(const char *name)
{
	struct protoent *p;
	endprotoent();
	do p = getprotoent();
	while (p && strcmp(name, p->p_name));
	return p;
}

struct protoent *getprotobynumber(int num)
{
	struct protoent *p;
	endprotoent();
	do p = getprotoent();
	while (p && p->p_proto != num);
	return p;
}
