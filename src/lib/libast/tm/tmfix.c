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
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * time conversion support
 */

#include <ast.h>
#include <tm.h>

#define DAYS(p)	(tm_data.days[(p)->tm_mon]+LEAP(p))
#define LEAP(p)	((p)->tm_mon==1&&!((p)->tm_year%4)&&(((p)->tm_year%100)||!((1900+(p)->tm_year)%400)))

/*
 * correct out of bounds fields in tm
 *
 * tm_wday, tm_yday and tm_isdst are not changed
 * as these can be computed from the other fields
 *
 * tm is the return value
 */

Tm_t*
tmfix(register Tm_t* tm)
{
	register int	n;

	if ((n = tm->tm_sec) < 0)
	{
		tm->tm_min -= (59 - n) / 60;
		tm->tm_sec = 60 - (-n) % 60;
	}
	else if (n > (59 + TM_MAXLEAP))
	{
		tm->tm_min += n / 60;
		tm->tm_sec %= 60;
	}
	if ((n = tm->tm_min) < 0)
	{
		tm->tm_hour -= (59 - n) / 60;
		tm->tm_min = 60 - (-n) % 60;
	}
	else if (n > 59)
	{
		tm->tm_hour += n / 60;
		tm->tm_min %= 60;
	}
	if ((n = tm->tm_hour) < 0)
	{
		tm->tm_mday -= (23 - n) / 24;
		tm->tm_hour = 24 - (-n) % 24;
	}
	else if (n > 24)
	{
		tm->tm_mday += n / 24;
		tm->tm_hour %= 24;
	}
	while (tm->tm_mday < 1)
	{
		if (--tm->tm_mon < 0)
		{
			tm->tm_mon = 11;
			tm->tm_year--;
		}
		tm->tm_mday += DAYS(tm);
	}
	while (tm->tm_mday > (n = DAYS(tm)))
	{
		tm->tm_mday -= n;
		if (++tm->tm_mon > 11)
		{
			tm->tm_mon = 0;
			tm->tm_year++;
		}
	}

	/*
	 * tm_isdst is adjusted by tmtime()
	 */

	return tm;
}
