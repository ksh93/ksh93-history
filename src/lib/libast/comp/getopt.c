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

#include <ast.h>

#if _lib_getopt

NoN(getopt)

#else

#if _WIN32
#undef	_BLD_ast	/* enable ast imports since we're user static */
#endif

#include <error.h>
#include <option.h>

int		opterr = 1;
int		optind = 1;
int		optopt;
char*		optarg;

static int	lastoptind;

extern int
getopt(int argc, char* const* argv, const char* optstring)
{
	int	n;

	NoP(argc);
	opt_info.index = (optind > 1 || optind == lastoptind) ? optind : 0;
	if (opt_info.index >= argc)
		return -1;
	switch (n = optget((char**)argv, optstring))
	{
	case ':':
		n = '?';
		/*FALLTHROUGH*/
	case '?':
		if (opterr && (!optstring || *optstring != ':'))
		{
			if (!error_info.id)
				error_info.id = argv[0];
			errormsg(NiL, 2, opt_info.arg);
		}
		optopt = opt_info.option[1];
		break;
	case 0:
		n = -1;
		break;
	}
	optarg = opt_info.arg;
	lastoptind = optind = opt_info.index;
	return n;
}

#endif
