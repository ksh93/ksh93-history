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
 *  completion.c - command and file completion for shell editors
 *
 */

#include	"defs.h"
#include	<ctype.h>
#include	"lexstates.h"
#include	"path.h"
#include	"io.h"
#include	"edit.h"
#include	"history.h"

static int charcmp(int a, int b, int nocase)
{
	if(nocase)
	{
		if(isupper(a))
			a = tolower(a);
		if(isupper(b))
			b = tolower(b);
	}
	return(a==b);
}

/*
 *  overwrites <str> to common prefix of <str> and <newstr>
 *  if <str> is equal to <newstr> returns  <str>+strlen(<str>)+1
 *  otherwise returns <str>+strlen(<str>)
 */
static char *overlaid(register char *str,register const char *newstr,int nocase)
{
	register int c,d;
	while((c= *(unsigned char *)str) && ((d= *(unsigned char*)newstr++),charcmp(c,d,nocase)))
		str++;
	if(*str)
		*str = 0;
	else if(*newstr==0)
		str++;
	return(str);
}


/*
 * file name generation for edit modes
 * non-zero exit for error, <0 ring bell
 * don't search back past beginning of the buffer
 * mode is '*' for inline expansion,
 * mode is '\' for filename completion
 * mode is '=' cause files to be listed in select format
 */

ed_expand(Edit_t *ep, char outbuff[],int *cur,int *eol,int mode)
{
	int offset = staktell();
	char *staksav = stakptr(0);
	struct comnod  *comptr = (struct comnod*)stakalloc(sizeof(struct comnod));
	struct argnod *ap = (struct argnod*)stakseek(ARGVAL);
	register char *out;
	char *begin;
	char *dir = 0;
	int addstar;
	int rval = 0;
	int strip;
	int nomarkdirs = !sh_isoption(SH_MARKDIRS);
#ifdef SHOPT_MULTIBYTE
	{
		register int c = *cur;
		register genchar *cp;
		/* adjust cur */
		cp = (genchar *)outbuff + *cur;
		c = *cp;
		*cp = 0;
		*cur = ed_external((genchar*)outbuff,(char*)stakptr(0));
		*cp = c;
		*eol = ed_external((genchar*)outbuff,outbuff);
	}
#endif /* SHOPT_MULTIBYTE */
	out = outbuff + *cur;
	comptr->comtyp = COMSCAN;
	comptr->comarg = ap;
	ap->argflag = (ARG_MAC|ARG_EXP);
	ap->argnxt.ap = 0;
	{
		register int c;
		if(out>outbuff)
		{
			/* go to beginning of word */
			do
			{
				out--;
				c = *(unsigned char*)out;
			}
			while(out>outbuff && !ismeta(c));
			/* copy word into arg */
			if(ismeta(c))
				out++;
		}
		else
			out = outbuff;
		begin = out;
		/* addstar set to zero if * should not be added */
		addstar = '*';
		strip = 1;
		/* copy word to arg */
		do
		{
			c = *(unsigned char*)out;
			if(isexp(c))
				addstar = 0;
			if (c == '/')
			{
				if(addstar == 0)
					strip = 0;
				dir = out+1;
			}
			stakputc(c);
			out++;

		} while (c && !ismeta(c));
		if(!dir || *dir==0)
			dir = ".";

		out--;
		if(mode=='\\')
			addstar = '*';
		if(*begin=='~' && !strchr(begin,'/'))
			addstar = 0;
		*stakptr(staktell()-1) = addstar;
		stakfreeze(1);
	}
	if(mode!='*')
		sh_onoption(SH_MARKDIRS);
	{
		register char **com;
		char	*cp=begin;
		char	*left=0;
		int	 narg,cmd_completion=0;
		register int size;
		while(cp>outbuff && ((size=cp[-1])==' ' || size=='\t'))
			cp--;
		if((cp==outbuff || strchr(";&|(",size)) && *begin!='~' && !strchr(ap->argval,'/'))
		{
			cmd_completion=1;
			sh_onstate(SH_COMPLETE);
		}
		com = sh_argbuild(&narg,comptr);
		sh_offstate(SH_COMPLETE);
                /* allow a search to be aborted */
		if(sh.trapnote&SH_SIGSET)
		{
			rval = -1;
			goto done;
		}
		/*  match? */
		if (*com==0 || (narg <= 1 && strcmp(ap->argval,*com)==0))
		{
			rval = -1;
			goto done;
		}
		if(mode=='=')
		{
			if (strip && !cmd_completion)
			{
				register char **ptrcom;
				for(ptrcom=com;*ptrcom;ptrcom++)
					/* trim directory prefix */
					*ptrcom = path_basename(*ptrcom);
			}
			sfputc(sfstderr,'\n');
			sh_menu(sfstderr,narg,com);
			sfsync(sfstderr);
			goto done;
		}
		/* see if there is enough room */
		size = *eol - (out-begin);
		if(mode=='\\')
		{
			/* just expand until name is unique */
			size += strlen(*com);
		}
		else
		{
			size += narg;
			{
				char **savcom = com;
				while (*com)
					size += strlen(sh_fmtq(*com++));
				com = savcom;
			}
		}
		/* see if room for expansion */
		if(outbuff+size >= &outbuff[MAXLINE])
		{
			com[0] = ap->argval;
			com[1] = 0;
		}
		/* save remainder of the buffer */
		if(*out)
			left=stakcopy(out);
		if(cmd_completion && mode=='\\')
			out = strcopy(begin,path_basename(cp= *com++));
		else if(mode=='*')
			out = strcopy(begin,sh_fmtq(*com++));
		else
			out = strcopy(begin,*com++);
		if(mode=='\\')
		{
			char *saveout;
			int nocase;
			if(saveout=astconf("PATH_ATTRIBUTES",dir,(char*)0))
				nocase = (strchr(saveout,'c')!=0);
			if(addstar==0)
				*out++ = '/';
			saveout= ++out;
			while (*com && *begin)
			{
				if(cmd_completion)
					out = overlaid(begin,path_basename(*com++),nocase);
				else
					out = overlaid(begin,*com++,nocase);
			}
			mode = (out==saveout);
			if(out[-1]==0)
				out--;
			if(mode && out[-1]!='/')
			{
				if(cmd_completion)
				{
					Namval_t *np;
					/* add as tracked alias */
					if(*cp=='/' && (np=nv_search(begin,sh.track_tree,NV_ADD)))
						path_alias(np,cp);
					out = strcopy(begin,cp);
				}
				/* add quotes if necessary */
				if((cp=sh_fmtq(begin))!=begin)
					out = strcopy(begin,cp);
				*out++ = ' ';
				*out = 0;
			}
			else if(out[-1]=='/' && (cp=sh_fmtq(begin))!=begin)
				out = strcopy(begin,cp);
			if(*begin==0)
				ed_ringbell();
		}
		else
			while (*com)
			{
				*out++  = ' ';
				out = strcopy(out,sh_fmtq(*com++));
			}
		*cur = (out-outbuff);
		/* restore rest of buffer */
		if(left)
			out = strcopy(out,left);
		*eol = (out-outbuff);
	}
 done:
	stakset(staksav,offset);
	if(nomarkdirs)
		sh_offoption(SH_MARKDIRS);
#ifdef SHOPT_MULTIBYTE
	{
		register int c;
		/* first re-adjust cur */
		out = outbuff + *cur;
		c = *out;
		*out = 0;
		*cur = ed_internal(outbuff,(genchar*)stakptr(0));
		*out = c;
		outbuff[*eol+1] = 0;
		*eol = ed_internal(outbuff,(genchar*)outbuff);
	}
#endif /* SHOPT_MULTIBYTE */
	return(rval);
}

