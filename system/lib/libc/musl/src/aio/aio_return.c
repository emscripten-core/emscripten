#include <aio.h>

ssize_t aio_return(struct aiocb *cb)
{
	return cb->__ret;
}
