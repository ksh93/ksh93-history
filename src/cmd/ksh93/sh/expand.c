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
 *	File name expansion
 *
 *	David Korn
 *	AT&T Labs
 *
 */

#ifdef KSHELL
#   include	"defs.h"
#   include	"variables.h"
#   include	"test.h"
#else
#   include	<ast.h>
#   include	<setjmp.h>
#endif /* KSHELL */
#include	<glob.h>
#include	<ls.h>
#include	<stak.h>
#include	<ast_dir.h>
#include	"io.h"
#include	"path.h"

#ifndef SHOPT_BRACEPAT
#   define SHOPT_BRACEPAT	0
#endif

#ifdef KSHELL
#   define argbegin	argnxt.cp
    static	const char	*sufstr;
    static	int		suflen;
    static void scantree(Dt_t*,const char*);
#else
#   define sh_sigcheck()	(0)
#   define sh_access		access
#   define suflen		0
#endif /* KSHELL */


/*
 * This routine builds a list of files that match a given pathname
 * Uses external routine strgrpmatch() to match each component
 * A leading . must match explicitly
 *
 */

struct glob
{
	int		argn;
	char		**argv;
	int		flags;
	struct argnod	*rescan;
	struct argnod	*match;	
	DIR		*dirf;
	char		*fignore;
#ifndef KSHELL
	char		*memlast;
	char		*last;
	struct argnod	*resume;
	sigjmp_buf	jmpbuf;
	char		begin[1];
#endif
};

#ifndef GLOB_AUGMENTED
#   define GLOB_AUGMENTED	0
#endif

#define GLOB_RESCAN 1
#define globptr()	((struct glob*)membase)

static struct glob	 *membase;

int path_expand(const char *pattern, struct argnod **arghead)
{
	glob_t gdata;
	register struct argnod *ap;
	register glob_t *gp= &gdata;
	register int flags;
	memset(gp,0,sizeof(gdata));
	flags = GLOB_AUGMENTED|GLOB_NOCHECK|GLOB_NOSORT|GLOB_STACK|GLOB_LIST|GLOB_DISC;
	if(sh_isoption(SH_MARKDIRS))
		flags |= GLOB_MARK;
	if(sh_isstate(SH_COMPLETE))
		flags |= GLOB_COMPLETE;
	gp->gl_fignore = nv_getval(nv_scoped(FIGNORENOD));
	if(suflen)
		gp->gl_suffix = sufstr;
	gp->gl_intr = &sh.trapnote; 
	suflen = 0;
	glob(pattern, flags, 0, gp);
	sh_sigcheck();
	for(ap= (struct argnod*)gp->gl_list; ap; ap = ap->argnxt.ap)
	{
		ap->argchn.ap = *arghead;
		*arghead = ap;
	}
	return(gp->gl_pathc);
}


#ifdef KSHELL

/*
 * scan tree and add each name that matches the given pattern
 */
static void scantree(Dt_t *tree, const char *pattern)
{
	register Namval_t *np;
	register struct argnod *ap;
	register struct glob *gp = globptr();
	register char *cp;
	np = (Namval_t*)dtfirst(tree);
	for(;np && !nv_isnull(np);(np = (Namval_t*)dtnext(tree,np)))
	{
		if(strmatch(cp=nv_name(np),pattern))
		{
			ap = (struct argnod*)stakseek(ARGVAL);
			stakputs(cp);
			ap = (struct argnod*)stakfreeze(1);
			ap->argbegin = NIL(char*);
			ap->argchn.ap = gp->match;
			ap->argflag = ARG_RAW|ARG_MAKE;
			gp->match = ap;
			gp->argn++;
		}
	}
}

/*
 * file name completion
 * generate the list of files found by adding an suffix to end of name
 * The number of matches is returned
 */

int path_complete(const char *name,register const char *suffix, struct argnod **arghead)
{
	sufstr = suffix;
	suflen = strlen(suffix);
	return(path_expand(name,arghead));
}

#endif

#ifdef SHOPT_BRACEPAT
int path_generate(struct argnod *todo, struct argnod **arghead)
/*@
	assume todo!=0;
	return count satisfying count>=1;
@*/
{
	register char *cp;
	register int brace;
	register struct argnod *ap;
	struct argnod *top = 0;
	struct argnod *apin;
	char *pat, *rescan, *bracep;
	char *sp;
	char comma;
	int count = 0;
	todo->argchn.ap = 0;
again:
	apin = ap = todo;
	todo = ap->argchn.ap;
	cp = ap->argval;
	comma = brace = 0;
	/* first search for {...,...} */
	while(1) switch(*cp++)
	{
		case '{':
			if(brace++==0)
				pat = cp;
			break;
		case '}':
			if(--brace>0)
				break;
			if(brace==0 && comma && *cp!='(')
				goto endloop1;
			comma = brace = 0;
			break;
		case ',':
			if(brace==1)
				comma = 1;
			break;
		case '\\':
			cp++;
			break;
		case 0:
			/* insert on stack */
			ap->argchn.ap = top;
			top = ap;
			if(todo)
				goto again;
			for(; ap; ap=apin)
			{
				apin = ap->argchn.ap;
				if((brace = path_expand(ap->argval,arghead)))
					count += brace;
				else
				{
					ap->argchn.ap = *arghead;
					*arghead = ap;
					count++;
				}
				(*arghead)->argflag |= ARG_MAKE;
			}
			return(count);
	}
endloop1:
	rescan = cp;
	bracep = cp = pat-1;
	*cp = 0;
	while(1)
	{
		brace = 0;
		/* generate each pattern and put on the todo list */
		while(1) switch(*++cp)
		{
			case '\\':
				cp++;
				break;
			case '{':
				brace++;
				break;
			case ',':
				if(brace==0)
					goto endloop2;
				break;
			case '}':
				if(--brace<0)
					goto endloop2;
		}
	endloop2:
		/* check for match of '{' */
		brace = *cp;
		*cp = 0;
		if(brace == '}')
		{
			apin->argchn.ap = todo;
			todo = apin;
			sp = strcopy(bracep,pat);
			sp = strcopy(sp,rescan);
			break;
		}
		ap = (struct argnod*)stakseek(ARGVAL);
		ap->argflag = ARG_RAW;
		ap->argchn.ap = todo;
		stakputs(apin->argval);
		stakputs(pat);
		stakputs(rescan);
		todo = ap = (struct argnod*)stakfreeze(1);
		pat = cp+1;
	}
	goto again;
}
#endif /* SHOPT_BRACEPAT */
