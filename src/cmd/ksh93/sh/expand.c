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
#include	<ls.h>
#include	<stak.h>
#include	<ast_dir.h>
#include	"io.h"
#include	"path.h"


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


#define GLOB_RESCAN 1
#define	argstart(ap)	((ap)->argbegin)
#define globptr()	((struct glob*)membase)

static struct glob	 *membase;

static void		addmatch(const char*,const char*,const char*,char*);
static void		glob_dir(struct argnod*);

int path_expand(const char *pattern, struct argnod **arghead)
{
	register struct argnod *ap;
	register struct glob *gp;
	register char *pat;
#ifdef KSHELL
	struct glob globdata;
	membase = &globdata;
#endif /* KSHELL */
	gp = globptr();
	ap = (struct argnod*)stakalloc(strlen(pattern)+sizeof(struct argnod)+suflen);
	gp->rescan =  ap;
	gp->argn = 0;
#ifdef KSHELL
	gp->fignore = nv_getval(nv_scoped(FIGNORENOD));
	gp->match = *arghead;
#else
	gp->fignore = getenv("FIGNORE");
	gp->match = 0;
#endif /* KSHELL */
	ap->argbegin = ap->argval;
	ap->argchn.ap = 0;
#ifdef KSHELL
	pat = strcopy(ap->argval,pattern);
	if(suflen)
		strcpy(pat,sufstr);
	else
		*pat = 0;
#else
	strcpy(ap->argval,pat);
#endif /* KSHELL */
	suflen = 0;
	do
	{
		gp->rescan = ap->argchn.ap;
		glob_dir(ap);
	}
	while(ap = gp->rescan);
#ifdef KSHELL
	*arghead = gp->match;
#endif /* KSHELL */
	return(gp->argn);
}

static void glob_dir(struct argnod *ap)
{
	register char		*rescan;
	register char		*prefix;
	register char		*pat;
	register struct dirent	*dirp;
	DIR 			*dirf;
	char			*path=0;
	char			quote = 0;
	char			savequote = 0;
	char			meta = 0;
	char			bracket = 0;
	char			first;
	char			*dirname;
	char			*fignore = globptr()->fignore;
	struct stat		statb;
#ifdef SHOPT_NOCASE
	int			nocase = STR_ICASE;
#else
	int			nocase = 0;
#endif /* SHOPT_NOCASE */
	sh_sigcheck();
	pat = rescan = argstart(ap);
	prefix = dirname = ap->argval;
	first = (rescan == prefix);
	/* check for special chars */
	while(1) switch(*rescan++)
	{
		case 0:
			if(meta)
			{
				rescan = 0;
				goto process;
			}
			if(first && !quote || !SHOPT_BRACEPAT)
				return;
			if(quote)
				sh_trim(argstart(ap));
 			/* treat trailing / as trailing /. */
			if(*rescan==0 && (--rescan,rescan[-1]=='/'))
 				*rescan = '.';
			else
				rescan = 0;
			if((SHOPT_BRACEPAT&&quote) || lstat(prefix,&statb)>=0)
				addmatch((char*)0,prefix,(char*)0,rescan);
			return;

		case '/':
			if(meta)
				goto process;
			pat = rescan;
			bracket = 0;
			savequote = quote;
			break;

		case '[':
			bracket = 1;
			break;

		case ']':
			meta |= bracket;
			break;

		case '*':
		case '?':
		case '(':
			meta=1;
			break;

		case '\\':
			quote = 1;
			rescan++;
	}
process:
	if(pat == prefix)
	{
		dirname = ".";
		prefix = 0;
#ifdef KSHELL
		if(!rescan && sh_isstate(SH_COMPLETE))
		{
			/* command completion */
			scantree(sh.alias_tree,pat);
			scantree(sh.fun_tree,pat);
			path = path_get("");
		}
#endif /* KSHELL */
	}
	else
	{
		if(pat==prefix+1)
			dirname = "/";
		*(pat-1) = 0;
		if(savequote)
			sh_trim(argstart(ap));
	}
	while(1)
	{
#ifdef KSHELL
		if(path)
		{
			if(*path==':')
			{
				while(*path==':')
					path++;
				prefix=0;
				dirname = ".";
			}
			else
			{
				dirname = prefix = path;
				while(*path && *path!=':')
					path++;
				if(*path==':')
					*path++ = 0;
			}
		}
#endif /* KSHELL */
		if(dirf=opendir(dirname))
		{
			char *cp;
			if(cp=astconf("PATH_ATTRIBUTES",dirname,(char*)0))
				nocase = strchr(cp,'c') ? STR_ICASE : 0;
			/* check for rescan */
			if(rescan)
				*(rescan-1) = 0;
			while(dirp = readdir(dirf))
			{
				register int c;
				if(!D_FILENO(dirp))
					continue;
				if(fignore && *fignore && strgrpmatch(dirp->d_name, fignore, NIL(int*), 0, STR_MAXIMAL|STR_LEFT|STR_RIGHT|nocase))
					continue;
				if(*dirp->d_name=='.' && *pat != '.' && (!fignore ||
				    (c=dirp->d_name[1])==0 || 
				    (c=='.'  && dirp->d_name[2]==0)))
					continue;
				if(strgrpmatch(dirp->d_name, pat, NIL(int*), 0, STR_MAXIMAL|STR_LEFT|STR_RIGHT|nocase))
					addmatch(prefix,dirp->d_name,rescan,(char*)0);
			}
			closedir(dirf);
		}
		if(!path || *path==0)
			break;
		sh_sigcheck();
	}
	return;
}

