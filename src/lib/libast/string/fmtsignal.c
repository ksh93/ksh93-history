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
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * if sig>=0 then return signal text for signal sig
 * otherwise return signal name for signal -sig
 */

#include <ast.h>
#include <sig.h>

char*
fmtsignal(register int sig)
{
	char*	buf;
	int	z;

	if (sig >= 0)
	{
		if (sig <= sig_info.sigmax)
			buf = sig_info.text[sig];
		else
		{
			buf = fmtbuf(z = 20);
			sfsprintf(buf, z, "Signal %d", sig);
		}
	}
	else
	{
		sig = -sig;
		if (sig <= sig_info.sigmax)
			buf = sig_info.name[sig];
		else
		{
			buf = fmtbuf(z = 20);
			sfsprintf(buf, z, "%d", sig);
		}
	}
	return buf;
}
