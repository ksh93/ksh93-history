/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1982-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*                David Korn <dgk@research.att.com>                 *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * AT&T Labs
 *
 */

#include	"defs.h"
#include	<ctype.h>
#include	"variables.h"
#include	"path.h"
#include	"lexstates.h"
#include	"timeout.h"
#include	"FEATURE/locale"
#include	"FEATURE/externs"
#include	"national.h"
/* NOTE: the ast pathnative() [uwin_path() api] can be used in all cases */
#ifdef _lib_uwin_path
#   include	<uwin.h>
#else
#if __CYGWIN__
#define _lib_uwin_path		1
#define uwin_path(a,b,c)	cygwin_conv_to_win32_path(a,b)
extern void			cygwin_conv_to_win32_path(const char*, char*);
#endif
#endif /* _lib_uwin_path */

static void	attstore(Namval_t*,void*);
#ifndef _ENV_H
static void	pushnam(Namval_t*,void*);
static char	*staknam(Namval_t*, char*);
#endif
static void	ltou(const char*,char*);
static void	rightjust(char*, int, int);

struct adata
{
	char    **argnam;
	int     attsize;
	char    *attval;
};

static int	maxbufsize;
static char	*curbuf;
static char	local;
#ifndef _ENV_H
static void(*nullscan)(Namval_t*,void*);
#endif

#if ( SFIO_VERSION  <= 20010201L )
#   define _data        data
#endif

/* ========	name value pair routines	======== */

#include	"shnodes.h"
#include	"builtins.h"

#ifdef _ENV_H
void env_put(Env_t* ep,Namval_t *np)
{
	int offset = staktell();
	Namarr_t *ap = nv_arrayptr(np);
	char *val;
	if(ap)
	{
		if(ap->nelem&ARRAY_UNDEF)
			nv_putsub(np,"0",0L);
		else if(!(val=nv_getsub(np)) || strcmp(val,"0"))
			return;
	}
	if(!(val = nv_getval(np)))
		return;
	stakputs(nv_name(np));
	stakputc('=');
	stakputs(val);
	stakseek(offset);
	env_add(ep,stakptr(offset),ENV_STRDUP);
}
#endif

static void *newnode(const char *name)
{
	register int s;
	register Namval_t *np = newof(0,Namval_t,1,s=strlen(name)+1);
	if(np)
	{
		np->nvname = (char*)np+sizeof(Namval_t);
		memcpy(np->nvname,name,s);
	}
	return((void*)np);
}

#ifdef SHOPT_NAMESPACE
/*
 * clone a numeric value
 */
static void *num_clone(register Namval_t *np, void *val)
{
	register int size;
	void *nval;
	if(!val)
		return(0);
	if(nv_isattr(np,NV_DOUBLE))
	{
		if(nv_isattr(np,NV_LONG))
			size = sizeof(Sfdouble_t);
		else if(nv_isattr(np,NV_SHORT))
			size = sizeof(float);
		else
			size = sizeof(double);
	}
	else
	{
		if(nv_isattr(np,NV_LONG))
			size = sizeof(Sflong_t);
		else if(nv_isattr(np,NV_SHORT))
			size = sizeof(short);
		else
			size = sizeof(long);
	}
	if(!(nval = malloc(size)))
		return(0);
	memcpy(nval,val,size);
	return(nval);
}

int nv_clone(Namval_t *np, Namval_t *mp, int flags)
{
	int size;
	Namfun_t *fp;
	if(fp=np->nvfun)
	{
	        register Namfun_t **mfp = &mp->nvfun, *nfp;
		if(flags&NV_MOVE)
		{
			mp->nvfun = fp;
			goto skip;
		}
		while(fp)
		{
			if(!(size=fp->dsize) && !(size=fp->disc->dsize))
				size = sizeof(Namfun_t);
			if(fp->disc->clone)
				nfp = (*fp->disc->clone)(np,mp,size,fp);
			else
			{
				if(!(nfp=newof(NIL(Namfun_t*),Namfun_t,1,size-sizeof(Namfun_t))))
					return(0);
				memcpy(nfp,fp,size);
			}
			nfp->next = 0;
			*mfp = nfp;
			mfp = &nfp->next;
	                fp = fp->next;
		}
	}
	if(flags&NV_APPEND)
		return(1);
skip:
        nv_setsize(mp,nv_size(np));
	if(!nv_isattr(np,NV_MINIMAL))
	        mp->nvenv = np->nvenv;
        mp->nvalue.cp = np->nvalue.cp;
        mp->nvflag = np->nvflag;
	if(flags&NV_MOVE)
	{
		np->nvfun = 0;
		np->nvalue.cp = 0;
		np->nvflag = 0;
		if(!nv_isattr(np,NV_MINIMAL))
		        np->nvenv = 0;
	        nv_setsize(np,0);
		return(1);
	}
	if(nv_istable(np))
	{
		Dt_t *oroot=np->nvalue.hp,*nroot=dtopen(&_Nvdisc,Dtset);
		if(!nroot)
			return(0);
		for(np=(Namval_t*)dtfirst(oroot);np;np=(Namval_t*)dtnext(oroot,np))
		{
			mp = (Namval_t*)dtinsert(nroot,newnode(np->nvname));
			nv_clone(np,mp,flags);
		}
		mp->nvalue.hp = nroot;
		return(1);
	}
	else if(nv_isarray(np))
	{
		Namarr_t *ap = nv_arrayptr(np);
		char *name, *sub= nv_getsub(np);
		if(sub)
			sub = strdup(sub);
		nv_putsub(np,NIL(char*),ARRAY_SCAN);
		do
		{
		        if(array_assoc(ap))
				name = (char*)((*ap->fun)(np,NIL(char*),NV_ANAME));
			else
				name = nv_getsub(np);
			nv_putsub(mp,name,ARRAY_ADD);
			if(nv_isattr(np,NV_INTEGER))
			{
				double d= nv_getnum(np);
				nv_putval(mp,(char*)&d,NV_DOUBLE|NV_INTEGER);
			}
			else
				nv_putval(mp,nv_getval(np),NV_RDONLY);
		}
		while(nv_nextsub(np));
		nv_putsub(np,sub,0L);
		free((void*)sub);
	}
	else if(nv_isattr(np,NV_INTEGER))
		mp->nvalue.ip = (int*)num_clone(np,(void*)np->nvalue.ip);
	else if(flags)
	        nv_onattr(np,NV_NOFREE);
	return(1);
}

/*
 *  The following discipline is for copy-on-write semantics
 */
static char* clone_getv(Namval_t *np, Namfun_t *handle)
{
	return(np->nvalue.np?nv_getval(np->nvalue.np):0);
}

static double clone_getn(Namval_t *np, Namfun_t *handle)
{
	return(np->nvalue.np?nv_getnum(np->nvalue.np):0);
}

static void clone_putv(Namval_t *np,const char* val,int flags,Namfun_t *handle)
{
	Namfun_t *dp = nv_stack(np,(Namfun_t*)0);
	Namval_t *mp = np->nvalue.np;
	if(!sh.subshell)
		free((void*)dp);
	if(val)
		nv_clone(mp,np,1);
	np->nvalue.cp = 0;
	nv_putval(np,val,flags);
}

static const Namdisc_t clone_disc =
{
	0,
	clone_putv,
	clone_getv,
	clone_getn
};

static Namval_t *mkclone(Namval_t *mp)
{
	Namval_t *np;
	Namfun_t *dp;
	np = newof(0,Namval_t,1,0);
	np->nvflag = mp->nvflag;
	np->nvsize = mp->nvsize;
	np->nvname = mp->nvname;
	np->nvalue.np = mp;
	np->nvflag = mp->nvflag;
	dp = newof(0,Namfun_t,1,0);
	dp->disc = &clone_disc;
	nv_stack(np,dp);
	dtinsert(sh.namespace->nvalue.hp,np);
	return(np);
}
#endif /* SHOPT_NAMESPACE */


/*
 * output variable name in format for re-input
 */
void nv_outname(Sfio_t *out, char *name, int len)
{
	const char *cp=name, *sp;
	int c, offset = staktell();
	while(sp= strchr(cp,'['))
	{
		if(len>0 && cp+len < sp)
			break;
		sfwrite(out,cp,++sp-cp);
		stakseek(offset);
		for(; c= *sp; sp++)
		{
			if(c==']')
				break;
			else if(c=='\\')
			{
				if(*sp=='[' || *sp==']' || *sp=='\\')
					c = *sp++;
			}
			stakputc(c);
		}
		stakputc(0);
		sfputr(out,sh_fmtq(stakptr(offset)),-1);
		if(len>0)
		{
			sfputc(out,']');
			return;
		}
		cp = sp;
	}
	if(*cp)
	{
		if(len>0)
			sfwrite(out,cp,len);
		else
			sfputr(out,cp,-1);
	}
	stakseek(offset);
}

/*
 * Perform parameter assignment for a linked list of parameters
 * <flags> contains attributes for the parameters
 */
