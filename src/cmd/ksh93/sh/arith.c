/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1982-2010 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Shell arithmetic - uses streval library
 *   David Korn
 *   AT&T Labs
 */

#include	"defs.h"
#include	"lexstates.h"
#include	"name.h"
#include	"streval.h"
#include	"variables.h"

#ifndef LLONG_MAX
#define LLONG_MAX	LONG_MAX
#endif

extern const Namdisc_t	ENUM_disc;
static Sfdouble_t	NaN, Inf, Fun;
static Namval_t Infnod =
{
	{ 0 },
	"Inf",
	NV_NOFREE|NV_LDOUBLE,NV_RDONLY
};

static Namval_t NaNnod =
{
	{ 0 },
	"NaN",
	NV_NOFREE|NV_LDOUBLE,NV_RDONLY
};

static Namval_t FunNode =
{
	{ 0 },
	"?",
	NV_NOFREE|NV_LDOUBLE,NV_RDONLY
};

static Namval_t *scope(register Namval_t *np,register struct lval *lvalue,int assign)
{
	register int flag = lvalue->flag;
	register char *sub=0, *cp=(char*)np;
	register Namval_t *mp;
	Shell_t		*shp = lvalue->shp;
	int	flags = HASH_NOSCOPE|HASH_SCOPE|HASH_BUCKET;
	int	nosub = lvalue->nosub;
	Dt_t	*sdict = (shp->st.real_fun? shp->st.real_fun->sdict:0);
	Dt_t	*root = shp->var_tree;
	assign = assign?NV_ASSIGN:NV_NOASSIGN;
	lvalue->nosub = 0;
	if(cp>=lvalue->expr &&  cp < lvalue->expr+lvalue->elen)
	{
		int offset;
		/* do binding to node now */
		int c = cp[flag];
		cp[flag] = 0;
		if((!(np = nv_open(cp,shp->var_tree,assign|NV_VARNAME|NV_NOADD|NV_NOFAIL)) || nv_isnull(np)) && sh_macfun(shp,cp, offset = staktell()))
		{
			Fun = sh_arith(shp,sub=stakptr(offset));
			FunNode.nvalue.ldp = &Fun;
			cp[flag] = c;
			return(&FunNode);
		}
		if(!np && assign)
			np = nv_open(cp,shp->var_tree,assign|NV_VARNAME);
		cp[flag] = c;
		if(!np)
			return(0);
		root = shp->last_root;
		if(cp[flag+1]=='[')
			flag++;
		else
			flag = 0;
		cp = (char*)np;
	}
	else if(assign==NV_ASSIGN  && nv_isnull(np) && !nv_isattr(np, ~(NV_MINIMAL|NV_NOFREE)))
		flags |= NV_ADD;
	if((lvalue->emode&ARITH_COMP) && dtvnext(root) && ((sdict && (mp=nv_search(cp,sdict,flags&~NV_ADD))) || (mp=nv_search(cp,root,flags&~NV_ADD))))
		np = mp;
	while(nv_isref(np))
	{
		sub = nv_refsub(np);
		np = nv_refnode(np);
		if(sub)
			nv_putsub(np,sub,assign==NV_ASSIGN?ARRAY_ADD:0);
	}
	if(!nosub && flag)
	{
		cp = 0;
		if(sub)
		{
			cp = (char*)&lvalue->expr[flag];
			goto skip;
		}
		else
			sub = (char*)&lvalue->expr[flag];
		while(1)
		{
			Namarr_t	*ap;
			Namval_t	*nq;
			cp = nv_endsubscript(np,sub,0);
			flag = *cp;
			*cp = 0;
			if(strchr(sub,'$'))
				sub = sh_mactrim(shp,sub,0);
			*cp = flag;
			nv_endsubscript(np,sub,NV_ADD|NV_SUBQUOTE);
			if(flag!='[')
				break;
		skip:
			if(nq = nv_opensub(np))
				np = nq;
			else
			{
				ap = nv_arrayptr(np);
				if(ap && !ap->table)
					ap->table = dtopen(&_Nvdisc,Dtoset);
				if(ap && ap->table && (nq=nv_search(nv_getsub(np),ap->table,NV_ADD)))
					nq->nvenv = (char*)np;
				if(nq && nv_isnull(nq))
					np = nv_arraychild(np,nq,0);
			}
			sub = cp;
		}
	}
	return(np);
}

