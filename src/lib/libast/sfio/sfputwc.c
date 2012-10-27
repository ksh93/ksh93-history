/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2012 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
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
#include	"sfhdr.h"

/*	Write out a rune (wide char) as a multibyte char on f.
**
**	Written by Kiem-Phong Vo.
*/

#if __STD_C
int sfputwc(Sfio_t* f, int w)
#else
int sfputwc(f,w)
Sfio_t*		f;	/* write a portable ulong to this stream */
int		w;	/* the unsigned value to be written */
#endif
{
	reg uchar	*s;
	reg char	*b;
	int		n, m;
	char		buf[32];
	SFMTXDECL(f);

	SFMTXENTER(f, -1);

	if(f->mode != SF_WRITE && _sfmode(f,SF_WRITE,0) < 0)
		SFMTXRETURN(f, -1);
	SFLOCK(f,0);

	n = mbconv(buf, w);

	if(n > 8 || SFWPEEK(f,s,m) < n)
		n = SFWRITE(f,(Void_t*)s,n); /* write the hard way */
	else
	{	b = buf;
		switch(n)
		{
		case 8 : *s++ = *b++;
		case 7 : *s++ = *b++;
		case 6 : *s++ = *b++;
		case 5 : *s++ = *b++;
		case 4 : *s++ = *b++;
		case 3 : *s++ = *b++;
		case 2 : *s++ = *b++;
		case 1 : *s++ = *b++;
		}
		f->next = s;
	}

	SFOPEN(f,0);
	SFMTXRETURN(f, (int)n);
}