void nv_setlist(register struct argnod *arg,register int flags)
{
	register char *cp;
	register Namval_t *np;
	int traceon = (sh_isoption(SH_XTRACE)!=0);
	if(sh_isoption(SH_ALLEXPORT))
		flags |= NV_EXPORT;
	if(sh.prefix)
	{
		flags &= ~(NV_IDENT|NV_EXPORT);
		flags |= NV_VARNAME;
	}
	for(;arg; arg=arg->argnxt.ap)
	{
		sh.used_pos = 0;
		if(arg->argflag&ARG_MAC)
			cp = sh_mactrim(arg->argval,-1);
		else
		{
			stakseek(0);
			if(*arg->argval==0 && arg->argchn.ap && !(arg->argflag&~(ARG_APPEND|ARG_QUOTED)))
			{
#ifdef SHOPT_COMPOUND_ARRAY
				int flag = (NV_VARNAME|NV_ASSIGN);
#else
				int flag = (NV_VARNAME|NV_ARRAY|NV_ASSIGN);
#endif /* SHOPT_COMPOUND_ARRAY */
				struct fornod *fp=(struct fornod*)arg->argchn.ap;
				register union anynode *tp=fp->fortre;
				char *prefix = sh.prefix;
				if(arg->argflag&ARG_QUOTED)
					cp = sh_mactrim(fp->fornam,-1);
				else
					cp = fp->fornam;
				error_info.line = fp->fortyp-sh.st.firstline;
				if(sh.fn_depth && (Namval_t*)tp->com.comnamp==SYSTYPESET)
			                flag |= NV_NOSCOPE;
				if(prefix)
					*--cp = '.';
				np = nv_open(cp,sh.var_tree,flag);
				if(prefix)
					*cp++ = 0;
				/* check for array assignment */
				if(tp->tre.tretyp!=TLST && tp->com.comarg && !tp->com.comset)
				{
					int argc;
					char **argv = sh_argbuild(&argc,&tp->com,0);
					if(!(arg->argflag&ARG_APPEND))
						nv_unset(np);
					nv_setvec(np,(arg->argflag&ARG_APPEND),argc,argv);
					if(traceon)
					{
						int n = -1;
						sh_trace(NIL(char**),0);
						if(arg->argflag&ARG_APPEND)
							n = '+';
						sfputr(sfstderr,nv_name (np),n);
						sfwrite(sfstderr,"=( ",3);
						while(cp= *argv++)
							sfputr(sfstderr,sh_fmtq(cp),' ');
						sfwrite(sfstderr,")\n",2);
					}
					continue;
				}
				if(tp->tre.tretyp==TLST || !tp->com.comset || tp->com.comset->argval[0]!='[')
				{
#ifdef SHOPT_COMPOUND_ARRAY
					if(*cp!='[' && strchr(cp,'['))
					{
						nv_close(np);
						np = nv_open(cp,sh.var_tree,flag);
					}
#endif /* SHOPT_COMPOUND_ARRAY */
					if((arg->argflag&ARG_APPEND) && !nv_isarray(np))
						nv_unset(np);
				}
				else
				{
					if((arg->argflag&ARG_APPEND) && (!nv_isarray(np) || (nv_aindex(np)>=0)))
					{
						nv_unset(np);
						nv_setarray(np,nv_associative);
					}
					else
						nv_setarray(np,nv_associative);
				}
				if(prefix)
				{
					int offset=0;
					stakputs(prefix);
					stakputc('.');
					if(*cp=='[')
						offset = staktell()+1;
					stakputs(cp);
					if(offset && sh_checkid(stakptr(offset),(char*)0))
						stakseek(staktell()-2);
					cp = stakfreeze(1);
				}
				sh.prefix = cp;
				sh_exec(tp,sh_isstate(SH_ERREXIT));
				sh.prefix = prefix;
				if(nv_isnull(np))
					nv_setvtree(np);
				continue;
			}
			cp = arg->argval;
		}
		if(sh.prefix && *cp=='.' && cp[1]=='=')
			cp++;
		np = nv_open(cp,sh.var_tree,flags);
		if(!np->nvfun)
		{
			if(sh.used_pos)
				nv_onattr(np,NV_PARAM);
			else
				nv_offattr(np,NV_PARAM);
		}
		if(traceon)
		{
			register char *sp=cp;
			sh_trace(NIL(char**),0);
			nv_outname(sfstderr,nv_name(np),-1);
			if(nv_isarray(np))
				sfprintf(sfstderr,"[%s]",sh_fmtq(nv_getsub(np)));
			if(cp=strchr(sp,'='))
			{
				if(cp[-1]=='+')
					sfputc(sfstderr,'+');
				sfprintf(sfstderr,"=%s\n",sh_fmtq(cp+1));
			}
		}
	}
}

/*
 * copy the subscript onto the stack
 */
static void stak_subscript(const char *sub, int last)
{
	register int c;
	stakputc('[');
	while(c= *sub++)
	{
		if(c=='[' || c==']' || c=='\\')
			stakputc('\\');
		stakputc(c);
	}
	stakputc(last);
}

/*
 * construct a new name from a prefix and base name on the stack
 */
static char *newname(register const char *prefix, register const char *name, const char *sub, char **lastptr)
{
	register int last=0,offset = staktell();
	if(prefix)
	{
		stakputs(prefix);
		if(*name=='.' && name[1]=='[')
			last = staktell()+2;
		if(*name!='[' && *name!='.' && *name!='=')
			stakputc('.');
	}
	stakputs(name);
	if(last && sh_checkid(stakptr(last),(char*)0))
		stakseek(staktell()-2);
	if(sub)
		stak_subscript(sub,']');
	if(lastptr)
	{
		char *cp = *lastptr;
		last = staktell();
		if(*cp)
			stakputs(cp);
	}
	stakputc(0);
	if(lastptr)
		*lastptr = stakptr(last);
	return(stakptr(offset));
}

/*
 * Put <arg> into associative memory.
 * If <flags> & NV_ARRAY then subscript is not allowed
 * If <flags> & NV_NOSCOPE then use the current scope only
 * If <flags> & NV_ASSIGN then assignment is allowed
 * If <flags> & NV_IDENT then name must be an identifier
 * If <flags> & NV_VARNAME then name must be a valid variable name
 * If <flags> & NV_NOADD then node will not be added if not found
 * If <flags> & NV_NOREF then don't follow reference
 * SH_INIT is only set while initializing the environment
 */
Namval_t	*nv_open(const char *name,Dt_t *root,int flags)
{
	register char *cp = (char*)name;
	register Namval_t	*np=0;
	register int sep = *cp;
	register char *lastdot = 0;
	register long mode = ((flags&NV_NOADD)?0:NV_ADD);
	char *sub, *ptr;
	int offset = -1, fun=0;
#ifdef SHOPT_APPEND
	int append=0;
#endif /* SHOPT_APPEND */

	if(root==sh.alias_tree)
	{
		while((sep= *(unsigned char*)cp++) && (sep!='=') && (sep!='/') && 
			(sep>=0x200 || !(sep=sh_lexstates[ST_NORM][sep]) || sep==S_EPAT));
		cp--;
	}
	else
	{
		sh.last_table = 0;
		if(!root)
			root = sh.var_tree;
		else if(root==sh_subfuntree(1))
		{
			fun=1;
			if(sh.namespace && !strchr(name,'.'))
			{
				name = cp = newname(nv_name(sh.namespace),name,(char*)0,(char**)0);
				flags &=  ~NV_IDENT;
			}
		}
		if(root==sh.var_tree && *name=='.' && name[1])
			root = sh.var_base;
		else
			sh.last_table = sh.namespace;
		if(!(flags&(NV_IDENT|NV_VARNAME|NV_ASSIGN)))
		{
			if(flags&NV_NOSCOPE)
				mode |= HASH_SCOPE|HASH_NOSCOPE;
			np = nv_search(name,root,mode);
			if(np && !(flags&NV_NOREF))
			{
				while(nv_isref(np))
				{
					sh.last_table = nv_table(np);
					np = nv_refnode(np);
				}
			}
			return(np);
		}
		if(sh.prefix && (flags&NV_ASSIGN))
		{
			offset = staktell();
			if(sh.prefix && *name=='.' && name[1]==0)
				cp = sh.prefix;
			else
				cp = newname(sh.prefix,name,(char*)0,(char**)0);
			name = cp;
			if(*cp=='.' && root==sh.var_tree)
				root = sh.var_base;
			sep = *(unsigned char*)cp;
		}
		/* first skip over alpha-numeric */
		while(1)
		{
			if(sep=='.')
			{
				if(flags&NV_IDENT)
					goto failed;
				if(root==sh.var_tree)
					flags &= ~(NV_NOSCOPE|NV_EXPORT);
				if(!lastdot && cp!=name && (flags&NV_VARNAME))
				{
					/* see whether first component is ref */
					*cp = 0;
					np = nv_search(name,sh.var_tree,0);
					*cp = '.';
					if(np && nv_isref(np))
					{
						/* don't optimize */
						sh.argaddr = 0; 
						/* substitute ref name */
						while(nv_isref(np))
						{
							sub = np->nvenv;
							sh.last_table = nv_table(np);
							np = nv_refnode(np);
						}
						ptr = cp;
						name=newname((char*)0,nv_name(np),sub,&ptr);
						cp = ptr;
					}
				}
				lastdot = cp++;
			}
			if(sep= *(unsigned char*)cp, !isaletter(sep))
			{
#ifdef SHOPT_COMPOUND_ARRAY
				if(sep!='[' || !(flags&NV_VARNAME) || fun)
					break;
				if(cp==lastdot+1)
					np = 0;
				ptr = nv_endsubscript(np,cp,0);
				if(cp==lastdot+1)
				{
					cp = lastdot = ptr;
					sep = *cp;
					continue;
				}
				if(!ptr || *ptr!='.')
					break;
				if((ptr-cp)==3 && cp[1]=='0')
				{
					strcpy(cp,ptr);
					ptr = cp;
				}
				*ptr = 0;
				np = nv_search(name,root,0);
				*ptr = '.';
				if(np)
					lastdot = ptr;
				else
				{
					*cp = 0;
					np = nv_search(name,root,0);
					*cp = '[';
					if(np)
						lastdot = cp;
				}
				if(!lastdot)
					lastdot = (char*)name;
				cp = ptr+1;
				if(sep= *(unsigned char*)cp, !isaletter(sep))
					goto failed;
#else
				break;
#endif /* SHOPT_COMPOUND_ARRAY */
			}
			while(sep= *(unsigned char*)(++cp),isaname(sep));
		}
		if(offset>=0)
			stakseek(offset);
#ifdef SHOPT_COMPOUND_ARRAY
	retry:
#endif /* SHOPT_COMPOUND_ARRAY */
		/* if name doesn't have to be an varname or ident skip to '=' */
#ifdef SHOPT_APPEND
		if(sep && sep!='=' && sep!='[' && sep!='+')
#else
		if(sep && sep!='=' && sep!='[')
#endif /* SHOPT_APPEND */
		{
			if(flags&NV_IDENT)
				goto failed;
			else if(flags&NV_VARNAME)
				errormsg(SH_DICT,ERROR_exit(1),(root==sh.var_tree?e_varname:e_funname),name);
			while((sep= *++cp) && sep!='=');
		}
		if(lastdot==name)
			lastdot = 0;
		if(lastdot)
		{
			*cp = 0;
			np = nv_search(name,root,0);
			*cp = sep;
			if(!np && sep=='[')
				ptr = nv_endsubscript(np,cp,0);
			else
				ptr = cp;
		}
		while(!np && lastdot && lastdot>name)
		{
			int c = *lastdot;
			*lastdot=0;
			np = nv_search(name,fun?sh.var_tree:root,0);
			*lastdot=c;
			if(c=='[')
				lastdot = nv_endsubscript(np,lastdot,0);
			if(np)
			{
				char *sp;
				Namval_t *nq;
				if(np->nvfun && np->nvfun->disc && !nv_isref(np) && (np->nvfun)->disc->create)
				{
					c = *ptr;
					*ptr = 0;
					np = nv_create(nq=np,lastdot+1,0,(Namfun_t*)np);
					*ptr = c;
					if(np==nq)
						errormsg(SH_DICT,ERROR_exit(1),e_varname,name);
				}
				else if(!fun && nv_istable(np))
				{
					sh.last_table = np;
					return(nv_open(lastdot+1,np->nvalue.hp,flags|NV_NOSCOPE));
				}
				else if((sp=strchr(lastdot+1,'.')) && sp<cp)
					errormsg(SH_DICT,ERROR_exit(1),e_noparent,name);
				else
					np = 0;
				break;
			}
			while(--lastdot>name && *lastdot!='.');
			if(lastdot==name)
				errormsg(SH_DICT,ERROR_exit(1),e_noparent,name);
		}
	}
	if(cp!=name)
	{
		if(sep && sh.subshell && root==sh.alias_tree)
			root = sh_subaliastree(1);
		if(sep)
			*cp = 0;
		if((flags&NV_NOSCOPE) && dtvnext(root) && root==sh.var_tree 
			&& (np=nv_search(name,sh.var_base,0)))
		{
			Namfun_t *disc = nv_cover(np);
			char *name = nv_name(np);
			if(np=nv_search((char*)np,root,mode|HASH_NOSCOPE|HASH_SCOPE|HASH_BUCKET))
			{
				np->nvfun = disc;
				np->nvname = name;
			}
		}
		if(!np)
		{
			if(flags&NV_NOSCOPE)
				mode |= HASH_SCOPE|HASH_NOSCOPE;
			np = nv_search(name,root,mode);
		}
#ifdef SHOPT_NAMESPACE
		if(*name!='.' && sh.namespace && np && root==sh.var_tree && dtsearch(sh.var_base,np)==np)
			np = mkclone(np);
#endif /* SHOPT_NAMESPACE */
		if(sep)
			*cp = sep;
		if(((flags&NV_NOREF) && sep==0) || (!np && (flags&NV_NOADD)))
			return(np);
		sub = 0;
		if(flags&NV_NOREF)
			nv_unref(np);
		else while(nv_isref(np))
		{
			sub = np->nvenv;
			sh.last_table = nv_table(np);
			np = nv_refnode(np);
		}
		if(sub)	/* nameref to a subscript */
		{
			int offset = staktell();
			stak_subscript(sub,']');
			stakputc(0);
			nv_endsubscript(np,stakptr(offset),NV_ADD);
			stakseek(offset);
		}
		/* check for subscript*/
		else if(sep=='[' && !(flags&NV_ARRAY))
		{
			sep = (flags&NV_ASSIGN?NV_ADD:0);
#ifdef SHOPT_COMPOUND_ARRAY
			lastdot = nv_endsubscript(np,cp,NV_ADD);
			if((sep = *lastdot)=='.')
			{
				sub = nv_getsub(np);
				ptr = lastdot;
				name = newname((char*)0,nv_name(np),sub?sub:"0",&ptr);
				cp= ptr;
				lastdot = 0;
				np = 0;
				while(1)
				{
					while(sep= *(unsigned char*)(++cp),isaname(sep));
					if(sep!='.')
						break;
					lastdot = cp;
				}
				goto retry;
			}
			cp = lastdot;
#else
			cp = nv_endsubscript(np,cp,NV_ADD);
			sep = *cp;
#endif /* SHOPT_COMPOUND_ARRAY */
		}
		else if(nv_isarray(np))
			nv_putsub(np,NIL(char*),ARRAY_UNDEF);
#ifdef SHOPT_APPEND
		if(sep=='+')
		{
			append = NV_APPEND;
			sep = *++cp;
		}
#endif /* SHOPT_APPEND */
		if(sep && ((sep!='=')||!(flags&NV_ASSIGN)))
		{
			if(sh_isstate(SH_INIT))
				return(0);
			goto failed;
		}
		if(sep == '=')
		{
			cp++;
			if(sh_isstate(SH_INIT))
			{
				nv_putval(np, cp, NV_RDONLY);
				if(np==PWDNOD)
					nv_onattr(np,NV_TAGGED);
			}
			else
			{
				sep = (root==sh.alias_tree?0:flags&NV_EXPORT);
#ifdef SHOPT_APPEND
				nv_putval(np, cp, append|sep);
#else
				nv_putval(np, cp, sep);
#endif /*SHOPT_APPEND */
#ifdef SHOPT_BSH
				if(flags&NV_EXPORT)
					nv_offattr(np,NV_IMPORT);
#endif /* SHOPT_BSH */
			}
			nv_onattr(np, flags&NV_ATTRIBUTES);
		}
		return(np);
	}
failed:
	if(!sh_isstate(SH_INIT))
		errormsg(SH_DICT,ERROR_exit(1),(root==sh.alias_tree?e_aliname:e_ident),name);
	return(0);
}

