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
*    If you have copied or used this software without agreeing     *
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
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * multi-pass commmand line option parse assist
 *
 *	int fun(char** argv, int last)
 *
 * each fun() argument parses as much of argv as
 * possible starting at (opt_info.index,opt_info.offset) using
 * optget()
 *
 * if last!=0 then fun is the last pass to view
 * the current arg, otherwise fun sets opt_info.again=1
 * and another pass will get a crack at it
 *
 * 0 fun() return causes immediate optjoin() 0 return
 *
 * optjoin() returns non-zero if more args remain
 * to be parsed at opt_info.index
 */

#include <optlib.h>

typedef int (*Optpass_f)(char**, int);

int
optjoin(char** argv, ...)
{
	va_list			ap;
	register Optpass_f	fun;
	register Optpass_f	rep;
	Optpass_f		err;
	int			more;
	int			user;
	int			last_index;
	int			last_offset;
	int			err_index;
	int			err_offset;

	if (!opt_info.state)
		optget(NiL, NiL);
	err = rep = 0;
	for (;;)
	{
		va_start(ap, argv);
		opt_info.state->join = 0;
		while (fun = va_arg(ap, Optpass_f))
		{
			last_index = opt_info.index;
			last_offset = opt_info.offset;
			opt_info.state->join++;
			user = (*fun)(argv, 0);
			more = argv[opt_info.index] != 0;
			if (!opt_info.again)
			{
				if (!more)
				{
					opt_info.state->join = 0;
					return 0;
				}
				if (!user)
				{
					if (*argv[opt_info.index] != '+')
					{
						opt_info.state->join = 0;
						return 1;
					}
					opt_info.again = -1;
				}
				else
					err = 0;
			}
			if (opt_info.again)
			{
				if (opt_info.again > 0 && (!err || err_index < opt_info.index || err_index == opt_info.index && err_offset < opt_info.offset))
				{
					err = fun;
					err_index = opt_info.index;
					err_offset = opt_info.offset;
				}
				opt_info.again = 0;
				opt_info.index = opt_info.state->pindex ? opt_info.state->pindex : 1;
				opt_info.offset = opt_info.state->poffset;
			}
			if (!rep || opt_info.index != last_index || opt_info.offset != last_offset)
				rep = fun;
			else if (fun == rep)
			{
				if (!err)
				{
					opt_info.state->join = 0;
					return 1;
				}
				(*err)(argv, 1);
				opt_info.offset = 0;
			}
		}
		va_end(ap);
	}
}
