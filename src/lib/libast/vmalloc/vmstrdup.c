/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
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
#if defined(_UWIN) && defined(_BLD_ast)

void _STUB_vmstrdup(){}

#else

#include "vmhdr.h"

/*
 * return a copy of s using vmalloc
 */

#if __STD_C
char* vmstrdup(Vmalloc_t* v, register const char* s)
#else
char* vmstrdup(v, s)
Vmalloc_t*	v;
register char*	s;
#endif
{
	register char*	t;
	register int	n;

	return((t = vmalloc(v, n = strlen(s) + 1)) ? (char*)memcpy(t, s, n) : (char*)0);
}

#endif