#ifdef SHOPT_MULTIBYTE
    static char *savep;
    static char savechars[8+1];
    static int ja_size(char*, int, int);
    static void ja_restore(void);
#endif /* SHOPT_MULTIBYTE */

/*
 * put value <string> into name-value node <np>.
 * If <np> is an array, then the element given by the
 *   current index is assigned to.
 * If <flags> contains NV_RDONLY, readonly attribute is ignored
 * If <flags> contains NV_INTEGER, string is a pointer to a number
 * If <flags> contains NV_NOFREE, previous value is freed, and <string>
 * becomes value of node and <flags> becomes attributes
 */
void nv_putval(register Namval_t *np, const char *string, int flags)
{
	register const char *sp=string;
	register union Value *up;
	register char *cp;
	register int size = 0;
	register int dot;
	if(!(flags&NV_RDONLY) && nv_isattr (np, NV_RDONLY))
		errormsg(SH_DICT,ERROR_exit(1),e_readonly, nv_name(np));
	/* The following could cause the shell to fork if assignment
	 * would cause a side effect
	 */
	sh.argaddr = 0;
	if(sh.subshell && !local)
		np = sh_assignok(np,1);
	if(np->nvfun && !nv_isattr(np,NV_REF))
	{
		/* This function contains disc */
		if(!local)
		{
			local=1;
			nv_putv(np,sp,flags,np->nvfun);
#ifdef _ENV_H
			if((flags&NV_EXPORT) || nv_isattr(np,NV_EXPORT))
				env_put(sh.env,np);
#endif
			return;
		}
		/* called from disc, assign the actual value */
		local=0;
	}
	flags &= ~NV_NODISC;
	if(flags&(NV_NOREF|NV_NOFREE))
	{
		if(!nv_isnull(np))
			nv_unset(np);
		np->nvalue.cp = (char*)sp;
		nv_setattr(np,flags|NV_NOFREE);
		return;
	}
	up= &np->nvalue;
#ifndef SHOPT_BSH
	if(nv_isattr(np,NV_EXPORT))
		nv_offattr(np,NV_IMPORT);
	np->nvenv = 0;
#endif /* SHOPT_BSH */
	if(nv_isattr (np, NV_INTEGER))
	{
		if(nv_isattr(np, NV_DOUBLE))
		{
			if(nv_isattr(np, NV_LONG) && sizeof(double)<sizeof(Sfdouble_t))
			{
				Sfdouble_t ld, old=0;
				if(flags&NV_INTEGER)
				{
					if(flags&NV_LONG)
						ld = (double)(*(Sfdouble_t*)sp);
					else
						ld = *(double*)sp;
				}
				else
					ld = sh_arith(sp);
				if(!up->ldp)
					up->ldp = new_of(Sfdouble_t,0);
				else if(flags&NV_APPEND)
					old = *(up->ldp);
				*(up->ldp) = ld+old;
			}
			else
			{
				double d,od=0;
				if(flags&NV_INTEGER)
				{
					if(flags&NV_LONG)
						d = (double)(*(Sfdouble_t*)sp);
					else
						d = *(double*)sp;
				}
				else
					d = sh_arith(sp);
				if(!up->dp)
					up->dp = new_of(double,0);
				else if(flags&NV_APPEND)
					od = *(up->dp);
				*(up->dp) = d+od;
			}
		}
		else
		{
			if(nv_isattr(np, NV_LONG) && sizeof(long)<sizeof(Sflong_t))
			{
				Sflong_t ll=0,oll=0;
				if(flags&NV_INTEGER)
				{
					if(flags&NV_LONG)
						ll = (double)(*(Sfdouble_t*)sp);
					else
						ll = *(double*)sp;
				}
				else if(sp)
					ll = (Sfulong_t)sh_arith(sp);
				if(!up->llp)
					up->llp = new_of(Sflong_t,0);
				else if(flags&NV_APPEND)
					oll = *(up->llp);
				*(up->llp) = ll+oll;
			}
			else
			{
				long l=0,ol=0;
				if(flags&NV_INTEGER)
				{
					if(flags&NV_DOUBLE)
					{
						if(flags&NV_LONG)
							l = (double)(*(Sfdouble_t*)sp);
						else
							l = *(double*)sp;
					}
					else
					{
						if(flags&NV_LONG)
							l = (long)(*(Sflong_t*)sp);
						else
							l = *(long*)sp;
					}
				}
				else if(sp)
					l = (unsigned long)sh_arith(sp);
				if(nv_size(np) <= 1)
					nv_setsize(np,10);
				if(nv_isattr (np, NV_SHORT))
				{
					short s=0;
					if(flags&NV_APPEND)
						s = up->s;
					up->s = s+(short)l;
				}
				else
				{
					if(!up->lp)
						up->lp = new_of(long,0);
					else if(flags&NV_APPEND)	
						ol =  *(up->lp);
					*(up->lp) = l+ol;
					if(l && *sp++ == '0')
						nv_onattr(np,NV_UNSIGN);
				}
			}
		}
	}
	else
	{
		const char *tofree=0;
		char numbuf[20];
#ifdef SHOPT_APPEND
		int offset;
#endif /* SHOPT_APPEND */
#ifdef _lib_uwin_path
		char buff[PATH_MAX];
#endif /* _lib_uwin_path */
		if(flags&NV_INTEGER)
		{
			if(flags&NV_DOUBLE)
				sfsprintf(numbuf,sizeof(numbuf),"%.*g\0",12,*((double*)sp));
			else if(flags&NV_LONG)
				sfsprintf(numbuf,sizeof(numbuf),"%lld\0",*((Sflong_t*)sp));
			else
				sfsprintf(numbuf,sizeof(numbuf),"%ld\0",*((long*)sp));
			sp = numbuf;
		}
		if(nv_isattr(np, NV_HOST)==NV_HOST && sp)
		{
#ifdef _lib_uwin_path
			/*
			 * return the host file name given the UNIX name
			 */
			uwin_path(sp,buff,sizeof(buff));
			if(buff[1]==':' && buff[2]=='/')
			{
				buff[2] = '\\';
				if(*buff>='A' &&  *buff<='Z')
					*buff += 'a'-'A';
			}
			sp = buff;
#else
			;
#endif /* _lib_uwin_path */
		}
		else if((nv_isattr(np, NV_RJUST|NV_ZFILL|NV_LJUST)) && sp)
		{
			for(;*sp == ' '|| *sp=='\t';sp++);
	        	if((nv_isattr(np,NV_ZFILL)) && (nv_isattr(np,NV_LJUST)))
				for(;*sp=='0';sp++);
			size = nv_size(np);
#ifdef SHOPT_MULTIBYTE
			if(size)
				size = ja_size((char*)sp,size,nv_isattr(np,NV_RJUST|NV_ZFILL));
#endif /* SHOPT_MULTIBYTE */
		}
#ifdef SHOPT_APPEND
		if(!up->cp)
			flags &= ~NV_APPEND;
		if(flags&NV_APPEND)
		{
			offset = staktell();
			stakputs(up->cp);
			stakputs(sp);
			stakputc(0);
			sp = stakptr(offset);
		}
#endif /*SHOPT_APPEND */
		if(!nv_isattr(np, NV_NOFREE|NV_NOALLOC))
		{
			/* delay free in case <sp> points into free region */
			tofree = up->cp;
		}
		if(nv_isattr(np, NV_NOALLOC))
			cp = (char*)up->cp;
		else
		{
			nv_offattr(np,NV_NOFREE);
       		 	if (sp)
			{
				dot = strlen(sp);
				if(size==0 && nv_isattr(np,NV_LJUST|NV_RJUST|NV_ZFILL))
					nv_setsize(np,size=dot);
				else if(size > dot)
					dot = size;
				cp = (char*)malloc(((unsigned)dot+1));
			}
			else
				cp = 0;
			up->cp = cp;
		}
		if(sp)
		{
			if(nv_isattr(np, NV_LTOU))
				ltou(sp,cp);
			else if(nv_isattr (np, NV_UTOL))
				sh_utol(sp,cp);
			else
       			 	strcpy(cp, sp);
			if(nv_isattr(np, NV_RJUST) && nv_isattr(np, NV_ZFILL))
				rightjust(cp,size,'0');
			else if(nv_isattr(np, NV_RJUST))
				rightjust(cp,size,' ');
			else if(nv_isattr(np, NV_LJUST))
			{
				register char *dp;
				dp = strlen (cp) + cp;
				*(cp = (cp + size)) = 0;
				for (; dp < cp; *dp++ = ' ');
			 }
#ifdef SHOPT_MULTIBYTE
			/* restore original string */
			if(savep)
				ja_restore();
#endif /* SHOPT_MULTIBYTE */
		}
#ifdef SHOPT_APPEND
		if(flags&NV_APPEND)
			stakseek(offset);
#endif /* SHOPT_APPEND */
		if(tofree)
			free((void*)tofree);
	}
#ifdef _ENV_H
	if(!local && (flags&NV_EXPORT) || nv_isattr(np,NV_EXPORT))
		env_put(sh.env,np);
#endif
	return;
}

