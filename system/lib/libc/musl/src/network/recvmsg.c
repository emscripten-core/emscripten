#include <sys/socket.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "syscall.h"
#ifdef __EMSCRIPTEN_PTHREADS__
#include "emscripten_fd_wait.h"
#endif

hidden void __convert_scm_timestamps(struct msghdr *, socklen_t);

void __convert_scm_timestamps(struct msghdr *msg, socklen_t csize)
{
	if (SCM_TIMESTAMP == SCM_TIMESTAMP_OLD) return;
	if (!msg->msg_control || !msg->msg_controllen) return;

	struct cmsghdr *cmsg, *last=0;
	long tmp;
	long long tvts[2];
	int type = 0;

	for (cmsg=CMSG_FIRSTHDR(msg); cmsg; cmsg=CMSG_NXTHDR(msg, cmsg)) {
		if (cmsg->cmsg_level==SOL_SOCKET) switch (cmsg->cmsg_type) {
		case SCM_TIMESTAMP_OLD:
			if (type) break;
			type = SCM_TIMESTAMP;
			goto common;
		case SCM_TIMESTAMPNS_OLD:
			type = SCM_TIMESTAMPNS;
		common:
			memcpy(&tmp, CMSG_DATA(cmsg), sizeof tmp);
			tvts[0] = tmp;
			memcpy(&tmp, CMSG_DATA(cmsg) + sizeof tmp, sizeof tmp);
			tvts[1] = tmp;
			break;
		}
		last = cmsg;
	}
	if (!last || !type) return;
	if (CMSG_SPACE(sizeof tvts) > csize-msg->msg_controllen) {
		msg->msg_flags |= MSG_CTRUNC;
		return;
	}
	msg->msg_controllen += CMSG_SPACE(sizeof tvts);
	cmsg = CMSG_NXTHDR(msg, last);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = type;
	cmsg->cmsg_len = CMSG_LEN(sizeof tvts);
	memcpy(CMSG_DATA(cmsg), &tvts, sizeof tvts);
}

ssize_t recvmsg(int fd, struct msghdr *msg, int flags)
{
	ssize_t r;
	socklen_t orig_controllen = msg->msg_controllen;
#if LONG_MAX > INT_MAX && !defined(__EMSCRIPTEN__)
	struct msghdr h, *orig = msg;
	if (msg) {
		h = *msg;
		h.__pad1 = h.__pad2 = 0;
		msg = &h;
	}
#endif
#ifdef __EMSCRIPTEN_PTHREADS__
	for (;;) {
		long rr = __socketcall_cp(recvmsg, fd, msg, flags, 0, 0, 0);
		if (rr != -EAGAIN || !__emscripten_sock_can_wait(fd, flags & MSG_DONTWAIT)
		    || _emscripten_fd_wait(fd, POLLIN)) {
			r = __syscall_ret(rr);
			break;
		}
	}
#else
	r = socketcall_cp(recvmsg, fd, msg, flags, 0, 0, 0);
#endif
	if (r >= 0) __convert_scm_timestamps(msg, orig_controllen);
#if LONG_MAX > INT_MAX && !defined(__EMSCRIPTEN__)
	if (orig) *orig = h;
#endif
	return r;
}
