/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2000 AT&T Corp.                *
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
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#include	"sfhdr.h"

/*	Print data with a given format
**
**	Written by Kiem-Phong Vo.
*/

#if __STD_C
int sfprintf(Sfio_t* f, const char* form, ...)
#else
int sfprintf(va_alist)
va_dcl
#endif
{
	va_list	args;
	reg int	rv;

#if __STD_C
	va_start(args,form);
#else
	reg Sfio_t*	f;
	reg char*	form;
	va_start(args);
	f = va_arg(args,Sfio_t*);
	form = va_arg(args,char*);
#endif
	rv = sfvprintf(f,form,args);

	va_end(args);
	return rv;
}

#if __STD_C
int sfvsprintf(char* s, int n, const char* form, va_list args)
#else
int sfvsprintf(s, n, form, args)
char*	s;
int	n;
char*	form;
va_list	args;
#endif
{
	Sfio_t	f;
	reg int	rv;

	if(!s || n <= 0)
		return -1;

	/* make a fake stream */
	SFCLEAR(&f, NIL(Vtmutex_t*));
	f.flags = SF_STRING|SF_WRITE;
	f.bits = SF_PRIVATE;
	f.mode = SF_WRITE;
	f.size = n-1;
	f.data = f.next = f.endr = (uchar*)s;
	f.endb = f.endw = f.data+f.size;

	rv = sfvprintf(&f,form,args);
	*f.next = '\0';
	_Sfi = f.next - f.data;

	return rv;
}

#if __STD_C
int sfsprintf(char* s, int n, const char* form, ...)
#else
int sfsprintf(va_alist)
va_dcl
#endif
{
	va_list	args;
	reg int	rv;

#if __STD_C
	va_start(args,form);
#else
	reg char*	s;
	reg int		n;
	reg char*	form;
	va_start(args);
	s = va_arg(args,char*);
	n = va_arg(args,int);
	form = va_arg(args,char*);
#endif

	rv = sfvsprintf(s,n,form,args);
	va_end(args);

	return rv;
}