/*
 *
 *   Right-justify <str> so that it contains no more than
 *   <size> characters.  If <str> contains fewer than <size>
 *   characters, left-pad with <fill>.  Trailing blanks
 *   in <str> will be ignored.
 *
 *   If the leftmost digit in <str> is not a digit, <fill>
 *   will default to a blank.
 */
static void rightjust(char *str, int size, int fill)
{
	register int n;
	register char *cp,*sp;
	n = strlen(str);

	/* ignore trailing blanks */
	for(cp=str+n;n && *--cp == ' ';n--);
	if (n == size)
		return;
	if(n > size)
        {
        	*(str+n) = 0;
        	for (sp = str, cp = str+n-size; sp <= str+size; *sp++ = *cp++);
        	return;
        }
	else *(sp = str+size) = 0;
	if (n == 0)  
        {
        	while (sp > str)
               		*--sp = ' ';
        	return;
        }
	while(n--)
	{
		sp--;
		*sp = *cp--;
	}
	if(!isdigit(*str))
		fill = ' ';
	while(sp>str)
		*--sp = fill;
	return;
}

#ifdef SHOPT_MULTIBYTE
    /*
     * handle left and right justified fields for multi-byte chars
     * given physical size, return a logical size which reflects the
     * screen width of multi-byte characters
     * Multi-width characters replaced by spaces if they cross the boundary
     * <type> is non-zero for right justified  fields
     */

    static int ja_size(char *str,int size,int type)
    {
	register char *cp = str;
	register int c, n=size;
	register int outsize;
	register char *oldcp=cp;
	int oldn;
	wchar_t w;
	while(*cp)
	{
		oldn = n;
		w = mbchar(cp);
		outsize = mbwidth(w);
		size -= outsize;
		c = cp-oldcp;
		n += (c-outsize);
		oldcp = cp;
		if(size<=0 && type==0)
			break;
	}
	/* check for right justified fields that need truncating */
	if(size <0)
	{
		if(type==0)
		{
			/* left justified and character crosses field boundary */
			n = oldn;
			/* save boundary char and replace with spaces */
			size = c;
			savechars[size] = 0;
			while(size--)
			{
				savechars[size] = cp[size];
				cp[size] = ' ';
			}
			savep = cp;
		}
		size = -size;
		if(type)
			n -= (ja_size(str,size,0)-size);
	}
	return(n);
    }

    static void ja_restore(void)
    {
	register char *cp = savechars;
	while(*cp)
		*savep++ = *cp++;
	savep = 0;
    }
#endif /* SHOPT_MULTIBYTE */

#ifndef _ENV_H
static char *staknam(register Namval_t *np, char *value)
{
	register char *p,*q;
	q = stakalloc(strlen(nv_name(np))+(value?strlen(value):0)+2);
	p=strcopy(q,nv_name(np));
	if(value)
	{
		*p++ = '=';
		strcpy(p,value);
	}
	return(q);
}
#endif

/*
 * put the name and attribute into value of attributes variable
 */
#ifdef _ENV_H
static void attstore(register Namval_t *np, void *data)
{
	register int flag, c = ' ';
	NOT_USED(data);
	if(!(nv_isattr(np,NV_EXPORT)))
		return;
	flag = nv_isattr(np,NV_RDONLY|NV_UTOL|NV_LTOU|NV_RJUST|NV_LJUST|NV_ZFILL|NV_INTEGER);
	stakputc('=');
	if((flag&NV_DOUBLE) && (flag&NV_INTEGER))
	{
		/* export doubles as integers for ksh88 compatibility */
		stakputc(c+(flag&~(NV_DOUBLE|NV_EXPNOTE)));
	}
	else
	{
		stakputc(c+flag);
		if(flag&NV_INTEGER)
			c +=  nv_size(np);
	}
	stakputc(c);
	stakputs(nv_name(np));
}
#else
static void attstore(register Namval_t *np, void *data)
{
	register int flag = np->nvflag;
	register struct adata *ap = (struct adata*)data;
	if(!(flag&NV_EXPORT) || (flag&NV_FUNCT))
		return;
	flag &= (NV_RDONLY|NV_UTOL|NV_LTOU|NV_RJUST|NV_LJUST|NV_ZFILL|NV_INTEGER);
	*ap->attval++ = '=';
	if((flag&NV_DOUBLE) && (flag&NV_INTEGER))
	{
		/* export doubles as integers for ksh88 compatibility */
		*ap->attval++ = ' '+(flag&~(NV_DOUBLE|NV_EXPNOTE));
		*ap->attval = ' ';
	}
	else
	{
		*ap->attval++ = ' '+flag;
		if(flag&NV_INTEGER)
			*ap->attval = ' ' + nv_size(np);
		else
			*ap->attval = ' ';
	}
	ap->attval = strcopy(++ap->attval,nv_name(np));
}
#endif

#ifndef _ENV_H
static void pushnam(Namval_t *np, void *data)
{
	register char *value;
	register struct adata *ap = (struct adata*)data;
	if(nv_isattr(np,NV_IMPORT))
	{
		if(np->nvenv)
			*ap->argnam++ = np->nvenv;
	}
	else if(value=nv_getval(np))
		*ap->argnam++ = staknam(np,value);
	if(nv_isattr(np,NV_RDONLY|NV_UTOL|NV_LTOU|NV_RJUST|NV_LJUST|NV_ZFILL|NV_INTEGER))
		ap->attsize += (strlen(nv_name(np))+4);
}
#endif

/*
 * Generate the environment list for the child.
 */

#ifdef _ENV_H
char **sh_envgen(void)
{
	int offset,tell;
	register char **er;
	env_delete(sh.env,"_");
	er = env_get(sh.env);
	offset = staktell();
	stakputs(e_envmarker);
	tell = staktell();
	nv_scan(sh.var_tree, attstore,(void*)0,0,(NV_RDONLY|NV_UTOL|NV_LTOU|NV_RJUST|NV_LJUST|NV_ZFILL|NV_INTEGER));
	if(tell ==staktell())
		stakseek(offset);
	else
		*--er = stakfreeze(1)+offset;
	return(er);
}
#else
char **sh_envgen(void)
{
	register char **er;
	register int namec;
	register char *cp;
	struct adata data;
	/* L_ARGNOD gets generated automatically as full path name of command */
	nv_offattr(L_ARGNOD,NV_EXPORT);
	data.attsize = 6;
	namec = nv_scan(sh.var_tree,nullscan,(void*)0,NV_EXPORT,NV_EXPORT);
	er = (char**)stakalloc((namec+4)*sizeof(char*));
	data.argnam = (er+=2);
	nv_scan(sh.var_tree, pushnam,&data,NV_EXPORT, NV_EXPORT);
	*data.argnam = (char*)stakalloc(data.attsize);
	cp = data.attval = strcopy(*data.argnam,e_envmarker);
	nv_scan(sh.var_tree, attstore,&data,0,(NV_RDONLY|NV_UTOL|NV_LTOU|NV_RJUST|NV_LJUST|NV_ZFILL|NV_INTEGER));
	*data.attval = 0;
	if(cp!=data.attval)
		data.argnam++;
	*data.argnam = 0;
	return(er);
}
#endif

struct scan
{
	void    (*scanfn)(Namval_t*, void*);
	int     scanmask;
	int     scanflags;
	int     scancount;
	void    *scandata;
};


static int scanfilter(Dt_t *dict, void *arg, void *data)
{
	register Namval_t *np = (Namval_t*)arg;
	register int k=np->nvflag;
	register struct scan *sp = (struct scan*)data;
	NOT_USED(dict);
	if(sp->scanmask?(k&sp->scanmask)==sp->scanflags:(!sp->scanflags || (k&sp->scanflags)))
	{
		if(!np->nvalue.cp && !nv_isattr(np,~NV_DEFAULT))
			return(0);
		if(sp->scanfn)
		{
			if(nv_isarray(np))
				nv_putsub(np,NIL(char*),0L);
			(*sp->scanfn)(np,sp->scandata);
		}
		sp->scancount++;
	}
	return(0);
}

