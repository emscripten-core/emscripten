/*
 * Copyright (c) 1988, 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)termios.h	8.3 (Berkeley) 3/28/94
 * $Id: termios.h,v 1.11 1998/05/05 21:49:37 jb Exp $
 */

#ifndef _SYS_TERMIOS_H_
#define _SYS_TERMIOS_H_

/*
 * Special Control Characters
 *
 * Index into c_cc[] character array.
 *
 *	Name	     Subscript	Enabled by
 */
#define	VEOF		0	/* ICANON */
#define	VEOL		1	/* ICANON */
#ifndef _POSIX_SOURCE
#define	VEOL2		2	/* ICANON together with IEXTEN */
#endif
#define	VERASE		3	/* ICANON */
#ifndef _POSIX_SOURCE
#define VWERASE 	4	/* ICANON together with IEXTEN */
#endif
#define VKILL		5	/* ICANON */
#ifndef _POSIX_SOURCE
#define	VREPRINT 	6	/* ICANON together with IEXTEN */
#endif
/*			7	   spare 1 */
#define VINTR		8	/* ISIG */
#define VQUIT		9	/* ISIG */
#define VSUSP		10	/* ISIG */
#ifndef _POSIX_SOURCE
#define VDSUSP		11	/* ISIG together with IEXTEN */
#endif
#define VSTART		12	/* IXON, IXOFF */
#define VSTOP		13	/* IXON, IXOFF */
#ifndef _POSIX_SOURCE
#define	VLNEXT		14	/* IEXTEN */
#define	VDISCARD	15	/* IEXTEN */
#endif
#define VMIN		16	/* !ICANON */
#define VTIME		17	/* !ICANON */
#ifndef _POSIX_SOURCE
#define VSTATUS		18	/* ICANON together with IEXTEN */
/*			19	   spare 2 */
#endif
#define	NCCS		20

#define	_POSIX_VDISABLE	0xff

#ifndef _POSIX_SOURCE
#define	CCEQ(val, c)	((c) == (val) ? (val) != _POSIX_VDISABLE : 0)
#endif

/*
 * Input flags - software input processing
 */
#define	IGNBRK		0x00000001	/* ignore BREAK condition */
#define	BRKINT		0x00000002	/* map BREAK to SIGINTR */
#define	IGNPAR		0x00000004	/* ignore (discard) parity errors */
#define	PARMRK		0x00000008	/* mark parity and framing errors */
#define	INPCK		0x00000010	/* enable checking of parity errors */
#define	ISTRIP		0x00000020	/* strip 8th bit off chars */
#define	INLCR		0x00000040	/* map NL into CR */
#define	IGNCR		0x00000080	/* ignore CR */
#define	ICRNL		0x00000100	/* map CR to NL (ala CRMOD) */
#define	IXON		0x00000200	/* enable output flow control */
#define	IXOFF		0x00000400	/* enable input flow control */
#ifndef _POSIX_SOURCE
#define	IXANY		0x00000800	/* any char will restart after stop */
#define IMAXBEL		0x00002000	/* ring bell on input queue full */
#endif  /*_POSIX_SOURCE */

/*
 * Output flags - software output processing
 */
#define	OPOST		0x00000001	/* enable following output processing */
#ifndef _POSIX_SOURCE
#define ONLCR		0x00000002	/* map NL to CR-NL (ala CRMOD) */
#define OXTABS		0x00000004	/* expand tabs to spaces */
#define ONOEOT		0x00000008	/* discard EOT's (^D) on output) */
#endif  /*_POSIX_SOURCE */

/*
 * Control flags - hardware control of terminal
 */
#ifndef _POSIX_SOURCE
#define	CIGNORE		0x00000001	/* ignore control flags */
#endif
#define CSIZE		0x00000300	/* character size mask */
#define     CS5		    0x00000000	    /* 5 bits (pseudo) */
#define     CS6		    0x00000100	    /* 6 bits */
#define     CS7		    0x00000200	    /* 7 bits */
#define     CS8		    0x00000300	    /* 8 bits */
#define CSTOPB		0x00000400	/* send 2 stop bits */
#define CREAD		0x00000800	/* enable receiver */
#define PARENB		0x00001000	/* parity enable */
#define PARODD		0x00002000	/* odd parity, else even */
#define HUPCL		0x00004000	/* hang up on last close */
#define CLOCAL		0x00008000	/* ignore modem status lines */
#ifndef _POSIX_SOURCE
#define CCTS_OFLOW	0x00010000	/* CTS flow control of output */
#define CRTSCTS		(CCTS_OFLOW | CRTS_IFLOW)
#define CRTS_IFLOW	0x00020000	/* RTS flow control of input */
#define	CDTR_IFLOW	0x00040000	/* DTR flow control of input */
#define CDSR_OFLOW	0x00080000	/* DSR flow control of output */
#define	CCAR_OFLOW	0x00100000	/* DCD flow control of output */
#define	MDMBUF		0x00100000	/* old name for CCAR_OFLOW */
#endif


