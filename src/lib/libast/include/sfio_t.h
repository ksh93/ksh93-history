/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1985-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*               Phong Vo <kpv@research.att.com>                *
*                                                              *
***************************************************************/
#ifndef _SFIO_T_H
#define _SFIO_T_H	1

/*	This header file is for library writers who need to know certain
**	internal info concerning the full Sfio_t structure. Including this
**	file means that you agree to track closely with sfio development
**	in case its internal architecture is changed.
**
**	Written by Kiem-Phong Vo (02/02/1993).
*/

/* the parts of Sfio_t private to sfio functions */
#define _SFIO_PRIVATE \
	Sfoff_t			extent;	/* current file	size		*/ \
	Sfoff_t			here;	/* current physical location	*/ \
	unsigned char		getr;	/* the last sfgetr separator 	*/ \
	unsigned char		tiny[1];/* for unbuffered read stream	*/ \
	unsigned short		bits;	/* private flags		*/ \
	unsigned int		mode;	/* current io mode		*/ \
	struct _sfdisc_s*	disc;	/* discipline			*/ \
	struct _sfpool_s*	pool;	/* the pool containing this	*/ \
	Void_t*			noop;	/* unused for now		*/

#include	"sfio.h"

/* mode bit to indicate that the structure hasn't been initialized */
#define SF_INIT		0000004

/* short-hand for common stream types */
#define SF_RDWR		(SF_READ|SF_WRITE)
#define SF_RDSTR	(SF_READ|SF_STRING)
#define SF_WRSTR	(SF_WRITE|SF_STRING)
#define SF_RDWRSTR	(SF_RDWR|SF_STRING)

/* macro to initialize an Sfio_t structure */
#define SFNEW(data,size,file,type,disc)	\
	{ (unsigned char*)(data),			/* next		*/ \
	  (unsigned char*)(data),			/* endw		*/ \
	  (unsigned char*)(data),			/* endr		*/ \
	  (unsigned char*)(data),			/* endb		*/ \
	  (Sfio_t*)0,					/* push		*/ \
	  (unsigned short)((type)&SF_FLAGS),		/* flags	*/ \
	  (short)(file),				/* file		*/ \
	  (unsigned char*)(data),			/* data		*/ \
	  (ssize_t)(size),				/* size		*/ \
	  (ssize_t)(-1),				/* val		*/ \
	  (Sfoff_t)0,					/* extent	*/ \
	  (Sfoff_t)0,					/* here		*/ \
	  0,						/* getr		*/ \
	  "",						/* tiny		*/ \
	  0,						/* bits		*/ \
	  (unsigned int)(((type)&(SF_RDWR))|SF_INIT),	/* mode		*/ \
	  (struct _sfdisc_s*)(disc),			/* disc		*/ \
	  (struct _sfpool_s*)0,				/* pool		*/ \
	  (Void_t*)0,					/* noop		*/ \
	}

#endif /* _SFIO_T_H */
