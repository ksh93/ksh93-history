/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*******************************************************************/
#include	"sfdchdr.h"

/*	Discipline to invoke UNIX processes as data filters.
**	These processes must be able to fit in pipelines.
**
**	Written by Kiem-Phong Vo, kpv@research.att.com, 03/18/1998.
*/

#ifndef O_NONBLOCK
#ifdef	O_NDELAY
#define O_NONBLOCK	O_NDELAY
#else
#ifdef	FNDELAY
#define O_NONBLOCK	FNDELAY
#else
#define O_NONBLOCK	0
#endif
#endif
#endif

typedef struct _filter_s
{	Sfdisc_t	disc;		/* discipline structure */
	Sfio_t*		filter;		/* the filter stream */
	char		raw[1024];	/* raw data buffer */
	char*		next;		/* remainder of data unwritten to pipe */
	char*		endb;		/* end of data */
} Filter_t;

/* read data from the filter */
#if __STD_C
static ssize_t filterread(Sfio_t* f, Void_t* buf, size_t n, Sfdisc_t* disc)
#else
static ssize_t filterread(f, buf, n, disc)
Sfio_t*		f;	/* stream reading from */
Void_t*		buf;	/* buffer to read into */
size_t		n;	/* number of bytes requested */
Sfdisc_t*	disc;	/* discipline */
#endif
{
	Filter_t*	fi;
	ssize_t		r, w;

	fi = (Filter_t*)disc;
	for(;;)
	{	if(!fi->next)
			fi->next = fi->endb = fi->raw;
		else
		{	/* try to get data from filter, if any */
			errno = 0;
			if((r = sfread(fi->filter,buf,n)) > 0)
				return r;
			if(errno != EWOULDBLOCK)
				return 0;
		}

		/* get some raw data to stuff down the pipe */
		if(fi->next >= fi->endb)
		{	if((r = sfrd(f,fi->raw,sizeof(fi->raw),disc)) > 0)
			{	fi->next = fi->raw;
				fi->endb = fi->raw+r;
			}
			else
			{	/* eof, close write end of pipes */
				sfset(fi->filter,SF_READ,0);
				close(sffileno(fi->filter));
				sfset(fi->filter,SF_READ,1);
			}
		}

		if((w = fi->endb - fi->next) > 0)
		{	errno = 0;
			if((w = sfwrite(fi->filter,fi->next,w)) > 0)
				fi->next += w;
			else if(errno != EWOULDBLOCK)
				return 0;
			/* pipe is full, sleep for a while, then continue */
			else	sleep(1);
		}
	}
}

#if __STD_C
static ssize_t filterwrite(Sfio_t* f, const Void_t* buf, size_t n, Sfdisc_t* disc)
#else
static ssize_t filterwrite(f, buf, n, disc)
Sfio_t*		f;	/* stream reading from */
Void_t*		buf;	/* buffer to read into */
size_t		n;	/* number of bytes requested */
Sfdisc_t*	disc;	/* discipline */
#endif
{
	return -1;
}

/* for the duration of this discipline, the stream is unseekable */
#if __STD_C
static Sfoff_t filterseek(Sfio_t* f, Sfoff_t addr, int offset, Sfdisc_t* disc)
#else
static Sfoff_t filterseek(f, addr, offset, disc)
Sfio_t*		f;
Sfoff_t		addr;
int		offset;
Sfdisc_t*	disc;
#endif
{	f = NIL(Sfio_t*);
	addr = 0;
	offset = 0;
	disc = NIL(Sfdisc_t*);
	return (Sfoff_t)(-1);
}

/* on close, remove the discipline */
#if __STD_C
static filterexcept(Sfio_t* f, int type, Void_t* data, Sfdisc_t* disc)
#else
static filterexcept(f,type,data,disc)
Sfio_t*		f;
int		type;
Void_t*		data;
Sfdisc_t*	disc;
#endif
{
	if(type == SF_FINAL || type == SF_DPOP)
	{	sfclose(((Filter_t*)disc)->filter);
		free(disc);
	}

	return 0;
}

#if __STD_C
int sfdcfilter(Sfio_t* f, const char* cmd)
#else
int sfdcfilter(f, cmd)
Sfio_t*	f;	/* stream to filter data	*/
char*	cmd;	/* program to run as a filter	*/
#endif
{
	reg Filter_t*	fi;
	reg Sfio_t*	filter;
	int		fl;

	/* open filter for read&write */
	if(!(filter = sfpopen(NIL(Sfio_t*),cmd,"r+")) )
		return -1;

	/* unbuffered so that write data will get to the pipe right away */
	sfsetbuf(filter,NIL(Void_t*),0);

	/* make the write descriptor nonblocking */
	sfset(filter,SF_READ,0);
	if ((fl = fcntl(sffileno(filter),F_GETFL,0)) == -1)
		fl = 0;
	fcntl(sffileno(filter),F_SETFL,fl|O_NONBLOCK);
	sfset(filter,SF_READ,1);

	/* same for the read descriptor */
	sfset(filter,SF_WRITE,0);
	if ((fl = fcntl(sffileno(filter),F_GETFL,0)) == -1)
		fl = 0;
	fcntl(sffileno(filter),F_SETFL,fl|O_NONBLOCK);
	sfset(filter,SF_WRITE,1);

	if(!(fi = (Filter_t*)malloc(sizeof(Filter_t))) )
	{	sfclose(filter);
		return -1;
	}
	memset(fi, 0, sizeof(*fi));

	fi->disc.readf = filterread;
	fi->disc.writef = filterwrite;
	fi->disc.seekf = filterseek;
	fi->disc.exceptf = filterexcept;
	fi->filter = filter;
	fi->next = fi->endb = NIL(char*);

	if(sfdisc(f,(Sfdisc_t*)fi) != (Sfdisc_t*)fi)
	{	sfclose(filter);
		free(fi);
		return -1;
	}

	return 0;
}
