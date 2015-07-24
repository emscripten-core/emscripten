#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "__dns.h"
#include "stdio_impl.h"

static int is_valid(const char *host)
{
	const unsigned char *s;
	if (strlen(host)-1 > 254 || mbstowcs(0, host, 0) > 255) return 0;
	for (s=(void *)host; *s>=0x80 || *s=='.' || *s=='-' || isalnum(*s); s++);
	return !*s;
}

#if 0
static int have_af(int family)
{
	struct sockaddr_in6 sin6 = { .sin6_family = family };
	socklen_t sl = family == AF_INET
		? sizeof(struct sockaddr_in)
		: sizeof(struct sockaddr_in6);
	int sock = socket(family, SOCK_STREAM, 0);
	int have = !bind(sock, (void *)&sin6, sl);
	close(sock);
	return have;
}
#endif

union sa {
	struct sockaddr_in sin;
	struct sockaddr_in6 sin6;
};

struct aibuf {
	struct addrinfo ai;
	union sa sa;
};

/* Extra slots needed for storing canonical name */
#define EXTRA ((256+sizeof(struct aibuf)-1)/sizeof(struct aibuf))

int getaddrinfo(const char *restrict host, const char *restrict serv, const struct addrinfo *restrict hint, struct addrinfo **restrict res)
{
	int flags = hint ? hint->ai_flags : 0;
	int family = hint ? hint->ai_family : AF_UNSPEC;
	int type = hint ? hint->ai_socktype : 0;
	int proto = hint ? hint->ai_protocol : 0;
	unsigned long port = 0;
	struct aibuf *buf;
	union sa sa = {{0}};
	unsigned char reply[1024];
	int i, j;
	char line[512];
	FILE *f, _f;
	unsigned char _buf[1024];
	char *z;
	int result;
	int cnt;

	if (family != AF_INET && family != AF_INET6 && family != AF_UNSPEC)
		return EAI_FAMILY;

	if (host && strlen(host)>255) return EAI_NONAME;
	if (serv && strlen(serv)>32) return EAI_SERVICE;

	if (type && !proto)
		proto = type==SOCK_DGRAM ? IPPROTO_UDP : IPPROTO_TCP;
	if (!type && proto)
		type = proto==IPPROTO_UDP ? SOCK_DGRAM : SOCK_STREAM;

	if (serv) {
		if (!*serv) return EAI_SERVICE;
		port = strtoul(serv, &z, 10);
		if (*z) {
			size_t servlen = strlen(serv);
			char *end = line;

			if (flags & AI_NUMERICSERV) return EAI_SERVICE;

			f = __fopen_rb_ca("/etc/services", &_f, _buf, sizeof _buf);
			if (!f) return EAI_SERVICE;
			while (fgets(line, sizeof line, f)) {
				if (strncmp(line, serv, servlen) || !isspace(line[servlen]))
					continue;
				port = strtoul(line+servlen, &end, 10);
				if (strncmp(end, proto==IPPROTO_UDP ? "/udp" : "/tcp", 4))
					continue;
				break;
			}
			__fclose_ca(f);
			if (feof(f)) return EAI_SERVICE;
		}
		if (port > 65535) return EAI_SERVICE;
		port = htons(port);
	}

	if (!host) {
		if (family == AF_UNSPEC) {
			cnt = 2; family = AF_INET;
		} else {
			cnt = 1;
		}
		buf = calloc(sizeof *buf, cnt);
		if (!buf) return EAI_MEMORY;
		for (i=0; i<cnt; i++) {
			if (i) family = AF_INET6;
			buf[i].ai.ai_protocol = proto;
			buf[i].ai.ai_socktype = type;
			buf[i].ai.ai_addr = (void *)&buf[i].sa;
			buf[i].ai.ai_addrlen = family==AF_INET6
				? sizeof sa.sin6 : sizeof sa.sin;
			buf[i].ai.ai_family = family;
			buf[i].sa.sin.sin_family = family;
			buf[i].sa.sin.sin_port = port;
			if (i+1<cnt) buf[i].ai.ai_next = &buf[i+1].ai;
			if (!(flags & AI_PASSIVE)) {
				if (family == AF_INET) {
					0[(uint8_t*)&buf[i].sa.sin.sin_addr.s_addr]=127;
					3[(uint8_t*)&buf[i].sa.sin.sin_addr.s_addr]=1;
				} else buf[i].sa.sin6.sin6_addr.s6_addr[15] = 1;
			}
		}
		*res = &buf->ai;
		return 0;
	}

	if (!*host) return EAI_NONAME;

	/* Try as a numeric address */
	if (__ipparse(&sa, family, host) >= 0) {
		buf = calloc(sizeof *buf, 1+EXTRA);
		if (!buf) return EAI_MEMORY;
		family = sa.sin.sin_family;
		buf->ai.ai_protocol = proto;
		buf->ai.ai_socktype = type;
		buf->ai.ai_addr = (void *)&buf->sa;
		buf->ai.ai_addrlen = family==AF_INET6 ? sizeof sa.sin6 : sizeof sa.sin;
		buf->ai.ai_family = family;
		buf->ai.ai_canonname = (char *)host;
		buf->sa = sa;
		buf->sa.sin.sin_port = port;
		*res = &buf->ai;
		return 0;
	}

	if (flags & AI_NUMERICHOST) return EAI_NONAME;

	f = __fopen_rb_ca("/etc/hosts", &_f, _buf, sizeof _buf);
	if (f) while (fgets(line, sizeof line, f)) {
		char *p;
		size_t l = strlen(host);

		if ((p=strchr(line, '#'))) *p++='\n', *p=0;
		for(p=line+1; (p=strstr(p, host)) &&
			(!isspace(p[-1]) || !isspace(p[l])); p++);
		if (!p) continue;
		__fclose_ca(f);

		/* Isolate IP address to parse */
		for (p=line; *p && !isspace(*p); p++);
		*p++ = 0;
		if (__ipparse(&sa, family, line) < 0) return EAI_NONAME;

		/* Allocate and fill result buffer */
		buf = calloc(sizeof *buf, 1+EXTRA);
		if (!buf) return EAI_MEMORY;
		family = sa.sin.sin_family;
		buf->ai.ai_protocol = proto;
		buf->ai.ai_socktype = type;
		buf->ai.ai_addr = (void *)&buf->sa;
		buf->ai.ai_addrlen = family==AF_INET6 ? sizeof sa.sin6 : sizeof sa.sin;
		buf->ai.ai_family = family;
		buf->sa = sa;
		buf->sa.sin.sin_port = port;

		/* Extract first name as canonical name */
		for (; *p && isspace(*p); p++);
		buf->ai.ai_canonname = (void *)(buf+1);
		snprintf(buf->ai.ai_canonname, 256, "%s", p);
		for (p=buf->ai.ai_canonname; *p && !isspace(*p); p++);
		*p = 0;
		if (!is_valid(buf->ai.ai_canonname))
			buf->ai.ai_canonname = 0;

		*res = &buf->ai;
		return 0;
	}
	if (f) __fclose_ca(f);

#if 0
	f = __fopen_rb_ca("/etc/resolv.conf", &_f, _buf, sizeof _buf);
	if (f) while (fgets(line, sizeof line, f)) {
		if (!isspace(line[10]) || (strncmp(line, "search", 6)
			&& strncmp(line, "domain", 6))) continue;
	}
	if (f) __fclose_ca(f);
#endif

	/* Perform one or more DNS queries for host */
	memset(reply, 0, sizeof reply);
	result = __dns_query(reply, host, family, 0);
	if (result < 0) return result;

	cnt = __dns_count_addrs(reply, result);
	if (cnt <= 0) return EAI_NONAME;

	buf = calloc(sizeof *buf, cnt+EXTRA);
	if (!buf) return EAI_MEMORY;

	i = 0;
	if (family != AF_INET6) {
		j = __dns_get_rr(&buf[i].sa.sin.sin_addr, sizeof *buf, 4, cnt-i, reply, RR_A, 0);
		while (j--) buf[i++].sa.sin.sin_family = AF_INET;
	}
	if (family != AF_INET) {
		j = __dns_get_rr(&buf[i].sa.sin6.sin6_addr, sizeof *buf, 16, cnt-i, reply, RR_AAAA, 0);
		while (j--) buf[i++].sa.sin.sin_family = AF_INET6;
	}
	if (result>1) {
		j = __dns_get_rr(&buf[i].sa.sin.sin_addr, sizeof *buf, 4, cnt-i, reply+512, RR_A, 0);
		while (j--) buf[i++].sa.sin.sin_family = AF_INET;
		j = __dns_get_rr(&buf[i].sa.sin6.sin6_addr, sizeof *buf, 16, cnt-i, reply+512, RR_AAAA, 0);
		while (j--) buf[i++].sa.sin.sin_family = AF_INET6;
	}

	if (__dns_get_rr((void *)&buf[cnt], 0, 256, 1, reply, RR_CNAME, 1) <= 0)
		strcpy((void *)&buf[cnt], host);

	for (i=0; i<cnt; i++) {
		buf[i].ai.ai_protocol = proto;
		buf[i].ai.ai_socktype = type;
		buf[i].ai.ai_addr = (void *)&buf[i].sa;
		buf[i].ai.ai_addrlen = buf[i].sa.sin.sin_family==AF_INET6
			? sizeof sa.sin6 : sizeof sa.sin;
		buf[i].ai.ai_family = buf[i].sa.sin.sin_family;
		buf[i].sa.sin.sin_port = port;
		buf[i].ai.ai_next = &buf[i+1].ai;
		buf[i].ai.ai_canonname = (void *)&buf[cnt];
	}
	buf[cnt-1].ai.ai_next = 0;
	*res = &buf->ai;

	return 0;
}