/*
 * Walk through the name-value pairs
 * if <mask> is non-zero, then only nodes with (nvflags&mask)==flags
 *	are visited
 * If <mask> is zero, and <flags> non-zero, then nodes with one or
 *	more of <flags> is visited
 * If <mask> and <flags> are zero, then all nodes are visted
 */
int nv_scan(Dt_t *root, void (*fn)(Namval_t*,void*), void *data,int mask, int flags)
{
	Dt_t *base=0;
	struct scan sdata;
	int (*hashfn)(Dt_t*, void*, void*);
	sdata.scanmask = mask;
	sdata.scanflags = flags&~NV_NOSCOPE;
	sdata.scanfn = fn;
	sdata.scancount = 0;
	sdata.scandata = data;
	hashfn = scanfilter;
	if(flags&NV_NOSCOPE)
		base = dtview((Dt_t*)root,0);
	dtwalk(root, hashfn,&sdata);
	if(base)
		 dtview((Dt_t*)root,base);
	return(sdata.scancount);
}

/*
 * create a new environment scope
 */
void nv_scope(struct argnod *envlist)
{
	register Dt_t *newscope;
	newscope = dtopen(&_Nvdisc,Dtbag);
	dtview(newscope,(Dt_t*)sh.var_tree);
	sh.var_tree = (Dt_t*)newscope;
	nv_setlist(envlist,NV_EXPORT|NV_NOSCOPE|NV_IDENT|NV_ASSIGN);
}

/* 
 * Remove freeable local space associated with the nvalue field
 * of nnod. This includes any strings representing the value(s) of the
 * node, as well as its dope vector, if it is an array.
 */

void	sh_envnolocal (register Namval_t *np, void *data)
{
	NOT_USED(data);
	if(nv_isattr(np,NV_EXPORT|NV_NOFREE))
	{
		if(nv_isref(np))
		{
			nv_offattr(np,NV_NOFREE|NV_REF);
			np->nvalue.cp = 0;
			np->nvfun = 0;
		}
		return;
	}
	if(nv_isarray(np))
		nv_putsub(np,NIL(char*),ARRAY_SCAN);
	_nv_unset(np,NV_RDONLY);
	nv_setattr(np,0);
}

/*
 * Currently this is a dummy, but someday will be needed
 * for reference counting
 */
void	nv_close(Namval_t *np)
{
	NOT_USED(np);
}

static void table_unset(register Dt_t *root, int flags, Dt_t *oroot)
{
	register Namval_t *np,*nq;
	for(np=(Namval_t*)dtfirst(root);np;np=nq)
	{
		_nv_unset(np,flags);
#ifdef _ENV_H
		if(oroot && (nq=nv_search(nv_name(np),oroot,0)) && nv_isattr(nq,NV_EXPORT))
			env_put(sh.env,nq);
#endif
		nq = (Namval_t*)dtnext(root,np);
		dtdelete(root,np);
		free((void*)np);
	}
}

/*
 *
 *   Set the value of <np> to 0, and nullify any attributes
 *   that <np> may have had.  Free any freeable space occupied
 *   by the value of <np>.  If <np> denotes an array member, it
 *   will retain its attributes.
 *   <flags> can contain NV_RDONLY to override the readonly attribute
 *	being cleared.
 */
void	_nv_unset(register Namval_t *np,int flags)
{
	register union Value *up = &np->nvalue;
	if(!(flags&NV_RDONLY) && nv_isattr (np,NV_RDONLY))
		errormsg(SH_DICT,ERROR_exit(1),e_readonly, nv_name(np));
	if(nv_istable(np) && !np->nvfun)
	{
		table_unset(np->nvalue.hp,flags,(Dt_t*)0);
		if(!nv_isattr(np,NV_NOFREE))
			dtclose(np->nvalue.hp);
		goto done;
	}
	if(sh.subshell && !nv_isnull(np))
		np = sh_assignok(np,0);
	nv_offattr(np,NV_NODISC);
	if(np->nvfun && !nv_isref(np))
	{
		/* This function contains disc */
		if(!local)
		{
			local=1;
			nv_putv(np,NIL(char*),flags,np->nvfun);
			return;
		}
		/* called from disc, assign the actual value */
		local=0;
	}
	if((!nv_isattr (np, NV_NOFREE)) && up->cp)
		free((void*)up->cp);
	up->cp = 0;
done:
#ifdef _ENV_H
	if(nv_isattr(np,NV_EXPORT))
	{
		char *sub;
		if(!nv_isarray(np) || (sub=nv_getsub(np)) && strcmp(sub,"0")==0)
			env_delete(sh.env,nv_name(np));
	}
#endif
	if(!nv_isarray(np) || !nv_arrayptr(np))
	{
		if(nv_isref(np))
		{
			if(np->nvenv)
				free((void*)np->nvenv);
			np->nvfun = 0;
		}
		nv_setsize(np,0);
		if(!nv_isattr(np,NV_MINIMAL) || nv_isattr(np,NV_EXPORT))
		{
			np->nvenv = 0;
			nv_setattr(np,0);
		}
		else
			nv_setattr(np,NV_MINIMAL);
	}
}

void	nv_unset(register Namval_t *np)
{
	_nv_unset(np,0);
}

/*
 * return the node pointer in the highest level scope
 */
Namval_t *nv_scoped(register Namval_t *np)
{
	if(!dtvnext(sh.var_tree))
		return(np);
	return(dtsearch(sh.var_tree,np));
}

/*
 * return space separated list of names of variables in given tree
 */
static char *tableval(Dt_t *root)
{
	static Sfio_t *out;
	register Namval_t *np;
	register int first=1;
	register Dt_t *base = dtview(root,0);
        if(out)
                sfseek(out,(Sfoff_t)0,SEEK_SET);
        else
                out =  sfnew((Sfio_t*)0,(char*)0,-1,-1,SF_WRITE|SF_STRING);
	for(np=(Namval_t*)dtfirst(root);np;np=(Namval_t*)dtnext(root,np))
	{
                if(!nv_isnull(np) || np->nvfun || nv_isattr(np,~NV_NOFREE))
		{
			if(!first)
				sfputc(out,' ');
			else
				first = 0;
			sfputr(out,np->nvname,-1);
		}
	}
	sfputc(out,0);
	if(base)
		dtview(root,base);
	return((char*)out->_data);
}

#ifdef SHOPT_OPTIMIZE
struct optimize
{
	Namfun_t	hdr;
	char		**ptr;
	struct optimize	*next;
	Namval_t	*np;
};

static struct optimize *opt_free;

static void optimize_clear(Namval_t* np, Namfun_t *fp)
{
	struct optimize *op = (struct optimize*)fp;
	nv_stack(np,fp);
	nv_stack(np,(Namfun_t*)0);
	for(;op && op->np==np; op=op->next)
	{
		if(op->ptr)
		{
			*op->ptr = 0;
			op->ptr = 0;
		}
	}
}

static void put_optimize(Namval_t* np,const char *val,int flags,Namfun_t *fp)
{
	nv_putv(np,val,flags,fp);
	optimize_clear(np,fp);
}

static const Namdisc_t optimize_disc  = {  0, put_optimize };

void nv_optimize(Namval_t *np)
{
	register Namfun_t *fp;
	register struct optimize *op, *xp;
	if(sh.argaddr)
	{
		for(fp=np->nvfun; fp; fp = fp->next)
		{
			if(fp->disc->getnum || fp->disc->getval)
			{
				sh.argaddr = 0;
				return;
			}
			if(fp->disc== &optimize_disc)
				break;
		}
		if((xp= (struct optimize*)fp) && xp->ptr==sh.argaddr)
			return;
		if(op = opt_free)
			opt_free = op->next;
		else
			op=(struct optimize*)malloc(sizeof(struct optimize));
		op->ptr = sh.argaddr;
		op->np = np;
		if(xp)
		{
			op->hdr.disc = 0;
			op->next = xp->next;
			xp->next = op;
		}
		else
		{
			op->hdr.disc = &optimize_disc;
			op->next = (struct optimize*)sh.optlist;
			sh.optlist = (void*)op;
			nv_stack(np,&op->hdr);
		}
	}
}

void sh_optclear(Shell_t *shp, void *old)
{
	register struct optimize *op,*opnext;
	for(op=(struct optimize*)shp->optlist; op; op = opnext)
	{
		opnext = op->next;
		if(op->ptr && op->hdr.disc)
		{
			nv_stack(op->np,&op->hdr);
			nv_stack(op->np,(Namfun_t*)0);
		}
		op->next = opt_free;
		opt_free = op;
	}
	shp->optlist = old;
}

#else
#   define	optimize_clear(np,fp)
#endif /* SHOPT_OPTIMIZE */

/*
 *   Return a pointer to a character string that denotes the value
 *   of <np>.  If <np> refers to an array,  return a pointer to
 *   the value associated with the current index.
 *
 *   If the value of <np> is an integer, the string returned will
 *   be overwritten by the next call to nv_getval.
 *
 *   If <np> has no value, 0 is returned.
 */

char *nv_getval(register Namval_t *np)
{
	register union Value *up= &np->nvalue;
	register int numeric;
#ifdef SHOPT_OPTIMIZE
	if(!local && sh.argaddr)
		nv_optimize(np);
#endif /* SHOPT_OPTIMIZE */
	if(!np->nvfun && !nv_isattr(np,NV_ARRAY|NV_INTEGER|NV_FUNCT|NV_REF|NV_TABLE))
		goto done;
	if(nv_isref(np))
	{
		sh.last_table = nv_table(np);
		return((nv_name)(up->np));
	}
	if(np->nvfun)
	{
		if(!local)
		{
			local=1;
			return(nv_getv(np, np->nvfun));
		}
		local=0;
	}
	if(nv_istable(np))
		return(tableval(np->nvalue.hp));
	numeric = ((nv_isattr (np, NV_INTEGER)) != 0);
	if(numeric)
	{
		Sflong_t  ll;
		if(!up->cp)
			return((char*)0);
		if(nv_isattr (np,NV_DOUBLE))
		{
			Sfdouble_t ld;
			double d;
			char *format;
			long l = nv_size(np)+8;
			if(!curbuf)
				curbuf = (char*)malloc(maxbufsize=l);
			else if(l > maxbufsize)
				curbuf = (char*)realloc(curbuf,maxbufsize=l);
			if(nv_isattr(np,NV_LONG))
			{
				ld = *up->ldp;
				if(nv_isattr (np,NV_EXPNOTE))
					format = "%.*Lg\0";
				else
					format = "%.*Lf\0";
				sfsprintf(curbuf,l,format,nv_size(np),ld);
			}
			else
			{
				d = *up->dp;
				if(nv_isattr (np,NV_EXPNOTE))
					format = "%.*g\0";
				else
					format = "%.*f\0";
				sfsprintf(curbuf,l,format,nv_size(np),d);
			}
			return(curbuf);
		}
        	else if(nv_isattr (np,NV_LONG))
			ll = *up->llp;
        	else if(nv_isattr (np,NV_SHORT))
			ll = up->s;
		else if(nv_isattr(np,NV_UNSIGN))
			ll = (unsigned long)*(up->lp);
        	else
			ll = *(up->lp);
		if((numeric=nv_size(np))==10)
		{
			if(nv_isattr(np,NV_UNSIGN))
			{
				char *cp=fmtbuf(36);
				sfsprintf(cp,36,"%I*u",sizeof(ll),ll);
				return(cp);
			}
			numeric = 0;
		}
		return(fmtbase((long)ll,numeric, numeric&&numeric!=10));
	}
done:
#ifdef SHOPT_OO
	/* This is not nearly complete */
	if(!up->cp && (np=nv_class(np)))
		return(nv_getval(np));
#endif /* SHOPT_OO */
	return ((char*)up->cp);
}