/*
 * "Local" flags - dumping ground for other state
 *
 * Warning: some flags in this structure begin with
 * the letter "I" and look like they belong in the
 * input flag.
 */

#ifndef _POSIX_SOURCE
#define	ECHOKE		0x00000001	/* visual erase for line kill */
#endif  /*_POSIX_SOURCE */
#define	ECHOE		0x00000002	/* visually erase chars */
#define	ECHOK		0x00000004	/* echo NL after line kill */
#define ECHO		0x00000008	/* enable echoing */
#define	ECHONL		0x00000010	/* echo NL even if ECHO is off */
#ifndef _POSIX_SOURCE
#define	ECHOPRT		0x00000020	/* visual erase mode for hardcopy */
#define ECHOCTL  	0x00000040	/* echo control chars as ^(Char) */
#endif  /*_POSIX_SOURCE */
#define	ISIG		0x00000080	/* enable signals INTR, QUIT, [D]SUSP */
#define	ICANON		0x00000100	/* canonicalize input lines */
#ifndef _POSIX_SOURCE
#define ALTWERASE	0x00000200	/* use alternate WERASE algorithm */
#endif  /*_POSIX_SOURCE */
#define	IEXTEN		0x00000400	/* enable DISCARD and LNEXT */
#define EXTPROC         0x00000800      /* external processing */
#define TOSTOP		0x00400000	/* stop background jobs from output */
#ifndef _POSIX_SOURCE
#define FLUSHO		0x00800000	/* output being flushed (state) */
#define	NOKERNINFO	0x02000000	/* no kernel output from VSTATUS */
#define PENDIN		0x20000000	/* XXX retype pending input (state) */
#endif  /*_POSIX_SOURCE */
#define	NOFLSH		0x80000000	/* don't flush after interrupt */

typedef unsigned int	tcflag_t;
typedef unsigned char	cc_t;
typedef unsigned int	speed_t;

struct termios {
	tcflag_t	c_iflag;	/* input flags */
	tcflag_t	c_oflag;	/* output flags */
	tcflag_t	c_cflag;	/* control flags */
	tcflag_t	c_lflag;	/* local flags */
	cc_t		c_cc[NCCS];	/* control chars */
	speed_t		c_ispeed;	/* input speed */
	speed_t		c_ospeed;	/* output speed */
};

/*
 * Commands passed to tcsetattr() for setting the termios structure.
 */
#define	TCSANOW		0		/* make change immediate */
#define	TCSADRAIN	1		/* drain output, then change */
#define	TCSAFLUSH	2		/* drain output, flush input */
#ifndef _POSIX_SOURCE
#define TCSASOFT	0x10		/* flag - don't alter h.w. state */
#endif

/*
 * Standard speeds
 */
#define B0	0
#define B50	50
#define B75	75
#define B110	110
#define B134	134
#define B150	150
#define B200	200
#define B300	300
#define B600	600
#define B1200	1200
#define	B1800	1800
#define B2400	2400
#define B4800	4800
#define B9600	9600
#define B19200	19200
#define B38400	38400
#ifndef _POSIX_SOURCE
#define B7200	7200
#define B14400	14400
#define B28800	28800
#define B57600	57600
#define B76800	76800
#define B115200	115200
#define B230400	230400
#define EXTA	19200
#define EXTB	38400
#endif  /* !_POSIX_SOURCE */

#ifndef KERNEL

#define	TCIFLUSH	1
#define	TCOFLUSH	2
#define TCIOFLUSH	3
#define	TCOOFF		1
#define	TCOON		2
#define TCIOFF		3
#define TCION		4

#include <sys/cdefs.h>

__BEGIN_DECLS
speed_t	cfgetispeed __P((const struct termios *));
speed_t	cfgetospeed __P((const struct termios *));
int	cfsetispeed __P((struct termios *, speed_t));
int	cfsetospeed __P((struct termios *, speed_t));
int	tcgetattr __P((int, struct termios *));
int	tcsetattr __P((int, int, const struct termios *));
int	tcdrain __P((int));
int	tcflow __P((int, int));
int	tcflush __P((int, int));
int	tcsendbreak __P((int, int));

#ifndef _POSIX_SOURCE
void	cfmakeraw __P((struct termios *));
int	cfsetspeed __P((struct termios *, speed_t));
#endif /* !_POSIX_SOURCE */
__END_DECLS

#endif /* !KERNEL */

/* XXX Emscripten */
struct winsize {
  unsigned short ws_row;
  unsigned short ws_col;
  unsigned short ws_xpixel;
  unsigned short ws_ypixel;
};

/*
 * END OF PROTECTED INCLUDE.
 */
#endif /* !_SYS_TERMIOS_H_ */

#ifndef _POSIX_SOURCE
#include <sys/ttydefaults.h>
#endif