static Sfdouble_t arith(const char **ptr, struct lval *lvalue, int type, Sfdouble_t n)
{
	Shell_t		*shp = lvalue->shp;
	register Sfdouble_t r= 0;
	char *str = (char*)*ptr;
	register char *cp;
	switch(type)
	{
	    case ASSIGN:
	    {
		register Namval_t *np = (Namval_t*)(lvalue->value);
		np = scope(np,lvalue,1);
		nv_putval(np, (char*)&n, NV_LDOUBLE);
		if(lvalue->eflag)
			lvalue->ptr = (void*)nv_hasdisc(np,&ENUM_disc);
		lvalue->eflag = 0;
		r=nv_getnum(np);
		lvalue->value = (char*)np;
		break;
	    }
	    case LOOKUP:
	    {
		register int c = *str;
		register char *xp=str;
		lvalue->value = (char*)0;
		if(c=='.')
			str++;
		c = mbchar(str);
		if(isaletter(c))
		{
			register Namval_t *np;
			int dot=0;
			while(1)
			{
				while(xp=str, c=mbchar(str), isaname(c));
				str = xp;
				while(c=='[' && dot==NV_NOADD)
				{
					str = nv_endsubscript((Namval_t*)0,str,0);
					c = *str;
				}
				if(c!='.')
					break;
				dot=NV_NOADD;
				if((c = *++str) !='[')
					continue;
				str = nv_endsubscript((Namval_t*)0,cp=str,NV_SUBQUOTE)-1;
				if(sh_checkid(cp+1,(char*)0))
					str -=2;
			}
			if(c=='(')
			{
				int off=stktell(shp->stk);
				int fsize = str- (char*)(*ptr);
				const struct mathtab *tp;
				Namval_t	*np;
				c = **ptr;
				lvalue->fun = 0;
				sfprintf(shp->stk,".sh.math.%.*s%c",fsize,*ptr,0);
				stkseek(shp->stk,off);
				if(np=nv_search(stkptr(shp->stk,off),shp->fun_tree,0))
				{
						lvalue->nargs = -np->nvalue.rp->argc;
						lvalue->fun = (Sfdouble_t(*)(Sfdouble_t,...))np;
						break;
				}
				if(fsize<=(sizeof(tp->fname)-2)) for(tp=shtab_math; *tp->fname; tp++)
				{
					if(*tp->fname > c)
						break;
					if(tp->fname[1]==c && tp->fname[fsize+1]==0 && strncmp(&tp->fname[1],*ptr,fsize)==0)
					{
						lvalue->fun = tp->fnptr;
						lvalue->nargs = *tp->fname;
						break;
					}
				}
				if(lvalue->fun)
					break;
				lvalue->value = (char*)ERROR_dictionary(e_function);
				return(r);
			}
			if((lvalue->emode&ARITH_COMP) && dot)
			{
				lvalue->value = (char*)*ptr;
				lvalue->flag =  str-lvalue->value;
				break;
			}
			*str = 0;
			if(sh_isoption(SH_NOEXEC))
				np = L_ARGNOD;
			else
			{
				int offset = staktell();
				char *saveptr = stakfreeze(0);
				Dt_t  *root = (lvalue->emode&ARITH_COMP)?shp->var_base:shp->var_tree;
				*str = c;
				cp = str;
				while(c=='[' || c=='.')
				{
					if(c=='[')
					{
						str = nv_endsubscript(np,str,0);
						if((c= *str)!='[' &&  c!='.')
						{
							str = cp;
							c = '[';
							break;
						}
					}
					else
					{
						dot = NV_NOADD|NV_NOFAIL;
						str++;
						while(xp=str, c=mbchar(str), isaname(c));
						str = xp;
					}
				}
				*str = 0;
				cp = (char*)*ptr;
				if ((cp[0] == 'i' || cp[0] == 'I') && (cp[1] == 'n' || cp[1] == 'N') && (cp[2] == 'f' || cp[2] == 'F') && cp[3] == 0)
				{
					Inf = strtold("Inf", NiL);
					Infnod.nvalue.ldp = &Inf;
					np = &Infnod;
				}
				else if ((cp[0] == 'n' || cp[0] == 'N') && (cp[1] == 'a' || cp[1] == 'A') && (cp[2] == 'n' || cp[2] == 'N') && cp[3] == 0)
				{
					NaN = strtold("NaN", NiL);
					NaNnod.nvalue.ldp = &NaN;
					np = &NaNnod;
				}
				else if(!(np = nv_open(*ptr,root,NV_NOREF|NV_NOASSIGN|NV_VARNAME|dot)))
				{
					lvalue->value = (char*)*ptr;
					lvalue->flag =  str-lvalue->value;
				}
				if(saveptr != stakptr(0))
					stakset(saveptr,offset);
				else
					stakseek(offset);
			}
			*str = c;
			if(!np && lvalue->value)
				break;
			lvalue->value = (char*)np;
			/* bind subscript later */
			if(nv_isattr(np,NV_DOUBLE)==NV_DOUBLE)
				lvalue->isfloat=1;
			lvalue->flag = 0;
			if(c=='[')
			{
				lvalue->flag = (str-lvalue->expr);
				do
					str = nv_endsubscript(np,str,0);
				while((c= *str)=='[');
				break;
			}
		}
		else
		{
			char	lastbase=0, *val = xp, oerrno = errno;
			lvalue->eflag = 0;
			errno = 0;
			r = strtonll(val,&str, &lastbase,-1);
			if(*str=='8' || *str=='9')
			{
				lastbase=10;
				errno = 0;
				r = strtonll(val,&str, &lastbase,-1);
			}
			if(lastbase<=1)
				lastbase=10;
			if(*val=='0')
			{
				while(*val=='0')
					val++;
				if(*val==0 || *val=='.' || *val=='x' || *val=='X')
					val--;
			}
			if(r==LLONG_MAX && errno)
				c='e';
			else
				c = *str;
			if(c==GETDECIMAL(0) || c=='e' || c == 'E' || lastbase ==
 16 && (c == 'p' || c == 'P'))
			{
				lvalue->isfloat=1;
				r = strtold(val,&str);
			}
			else if(lastbase==10 && val[1])
			{
				if(val[2]=='#')
					val += 3;
				if((str-val)>2*sizeof(Sflong_t))
				{
					Sfdouble_t rr;
					rr = strtold(val,&str);
					if(rr!=r)
					{
						r = rr;
						lvalue->isfloat=1;
					}
				}
			}
			errno = oerrno;
		}
		break;
	    }
	    case VALUE:
	    {
		register Namval_t *np = (Namval_t*)(lvalue->value);
		if(sh_isoption(SH_NOEXEC))
			return(0);
		np = scope(np,lvalue,0);
		if(!np)
		{
			if(sh_isoption(SH_NOUNSET))
			{
				*ptr = lvalue->value;
				goto skip;
			}
			return(0);
		}
		if(lvalue->eflag)
			lvalue->ptr = (void*)nv_hasdisc(np,&ENUM_disc);
		else if((Namfun_t*)lvalue->ptr && !nv_hasdisc(np,&ENUM_disc) && !nv_isattr(np,NV_INTEGER))
		{
			Namval_t *mp,node;
			mp = ((Namfun_t*)lvalue->ptr)->type;
			memset(&node,0,sizeof(node));
			nv_clone(mp,&node,0);
			nv_offattr(&node,NV_RDONLY|NV_NOFREE);
			nv_putval(&node,np->nvname,0);
			if(nv_isattr(&node,NV_NOFREE))
				return(r=nv_getnum(&node));
		}
		lvalue->eflag = 0;
		if(((lvalue->emode&2) || lvalue->level>1 || sh_isoption(SH_NOUNSET)) && nv_isnull(np) && !nv_isattr(np,NV_INTEGER))
		{
			*ptr = nv_name(np);
		skip:
			lvalue->value = (char*)ERROR_dictionary(e_notset);
			lvalue->emode |= 010;
			return(0);
		}
		r = nv_getnum(np);
		if(nv_isattr(np,NV_INTEGER|NV_BINARY)==(NV_INTEGER|NV_BINARY))
			lvalue->isfloat= (r!=(Sflong_t)r);
		else if(nv_isattr(np,NV_DOUBLE)==NV_DOUBLE)
			lvalue->isfloat=1;
		return(r);
	    }

	    case MESSAGE:
		sfsync(NIL(Sfio_t*));
#if 0
		if(warn)
			errormsg(SH_DICT,ERROR_warn(0),lvalue->value,*ptr);
		else
#endif
			errormsg(SH_DICT,ERROR_exit((lvalue->emode&3)!=0),lvalue->value,*ptr);
	}
	*ptr = str;
	return(r);
}

