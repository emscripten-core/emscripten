#include <aio.h>

int aio_error(const struct aiocb *cb)
{
	return cb->__err;
}
