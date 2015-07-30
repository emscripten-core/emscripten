/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

#include <wchar.h>

int mbsinit(const mbstate_t *st)
{
	return !st || !*(unsigned *)st;
}
