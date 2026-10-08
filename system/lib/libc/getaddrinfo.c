// Emscripten-specific version of musl/src/network/getaddrinfo.c: the
// synchronous call variant of emscripten_dns_lookup (see libcore.js).

#include <netdb.h>
#include <stdint.h>
#include <emscripten/emscripten.h>

int getaddrinfo(const char *restrict host, const char *restrict serv, const struct addrinfo *restrict hint, struct addrinfo **restrict res)
{
	intptr_t r = emscripten_dns_lookup(host, serv, hint);
	if (r < 0) return r;
	*res = (struct addrinfo*)r;
	return 0;
}
