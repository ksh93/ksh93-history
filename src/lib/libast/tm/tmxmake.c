/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
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
/*
 * Glenn Fowler
 * AT&T Research
 *
 * Time_t conversion support
 */

#include <tmx.h>

static const unsigned char yday2mon[] =
{
	0, 1,
	0, 2,
	0, 3,
	0, 4,
	0, 5,
	0, 6,
	0, 7,
	0, 8,
	0, 9,
	0, 10,
	0, 11,
	0, 12,
	0, 13,
	0, 14,
	0, 15,
	0, 16,
	0, 17,
	0, 18,
	0, 19,
	0, 20,
	0, 21,
	0, 22,
	0, 23,
	0, 24,
	0, 25,
	0, 26,
	0, 27,
	0, 28,
	0, 29,
	0, 30,
	0, 31,
	1, 1,
	1, 2,
	1, 3,
	1, 4,
	1, 5,
	1, 6,
	1, 7,
	1, 8,
	1, 9,
	1, 10,
	1, 11,
	1, 12,
	1, 13,
	1, 14,
	1, 15,
	1, 16,
	1, 17,
	1, 18,
	1, 19,
	1, 20,
	1, 21,
	1, 22,
	1, 23,
	1, 24,
	1, 25,
	1, 26,
	1, 27,
	1, 28,
	2, 1,
	2, 2,
	2, 3,
	2, 4,
	2, 5,
	2, 6,
	2, 7,
	2, 8,
	2, 9,
	2, 10,
	2, 11,
	2, 12,
	2, 13,
	2, 14,
	2, 15,
	2, 16,
	2, 17,
	2, 18,
	2, 19,
	2, 20,
	2, 21,
	2, 22,
	2, 23,
	2, 24,
	2, 25,
	2, 26,
	2, 27,
	2, 28,
	2, 29,
	2, 30,
	2, 31,
	3, 1,
	3, 2,
	3, 3,
	3, 4,
	3, 5,
	3, 6,
	3, 7,
	3, 8,
	3, 9,
	3, 10,
	3, 11,
	3, 12,
	3, 13,
	3, 14,
	3, 15,
	3, 16,
	3, 17,
	3, 18,
	3, 19,
	3, 20,
	3, 21,
	3, 22,
	3, 23,
	3, 24,
	3, 25,
	3, 26,
	3, 27,
	3, 28,
	3, 29,
	3, 30,
	4, 1,
	4, 2,
	4, 3,
	4, 4,
	4, 5,
	4, 6,
	4, 7,
	4, 8,
	4, 9,
	4, 10,
	4, 11,
	4, 12,
	4, 13,
	4, 14,
	4, 15,
	4, 16,
	4, 17,
	4, 18,
	4, 19,
	4, 20,
	4, 21,
	4, 22,
	4, 23,
	4, 24,
	4, 25,
	4, 26,
	4, 27,
	4, 28,
	4, 29,
	4, 30,
	4, 31,
	5, 1,
	5, 2,
	5, 3,
	5, 4,
	5, 5,
	5, 6,
	5, 7,
	5, 8,
	5, 9,
	5, 10,
	5, 11,
	5, 12,
	5, 13,
	5, 14,
	5, 15,
	5, 16,
	5, 17,
	5, 18,
	5, 19,
	5, 20,
	5, 21,
	5, 22,
	5, 23,
	5, 24,
	5, 25,
	5, 26,
	5, 27,
	5, 28,
	5, 29,
	5, 30,
	6, 1,
	6, 2,
	6, 3,
	6, 4,
	6, 5,
	6, 6,
	6, 7,
	6, 8,
	6, 9,
	6, 10,
	6, 11,
	6, 12,
	6, 13,
	6, 14,
	6, 15,
	6, 16,
	6, 17,
	6, 18,
	6, 19,
	6, 20,
	6, 21,
	6, 22,
	6, 23,
	6, 24,
	6, 25,
	6, 26,
	6, 27,
	6, 28,
	6, 29,
	6, 30,
	6, 31,
	7, 1,
	7, 2,
	7, 3,
	7, 4,
	7, 5,
	7, 6,
	7, 7,
	7, 8,
	7, 9,
	7, 10,
	7, 11,
	7, 12,
	7, 13,
	7, 14,
	7, 15,
	7, 16,
	7, 17,
	7, 18,
	7, 19,
	7, 20,
	7, 21,
	7, 22,
	7, 23,
	7, 24,
	7, 25,
	7, 26,
	7, 27,
	7, 28,
	7, 29,
	7, 30,
	7, 31,
	8, 1,
	8, 2,
	8, 3,
	8, 4,
	8, 5,
	8, 6,
	8, 7,
	8, 8,
	8, 9,
	8, 10,
	8, 11,
	8, 12,
	8, 13,
	8, 14,
	8, 15,
	8, 16,
	8, 17,
	8, 18,
	8, 19,
	8, 20,
	8, 21,
	8, 22,
	8, 23,
	8, 24,
	8, 25,
	8, 26,
	8, 27,
	8, 28,
	8, 29,
	8, 30,
	9, 1,
	9, 2,
	9, 3,
	9, 4,
	9, 5,
	9, 6,
	9, 7,
	9, 8,
	9, 9,
	9, 10,
	9, 11,
	9, 12,
	9, 13,
	9, 14,
	9, 15,
	9, 16,
	9, 17,
	9, 18,
	9, 19,
	9, 20,
	9, 21,
	9, 22,
	9, 23,
	9, 24,
	9, 25,
	9, 26,
	9, 27,
	9, 28,
	9, 29,
	9, 30,
	9, 31,
	10, 1,
	10, 2,
	10, 3,
	10, 4,
	10, 5,
	10, 6,
	10, 7,
	10, 8,
	10, 9,
	10, 10,
	10, 11,
	10, 12,
	10, 13,
	10, 14,
	10, 15,
	10, 16,
	10, 17,
	10, 18,
	10, 19,
	10, 20,
	10, 21,
	10, 22,
	10, 23,
	10, 24,
	10, 25,
	10, 26,
	10, 27,
	10, 28,
	10, 29,
	10, 30,
	11, 1,
	11, 2,
	11, 3,
	11, 4,
	11, 5,
	11, 6,
	11, 7,
	11, 8,
	11, 9,
	11, 10,
	11, 11,
	11, 12,
	11, 13,
	11, 14,
	11, 15,
	11, 16,
	11, 17,
	11, 18,
	11, 19,
	11, 20,
	11, 21,
	11, 22,
	11, 23,
	11, 24,
	11, 25,
	11, 26,
	11, 27,
	11, 28,
	11, 29,
	11, 30,
	11, 31,
};

