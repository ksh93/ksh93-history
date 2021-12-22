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
#include	"defs.h"
#include	"shnodes.h"
#include	"path.h"
#include	"io.h"

static int p_comlist __PROTO__((const struct dolnod*));
static int p_arg __PROTO__((const struct argnod*));
static int p_comarg __PROTO__((const struct comnod*));
static int p_redirect __PROTO__((const struct ionod*));
static int p_switch __PROTO__((const struct regnod*));
static int p_tree __PROTO__((const union anynode*));
static int p_string __PROTO__((const char*));

static Sfio_t *outfile;

int sh_tdump __PARAM__((Sfio_t *out, const union anynode *t), (out, t)) __OTORP__(Sfio_t *out; const union anynode *t;){
	outfile = out;
	return(p_tree(t));
}
/*
 * print script corresponding to shell tree <t>
 */
static int p_tree __PARAM__((const union anynode *t), (t)) __OTORP__(const union anynode *t;){
	if(!t)
		return(sfputl(outfile,-1));
	if(sfputl(outfile,t->tre.tretyp)<0)
		return(-1);
	switch(t->tre.tretyp&COMMSK)
	{
		case TTIME:
		case TPAR:
			return(p_tree(t->par.partre)); 
		case TCOM:
			return(p_comarg((struct comnod*)t));
		case TSETIO:
		case TFORK:
			if(sfputu(outfile,t->fork.forkline)<0)
				return(-1);
			if(p_tree(t->fork.forktre)<0)
				return(-1);
			return(p_redirect(t->fork.forkio));
		case TIF:
			if(p_tree(t->if_.iftre)<0)
				return(-1);
			if(p_tree(t->if_.thtre)<0)
				return(-1);
			return(p_tree(t->if_.eltre));
		case TWH:
			if(t->wh.whinc)
			{
				if(p_tree((union anynode*)(t->wh.whinc))<0)
					return(-1);
			}
			else
			{
				if(sfputl(outfile,-1)<0)
					return(-1);
			}
			if(p_tree(t->wh.whtre)<0)
				return(-1);
			return(p_tree(t->wh.dotre));
		case TLST:
		case TAND:
		case TORF:
		case TFIL:
			if(p_tree(t->lst.lstlef)<0)
				return(-1);
			return(p_tree(t->lst.lstrit));
		case TARITH:
			if(sfputu(outfile,t->ar.arline)<0)
				return(-1);
			return(p_arg(t->ar.arexpr));
		case TFOR:
			if(p_tree(t->for_.fortre)<0)
				return(-1);
			if(p_string(t->for_.fornam)<0)
				return(-1);
			return(p_tree((union anynode*)t->for_.forlst));
		case TSW:
			if(p_arg(t->sw.swarg)<0)
				return(-1);
			return(p_switch(t->sw.swlst));
		case TFUN:
			if(sfputu(outfile,t->funct.functline)<0)
				return(-1);
			if(p_string(t->funct.functnam)<0)
				return(-1);
			if(p_tree(t->funct.functtre)<0)
				return(-1);
			return(p_tree((union anynode*)t->funct.functargs));
		case TTST:
			if(sfputu(outfile,t->tst.tstline)<0)
				return(-1);
			if((t->tre.tretyp&TPAREN)==TPAREN)
				return(p_tree(t->lst.lstlef)); 
			else
			{
				if(p_arg(&(t->lst.lstlef->arg))<0)
					return(-1);
				if((t->tre.tretyp&TBINARY))
					return(p_arg(&(t->lst.lstrit->arg)));
			}
	}
}

static int p_arg __PARAM__((const struct argnod *arg), (arg)) __OTORP__(const struct argnod *arg;){
	int n;
	struct fornod *fp;
	while(arg)
	{
		if((n = strlen(arg->argval)) || arg->argflag)
			fp=0;
		else
		{
			fp=(struct fornod*)arg->argchn.ap;
			n = strlen(fp->fornam)+1;
		}
		sfputu(outfile,n+1);
		if(fp)
			sfputc(outfile,0);
		sfwrite(outfile,fp?fp->fornam:arg->argval,n);
		sfputc(outfile,arg->argflag);
		if(fp)
			p_tree(fp->fortre);
		arg = arg->argnxt.ap;
	}
	return(sfputu(outfile,0));
}

static int p_redirect __PARAM__((const struct ionod *iop), (iop)) __OTORP__(const struct ionod *iop;){
	while(iop)
	{
		sfputl(outfile,iop->iofile);
		p_string(iop->ioname);
		if(iop->iodelim)
		{
			p_string(iop->iodelim);
			sfputl(outfile,iop->iosize);
			sfseek(sh.heredocs,iop->iooffset,SEEK_SET);
			sfmove(sh.heredocs,outfile, iop->iosize,-1);
		}
		else
			sfputu(outfile,0);
		iop = iop->ionxt;
	}
	return(sfputl(outfile,-1));
}

static int p_comarg __PARAM__((const struct comnod *com), (com)) __OTORP__(const struct comnod *com;){
	p_redirect(com->comio);
	p_arg(com->comset);
	if(!com->comarg)
		sfputl(outfile,-1);
	else if(com->comtyp&COMSCAN)
		p_arg(com->comarg);
	else
		p_comlist((struct dolnod*)com->comarg);
	return(sfputu(outfile,com->comline));
}

static int p_comlist __PARAM__((const struct dolnod *dol), (dol)) __OTORP__(const struct dolnod *dol;){
	char *cp, *const*argv;
	int n;
	argv = dol->dolval+ARG_SPARE;
	while(cp = *argv)
		argv++;
	n = argv - (dol->dolval+1);
	sfputl(outfile,n);
	argv = dol->dolval+ARG_SPARE;
	while(cp  = *argv++)
		p_string(cp);
	return(sfputu(outfile,0));
}

static int p_switch __PARAM__((const struct regnod *reg), (reg)) __OTORP__(const struct regnod *reg;){
	while(reg)
	{
		sfputl(outfile,reg->regflag);
		p_arg(reg->regptr);
		p_tree(reg->regcom);
		reg = reg->regnxt;
	}
	return(sfputl(outfile,-1));
}

static int p_string __PARAM__((const char *string), (string)) __OTORP__(const char *string;){
	size_t n=strlen(string);
	if(sfputu(outfile,n+1)<0)
		return(-1);
	return(sfwrite(outfile,string,n));
}
