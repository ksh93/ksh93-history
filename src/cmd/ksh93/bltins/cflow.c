/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1982-2000 AT&T Corp.              *
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
*              David Korn <dgk@research.att.com>               *
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * break [n]
 * continue [n]
 * return [n]
 * exit [n]
 *
 *   David Korn
 *   AT&T Labs
 *   dgk@research.att.com
 *
 */

#include	"defs.h"
#include	<ast.h>
#include	<error.h>
#include	<ctype.h>
#include	"shnodes.h"
#include	"builtins.h"

/*
 * return and exit
 */
int	b_ret_exit(register int n, register char *argv[],void *extra)
{
	register char *arg;
	register Shell_t *shp = (Shell_t*)extra;
	struct checkpt *pp = (struct checkpt*)shp->jmplist;
	const char *options = (**argv=='r'?sh_optreturn:sh_optexit);
	while((n = optget(argv,options))) switch(n)
	{
	    case ':':
		if(!strmatch(argv[opt_info.index],"[+-]+([0-9])"))
			errormsg(SH_DICT,2, opt_info.arg);
		goto done;
	    case '?':
		errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
		return(2);
	}
done:
	if(error_info.errors)
		errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	pp->mode = (**argv=='e'?SH_JMPEXIT:SH_JMPFUN);
	argv += opt_info.index;
	n = (((arg= *argv)?(int)strtol(arg, (char**)0, 10):shp->oldexit)&SH_EXITMASK);
	/* return outside of function, dotscript and profile is exit */
	if(shp->fn_depth==0 && shp->dot_depth==0 && !sh_isstate(SH_PROFILE))
		pp->mode = SH_JMPEXIT;
	sh_exit(n);
	return(1);
}


/*
 * break and continue
 */
int	b_brk_cont(register int n, register char *argv[],void *extra)
{
	char *arg;
	register int cont= **argv=='c';
	register Shell_t *shp = (Shell_t*)extra;
	while((n = optget(argv,cont?sh_optcont:sh_optbreak))) switch(n)
	{
	    case ':':
		errormsg(SH_DICT,2, opt_info.arg);
		break;
	    case '?':
		errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
		return(2);
	}
	if(error_info.errors)
		errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	argv += opt_info.index;
	n=1;
	if(arg= *argv)
	{
		n = strtol(arg,&arg,10);
		if(n<=0 || *arg)
			errormsg(SH_DICT,ERROR_exit(1),e_nolabels,*argv);
	}
	if(shp->st.loopcnt)
	{
		shp->st.execbrk = shp->st.breakcnt = n;
		if(shp->st.breakcnt > shp->st.loopcnt)
			shp->st.breakcnt = shp->st.loopcnt;
		if(cont)
			shp->st.breakcnt = -shp->st.breakcnt;
	}
	return(0);
}