/*
 * return Tm_t for t
 * time zone and leap seconds accounted for in return value
 */

Tm_t*
tmxmake(Time_t t)
{
	register struct tm*	tp;
	register Tm_leap_t*	lp;
	Time_t			x;
	time_t			now;
	int			leapsec;
	int			y;
	unsigned _ast_int4_t	n;
	unsigned _ast_int4_t	o;
#if TMX_FLOAT
	Time_t			z;
	unsigned _ast_int4_t	i;
#endif
	Tm_t			tm;

	static Tm_t		ts;

	tmset(tm_info.zone);
	leapsec = 0;
	if ((tm_info.flags & (TM_ADJUST|TM_LEAP)) == (TM_ADJUST|TM_LEAP) && (n = tmxsec(t)))
	{
		for (lp = &tm_data.leap[0]; n < lp->time; lp++);
		if (lp->total)
		{
			if (n == lp->time && (leapsec = (lp->total - (lp+1)->total)) < 0)
				leapsec = 0;
			t = tmxsns(n - lp->total, tmxnsec(t));
		}
	}
	x = tmxsec(t);
	if (tm_info.flags & TM_UTC)
		tm.tm_zone = &tm_data.zone[2];
	else
	{
		tm.tm_zone = tm_info.zone;
		o = 60 * tm.tm_zone->west;
		if (x > o)
		{
			x -= o;
			o = 0;
		}
	}
#if TMX_FLOAT
	i = x / (24 * 60 * 60);
	z = i;
	n = x - z * (24 * 60 * 60);
	tm.tm_sec = n % 60 + leapsec;
	n /= 60;
	tm.tm_min = n % 60;
	n /= 60;
	tm.tm_hour = n % 24;
#define x	i
#else
	tm.tm_sec = x % 60 + leapsec;
	x /= 60;
	tm.tm_min = x % 60;
	x /= 60;
	tm.tm_hour = x % 24;
	x /= 24;
#endif
	tm.tm_wday = (x + 4) % 7;
	tm.tm_year = (400 * (x + 25202)) / 146097 + 1;
	n = tm.tm_year - 1;
	x -= n * 365 + n / 4 - n / 100 + (n + (1900 - 1600)) / 400 - (1970 - 1901) * 365 - (1970 - 1901) / 4;
	if (x == 366 || x == 365 && !tmisleapyear(tm.tm_year))
	{
		tm.tm_year++;
		x = 0;
	}
	tm.tm_yday = x;
	if (x < 59 || !tmisleapyear(tm.tm_year) || x > 59 && x--)
	{
		tm.tm_mon = yday2mon[x * 2];
		tm.tm_mday = yday2mon[x * 2 + 1];
	}
	else
	{
		tm.tm_mon = 1;
		tm.tm_mday = 29;
	}
	n += 1900;
	tm.tm_isdst = 0;
	if (tm.tm_zone->daylight)
	{
		if ((y = tmequiv(&tm) - 1900) == tm.tm_year && !getenv("_AST_tmxmake_DEBUG"))
			now = tmxsec(t);
		else
		{
			Tm_t	te;

			te = tm;
			te.tm_year = y;
			now = tmxsec(tmxtime(&te, tm.tm_zone->west));
		}
		if ((tp = localtime(&now)) && ((tm.tm_isdst = tp->tm_isdst) || o))
		{
			tm.tm_min -= o / 60 + (tm.tm_isdst ? tm.tm_zone->dst : 0);
			tmfix(&tm);
		}
	}
	tm.tm_nsec = tmxnsec(t);
	ts = tm;
	return &ts;
}