static  void addmatch(const char *dir,const char *pat,const register char *rescan,char *endslash)
{
	register struct argnod *ap = (struct argnod*)stakseek(ARGVAL);
	register struct glob *gp = globptr();
	struct stat statb;
	if(dir)
	{
		stakputs(dir);
		stakputc('/');
	}
	if(endslash)
		*endslash = 0;
	stakputs(pat);
	if(rescan)
	{
		int offset;
		if(stat(stakptr(ARGVAL),&statb)<0 || !S_ISDIR(statb.st_mode))
			return;
		stakputc('/');
		offset = staktell();
 		/* if null, reserve room for . */
 		if(*rescan)
 			stakputs(rescan);
 		else
 			stakputc(0);
		stakputc(0);
		rescan = stakptr(offset);
		ap = (struct argnod*)stakfreeze(0);
		ap->argbegin = (char*)rescan;
		ap->argchn.ap = gp->rescan;
		gp->rescan = ap;
	}
	else
	{
#ifdef KSHELL
		if(!endslash && sh_isoption(SH_MARKDIRS) && stat(ap->argval,&statb)>=0)
		{
			if(sh_isstate(SH_COMPLETE) && (!S_ISREG(statb.st_mode) || !(statb.st_mode&(S_IXUSR|S_IXGRP|S_IXOTH))))
			{
				stakseek(0);
				return;
			}
			else if(S_ISDIR(statb.st_mode))
				stakputc('/');
		}
#endif /* KSHELL */
		ap = (struct argnod*)stakfreeze(1);
		ap->argchn.ap = gp->match;
		gp->match = ap;
		gp->argn++;
	}
#ifdef KSHELL
	ap->argflag = ARG_RAW;
	if(sh_isstate(SH_COMPLETE))
		ap->argflag |= ARG_MAKE;
#endif /* KSHELL */
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

#else

/*
 * remove backslashes
 */

static void sh_trim(sp)
register char *sp;
{
	register char *dp = sp;
	register int c;
	while(1)
	{
		if((c= *sp++) == '\\')
			c = *sp++;
		*dp++ = c;
		if(c==0)
			break;
	}
}
#endif /* KSHELL */

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