double nv_getnum(register Namval_t *np)
{
	register union Value *up;
	register double r=0;
	register char *str;
#ifdef SHOPT_OPTIMIZE
	if(!local && sh.argaddr)
		nv_optimize(np);
#endif /* SHOPT_OPTIMIZE */
     	if(np->nvfun)
	{
		if(!local)
		{
			local=1;
			return(nv_getn(np, np->nvfun));
		}
		local=0;
	}
     	if(nv_isattr (np, NV_INTEGER))
	{
		up= &np->nvalue;
		if(!up->lp)
			r = 0;
		else if(nv_isattr(np, NV_DOUBLE))
                       	r = *up->dp;
		else if(nv_isattr(np, NV_UNSIGN))
                       	r = *((unsigned long*)up->lp);
		else
                       	r = *up->lp;
	}
	else if((str=nv_getval(np)) && *str!=0)
	{
	     	if(nv_isattr (np, NV_ZFILL))
		{
			while(*str=='0')
				str++;
		}
		r = sh_arith(str);
	}
	return(r);
}
/*
 *   Give <np> the attributes <newatts,> and change its current
 *   value to conform to <newatts>.  The <size> of left and right
 *   justified fields may be given.
 */
void nv_newattr (register Namval_t *np, unsigned newatts, int size)
{
	register char *sp;
	register char *cp = 0;
	register unsigned int n;
	Namarr_t *ap = 0;
	int oldsize,oldatts;

	/* check for restrictions */
	if(sh_isoption(SH_RESTRICTED) && ((sp=nv_name(np))==nv_name(PATHNOD) || sp==nv_name(SHELLNOD) || sp==nv_name(ENVNOD) ))
		errormsg(SH_DICT,ERROR_exit(1),e_restricted,nv_name(np));
	/* handle attributes that do not change data separately */
	n = np->nvflag;
#ifdef SHOPT_BSH
	if(newatts&NV_EXPORT)
		nv_offattr(np,NV_IMPORT);
#endif /* SHOPT_BSH */
#ifdef _ENV_H
	if(((n^newatts)&NV_EXPORT))
	/* record changes to the environment */
	{
		if(n&NV_EXPORT)
			env_delete(sh.env,nv_name(np));
		else
			env_put(sh.env,np);
	}
#endif
	if((size==0||(n&NV_INTEGER)) && ((n^newatts)&~NV_NOCHANGE)==0)
	{
		if(size)
			nv_setsize(np,size);
		nv_offattr(np, ~NV_NOFREE);
		nv_onattr(np, newatts);
		return;
	}
	/* for an array, change all the elements */
	if((ap=nv_arrayptr(np)) && ap->nelem>0)
		nv_putsub(np,NIL(char*),ARRAY_SCAN);
	oldsize = nv_size(np);
	oldatts = np->nvflag;
	if(ap) /* add element to prevent array deletion */
		ap->nelem++;
	do
	{
		nv_setsize(np,oldsize);
		np->nvflag = oldatts;
		if (sp = nv_getval(np))
 		{
			if(nv_isattr(np,NV_ZFILL))
				while(*sp=='0') sp++;
			cp = (char*)malloc((n=strlen (sp)) + 1);
			strcpy(cp, sp);
			if(ap)
			{
				Namval_t *mp;
				ap->nelem &= ~ARRAY_SCAN;
				if(mp=nv_opensub(np))
					nv_onattr(mp,NV_NOFREE);
			}
			nv_unset(np);
			if(ap)
				ap->nelem |= ARRAY_SCAN;
			if(size==0 && (newatts&(NV_LJUST|NV_RJUST|NV_ZFILL)))
				size = n;
		}
		else
			nv_unset(np);
		nv_setsize(np,size);
		np->nvflag &= NV_ARRAY;
		np->nvflag |= newatts;
		if (cp)
		{
			nv_putval (np, cp, NV_RDONLY);
			free(cp);
		}
	}
	while(ap && nv_nextsub(np));
	if(ap)
		ap->nelem--;
	return;
}

#ifndef _NEXT_SOURCE
static char *oldgetenv(const char *string)
{
	register char c0,c1;
	register const char *cp, *sp;
	register char **av = environ;
	if(!string || (c0= *string)==0)
		return(0);
	if((c1=*++string)==0)
		c1= '=';
	while(cp = *av++)
	{
		if(cp[0]!=c0 || cp[1]!=c1) 
			continue;
		sp = string;
		while(*sp && *sp++ == *++cp);
		if(*sp==0 && *++cp=='=')
			return((char*)(cp+1));
	}
	return(0);
}

/*
 * This version of getenv the hash storage to access environment values
 */
char *getenv(const char *name)
/*@
	assume name!=0;
@*/ 
{
	register Namval_t *np;
	if(!sh.var_tree)
		return(oldgetenv(name));
	if((np = nv_search(name,sh.var_tree,0)) && nv_isattr(np,NV_EXPORT))
		return(nv_getval(np));
	return(0);
}
#endif /* _NEXT_SOURCE */

#undef putenv
/*
 * This version of putenv uses the hash storage to assign environment values
 */
int putenv(const char *name)
{
	register Namval_t *np;
	if(name)
	{
		np = nv_open(name,sh.var_tree,NV_EXPORT|NV_IDENT|NV_NOARRAY|NV_ASSIGN);
		if(!strchr(name,'='))
			nv_unset(np);
	}
	return(0);
}


/*
 * Override libast setenv()
 */
char* setenviron(const char *name)
{
	register Namval_t *np;
	if(name)
	{
		np = nv_open(name,sh.var_tree,NV_EXPORT|NV_IDENT|NV_NOARRAY|NV_ASSIGN);
		if(strchr(name,'='))
			return(nv_getval(np));
		nv_unset(np);
	}
	return("");
}

/*
 * copy <str1> to <str2> changing lower case to upper case
 * <str2> must be big enough to hold <str1>
 * <str1> and <str2> may point to the same place.
 */

static void ltou(register char const *str1,register char *str2)
/*@
	assume str1!=0 && str2!=0;
	return x satisfying strlen(in str1)==strlen(in str2);
@*/ 
{
	register int c;
	for(; c= *((unsigned char*)str1); str1++,str2++)
	{
		if(islower(c))
			*str2 = toupper(c);
		else
			*str2 = c;
	}
	*str2 = 0;
}

/*
 * Push or pop getval and putval functions to node <np>
 * <fp> is null to pop, otherwise, <fp> is pushed onto stack
 * The top of the stack is returned
 */
Namfun_t *nv_disc(register Namval_t *np, register Namfun_t* fp, int flags)
{
	Namfun_t *lp, **lpp;
	if(fp)
	{
		if((lp=np->nvfun)==fp)
		{
			if(flags==NV_FIRST || flags==0)
				return(fp);
			np->nvfun = lp->next;
			if(flags==NV_POP)
				return(fp);
		}
		/* see if <fp> is on the list already */
		lpp = &np->nvfun;
		if(lp)
		{
			while(lp->next)
			{
				if(lp->next==fp)
				{
					lp->next = fp->next;
					if(flags==NV_POP)
						return(fp);
					if(flags!=NV_LAST)
						break;
				}
				lp = lp->next;
			}
			if(flags==NV_LAST)
				lpp = &lp;
		}
		if(flags==NV_POP)
			return(0);
		/* push */
		nv_offattr(np,NV_NODISC);
		fp->nofree = 0;
		fp->next = *lpp;
		*lpp = fp;
	}
	else
	{
		if(flags==NV_FIRST)
			return(np->nvfun);
		else if(flags==NV_LAST)
			for(lp=np->nvfun; lp; fp=lp,lp=lp->next);
		else if(fp = np->nvfun)
			np->nvfun = fp->next;
	}
	return(fp);
}

/*
 * add or replace built-in version of command commresponding to <path>
 * The <bltin> argument is a pointer to the built-in
 * if <extra>==1, the built-in will be deleted
 * Special builtins cannot be added or deleted return failure
 * The return value for adding builtins is a pointer to the node or NULL on
 *   failure.  For delete NULL means success and the node that cannot be
 *   deleted is returned on failure.
 */
Namval_t *sh_addbuiltin(const char *path, int (*bltin)(int, char*[],void*),void *extra)
{
	register const char *cp, *name = path_basename(path);
	register Namval_t *np, *nq=0;
	if(name==path && (cp=strchr(name,'.')) && cp!=name)
	{
		int offset = staktell();
		stakwrite(name,cp-name);
		stakputc(0);
		nq=nv_open(stakptr(offset),0,NV_VARNAME|NV_NOARRAY);
		offset = staktell();
		stakputs(nv_name(nq));
		stakputs(cp);
		stakputc(0);
		path = name = stakptr(offset);
	}
	if(np = nv_search(path,sh.bltin_tree,0))
	{
		/* exists without a path */
		if(extra == (void*)1)
		{
			dtdelete(sh.bltin_tree,np);
				return(0);
		}
		if(!bltin)
			return(np);
	}
	else for(np=(Namval_t*)dtfirst(sh.bltin_tree);np;np=(Namval_t*)dtnext(sh.bltin_tree,np))
	{
		if(strcmp(name,path_basename(nv_name(np))))
			continue;
		/* exists probably with different path so delete it */
		if(strcmp(path,nv_name(np)))
		{
			if(nv_isattr(np,BLT_SPC))
				return(np);
			if(!bltin)
				bltin = np->nvalue.bfp;
			dtdelete(sh.bltin_tree,np);
			if(extra == (void*)1)
				return(0);
			np = 0;
		}
		break;
	}
	if(!np && !(np = nv_search(path,sh.bltin_tree,bltin?NV_ADD:0)))
		return(0);
	if(nv_isattr(np,BLT_SPC))
		return(np);
	np->nvenv = 0;
	np->nvfun = 0;
	nv_setattr(np,0);
	if(bltin)
	{
		np->nvalue.bfp = bltin;
		nv_onattr(np,NV_BLTIN);
		np->nvfun = (Namfun_t*)extra;
	}
	if(nq)
	{
		cp=nv_setdisc(nq,cp+1,np,(Namfun_t*)nq);
		nv_close(nq);
		if(!cp)
			errormsg(SH_DICT,ERROR_exit(1),e_baddisc,name);
	}
	if(extra == (void*)1)
		return(0);
	return(np);
}

