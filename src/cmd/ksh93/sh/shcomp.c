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
 * David Korn
 * AT&T Labs
 *
 * shell script to shell binary converter
 *
 */

#include	<shell.h>
#include	"shnodes.h"
#include	"path.h"
#include	"io.h"

#define CNTL(x)	((x)&037)
#define VERSION	2
static const char id[] = "\n@(#)shcomp (AT&T Labs) 12/28/93\0\n";
static const char e_dict[] = "ksh";
static const char header[6] = { CNTL('k'),CNTL('s'),CNTL('h'),0,VERSION,0 };

main(int argc, char *argv[])
{
	Sfio_t *in, *out;
	union anynode *t;
	char *cp;
	int n, nflag=0, vflag=0, dflag=0;
	error_info.id = argv[0];
	while(n = optget(argv, "Dnv [infile [outfile]]" )) switch(n)
	{
	    case 'D':
		dflag=1;
		break;
	    case 'v':
		vflag=1;
		break;
	    case 'n':
		nflag=1;
		break;
	    case ':':
		errormsg(SH_DICT,2,"%s",opt_info.arg);
		break;
	    case '?':
		errormsg(SH_DICT,ERROR_usage(2),"%s",opt_info.arg);
		break;
	}
	sh_init(argc,argv,(void(*)(int))0);
	argv += opt_info.index;
	argc -= opt_info.index;
	if(error_info.errors || argc>2)
		errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	if(cp= *argv)
	{
		argv++;
		if((n=path_open(cp,path_get(cp))) < 0)
			n = path_open(cp,"");
		if(n < 0)
			errormsg(SH_DICT,ERROR_system(1),"%s: cannot open",cp);
		in = sh_iostream(n);
	}
	else
		in = sfstdin;
	if(cp= *argv)
	{
		if(!(out = sfopen((Sfio_t*)0,cp,"w")))
			errormsg(SH_DICT,ERROR_system(1),"%s: cannot create",cp);
	}
	else
		out = sfstdout;
	if(dflag)
		sh_onoption(SH_DICTIONARY|SH_NOEXEC);
	if(nflag)
		sh_onoption(SH_NOEXEC);
	if(vflag)
		sh_onoption(SH_VERBOSE);
	if(!dflag)
		sfwrite(out,header,sizeof(header));
	sh.inlineno = 1;
	while(1)
	{
		stakset((char*)0,0);
		if(t = (union anynode*)sh_parse(in,0))
		{
			if(!dflag && sh_tdump(out,t) < 0)
				errormsg(SH_DICT,ERROR_exit(1),"dump failed");
		}
		else if(sfeof(in))
			break;
		if(sferror(in))
			errormsg(SH_DICT,ERROR_system(1),"I/O error");
	}
	if(in!=sfstdin)
		sfclose(in);
	if(out!=sfstdout)
		sfclose(out);
	return(0);
}
