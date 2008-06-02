/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2008 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped

#if !_IP6_H
#define _IP6_H		1

typedef unsigned char Ip6addr_t[16];

typedef struct Ip6prefix_s
{
	Ip6addr_t	addr;
	unsigned char	bits;
} Ip6prefix_t;

extern int	strtoip6(const char*, char**, Ip6addr_t*, unsigned char*);
extern char*	fmtip6(Ip6addr_t*, int);

#endif