/*
 * call the next getval function in the chain
 */
char *nv_getv(Namval_t *np, register Namfun_t *nfp)
{
	register Namfun_t	*fp;
	static char numbuf[20];
	register char *cp;
	if((fp = nfp) != NIL(Namfun_t*) && !local)
		fp = nfp = nfp->next;
	local=0;
	for(; fp; fp=fp->next)
	{
		if(!fp->disc->getnum && !fp->disc->getval)
			continue;
		if(!nv_isattr(np,NV_NODISC) || fp==(Namfun_t*)nv_arrayptr(np))
			break;
	}
	if(fp && fp->disc->getval)
		cp = (*fp->disc->getval)(np,fp);
	else if(fp && fp->disc->getnum)
		sfsprintf(cp=numbuf,sizeof(numbuf),"%.*g\0",12,(*fp->disc->getnum)(np,fp));
	else
	{
		local=1;
		cp = nv_getval(np);
	}
	return(cp);
}

/*
 * call the next getnum function in the chain
 */
double nv_getn(Namval_t *np, register Namfun_t *nfp)
{
	register Namfun_t	*fp;
	register double d=0;
	char *str;
	if((fp = nfp) != NIL(Namfun_t*) && !local)
		fp = nfp = nfp->next;
	local=0;
	for(; fp; fp=fp->next)
	{
		if(!fp->disc->getnum && !fp->disc->getval)
			continue;
		if(!nv_isattr(np,NV_NODISC) || fp==(Namfun_t*)nv_arrayptr(np))
			break;
	}
	if(fp && fp->disc->getnum)
		d = (*fp->disc->getnum)(np,fp);
	else if((str=nv_getv(np,fp?fp:nfp)) && *str!=0)
	{
	     	if(nv_isattr (np, NV_ZFILL))
		{
			while(*str=='0')
				str++;
		}
		d = sh_arith(str);
	}
	return(d);
}

/*
 * call the next assign function in the chain
 */
void nv_putv(Namval_t *np, const char *value, int flags, register Namfun_t *nfp)
{
	register Namfun_t	*fp;
	if((fp=nfp) != NIL(Namfun_t*) && !local)
		fp = nfp = nfp->next;
	local=0;
	for(; fp; fp=fp->next)
	{
		if(!fp->disc->putval)
			continue;
		if(!nv_isattr(np,NV_NODISC) || fp==(Namfun_t*)nv_arrayptr(np))
			break;
	}
	if(fp && nfp->disc->putval)
		(*fp->disc->putval)(np,value, flags, fp);
	else
	{
		local=1;
		if(value)
			nv_putval(np, value, flags);
		else
			_nv_unset(np, flags&NV_RDONLY);
	}
}

#define	LOOKUP		0
#define	ASSIGN		1
#define	UNASSIGN	2
#define BLOCKED		((Namval_t*)&local)

struct	vardisc
{
	Namfun_t	fun;
	Namval_t	*disc[3];
};

/*
 * free discipline if no more discipline functions
 */
static void chktfree(register Namval_t *np, register struct vardisc *vp)
{
	register int n;
	for(n=0; n< sizeof(vp->disc)/sizeof(*vp->disc); n++)
	{
		if(vp->disc[n])
			break;
	}
	if(n>=sizeof(vp->disc)/sizeof(*vp->disc))
	{
		/* no disc left so pop */
		Namfun_t *fp;
		if((fp=nv_stack(np, NIL(Namfun_t*))) && !fp->nofree)
			free((void*)fp);
	}
}

/*
 * This function performs an assignment disc on the given node <np>
 */
static void	assign(Namval_t *np,const char* val,int flags,Namfun_t *handle)
{
	register struct vardisc *vp = (struct vardisc*)handle;
	register Namval_t *nq, **disc;
	if(val)
	{
		if(!(nq=vp->disc[ASSIGN]))
		{
			nv_putv(np,val,flags,handle);
			return;
		}
		nv_putval(SH_VALNOD, val, (flags&NV_INTEGER)?NV_NOFREE|NV_INTEGER|NV_DOUBLE|NV_EXPNOTE:NV_NOFREE);
	}
	disc = (val? &(vp->disc[ASSIGN]):&(vp->disc[UNASSIGN]));
	if((nq= *disc) && nq!=BLOCKED)
	{
		*disc=BLOCKED;
		sh_fun(nq,np,(char**)0);
		if(*disc==BLOCKED)
			*disc=nq;
		else if(!*disc)
			chktfree(np,vp);
	}
	if(val)
	{
		register char *cp;
		double d;
		if(nv_isnull(SH_VALNOD))
			cp=0;
		else if(flags&NV_INTEGER)
		{
			d = nv_getnum(SH_VALNOD);
			cp = (char*)(&d);
		}
		else
			cp = nv_getval(SH_VALNOD);
		if(cp)
			nv_putv(np,cp,flags|NV_RDONLY,handle);
		nv_unset(SH_VALNOD);
	}
	else if(!nq || nq==BLOCKED)
	{
		if(vp->disc[ASSIGN])
			nv_unset(vp->disc[ASSIGN]);
		if(vp->disc[LOOKUP])
			nv_unset(vp->disc[LOOKUP]);
		if(vp->disc[UNASSIGN])
			nv_unset(vp->disc[UNASSIGN]);
		if((handle=nv_stack(np, NIL(Namfun_t*))) && !handle->nofree)
			free((void*)handle);
		nv_unset(np);
	}
}

/*
 * This function executes a lookup disc and then performs
 * the lookup on the given node <np>
 */
static char*	lookup(Namval_t *np, Namfun_t *handle)
{
	register struct vardisc *vp = (struct vardisc*)handle;
	register Namval_t *nq;
	register char *cp=0;
	if((nq=vp->disc[LOOKUP]) &&  nq!=BLOCKED)
	{
		nv_unset(SH_VALNOD);
		vp->disc[LOOKUP]=BLOCKED;
		sh_fun(nq,np,(char**)0);
		if(vp->disc[LOOKUP]==BLOCKED)
			vp->disc[LOOKUP]=nq;
		else if(!vp->disc[LOOKUP])
			chktfree(np,vp);
		cp = nv_getval(SH_VALNOD);
	}
	if(!cp)
		cp = nv_getv(np,handle);
	return(cp);
}

/*
 * node creation discipline
 */
Namval_t *nv_create(register Namval_t* np,const char *name,register int flags,Namfun_t *fp)
{
	if(np == (Namval_t*)fp)
		fp = np->nvfun;
	else if(fp)
		fp = fp->next;
	while(fp && !fp->disc->create)
		fp = fp->next;
	if(fp && fp->disc->create)
		return((*fp->disc->create)(np,name,flags,fp));
	return(NIL(Namval_t*));
}

static const Namdisc_t shdisc =
{
	sizeof(struct vardisc),
	assign,
	lookup
};


/*
 * Set disc on given <event> to <action>
 * If action==np, the current disc is returned
 * A null return value indicates that no <event> is known for <np>
 * If <event> is NULL, then return the event name after <action>
 * If <event> is NULL, and <action> is NULL, return the first event
 */
char *nv_setdisc(register Namval_t* np,register const char *event,Namval_t *action,register Namfun_t *fp)
{
	register struct vardisc *vp = (struct vardisc*)np->nvfun;
	register int type;
	if(np == (Namval_t*)fp)
	{
		static const char *discnames[] = { "get", "set", "unset", 0 };
		register const char *name;
		register int getname=0;
		/* top level call, check for get/set */
		if(!event)
		{
			if(!action)
				return((char*)discnames[0]);
			getname=1;
			event = (char*)action;
		}
		for(type=0; name=discnames[type]; type++)
		{
			if(strcmp(event,name)==0)
				break;
		}
		if(getname)
		{
			event = 0;
			if(name && !(name = discnames[++type]))
				action = 0;
		}
		if(!name)
		{
			if((fp=(Namfun_t*)vp) && fp->disc->setdisc)
				return((*fp->disc->setdisc)(np,event,action,fp));
		}
		else if(getname)
			return((char*)name);
	}
	if(!fp)
		return(NIL(char*));
	if(np != (Namval_t*)fp)
	{
		/* not the top level */
		while(fp = fp->next)
		{
			if(fp->disc->setdisc)
				return((*fp->disc->setdisc)(np,event,action,fp));
		}
		return(NIL(char*));
	}
	/* Handle GET/SET/UNSET disc */
	if(vp && vp->fun.disc->putval!=assign)
		vp = 0;
	if(!vp)
	{
		if(action==np)
			return((char*)action);
		if(!(vp = newof(NIL(struct vardisc*),struct vardisc,1,0)))
			return(0);
		vp->fun.disc = &shdisc;
		nv_stack(np, (Namfun_t*)vp);
	}
	if(action==np)
		action = vp->disc[type];
	else if(action)
		vp->disc[type] = action;
	else
	{
		action = vp->disc[type];
		vp->disc[type] = 0;
		if(action!=BLOCKED)
			chktfree(np,vp);
	}
	return(action?(char*)action:"");
}

/*
 * normalize <cp> and return pointer to subscript if any
 */
static char *lastdot(register char *cp)
{
	register char *dp=cp, *ep=0;
	register int c;
	while(c= *cp++)
	{
		*dp++ = c;
		if(c=='[')
			ep = cp;
		else if(c=='.')
		{
			if(*cp=='[')
			{
				ep = nv_endsubscript((Namval_t*)0,cp,0);
				c = ep-cp;
				memcpy(dp,cp,c);
				dp = sh_checkid(dp+1,dp+c);
				cp = ep;
			}
			ep = 0;
		}
	}
	*dp = 0;
	return(ep);
}

/*
 * Create a reference node from <np>
 */
