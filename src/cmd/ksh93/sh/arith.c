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
 * Shell arithmetic - uses streval library
 *   David Korn
 *   AT&T Labs
 *   Room 2B-102
 *   Murray Hill, N. J. 07974
 *   Tel. x7975
 *   research!dgk
 */

#include	"defs.h"
#include	<ctype.h>
#include	"lexstates.h"
#include	"name.h"
#include	"streval.h"
#include	"FEATURE/locale"

static int level;
static int fatal;
static double arith(const char **ptr, struct lval *lvalue, int type, double n)
{
	register double r= 0;
	char *str = (char*)*ptr;
	switch(type)
	{
	    case ASSIGN:
	    {
		register Namval_t *np = (Namval_t*)(lvalue->value);
		register Namarr_t *ap;
		if(ap = nv_arrayptr(np))
		{
			if(!array_assoc(ap))
				nv_putsub(np, NIL(char*),lvalue->flag);
		}
		nv_putval(np, (char*)&n, NV_INTEGER);
		r=nv_getnum(np);
		break;
	    }
	    case LOOKUP:
	    {
		register int c = *str;
		lvalue->value = (char*)0;
		if(c=='.')
			c = str[1];
		if(isaletter(c))
		{
			register Namval_t *np;
			while(c= *++str, isaname(c)||c=='.');
			if(c=='(')
			{
				int fsize = str- (char*)(*ptr);
				const struct mathtab *tp;
				c = **ptr;
				lvalue->fun = 0;
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
				lvalue->value = (char*)e_function;
				return(r);
			}
			*str = 0;
			np = nv_open(*ptr,sh.var_tree,NV_NOASSIGN|NV_VARNAME);
			*str = c;
			if(nv_isattr(np,NV_INTEGER|NV_DOUBLE)==(NV_INTEGER|NV_DOUBLE))
				lvalue->isfloat=1;
			if(c=='[')
				str = nv_endsubscript(np,str,NV_ADD);
			else if(nv_isattr(np,NV_ARRAY))
				nv_putsub(np,NIL(char*),ARRAY_UNDEF);
			lvalue->value = (char*)np;
			lvalue->flag = nv_aindex(np);
		}
		else
		{
			char	*val = str;
			sh.lastbase = 0;
			r = strton(val,&str, &sh.lastbase,-1);
			if(*str=='8' || *str=='9')
			{
				sh.lastbase=10;
				r = strton(val,&str, &sh.lastbase,-1);
			}
			if(sh.lastbase<=1)
				sh.lastbase=10;
			if(*val=='0')
			{
				while(*val=='0')
					val++;
				if(*val==0 || *val=='.')
					val--;
			}
			if((c= *str)==GETDECIMAL(0) || c=='e' || c == 'E')
			{
				lvalue->isfloat=1;
				r = strtod(val,&str);
			}
			else if(sh.lastbase==10 && val[1])
			{
				if(val[2]=='#')
					val += 3;
				if((str-val)>2*sizeof(long))
				{
					double rr;
					rr = strtod(val,&str);
					if(rr!=r)
					{
						r = rr;
						lvalue->isfloat=1;
					}
				}
			}
		}
		break;
	    }
	    case VALUE:
	    {
		register Namval_t *np = (Namval_t*)(lvalue->value);
		if(sh_isoption(SH_NOEXEC))
			return(0);
		if((fatal==2 || level || sh_isoption(SH_NOUNSET)) && nv_isnull(np) && !nv_isattr(np,NV_INTEGER))
		{
			*ptr = nv_name(np);
			lvalue->value = (char*)e_notset;
			return(0);
		}
		level++;
		r = nv_getnum(np);
		level--;
		return(r);
	    }

	    case ERRMSG:
		level=0;
		sfsync(NIL(Sfio_t*));
#if 0
		if(warn)
			errormsg(SH_DICT,ERROR_warn(0),lvalue->value,*ptr);
		else
#endif
			errormsg(SH_DICT,ERROR_exit(fatal),lvalue->value,*ptr);
	}
	*ptr = str;
	return(r);
}

/*
 * convert number defined by string to a double
 * ptr is set to the last character processed
 * if mode>0, an error will be fatal with value <mode>
 */
double sh_strnum(register const char *str, char** ptr, int mode)
{
	fatal=mode;
	return(strval(str,(char**)ptr, arith));
}

double sh_arith(register const char *str)
{
	const char *ptr = str;
	register double d;
	fatal = 1;
	if(*str==0)
		return(0);
	d = strval(str,(char**)&ptr,arith);
	if(*ptr)
		errormsg(SH_DICT,ERROR_exit(1),e_lexbadchar,*ptr,str);
	return(d);
}