/*
 * look for edit macro named _i
 * if found, puts the macro definition into lookahead buffer and returns 1
 */
ed_macro(Edit_t *ep, register int i)
{
	register char *out;
	Namval_t *np;
	genchar buff[LOOKAHEAD+1];
	if(i != '@')
		ep->e_macro[1] = i;
	/* undocumented feature, macros of the form <ESC>[c evoke alias __c */
	if(i=='_')
		ep->e_macro[2] = ed_getchar(ep,1);
	else
		ep->e_macro[2] = 0;
	if (isalnum(i)&&(np=nv_search(ep->e_macro,sh.alias_tree,HASH_SCOPE))&&(out=nv_getval(np)))
	{
#ifdef SHOPT_MULTIBYTE
		/* copy to buff in internal representation */
		int c = out[LOOKAHEAD];
		out[LOOKAHEAD] = 0;
		i = ed_internal(out,buff);
		out[LOOKAHEAD] = c;
#else
		strncpy((char*)buff,out,LOOKAHEAD);
		i = strlen((char*)buff);
#endif /* SHOPT_MULTIBYTE */
		while(i-- > 0)
			ed_ungetchar(ep,buff[i]);
		return(1);
	} 
	return(0);
}

/*
 * Enter the fc command on the current history line
 */
ed_fulledit(Edit_t *ep)
{
	register char *cp;
	if(!sh.hist_ptr)
		return(-1);
	/* use EDITOR on current command */
	if(ep->e_hline == ep->e_hismax)
	{
		if(ep->e_eol<0)
			return(-1);
#ifdef SHOPT_MULTIBYTE
		ep->e_inbuf[ep->e_eol+1] = 0;
		ed_external(ep->e_inbuf, (char *)ep->e_inbuf);
#endif /* SHOPT_MULTIBYTE */
		sfwrite(sh.hist_ptr->histfp,(char*)ep->e_inbuf,ep->e_eol+1);
		sh_onstate(SH_HISTORY);
		hist_flush(sh.hist_ptr);
	}
	cp = strcopy((char*)ep->e_inbuf,e_runvi);
	cp = strcopy(cp, fmtbase((long)ep->e_hline,10,0));
	ep->e_eol = ((unsigned char*)cp - (unsigned char*)ep->e_inbuf)-1;
	return(0);
}
