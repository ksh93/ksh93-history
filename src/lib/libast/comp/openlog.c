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
 * openlog implementation
 */

#include <ast.h>

#if _lib_syslog

NoN(openlog)

#else

#include "sysloglib.h"

void
openlog(const char* ident, int flags, int facility)
{
	int		n;

	if (ident)
	{
		n = strlen(ident);
		if (n >= sizeof(log.ident))
			n = sizeof(log.ident) - 1;
		memcpy(log.ident, ident, n);
		log.ident[n] = 0;
	}
	else
		log.ident[0] = 0;
	log.facility = facility;
	log.flags = flags;
	if (!(log.flags & LOG_ODELAY))
		sendlog(NiL);
}

#endif
