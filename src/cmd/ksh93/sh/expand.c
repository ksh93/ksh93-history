/*
 * CDE - Common Desktop Environment
 *
 * Copyright (c) 1993-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these libraries and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
 */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF        *
*                    AT&T BELL LABORATORIES                    *
*         AND IS NOT TO BE DISCLOSED OR USED EXCEPT IN         *
*            ACCORDANCE WITH APPLICABLE AGREEMENTS             *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*              Unpublished & Not for Publication               *
*                     All Rights Reserved                      *
*                                                              *
*       The copyright notice above does not evidence any       *
*      actual or intended publication of such source code      *
*                                                              *
*               This software was created by the               *
*           Advanced Software Technology Department            *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                    {research,attmail}!dgk                    *
*                                                              *
***************************************************************/

/* : : generated by proto : : */

#if !defined(__PROTO__)
#if defined(__STDC__) || defined(__cplusplus) || defined(_proto) || defined(c_plusplus)
#if defined(__cplusplus)
#define __MANGLE__	"C"
#else
#define __MANGLE__
#endif
#define __STDARG__
#define __PROTO__(x)	x
#define __OTORP__(x)
#define __PARAM__(n,o)	n
#if !defined(__STDC__) && !defined(__cplusplus)
#if !defined(c_plusplus)
#define const
#endif
#define signed
#define void		int
#define volatile
#define __V_		char
#else
#define __V_		void
#endif
#else
#define __PROTO__(x)	()
#define __OTORP__(x)	x
#define __PARAM__(n,o)	o
#define __MANGLE__
#define __V_		char
#define const
#define signed
#define void		int
#define volatile
#endif
#if defined(__cplusplus) || defined(c_plusplus)
#define __VARARG__	...
#else
#define __VARARG__
#endif
#if defined(__STDARG__)
#define __VA_START__(p,a)	va_start(p,a)
#else
#define __VA_START__(p,a)	va_start(p)
#endif
#endif
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
    static void scantree __PROTO__((Hashtab_t*,const char*));
#else
#   define sh_sigcheck()	(0)
#   define sh_access		access
#   define suflen		0
#endif /* KSHELL */


/*
 * This routine builds a list of files that match a given pathname
 * Uses external routine strmatch() to match each component
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

static void		addmatch __PROTO__((const char*,const char*,const char*,char*));
static void		glob_dir __PROTO__((struct argnod*));


#ifdef SHOPT_NOCASE

#   include	<ctype.h>
    static void utol __PARAM__((char const *str1,char *str2), (str1, str2)) __OTORP__(char const *str1;char *str2;){
        int c;
        for(; c= *((unsigned char*)str1); str1++,str2++)
        {               
                if(isupper(c))  
                        *str2 = tolower(c);
                else            
                        *str2 = c;
        }
        *str2 = 0;
    }

    static int  lowermatch __PARAM__((const char *str, const char *pat), (str, pat)) __OTORP__(const char *str; const char *pat;){
	char buff[NAME_MAX+1];
	utol(str,buff);
	return(strmatch(buff,pat));
    }
#   define strmatch	lowermatch
#endif /* SHOPT_NOCASE */


int path_expand __PARAM__((const char *pattern, struct argnod **arghead), (pattern, arghead)) __OTORP__(const char *pattern; struct argnod **arghead;){
	struct argnod *ap;
	struct glob *gp;
	char *pat;
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

static void glob_dir __PARAM__((struct argnod *ap), (ap)) __OTORP__(struct argnod *ap;){
	char		*rescan;
	char		*prefix;
	char		*pat;
	struct dirent	*dirp;
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
			if(first)
				return;
			if(quote)
				sh_trim(argstart(ap));
 			/* treat trailing / as trailing /. */
			if(*rescan==0 && (--rescan,rescan[-1]=='/'))
 				*rescan = '.';
			else
				rescan = 0;
			if(lstat(prefix,&statb)>=0)
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
			/* check for rescan */
			if(rescan)
				*(rescan-1) = 0;
#ifdef SHOPT_NOCASE
			utol(pat,pat);
#endif /* SHOPT_NOCASE */
			while(dirp = readdir(dirf))
			{
				int c;
				if(!D_FILENO(dirp))
					continue;
				if(fignore && *fignore && strmatch(dirp->d_name, fignore))
					continue;
				if(*dirp->d_name=='.' && *pat != '.' && (!fignore ||
				    (c=dirp->d_name[1])==0 || 
				    (c=='.'  && dirp->d_name[2]==0)))
					continue;
				if(strmatch(dirp->d_name, pat))
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

static  void addmatch __PARAM__((const char *dir,const char *pat,const char *rescan,char *endslash), (dir, pat, rescan, endslash)) __OTORP__(const char *dir;const char *pat;const char *rescan;char *endslash;){
	struct argnod *ap = (struct argnod*)stakseek(ARGVAL);
	struct glob *gp = globptr();
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
static void scantree __PARAM__((Hashtab_t *tree, const char *pattern), (tree, pattern)) __OTORP__(Hashtab_t *tree; const char *pattern;){
	Hashpos_t *hp;
	Namval_t *np;
	struct argnod *ap;
	struct glob *gp = globptr();
	char *cp;
	if(hp = hashscan(tree,0))
	{
		while((np = (Namval_t*)hashnext(hp)) && !nv_isnull(np))
		{
			if(is_abuiltin(np) && np->nvenv && *np->nvenv=='/')
				continue;
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
}

/*
 * file name completion
 * generate the list of files found by adding an suffix to end of name
 * The number of matches is returned
 */

int path_complete __PARAM__((const char *name,const char *suffix, struct argnod **arghead), (name, suffix, arghead)) __OTORP__(const char *name;const char *suffix; struct argnod **arghead;){
	sufstr = suffix;
	suflen = strlen(suffix);
	return(path_expand(name,arghead));
}

#else

/*
 * remove backslashes
 */

static void sh_trim(sp)
char *sp;
{
	char *dp = sp;
	int c;
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
int path_generate __PARAM__((struct argnod *todo, struct argnod **arghead), (todo, arghead)) __OTORP__(struct argnod *todo; struct argnod **arghead;){
	char *cp;
	int brace;
	struct argnod *ap;
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
			if(brace==0 && comma)
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