/*
 * convert number defined by string to a Sfdouble_t
 * ptr is set to the last character processed
 * if mode>0, an error will be fatal with value <mode>
 */

Sfdouble_t sh_strnum(register const char *str, char** ptr, int mode)
{
	Shell_t	*shp = sh_getinterp();
	register Sfdouble_t d;
	char base=0, *last;
	if(*str==0)
	{
		if(ptr)
			*ptr = (char*)str;
		return(0);
	}
	errno = 0;
	d = strtonll(str,&last,&base,-1);
	if(*last || errno)
	{
		if(!last || *last!='.' || last[1]!='.')
			d = strval(shp,str,&last,arith,mode);
		if(!ptr && *last && mode>0)
			errormsg(SH_DICT,ERROR_exit(1),e_lexbadchar,*last,str);
	}
	else if (!d && *str=='-')
		d = -0.0;
	if(ptr)
		*ptr = last;
	return(d);
}

Sfdouble_t sh_arith(Shell_t *shp,register const char *str)
{
	return(sh_strnum(str, (char**)0, 1));
}

void	*sh_arithcomp(Shell_t *shp,register char *str)
{
	const char *ptr = str;
	Arith_t *ep;
	ep = arith_compile(shp,str,(char**)&ptr,arith,ARITH_COMP|1);
	if(*ptr)
		errormsg(SH_DICT,ERROR_exit(1),e_lexbadchar,*ptr,str);
	return((void*)ep);
}
