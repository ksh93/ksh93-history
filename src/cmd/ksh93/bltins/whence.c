/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1982-2001 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*******************************************************************/
#pragma prototyped
/*
 * command [-pvV] name [arg...]
 * whence [-afvp] name...
 *
 *   David Korn
 *   AT&T Labs
 *   research!dgk
 *
 */

#include	"defs.h"
#include	<error.h>
#include	"shtable.h"
#include	"name.h"
#include	"path.h"
#include	"shlex.h"
#include	"builtins.h"

#define P_FLAG	1
#define V_FLAG	2
#define A_FLAG	4
#define F_FLAG	010
#define X_FLAG	020

static int whence(Shell_t *,char**, int);

/*
 * command is called with argc==0 when checking for -V or -v option
 * In this case return 0 when -v or -V or unknown option, otherwise
 *   the shift count to the command is returned
 */
int	b_command(register int argc,char *argv[],void *extra)
{
	register int n, flags=0;
	register Shell_t *shp = (Shell_t*)extra;
	opt_info.index = opt_info.offset = 0;
	while((n = optget(argv,sh_optcommand))) switch(n)
	{
	    case 'p':
		sh_onstate(SH_DEFPATH);
		break;
	    case 'v':
		flags |= X_FLAG;
		break;
	    case 'V':
		flags |= V_FLAG;
		break;
	    case ':':
		if(argc==0)
			return(0);
		errormsg(SH_DICT,2, opt_info.arg);
		break;
	    case '?':
		if(argc==0)
			return(0);
		errormsg(SH_DICT,ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	if(argc==0)
		return(flags?0:opt_info.index);
	argv += opt_info.index;
	if(error_info.errors || !*argv)
		errormsg(SH_DICT,ERROR_usage(2),"%s", optusage((char*)0));
	return(whence(shp,argv, flags));
}

/*
 *  for the whence command
 */
int	b_whence(int argc,char *argv[],void *extra)
{
	register int flags=0, n;
	register Shell_t *shp = (Shell_t*)extra;
	NOT_USED(argc);
	if(*argv[0]=='t')
		flags = V_FLAG;
	while((n = optget(argv,sh_optwhence))) switch(n)
	{
	    case 'a':
		flags |= A_FLAG;
		/* FALL THRU */
	    case 'v':
		flags |= V_FLAG;
		break;
	    case 'f':
		flags |= F_FLAG;
		break;
	    case 'p':
		flags |= P_FLAG;
		break;
	    case ':':
		errormsg(SH_DICT,2, opt_info.arg);
		break;
	    case '?':
		errormsg(SH_DICT,ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	argv += opt_info.index;
	if(error_info.errors || !*argv)
		errormsg(SH_DICT,ERROR_usage(2),optusage((char*)0));
	return(whence(shp, argv, flags));
}

static int whence(Shell_t *shp,char **argv, register int flags)
{
	register const char *name;
	register Namval_t *np;
	register const char *cp;
	register int aflag,r=0;
	register const char *msg;
	int notrack = 1;
	while(name= *argv++)
	{
		aflag = ((flags&A_FLAG)!=0);
		cp = 0;
		np = 0;
		if(flags&P_FLAG)
			goto search;
		/* reserved words first */
		if(sh_lookup(name,shtab_reserved))
		{
			sfprintf(sfstdout,"%s%s\n",name,(flags&V_FLAG)?sh_translate(is_reserved):"");
			if(!aflag)
				continue;
			aflag++;
		}
		/* non-tracked aliases */
		if((np=nv_search(name,shp->alias_tree,0))
			&& !nv_isnull(np) && !(notrack=nv_isattr(np,NV_TAGGED))
			&& (cp=nv_getval(np))) 
		{
			if(flags&V_FLAG)
			{
				if(nv_isattr(np,NV_EXPORT))
					msg = sh_translate(is_xalias);
				else
					msg = sh_translate(is_alias);
				sfprintf(sfstdout,msg,name);
			}
			sfputr(sfstdout,sh_fmtq(cp),'\n');
			if(!aflag)
				continue;
			cp = 0;
			aflag++;
		}
		/* built-ins and functions next */
		if((np=nv_search(name,shp->fun_tree,0)) && nv_isattr(np,NV_FUNCTION|NV_BLTIN))
		{
			if((flags&F_FLAG) && nv_isattr(np,NV_FUNCTION))
				if(!(np=nv_search(name,shp->bltin_tree,0)) || nv_isnull(np))
					goto search;
			cp = "";
			if(flags&V_FLAG)
			{
				if(nv_isnull(np))
				{
					if(!nv_isattr(np,NV_FUNCTION))
						goto search;
					cp = sh_translate(is_ufunction);
				}
				else if(is_abuiltin(np))
					cp = sh_translate(is_builtin);
				else if(nv_isattr(np,NV_EXPORT))
					cp = sh_translate(is_xfunction);
				else
					cp = sh_translate(is_function);
			}
			sfprintf(sfstdout,"%s%s\n",name,cp);
			if(!aflag)
				continue;
			cp = 0;
			aflag++;
		}
	search:
		if(sh_isstate(SH_DEFPATH))
		{
			cp=0;
			notrack=1;
		}
		if(path_search(name,cp,2))
			cp = name;
		else
			cp = shp->lastpath;
		shp->lastpath = 0;
		if(cp)
		{
			if(flags&V_FLAG)
			{
				if(*cp!= '/')
				{
					if(!np || nv_isnull(np))
						sfprintf(sfstdout,"%s%s\n",name,sh_translate(is_ufunction));
					continue;
				}
				sfputr(sfstdout,sh_fmtq(name),' ');
				/* built-in version of program */
				if(*cp=='/' && (np=nv_search(cp,shp->bltin_tree,0)))
					msg = sh_translate(is_builtver);
				/* tracked aliases next */
				else if(!notrack && *name == '/')
					msg = sh_translate("is");
				else
					msg = sh_translate(is_talias);
				sfputr(sfstdout,msg,' ');
			}
			sfputr(sfstdout,sh_fmtq(cp),'\n');
		}
		else if(aflag<=1) 
		{
			r |= 1;
			if(flags&V_FLAG)
			{
				sfprintf(sfstdout,sh_translate(e_found),sh_fmtq(name));
				sfputc(sfstdout,'\n');
			}
		}
	}
	return(r);
}

