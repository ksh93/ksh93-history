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
 * getopts  optstring name [arg...]
 *
 *   David Korn
 *   AT&T Labs
 *   research!dgk
 *
 */

#include	"defs.h"
#include	"variables.h"
#include	<error.h>
#include	<nval.h>
#include	"builtins.h"

int	b_getopts(int argc,char *argv[],void *extra)
{
	register char *options=error_info.context->id;
	register Namval_t *np;
	register int flag, mode, r=0;
	register Shell_t *shp = (Shell_t*)extra;
	char value[2], key[2];
	int jmpval;
	struct checkpt buff, *pp;
	value[1] = 0;
	key[1] = 0;
	while((flag = optget(argv,sh_optgetopts))) switch(flag)
	{
	    case 'a':
		options = opt_info.arg;
		break;
	    case ':':
		errormsg(SH_DICT,2, "%s", opt_info.arg);
		break;
	    case '?':
		errormsg(SH_DICT,ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	argv += opt_info.index;
	argc -= opt_info.index;
	if(error_info.errors || argc<2)
		errormsg(SH_DICT,ERROR_usage(2), "%s", optusage((char*)0));
	error_info.context->flags |= ERROR_SILENT;
	error_info.id = options;
	options = argv[0];
	np = nv_open(argv[1],shp->var_tree,NV_NOASSIGN|NV_VARNAME);
	if(argc>2)
	{
		argv +=1;
		argc -=1;
	}
	else
	{
		argv = shp->st.dolv;
		argc = shp->st.dolc;
	}
	opt_info.index = shp->st.optindex;
	opt_info.offset = shp->st.optchar;
	if(mode= (*options==':'))
		options++;
	sh_pushcontext(&buff,1);
	jmpval = sigsetjmp(buff.buff,0);
	if(jmpval)
	{
		sh_popcontext(&buff);
		pp = (struct checkpt*)shp->jmplist;
		pp->mode = SH_JMPERREXIT;
		sh_exit(2);
	}
	switch(opt_info.index<=argc?(opt_info.num= LONG_MIN,flag=optget(argv,options)):0)
	{
	    case '?':
		if(mode==0)
			errormsg(SH_DICT,ERROR_usage(2), "%s", opt_info.arg);
		opt_info.option[1] = '?';
		/* FALL THRU */
	    case ':':
		key[0] = opt_info.option[1];
		if(strmatch(opt_info.arg,"*unknown*"))
			flag = '?';
		if(mode)
			opt_info.arg = key;
		else
		{
			errormsg(SH_DICT,2, "%s", opt_info.arg);
			opt_info.arg = 0;
			flag = '?';
		}
		*(options = value) = flag;
		shp->st.opterror = 1;
		if (opt_info.offset != 0 && !argv[opt_info.index][opt_info.offset])
		{
			opt_info.offset = 0;
			opt_info.index++;
		}
		break;
	    case 0:
		if(shp->st.opterror)
		{
			char *com[2];
			com[0] = "-?";
			com[1] = 0;
			flag = opt_info.index;
			opt_info.index = 0;
			optget(com,options);
			opt_info.index = flag;
			if(!mode && strchr(options,' '))
				errormsg(SH_DICT,ERROR_usage(2), "%s", optusage((char*)0));
		}
		opt_info.arg = 0;
		options = value;
		*options = '?';
		r=1;
		opt_info.offset = 0;
		break;
	    default:
		options = opt_info.option + (*opt_info.option!='+');
	}
	error_info.context->flags &= ~ERROR_SILENT;
	shp->st.optindex = opt_info.index;
	shp->st.optchar = opt_info.offset;
	nv_putval(np, options, 0);
	nv_close(np);
	np = nv_open(nv_name(OPTARGNOD),shp->var_tree,NV_NOSCOPE);
	if(opt_info.num != LONG_MIN)
	{
		double d = opt_info.num;
		nv_putval(np, (char*)&d, NV_INTEGER|NV_RDONLY);
	}
	else
		nv_putval(np, opt_info.arg, NV_RDONLY);
	nv_close(np);
	sh_popcontext(&buff);
	return(r);
}