void nv_setref(register Namval_t *np)
{
	register Namval_t *nq, *nr;
	register char *ep,*cp;
	Dt_t *hp=sh.var_tree;
	if(nv_isref(np))
		return;
	if(nv_isarray(np))
		errormsg(SH_DICT,ERROR_exit(1),e_badref,nv_name(np));
	if(!(cp=nv_getval(np)))
		errormsg(SH_DICT,ERROR_exit(1),e_noref,nv_name(np));
	ep = lastdot(cp);
	if(nv_isattr(np,NV_PARAM))
	{
		if(sh.st.prevst && !(hp=(Dt_t*)sh.st.prevst->save_tree))
		{
			if(!(hp=dtvnext(sh.var_tree)))
				hp = sh.var_tree;
		}
	}
	nr= nq = nv_open(cp, hp, NV_NOARRAY|NV_VARNAME|NV_NOREF);
	if(*cp=='.')
		sh.last_table = 0;
	while(nv_isref(nr))
	{
		sh.last_table = nv_table(nr);
		nr = nv_refnode(nr);
	}
	if(nr==np) 
	{
		if(sh.namespace && sh.namespace->nvalue.hp==hp)
			errormsg(SH_DICT,ERROR_exit(1),e_selfref,nv_name(np));
		/* bind to earlier scope, or add to global scope */
		if(!(hp=dtvnext(hp)) || (nq=nv_search((char*)np,hp,NV_ADD|HASH_BUCKET))==np)
			errormsg(SH_DICT,ERROR_exit(1),e_selfref,nv_name(np));
	}
	if(ep)
	{
		/* cause subscript evaluation and return result */
		nv_endsubscript(nq,ep,NV_ADD);
		ep = nv_getsub(nq);
	}
	nv_unset(np);
	np->nvalue.np = nq;
	if(ep)
		np->nvenv = strdup(ep);
	np->nvfun = (Namfun_t*)sh.last_table;
	nv_onattr(np,NV_REF|NV_NOFREE);
}

/*
 * get the scope corresponding to <index>
 * whence uses the same values as lseeek()
 */
Shscope_t *sh_getscope(int index, int whence)
{
	register struct sh_scoped *sp, *topmost;
	if(whence==SEEK_CUR)
		sp = &sh.st;
	else
	{
		if ((struct sh_scoped*)sh.topscope != sh.st.self)
			topmost = (struct sh_scoped*)sh.topscope;
		else
			topmost = &(sh.st);
		sp = topmost;
		if(whence==SEEK_SET)
		{
			int n =0;
			while(sp = sp->prevst)
				n++;
			index = n - index;
			sp = topmost;
		}
	}
	if(index < 0)
		return((Shscope_t*)0);
	while(index-- && (sp = sp->prevst));
	return((Shscope_t*)sp);
}

/*
 * make <scoped> the top scope and return previous scope
 */
Shscope_t *sh_setscope(Shscope_t *scope)
{
	Shscope_t *old = (Shscope_t*)sh.st.self;
	*sh.st.self = sh.st;
	sh.st = *((struct sh_scoped*)scope);
	sh.var_tree = scope->var_tree;
	return(old);
}

Namval_t *nv_search(const char *name, Dt_t *root, int mode)
{
	register Namval_t *np;
	register Dt_t *dp = 0;
	if(mode&HASH_NOSCOPE)
		dp = dtview(root,0);
	if(mode&HASH_BUCKET)
	{
		np = (void*)name;
		name = nv_name(np);
		np = dtsearch(root,np);
	}
	else
	{
		if(*name=='.' && root==sh.var_tree)
			root = sh.var_base;
		np = dtmatch(root,(void*)name);
	}
	if(!np && (mode&NV_ADD))
	{
		if(sh.namespace && !(mode&HASH_NOSCOPE) && root==sh.var_tree)
			root = sh.namespace->nvalue.hp;
		else if(!dp && !(mode&HASH_NOSCOPE))
		{
			register Dt_t *next;
			while(next=dtvnext(root))
				root = next;
		}
		np = (Namval_t*)dtinsert(root,newnode(name));
	}
	if(dp)
		dtview(root,dp);
	return(np);
}

void nv_unscope(void)
{
	register Dt_t *root = sh.var_tree;
	register Dt_t *dp = dtview(root,(Dt_t*)0);
	sh.var_tree=dp;
	table_unset(root,NV_RDONLY|NV_NOSCOPE,dp);
	dtclose(root);
}

/*
 * The inverse of creating a reference node
 */
void nv_unref(register Namval_t *np)
{
	Namval_t *nq = nv_refnode(np);
	if(!nv_isref(np))
		return;
	nv_offattr(np,NV_NOFREE|NV_REF);
	np->nvalue.cp = strdup(nv_name(nq=nv_refnode(np)));
	np->nvfun = 0;
#ifdef SHOPT_OPTIMIZE
	{
		Namfun_t *fp;
		for(fp=nq->nvfun; fp; fp = fp->next)
		{
			if(fp->disc== &optimize_disc)
			{
				optimize_clear(nq,fp);
				return;
			}
		}
	}
#endif
}

/*
 * These following are for binary compatibility with the old hash library
 * They will be removed someday
 */

#if defined(__IMPORT__) && defined(__EXPORT__)
#   define extern __EXPORT__
#endif

#undef	hashscope

extern Dt_t *hashscope(Dt_t *root)
{
	return(dtvnext(root));
}

#undef	hashfree

extern Dt_t	*hashfree(Dt_t *root)
{
	Dt_t *dp = dtvnext(root);
	dtclose(root);
	return(dp);
}

#undef	hashname

extern char	*hashname(void *obj)
{
	Namval_t *np = (Namval_t*)obj;
	return(np->nvname);
}

#undef	hashlook

extern void *hashlook(Dt_t *root, const char *name, int mode,int size)
{
	NOT_USED(size);
	return((void*)nv_search(name,root,mode));
}

#undef nv_name
char *nv_name(register Namval_t *np)
{
	register int len;
	register Namval_t *table;
	if(is_abuiltin(np) || is_afunction(np))
		return(np->nvname);
	if(np->nvfun && np->nvfun->disc && np->nvfun->disc->name)
		return((*np->nvfun->disc->name)(np,np->nvfun));
	if(!(table = sh.last_table) || *np->nvname=='.' || table==sh.namespace || np==sh.last_table)
		return(np->nvname);
	len = strlen(table->nvname) + strlen(np->nvname) + 2;
	if(!curbuf)
		curbuf = (char*)malloc(maxbufsize=len);
	else if(len > maxbufsize)
		curbuf = (char*)realloc(curbuf,maxbufsize=len);
	sfsprintf(curbuf,len,"%s.%s\0",table->nvname,np->nvname);
	return(curbuf);
}

#undef nv_context
/*
 * returns the data context for a builtin
 */
void *nv_context(Namval_t *np)
{
	return((void*)np->nvfun);
}

#undef nv_isnull
int nv_isnull(register Namval_t *np)
{
	return(!np->nvalue.cp && !np->nvfun);
}

#undef nv_setsize
int nv_setsize(register Namval_t *np, int size)
{
	int oldsize = nv_size(np);
	if(size>=0)
		np->nvsize = size;
	return(oldsize);
}

struct notify
{
	Namfun_t	hdr;
	char		**ptr;
};

static void put_notify(Namval_t* np,const char *val,int flags,Namfun_t *fp)
{
	struct notify *pp = (struct notify*)fp;
	nv_putv(np,val,flags,fp);
	nv_stack(np,fp);
	nv_stack(np,(Namfun_t*)0);
	*pp->ptr = 0;
	if(!fp->nofree)
		free((void*)fp);
}

static const Namdisc_t notify_disc  = {  0, put_notify };

int nv_unsetnotify(Namval_t *np, char **addr)
{
	register Namfun_t *fp;
	for(fp=np->nvfun;fp;fp=fp->next)
	{
		if(fp->disc->putval==put_notify && ((struct notify*)fp)->ptr==addr)
		{
			nv_stack(np,fp);
			nv_stack(np,(Namfun_t*)0);
			if(!fp->nofree)
				free((void*)fp);
			return(1);
		}
	}
	return(0);
}

int nv_setnotify(Namval_t *np, char **addr)
{
	struct notify *pp = newof(0,struct notify, 1,0);
	if(!pp)
		return(0);
	pp->ptr = addr;
	pp->hdr.disc = &notify_disc;
	nv_stack(np,&pp->hdr);
	return(1);
}

#ifdef future
struct	Vardisc
{
	Namfun_t	fun;
	const char	**discnames;
	Namval_t	*disc[1];
};


/*
 * Set disc on given <event> to <action>
 * If action==np, the current disc is returned
 * A null return value indicates that no <event> is known for <np>
 * If <event> is NULL, then return the event name after <action>
 * If <event> is NULL, and <action> is NULL, return the first event
 */
static char *setdisc(register Namval_t* np,register const char *event,Namval_t *action,register Namfun_t *fp)
{
	register struct Vardisc *vp = (struct Vardisc*)np->nvfun;
	register int type,getname=0;
	register const char *name;
	const char **discnames = vp->discnames;
	/* top level call, check for discipline match */
	if(!event)
	{
		if(!action)
			return((char*)discnames[0]);
		getname=1;
		event = (char*)action;
	}
	for(type=0; name=discnames[type]; type++)
	{
		if(strcmp(event,name)==0)
			break;
	}
	if(getname)
	{
		event = 0;
		if(name && !(name = discnames[++type]))
			action = 0;
	}
	if(!name)
		return(nv_setdisc(np,event,action,fp));
#if 0
	{
		if((fp=(Namfun_t*)vp) && fp->disc->setdisc)
			return((*fp->disc->setdisc)(np,event,action,fp));
	}
#endif
	else if(getname)
		return((char*)name);
	/* Handle the disciplines */
	if(action==np)
		action = vp->disc[type];
	else if(action)
		vp->disc[type] = action;
	else
	{
		action = vp->disc[type];
		vp->disc[type] = 0;
	}
	return(action?(char*)action:"");
}

static const Namdisc_t add_disc	= {   0, 0, 0, 0, setdisc };

int nv_adddisc(Namval_t *np, const char **names)
{
	register struct Vardisc *vp;
	register int n=0;
	register const char **av=names;
	while(*av++)
		n++;
	if(!(vp = newof(NIL(struct Vardisc*),struct Vardisc,1,n*sizeof(Namval_t*))))
		return(0);
	while(n>=0)
		vp->disc[n--] = 0;
	vp->fun.disc = &add_disc;
	vp->discnames = names; 
	nv_stack(np,&vp->fun);
	return(1);
}
#endif

#undef nv_stack
Namfun_t *nv_stack(register Namval_t *np, register Namfun_t* fp)
{
	return(nv_disc(np,fp,0));
}
