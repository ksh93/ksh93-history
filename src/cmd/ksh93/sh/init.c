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
 *
 * Shell initialization
 *
 *   David Korn
 *   AT&T Labs
 *   research!dgk
 *
 */

#include        "defs.h"
#include        <stak.h>
#include        <ctype.h>
#include        <ccode.h>
#include        "variables.h"
#include        "path.h"
#include        "fault.h"
#include        "name.h"
#include	"jobs.h"
#include	"io.h"
#include	"FEATURE/time"
#include	"FEATURE/dynamic"
#include	"lexstates.h"
#include	"FEATURE/locale"
#include	"national.h"

#ifdef SHOPT_MULTIBYTE
    char e_version[]	= "\n@(#)$Id: Version M 1993-12-28 l+ $\0\n";
#else
    char e_version[]	= "\n@(#)$Id: Version 1993-12-28 l+ $\0\n";
#endif /* SHOPT_MULTIBYTE */

#if _hdr_wchar && _lib_wctype && _lib_iswctype
#   include <wchar.h>
#   if _hdr_wctype
#	include <wctype.h>
#   endif
#   undef  isalpha
#   define isalpha(x)      iswalpha(x)
#   undef  isblank
#   define isblank(x)      iswblank(x)
#   if !defined(iswblank) && !_lib_iswblank

	static int
	iswblank(wchar_t wc)
	{
		static int      initialized;
		static wctype_t wt;

		if (!initialized)
		{
			initialized = 1;
			wt = wctype("blank");
		}
		return(iswctype(wc, wt));
	}
#   endif
#else
#   undef  _lib_wctype
#   ifndef isblank
#	define isblank(x)      ((x)==' '||(x)=='\t')
#   endif
#endif

#ifdef _hdr_locale
#   include	<locale.h>
#   ifndef LC_MESSAGES
#	define LC_MESSAGES	LC_ALL
#   endif /* LC_MESSAGES */
#endif /* _hdr_locale */

#define RANDMASK	0x7fff
#ifndef CLK_TCK
#   define CLK_TCK	60
#endif /* CLK_TCK */

#ifndef environ
    extern char	**environ;
#endif

struct seconds
{
	Namfun_t	hdr;
	Shell_t		*sh;
	double		sec_offset;
	char		*bufptr;
	int		maxbufsize;
};

struct rand
{
	Namfun_t	hdr;
	Shell_t		*sh;
	long		rand_last;
};

struct ifs
{
	Namfun_t	hdr;
	Shell_t		*sh;
	Namval_t	*ifsnp;
};

struct shell
{
	Namfun_t	hdr;
	Shell_t		*sh;
};

struct match
{
	Namfun_t	hdr;
	char		*val;
	char		*rval;
	int		vsize;
	int		nmatch;
	int		match[40];
};

typedef struct _init_
{
	Shell_t		*sh;
#ifdef SHOPT_FS_3D
	Namfun_t	VPATH_init;
#endif /* SHOPT_FS_3D */
	struct ifs	IFS_init;
	struct shell	PATH_init;
#ifdef PATH_BFPATH
	struct shell	FPATH_init;
	struct shell	CDPATH_init;
#endif
	struct shell	SHELL_init;
	struct shell	ENV_init;
	struct shell	VISUAL_init;
	struct shell	EDITOR_init;
	struct shell	OPTINDEX_init;
	struct seconds	SECONDS_init;
	struct rand	RAND_init;
	struct shell	LINENO_init;
	struct shell	L_ARG_init;
#ifdef _hdr_locale
	struct shell	LC_TYPE_init;
	struct shell	LC_NUM_init;
	struct shell	LC_COLL_init;
	struct shell	LC_MSG_init;
	struct shell	LC_ALL_init;
	struct shell	LANG_init;
	struct match	SH_MATCH_init;
#endif /* _hdr_locale */
#ifdef SHOPT_MULTIBYTE
	Namfun_t	CSWIDTH_init;
#endif /* SHOPT_MULTIBYTE */
} Init_t;

static void		env_init(Shell_t*);
static Init_t		*nv_init(Shell_t*);
static Dt_t		*inittree(Shell_t*,const struct shtable2*);

static const char	rsh_pattern[] = "@(rk|kr|r)sh";
static int		rand_shift;


/*
 * Invalidate all path name bindings
 */
static void rehash(register Namval_t *np,void *data)
{
	NOT_USED(data);
	nv_onattr(np,NV_NOALIAS);
}

/*
 * out of memory routine for stak routines
 */
static char *nospace(int unused)
{
	NOT_USED(unused);
	errormsg(SH_DICT,ERROR_exit(3),e_nospace);
	return(NIL(char*));
}

#ifdef SHOPT_MULTIBYTE
#   include	"edit.h"
    /* Trap for CSWIDTH variable */
    static void put_cswidth(Namval_t* np,const char *val,int flags,Namfun_t *fp)
    {
	if(ed_setwidth(val?val:"") && !(flags&NV_IMPORT))
		errormsg(SH_DICT,ERROR_exit(1),e_format,nv_name(np));
	nv_putv(np, val, flags, fp);
    }
#endif /* SHOPT_MULTIBYTE */

/* Trap for VISUAL and EDITOR variables */
static void put_ed(register Namval_t* np,const char *val,int flags,Namfun_t *fp)
{
	register const char *cp, *name=nv_name(np);
	if(*name=='E' && nv_getval(nv_scoped(VISINOD)))
		goto done;
	sh_offoption(SH_VI|SH_EMACS|SH_GMACS);
	if(!(cp=val) && (*name=='E' || !(cp=nv_getval(nv_scoped(EDITNOD)))))
		goto done;
	/* turn on vi or emacs option if editor name is either*/
	cp = path_basename(cp);
	if(strmatch(cp,"*vi"))
		sh_onoption(SH_VI);
	if(strmatch(cp,"*macs"))
	{
		if(*cp=='g')
			sh_onoption(SH_GMACS);
		else
			sh_onoption(SH_EMACS);
	}
done:
	nv_putv(np, val, flags, fp);
}

/* Trap for OPTINDEX */
static void put_optindex(Namval_t* np,const char *val,int flags,Namfun_t *fp)
{
	Shell_t *shp = ((struct shell*)fp)->sh;
	shp->st.opterror = shp->st.optchar = 0;
	nv_putv(np, val, flags, fp);
}

/* Trap for restricted variables FPATH, PATH, SHELL, ENV */
static void put_restricted(register Namval_t* np,const char *val,int flags,Namfun_t *fp)
{
	Shell_t *shp = ((struct shell*)fp)->sh;
#ifdef PATH_BFPATH
	Pathcomp_t *pp;
#endif
	if(!(flags&NV_RDONLY) && sh_isoption(SH_RESTRICTED))
		errormsg(SH_DICT,ERROR_exit(1),e_restricted,nv_name(np));
	if(nv_name(np)==nv_name(PATHNOD))			
	{
#ifndef PATH_BFPATH
		shp->lastpath = 0;
#endif
		nv_scan(shp->track_tree,rehash,(void*)0,NV_TAGGED,NV_TAGGED);
	}
	if(val && np->nvalue.cp && strcmp(val,np->nvalue.cp)==0)
		 return;
#ifdef PATH_BFPATH
	if(shp->defpathlist  && nv_name(np)==nv_name(FPATHNOD))
		shp->pathlist = (void*)path_unsetfpath((Pathcomp_t*)shp->pathlist);
#endif
	nv_putv(np, val, flags, fp);
#ifdef PATH_BFPATH
	if(shp->defpathlist)
	{
		val = np->nvalue.cp;
		if(nv_name(np)==nv_name(PATHNOD))
			pp = (void*)path_addpath((Pathcomp_t*)shp->pathlist,val,PATH_PATH);
		else if(val && nv_name(np)==nv_name(FPATHNOD))
			pp = (void*)path_addpath((Pathcomp_t*)shp->pathlist,val,PATH_FPATH);
		else
			return;
		if(shp->pathlist = (void*)pp)
			pp->shp = shp;
#if 0
sfprintf(sfstderr,"%d: name=%s val=%s\n",getpid(),nv_name(np),val);
path_dump((Pathcomp_t*)shp->pathlist);
#endif
	}
#endif
}

#ifdef PATH_BFPATH
static void put_cdpath(register Namval_t* np,const char *val,int flags,Namfun_t *fp)
{
	Pathcomp_t *pp;
	Shell_t *shp = ((struct shell*)fp)->sh;
	nv_putv(np, val, flags, fp);
	if(!shp->cdpathlist)
		return;
	val = np->nvalue.cp;
	pp = (void*)path_addpath((Pathcomp_t*)shp->cdpathlist,val,PATH_CDPATH);
	if(shp->cdpathlist = (void*)pp)
		pp->shp = shp;
}
#endif

#ifdef _hdr_locale
    /*
     * This function needs to be modified to handle international
     * error message translations
     */
#if ERROR_VERSION >= 20000101L
    static char* msg_translate(const char* catalog, const char* message)
    {
	NOT_USED(catalog);
	return((char*)message);
    }
#else
    static char* msg_translate(const char* message, int type)
    {
	NOT_USED(type);
	return((char*)message);
    }
#endif
#   ifdef SHOPT_MULTIBYTE
	void charsize_init(void)
	{
		static char fc[3] = { 0301,  ESS2, ESS3};
		char buff[8];
		register int i,n;
		wchar_t wc;
		memset(buff,0301,MB_CUR_MAX);
		for(i=0; i<=2; i++)
		{
			buff[0] = fc[i];
			if((n=mbtowc(&wc,buff,MB_CUR_MAX))>0)
			{
				if((int_charsize[i+1] = n-(i>0))>0)
					int_charsize[i+5] = wcwidth(wc);
				else
					int_charsize[i+5] = 0;
			}
		}
	}
#   endif /* SHOPT_MULTIBYTE */

    /* Trap for LC_ALL, LC_TYPE, LC_MESSAGES, LC_COLLATE and LANG */
    static void put_lang(Namval_t* np,const char *val,int flags,Namfun_t *fp)
    {
	int type;
	char *lc_all = nv_getval(LCALLNOD);
	if(nv_name(np)==nv_name(LCALLNOD))
		type = LC_ALL;
	else if(nv_name(np)==nv_name(LCTYPENOD))
		type = LC_CTYPE;
	else if(nv_name(np)==nv_name(LCMSGNOD))
		type = LC_MESSAGES;
	else if(nv_name(np)==nv_name(LCCOLLNOD))
		type = LC_COLLATE;
	else if(nv_name(np)==nv_name(LCNUMNOD))
		type = LC_NUMERIC;
	else if(nv_name(np)==nv_name(LANGNOD) && (!lc_all || *lc_all==0))
		type = LC_ALL;
	else
		type= -1;
	if(type>=0 && type!=LC_ALL && lc_all && *lc_all)
		type= -1;
	if(type>=0)
	{
		if(!setlocale(type,val?val:""))
		{
			errormsg(SH_DICT,0,e_badlocale,val);
			return;
		}
	}
	if(CC_NATIVE==CC_ASCII && (type==LC_ALL || type==LC_CTYPE))
	{
		if(sh_lexstates[ST_BEGIN]!=sh_lexrstates[ST_BEGIN])
			free((void*)sh_lexstates[ST_BEGIN]);
		if(ast.locale.set&(1<<AST_LC_CTYPE))
		{
			register int c;
			char *state[4];
			sh_lexstates[ST_BEGIN] = state[0] = (char*)malloc(4*(1<<CHAR_BIT));
			memcpy(state[0],sh_lexrstates[ST_BEGIN],(1<<CHAR_BIT));
			sh_lexstates[ST_NAME] = state[1] = state[0] + (1<<CHAR_BIT);
			memcpy(state[1],sh_lexrstates[ST_NAME],(1<<CHAR_BIT));
			sh_lexstates[ST_DOL] = state[2] = state[1] + (1<<CHAR_BIT);
			memcpy(state[2],sh_lexrstates[ST_DOL],(1<<CHAR_BIT));
			sh_lexstates[ST_BRACE] = state[3] = state[2] + (1<<CHAR_BIT);
			memcpy(state[3],sh_lexrstates[ST_BRACE],(1<<CHAR_BIT));
			for(c=0; c<(1<<CHAR_BIT); c++)
			{
				if(state[0][c]!=S_REG)
					continue;
				if(state[2][c]!=S_ERR)
					continue;
				if(isblank(c))
				{
					state[0][c]=0;
					state[1][c]=S_BREAK;
					state[2][c]=S_BREAK;
					continue;
				}
				if(!isalpha(c))
					continue;
				state[0][c]=S_NAME;
				if(state[1][c]==S_REG)
					state[1][c]=0;
				state[2][c]=S_ALP;
				if(state[3][c]==S_ERR)
					state[3][c]=0;
			}
		}
		else
		{
			sh_lexstates[ST_BEGIN]=(char*)sh_lexrstates[ST_BEGIN];
			sh_lexstates[ST_NAME]=(char*)sh_lexrstates[ST_NAME];
			sh_lexstates[ST_DOL]=(char*)sh_lexrstates[ST_DOL];
			sh_lexstates[ST_BRACE]=(char*)sh_lexrstates[ST_BRACE];
		}
#ifdef SHOPT_MULTIBYTE
        	charsize_init();
#endif /* SHOPT_MULTIBYTE */
	}
#if ERROR_VERSION < 20000101L
	if(type==LC_ALL || type==LC_MESSAGES)
		error_info.translate = msg_translate;
#endif
	nv_putv(np, val, flags, fp);
    }
#endif /* _hdr_locale */

/* Trap for IFS assignment and invalidates state table */
static void put_ifs(register Namval_t* np,const char *val,int flags,Namfun_t *fp)
{
	register struct ifs *ip = (struct ifs*)fp;
	ip->ifsnp = 0;
	nv_putv(np, val, flags, fp);
}

/*
 * This is the lookup function for IFS
 * It keeps the sh.ifstable up to date
 */
static char* get_ifs(register Namval_t* np, Namfun_t *fp)
{
	register struct ifs *ip = (struct ifs*)fp;
	register char *cp, *value;
	register int c,n;
	register Shell_t *shp = ip->sh;
	value = nv_getv(np,fp);
	if(np!=ip->ifsnp)
	{
		ip->ifsnp = np;
		memset(shp->ifstable,0,(1<<CHAR_BIT));
		if(cp=value)
		{
#ifdef SHOPT_MULTIBYTE
			while(n=mbtowc(0,cp,MB_CUR_MAX),c= *(unsigned char*)cp)
#else
			while(c= *(unsigned char*)cp++)
#endif /* SHOPT_MULTIBYTE */
			{
#ifdef SHOPT_MULTIBYTE
				cp+=n;
				if(n>1)
				{
					shp->ifstable[c] = S_MBYTE;
					continue;
				}
#endif /* SHOPT_MULTIBYTE */
				n = S_DELIM;
				if(c== *cp)
					cp++;
				else if(isspace(c))
					n = S_SPACE;
				shp->ifstable[c] = n;
			}
		}
		else
		{
			shp->ifstable[' '] = shp->ifstable['\t'] = S_SPACE;
			shp->ifstable['\n'] = S_NL;
		}
	}
	return(value);
}

/*
 * these functions are used to get and set the SECONDS variable
 */
#ifdef timeofday
#   define dtime(tp) ((double)((tp)->tv_sec)+1e-6*((double)((tp)->tv_usec)))
#   define tms	timeval
#else
#   define dtime(tp)	(((double)times(tp))/sh.lim.clk_tck)
#   define timeofday(a)
#endif

static void put_seconds(register Namval_t* np,const char *val,int flags,Namfun_t *fp)
{
	struct seconds *sp = (struct seconds*)fp;
	double d;
	struct tms tp;
	if(!val)
	{
		nv_stack(np, NIL(Namfun_t*));
		nv_unset(np);
		return;
	}
	if(flags&NV_INTEGER)
		d = *(double*)val;
	else
		d = sh_arith(val);
	timeofday(&tp);
	sp->sec_offset = dtime(&tp)-d;
	if(!np->nvalue.dp)
	{
		nv_setsize(np,3);
		np->nvalue.dp = &sp->sec_offset;
	}
}

static char* get_seconds(register Namval_t* np, Namfun_t *fp)
{
	static char *bufptr;
	static int maxbufsize;
	register int places = nv_size(np);
	struct tms tp;
	double d;
	NOT_USED(fp);
	timeofday(&tp);
	d = dtime(&tp)- *np->nvalue.dp;
	if(!bufptr)
		bufptr = (char*)malloc(maxbufsize=places+8);
	else if(places+8 > maxbufsize)
		bufptr = (char*)realloc(bufptr,maxbufsize=places+8);
	sfsprintf(bufptr,maxbufsize,"%.*f\0",places,d);
	return(bufptr);
}

static double nget_seconds(register Namval_t* np, Namfun_t *fp)
{
	struct tms tp;
	NOT_USED(fp);
	timeofday(&tp);
	return(dtime(&tp)- *np->nvalue.dp);
}

/*
 * These three functions are used to get and set the RANDOM variable
 */
static void put_rand(register Namval_t* np,const char *val,int flags,Namfun_t *fp)
{
	struct rand *rp = (struct rand*)fp;
	register long n;
	if(!val)
	{
		nv_stack(np, NIL(Namfun_t*));
		nv_unset(np);
		return;
	}
	if(flags&NV_INTEGER)
		n = *(double*)val;
	else
		n = sh_arith(val);
	srand((int)(n&RANDMASK));
	rp->rand_last = -1;
	if(!np->nvalue.lp)
		np->nvalue.lp = &rp->rand_last;
}

/*
 * get random number in range of 0 - 2**15
 * never pick same number twice in a row
 */
static double nget_rand(register Namval_t* np, Namfun_t *fp)
{
	register long cur, last= *np->nvalue.lp;
	NOT_USED(fp);
	do
		cur = (rand()>>rand_shift)&RANDMASK;
	while(cur==last);
	*np->nvalue.lp = cur;
	return((double)cur);
}

static char* get_rand(register Namval_t* np, Namfun_t *fp)
{
	register long n = nget_rand(np,fp);
	return(fmtbase(n, 10, 0));
}

/*
 * These three routines are for LINENO
 */
static double nget_lineno(Namval_t* np, Namfun_t *fp)
{
	double d=1;
	if(error_info.line >0)
		d = error_info.line;
	else if(error_info.context && error_info.context->line>0)
		d = error_info.context->line;
	NOT_USED(np);
	NOT_USED(fp);
	return(d);
}

static void put_lineno(Namval_t* np,const char *val,int flags,Namfun_t *fp)
{
	register long n;
	Shell_t *shp = ((struct shell*)fp)->sh;
	if(!val)
	{
		nv_stack(np, NIL(Namfun_t*));
		nv_unset(np);
		return;
	}
	if(flags&NV_INTEGER)
		n = *(double*)val;
	else
		n = sh_arith(val);
	shp->st.firstline += nget_lineno(np,fp)+1-n;
}

static char* get_lineno(register Namval_t* np, Namfun_t *fp)
{
	register long n = nget_lineno(np,fp);
	return(fmtbase(n, 10, 0));
}

static char* get_lastarg(Namval_t* np, Namfun_t *fp)
{
	NOT_USED(np);
	return(sh.lastarg);
}

static void put_lastarg(Namval_t* np,const char *val,int flags,Namfun_t *fp)
{
	char numbuf[20];
	if(flags&NV_INTEGER)
	{
		sfsprintf(numbuf,sizeof(numbuf),"%.*g\0",12,*((double*)val));
		val = numbuf;
	}
	if(sh.lastarg && !nv_isattr(np,NV_NOFREE))
		free((void*)sh.lastarg);
	else
		nv_offattr(np,NV_NOFREE);
	if(val)
		sh.lastarg = strdup(val);
	else
		sh.lastarg = 0;
}

static int hasgetdisc(register Namfun_t *fp)
{
        while(fp && !fp->disc->getnum && !fp->disc->getval)
                fp = fp->next;
	return(fp!=0);
}

/*
 * store the most recent value for use in .sh.match
 */
void sh_setmatch(const char *v, int vsize, int nmatch, int match[])
{
	struct match *mp = (struct match*)(SH_MATCHNOD->nvfun);
	if(mp->nmatch = nmatch)
	{
		memcpy(mp->match,match,nmatch*2*sizeof(int));
		if(vsize > mp->vsize)
		{
			if(mp->vsize)
				mp->val = (char*)realloc(mp->val,vsize+1);
			else
				mp->val = (char*)malloc(vsize+1);
			mp->vsize = vsize;
		}
		memcpy(mp->val,v,vsize);
		mp->val[vsize] = 0;
	}
} 

#define array_scan(np)	((nv_arrayptr(np)->nelem&ARRAY_SCAN))

static char* get_match(register Namval_t* np, Namfun_t *fp)
{
	struct match *mp = (struct match*)fp;
	int sub,n;
	char *val;
	if(mp->rval)
	{
		free((void*)mp->val);
		mp->rval = 0;
	}
	sub = nv_aindex(np);
	if(sub>=mp->nmatch)
		return(0);
	n = mp->match[2*sub+1]-mp->match[2*sub];
	if(n<=0)
		return("");
	val = mp->val+mp->match[2*sub];
	if(mp->val[mp->match[2*sub+1]]==0)
		return(val);
	mp->rval = (char*)malloc(n+1);
	memcpy(mp->rval,val,n);
	mp->rval[n] = 0;
	return(mp->rval);
}

static const Namdisc_t SH_MATCH_disc  = {  0, 0, get_match };

#ifdef SHOPT_FS_3D
    /*
     * set or unset the mappings given a colon separated list of directories
     */
    static void vpath_set(char *str, int mode)
    {
	register char *lastp, *oldp=str, *newp=strchr(oldp,':');
	if(!sh.lim.fs3d)
		return;
	while(newp)
	{
		*newp++ = 0;
		if(lastp=strchr(newp,':'))
			*lastp = 0;
		mount((mode?newp:""),oldp,FS3D_VIEW,0);
		newp[-1] = ':';
		oldp = newp;
		newp=lastp;
	}
    }

    /* catch vpath assignments */
    static void put_vpath(register Namval_t* np,const char *val,int flags,Namfun_t *fp)
    {
	register char *cp;
	if(cp = nv_getval(np))
		vpath_set(cp,0);
	if(val)
		vpath_set((char*)val,1);
	nv_putv(np,val,flags,fp);
    }
    static const Namdisc_t VPATH_disc	= { 0, put_vpath  };
    static Namfun_t VPATH_init	= { &VPATH_disc  };
#endif /* SHOPT_FS_3D */

static const Namdisc_t IFS_disc		= {  0, put_ifs, get_ifs };
static const Namdisc_t RESTRICTED_disc	= {  0, put_restricted };
#ifdef PATH_BFPATH
static const Namdisc_t CDPATH_disc	= {  0, put_cdpath }; 
#endif
static const Namdisc_t EDITOR_disc	= {  0, put_ed };
static const Namdisc_t OPTINDEX_disc	= {  0, put_optindex };
static const Namdisc_t SECONDS_disc	= {  0, put_seconds, get_seconds, nget_seconds };
static const Namdisc_t RAND_disc	= {  0, put_rand, get_rand, nget_rand };
static const Namdisc_t LINENO_disc	= {  0, put_lineno, get_lineno, nget_lineno };
static const Namdisc_t L_ARG_disc	= {  0, put_lastarg, get_lastarg };

#ifdef SHOPT_NAMESPACE
    static char* get_nspace(Namval_t* np, Namfun_t *fp)
    {
	if(sh.namespace)
		return(nv_name(sh.namespace));
	return((char*)np->nvalue.cp);
    }
    static const Namdisc_t NSPACE_disc	= {  0, 0, get_nspace };
    static Namfun_t NSPACE_init	= {  &NSPACE_disc};
#endif /* SHOPT_NAMESPACE */

#ifdef _hdr_locale
    static const Namdisc_t LC_disc	= {  0, put_lang };
#endif /* _hdr_locale */
#ifdef SHOPT_MULTIBYTE
    static const Namdisc_t CSWIDTH_disc	= {  0, put_cswidth };
#endif /* SHOPT_MULTIBYTE */

/*
 * This function will get called whenever a configuration parameter changes
 */
static int newconf(const char *name, const char *path, const char *value)
{
	register char *arg;
	if(!name)
		setenviron(value);
	else if(strcmp(name,"UNIVERSE")==0 && strcmp(astconf(name,0,0),value))
	{
		sh.universe = 0;
		/* set directory in new universe */
		if(*(arg = path_pwd(0))=='/')
			chdir(arg);
		/* clear out old tracked alias */
		stakseek(0);
		stakputs(nv_getval(PATHNOD));
		stakputc(0);
		nv_putval(PATHNOD,stakseek(0),NV_RDONLY);
	}
	return(1);
}

#if	(CC_NATIVE != CC_ASCII)
    static void a2e(char *d, const char *s)
    {
	register const unsigned char *t;
	register int i;
	t = CCMAP(CC_ASCII, CC_NATIVE);
	for(i=0; i<(1<<CHAR_BIT); i++)
		d[t[i]] = s[i];
    }

    static void init_ebcdic(void)
    {
	int i;
	char *cp = (char*)malloc(ST_NONE*(1<<CHAR_BIT));
	for(i=0; i < ST_NONE; i++)
	{
		a2e(cp,sh_lexrstates[i]);
		sh_lexstates[i] = cp;
		cp += (1<<CHAR_BIT);
	}
    }
#endif

/*
 * initialize the shell
 */
int sh_init(register int argc,register char *argv[], void(*userinit)(int))
{
	register char *name;
	register int n,prof;
	n = strlen(e_version);
	if(e_version[n-1]=='$' && e_version[n-2]==' ')
		e_version[n-2]=0;
#if	(CC_NATIVE == CC_ASCII)
	memcpy(sh_lexstates,sh_lexrstates,ST_NONE*sizeof(char*));
#else
	init_ebcdic();
#endif
	sh.mac_context = sh_macopen(&sh);
	sh.arg_context = sh_argopen(&sh);
	sh.lex_context = (void*)sh_lexopen(0,&sh,1);
	sh.ed_context = (void*)ed_open(&sh);
	sh_onstate(SH_INIT);
	error_info.exit = sh_exit;
	error_info.id = path_basename(argv[0]);
#if ERROR_VERSION >= 20000102L
	error_info.catalog = e_dict;
#endif
	sh.cpipe[0] = -1;
	sh.coutpipe = -1;
	sh.userid=getuid();
	sh.euserid=geteuid();
	sh.groupid=getgid();
	sh.egroupid=getegid();
	for(n=0;n < 10; n++)
	{
		/* don't use lower bits when rand() generates large numbers */
		if(rand() > RANDMASK)
		{
			rand_shift = 3;
			break;
		}
	}
	sh.lim.clk_tck = sysconf(_SC_CLK_TCK);
	sh.lim.open_max = sysconf(_SC_OPEN_MAX);
	sh.lim.child_max = sysconf(_SC_CHILD_MAX);
	sh.lim.ngroups_max = sysconf(_SC_NGROUPS_MAX);
	sh.lim.posix_version = sysconf(_SC_VERSION);
	sh.lim.posix_jobcontrol = sysconf(_SC_JOB_CONTROL);
	if(sh.lim.child_max <=0)
		sh.lim.child_max = CHILD_MAX;
	if(sh.lim.open_max <0)
		sh.lim.open_max = OPEN_MAX;
	if(sh.lim.clk_tck <=0)
		sh.lim.clk_tck = CLK_TCK;
#ifdef SHOPT_FS_3D
	if(mount(".", NIL(char*),FS3D_GET|FS3D_VERSION,0) >= 0)
		sh.lim.fs3d = 1;
#endif /* SHOPT_FS_3D */
	sh_ioinit();
	/* initialize signal handling */
	sh_siginit();
	stakinstall(NIL(Stak_t*),nospace);
	/* set up memory for name-value pairs */
	sh.init_context =  nv_init(&sh);
	/* read the environment */
#ifdef SHOPT_MULTIBYTE
       	charsize_init();
#endif /* SHOPT_MULTIBYTE */
	env_init(&sh);
	nv_putval(IFSNOD,(char*)e_sptbnl,NV_RDONLY);
#ifdef SHOPT_FS_3D
	nv_stack(VPATHNOD, &VPATH_init);
#endif /* SHOPT_FS_3D */
	astconfdisc(newconf);
#ifdef SHOPT_TIMEOUT
	sh.st.tmout = SHOPT_TIMEOUT;
#endif /* SHOPT_TIMEOUT */
	/* initialize jobs table */
	job_clear();
	if(argc>0)
	{
		name = path_basename(*argv);
		if(*name=='-')
		{
			name++;
			sh.login_sh = 2;
		}
		/* check for restricted shell */
		if(argc>0 && strmatch(name,rsh_pattern))
			sh_onoption(SH_RESTRICTED);
		/* look for options */
		/* sh.st.dolc is $#	*/
		if((sh.st.dolc = sh_argopts(-argc,argv)) < 0)
		{
			sh.exitval = 2;
			sh_done(0);
		}
		opt_info.disc = 0;
		sh.st.dolv=argv+(argc-1)-sh.st.dolc;
		sh.st.dolv[0] = argv[0];
		if(sh.st.dolc < 1)
			sh_onoption(SH_SFLAG);
		if(!sh_isoption(SH_SFLAG))
		{
			sh.st.dolc--;
			sh.st.dolv++;
#ifdef _WIN32
			name = sh.st.dolv[0];
			if(name[1]==':' && (name[2]=='/' || name[2]=='\\'))
			{
				name[1] = name[0];
				name[0] = '/';
				name[2] = '/';
			}
#endif /*_WIN32 */
		}
	}
	/* set[ug]id scripts require the -p flag */
	prof = !sh_isoption(SH_PRIVILEGED);
	if(sh.userid!=sh.euserid || sh.groupid!=sh.egroupid)
	{
#ifdef SHOPT_P_SUID
		/* require sh -p to run setuid and/or setgid */
		if(!sh_isoption(SH_PRIVILEGED) && sh.euserid < SHOPT_P_SUID)
		{
			setuid(sh.euserid=sh.userid);
			setgid(sh.egroupid=sh.groupid);
		}
		else
#else
		{
			sh_onoption(SH_PRIVILEGED);
			prof = 0;
		}
#endif /* SHOPT_P_SUID */
#ifdef SHELLMAGIC
		/* careful of #! setuid scripts with name beginning with - */
		if(sh.login_sh && argv[1] && strcmp(argv[0],argv[1])==0)
			errormsg(SH_DICT,ERROR_exit(1),e_prohibited);
#endif /*SHELLMAGIC*/
	}
	else
		sh_offoption(SH_PRIVILEGED);
	sh.shname = strdup(sh.st.dolv[0]); /* shname for $0 in profiles */
	/*
	 * return here for shell script execution
	 * but not for parenthesis subshells
	 */
	error_info.id = strdup(sh.st.dolv[0]); /* error_info.id is $0 */
	sh.jmpbuffer = (void*)&sh.checkbase;
	sh_pushcontext(&sh.checkbase,SH_JMPSCRIPT);
	sh.st.self = &sh.global;
        sh.topscope = (Shscope_t*)sh.st.self;
	sh_offstate(SH_INIT);
	if(sh.userinit=userinit)
		(*userinit)(0);
	return(prof);
}

Shell_t *sh_getinterp(void)
{
	return(&sh);
}

/*
 * reinitialize before executing a script
 */
int sh_reinit(char *argv[])
{
	dtclear(sh.fun_tree);
	dtclose(sh.alias_tree);
	sh.alias_tree = inittree(&sh,shtab_aliases);
	sh.namespace = 0;
	if(sh.userinit)
		(*sh.userinit)(1);
	if(sh.heredocs)
	{
		sfclose(sh.heredocs);
		sh.heredocs = 0;
	}
	/* remove locals */
	nv_scan(sh.var_tree,sh_envnolocal,(void*)0,NV_EXPORT,0);
	memset(sh.st.trapcom,0,(sh.st.trapmax+1)*sizeof(char*));
	sh_offoption(~(SH_TRACKALL|SH_EMACS|SH_GMACS|SH_VIRAW|SH_VI));
	/* set up new args */
	if(argv)
		sh.arglist = sh_argcreate(argv);
	if(sh.arglist)
		sh_argreset(sh.arglist,NIL(struct dolnod*));
	sh.envlist=0;
	sh.curenv = 0;
	sh.shname = error_info.id = strdup(sh.st.dolv[0]);
	sh_offstate(SH_FORKED);
	sh.fn_depth = sh.dot_depth = 0;
	return(1);
}

/*
 * set when creating a local variable of this name
 */
Namfun_t *nv_cover(register Namval_t *np)
{
#ifdef PATH_BFPATH
	if(np==IFSNOD || np==PATHNOD || np==SHELLNOD || np==FPATHNOD || np==CDPNOD || np==SECONDS)
#else
	if(np==IFSNOD || np==PATHNOD || np==SHELLNOD || np==SECONDS)
#endif
		return(np->nvfun);
#ifdef _hdr_locale
	if(np==LCALLNOD || np==LCTYPENOD || np==LCMSGNOD || np==LCCOLLNOD || np==LCNUMNOD || np==LANGNOD)
		return(np->nvfun);
#endif
	 return(0);
}

/*
 * Initialize the shell name and alias table
 */
static Init_t *nv_init(Shell_t *shp)
{
	register Init_t *ip;
	double d=0;
	ip = newof(0,Init_t,1,0);
	if(!ip)
		return(0);
	ip->sh = shp;
	shp->var_base = shp->var_tree = inittree(shp,shtab_variables);
	ip->IFS_init.hdr.disc = &IFS_disc;
	ip->IFS_init.sh = shp;
	ip->PATH_init.hdr.disc = &RESTRICTED_disc;
	ip->PATH_init.sh = shp;
#ifdef PATH_BFPATH
	ip->FPATH_init.hdr.disc = &RESTRICTED_disc;
	ip->FPATH_init.sh = shp;
	ip->CDPATH_init.hdr.disc = &CDPATH_disc;
	ip->CDPATH_init.sh = shp;
#endif
	ip->SHELL_init.hdr.disc = &RESTRICTED_disc;
	ip->SHELL_init.sh = shp;
	ip->ENV_init.hdr.disc = &RESTRICTED_disc;
	ip->ENV_init.sh = shp;
	ip->VISUAL_init.hdr.disc = &EDITOR_disc;
	ip->VISUAL_init.sh = shp;
	ip->EDITOR_init.hdr.disc = &EDITOR_disc;
	ip->EDITOR_init.sh = shp;
	ip->OPTINDEX_init.hdr.disc = &OPTINDEX_disc;
	ip->OPTINDEX_init.sh = shp;
	ip->SECONDS_init.hdr.disc = &SECONDS_disc;
	ip->SECONDS_init.sh = shp;
	ip->RAND_init.hdr.disc = &RAND_disc;
	ip->SH_MATCH_init.hdr.disc = &SH_MATCH_disc;
	ip->LINENO_init.hdr.disc = &LINENO_disc;
	ip->LINENO_init.sh = shp;
	ip->L_ARG_init.hdr.disc = &L_ARG_disc;
#ifdef _hdr_locale
	ip->LC_TYPE_init.hdr.disc = &LC_disc;
	ip->LC_NUM_init.hdr.disc = &LC_disc;
	ip->LC_COLL_init.hdr.disc = &LC_disc;
	ip->LC_MSG_init.hdr.disc = &LC_disc;
	ip->LC_ALL_init.hdr.disc = &LC_disc;
	ip->LANG_init.hdr.disc = &LC_disc;
	ip->LC_TYPE_init.sh = shp;
	ip->LC_NUM_init.sh = shp;
	ip->LC_COLL_init.sh = shp;
	ip->LC_MSG_init.sh = shp;
	ip->LANG_init.sh = shp;
#endif /* _hdr_locale */
#ifdef SHOPT_MULTIBYTE
	ip->CSWIDTH_init.disc = &CSWIDTH_disc;
#endif /* SHOPT_MULTIBYTE */
	nv_stack(IFSNOD, &ip->IFS_init.hdr);
	nv_stack(PATHNOD, &ip->PATH_init.hdr);
#ifdef PATH_BFPATH
	nv_stack(FPATHNOD, &ip->FPATH_init.hdr);
	nv_stack(CDPNOD, &ip->CDPATH_init.hdr);
#endif
	nv_stack(SHELLNOD, &ip->SHELL_init.hdr);
	nv_stack(ENVNOD, &ip->ENV_init.hdr);
	nv_stack(VISINOD, &ip->VISUAL_init.hdr);
	nv_stack(EDITNOD, &ip->EDITOR_init.hdr);
	nv_stack(OPTINDNOD, &ip->OPTINDEX_init.hdr);
	nv_stack(SECONDS, &ip->SECONDS_init.hdr);
	nv_stack(L_ARGNOD, &ip->L_ARG_init.hdr);
	nv_putval(SECONDS, (char*)&d, NV_INTEGER);
	nv_stack(RANDNOD, &ip->RAND_init.hdr);
	d = (shp->pid&RANDMASK);
	nv_putval(RANDNOD, (char*)&d, NV_INTEGER);
	nv_stack(LINENO, &ip->LINENO_init.hdr);
	nv_putsub(SH_MATCHNOD,(char*)0,10);
	nv_onattr(SH_MATCHNOD,NV_RDONLY);
#ifdef _hdr_locale
	nv_stack(LCTYPENOD, &ip->LC_TYPE_init.hdr);
	nv_stack(LCALLNOD, &ip->LC_ALL_init.hdr);
	nv_stack(LCMSGNOD, &ip->LC_MSG_init.hdr);
	nv_stack(LCCOLLNOD, &ip->LC_COLL_init.hdr);
	nv_stack(LCNUMNOD, &ip->LC_NUM_init.hdr);
	nv_stack(LANGNOD, &ip->LANG_init.hdr);
	nv_stack(SH_MATCHNOD, &ip->SH_MATCH_init.hdr);
#endif /* _hdr_locale */
#ifdef SHOPT_MULTIBYTE
	nv_stack(CSWIDTHNOD, &ip->CSWIDTH_init);
#endif /* SHOPT_MULTIBYTE */
	(PPIDNOD)->nvalue.lp = (&shp->ppid);
	(TMOUTNOD)->nvalue.lp = (&shp->st.tmout);
	(MCHKNOD)->nvalue.lp = (long*)(&sh_mailchk);
	(OPTINDNOD)->nvalue.lp = (&shp->st.optindex);
	/* set up the seconds clock */
	shp->alias_tree = inittree(shp,shtab_aliases);
	shp->track_tree = dtopen(&_Nvdisc,Dtset);
	shp->bltin_tree = inittree(shp,(const struct shtable2*)shtab_builtins);
	shp->fun_tree = dtopen(&_Nvdisc,Dtset);
	dtview(shp->fun_tree,shp->bltin_tree);
#ifdef SHOPT_NAMESPACE
	{
	Namval_t *np;
	np = nv_search("global",DOTSHNOD->nvalue.hp,NV_ADD);
	nv_putval(np,(char*)shp->var_tree,NV_TABLE|NV_RDONLY|NV_NOFREE);
	np = nv_search("function",DOTSHNOD->nvalue.hp,NV_ADD);
	nv_putval(np,(char*)shp->fun_tree,NV_TABLE|NV_RDONLY|NV_NOFREE);
	np = nv_search("builtin",DOTSHNOD->nvalue.hp,NV_ADD);
	nv_putval(np,(char*)shp->bltin_tree,NV_TABLE|NV_RDONLY|NV_NOFREE);
	np = nv_search("alias",DOTSHNOD->nvalue.hp,NV_ADD);
	nv_putval(np,(char*)shp->alias_tree,NV_TABLE|NV_RDONLY|NV_NOFREE);
	np = nv_search("class",DOTSHNOD->nvalue.hp,NV_ADD);
	np->nvalue.hp = dtopen(&_Nvdisc,Dtset);
	nv_onattr(np,NV_TABLE|NV_RDONLY|NV_NOFREE);
	np = nv_search("namespace",DOTSHNOD->nvalue.hp,NV_ADD);
	nv_putval(np,".sh.global",NV_RDONLY|NV_NOFREE);
	nv_stack(np, &NSPACE_init);
	}
#endif /* SHOPT_NAMESPACE */
	return(ip);
}

/*
 * initialize name-value pairs
 */

static Dt_t *inittree(Shell_t *shp,const struct shtable2 *name_vals)
{
	register Namval_t *np;
	register const struct shtable2 *tp;
	register unsigned n = 0;
	register Dt_t *treep;
	Dt_t *base_treep;
	for(tp=name_vals;*tp->sh_name;tp++)
		n++;
	np = (Namval_t*)calloc(n,sizeof(Namval_t));
	if(!shp->bltin_nodes)
		shp->bltin_nodes = np;
	else if(name_vals==(const struct shtable2*)shtab_builtins)
		shp->bltin_cmds = np;
	base_treep = treep = dtopen(&_Nvdisc,Dtset);
	for(tp=name_vals;*tp->sh_name;tp++,np++)
	{
		if((np->nvname = strrchr(tp->sh_name,'.')) && np->nvname!=((char*)tp->sh_name))
			np->nvname++;
		else
		{
			np->nvname = (char*)tp->sh_name;
			treep = base_treep;
		}
		np->nvenv = 0;
		if(name_vals==(const struct shtable2*)shtab_builtins)
			np->nvalue.bfp = ((struct shtable3*)tp)->sh_value;
		else
			np->nvalue.cp = (char*)tp->sh_value;
		nv_setattr(np,tp->sh_number);
		if(nv_istable(np))
			np->nvalue.hp = dtopen(&_Nvdisc,Dtset);
		if(nv_isattr(np,NV_INTEGER))
			nv_setsize(np,10);
		else
			nv_setsize(np,0);
		dtinsert(treep,np);
		if(nv_istable(np))
			treep = np->nvalue.hp;
	}
	return(treep);
}

/*
 * read in the process environment and set up name-value pairs
 * skip over items that are not name-value pairs
 */

static void env_init(Shell_t *shp)
{
	register char *cp;
	register Namval_t	*np;
	register char **ep=environ;
	register char *next=0;
#ifdef _ENV_H
	shp->env = env_open(environ,3);
	env_delete(shp->env,"_");
#endif
	if(ep)
	{
		while(cp= *ep++)
		{
			if(*cp=='A' && cp[1]=='_' && cp[2]=='_' && cp[3]=='z' && cp[4]=='=')
				next = cp+4;
			else if(np=nv_open(cp,shp->var_tree,(NV_EXPORT|NV_IDENT|NV_ASSIGN))) 
			{
				nv_onattr(np,NV_IMPORT);
				np->nvenv = cp;
				nv_close(np);
			}
		}
		while(cp=next)
		{
			if(next = strchr(++cp,'='))
				*next = 0;
			np = nv_search(cp+2,shp->var_tree,NV_ADD);
			if(nv_isattr(np,NV_IMPORT|NV_EXPORT))
			{
				int flag = *(unsigned char*)cp-' ';
				int size = *(unsigned char*)(cp+1)-' ';
				if((flag&NV_INTEGER) && size==0)
				{
					/* check for floating*/
					char *ep,*val = nv_getval(np);
					strtol(val,&ep,10);
					if(*ep=='.' || *ep=='e' || *ep=='E')
					{
						char *lp;
						flag |= NV_DOUBLE;
						if(*ep=='.')
						{
							strtol(ep+1,&lp,10);
							if(*lp)
								ep = lp;
						}
						if(*ep && *ep!='.')
						{
							flag |= NV_EXPNOTE;
							size = ep-val;
						}
						else
							size = strlen(ep);
						size--;
					}
				}
				nv_newattr(np,flag|NV_IMPORT|NV_EXPORT,size);
			}
		}
	}
#ifdef _ENV_H
	env_delete(sh.env,e_envmarker);
#endif
	if(nv_isattr(PWDNOD,NV_TAGGED))
	{
		nv_offattr(PWDNOD,NV_TAGGED);
		path_pwd(0);
	}
	if(cp = nv_getval(SHELLNOD))
	{
		cp = path_basename(cp);
		if(strmatch(cp,rsh_pattern))
			sh_onoption(SH_RESTRICTED); /* restricted shell */
	}
	return;
}

/*
 * terminate shell and free up the space
 */
int sh_term(void)
{
	sfdisc(sfstdin,SF_POPDISC);
	free((char*)sh.outbuff);
	stakset(NIL(char*),0);
	return(0);
}

/* function versions of these */

#undef sh_isoption
Shopt_t sh_isoption(Shopt_t opt)
{
	return(sh.options & (opt));
}

#undef sh_onoption
Shopt_t sh_onoption(Shopt_t opt)
{
	return(sh.options |= (opt));
}

#undef sh_offoption
Shopt_t sh_offoption(Shopt_t opt)
{
	return(sh.options &= ~(opt));
}

#undef sh_sigcheck
void	sh_sigcheck(void)
{
	if(sh.trapnote&SH_SIGSET)
		sh_exit(SH_EXITSIG);
}

#undef sh_bltin_tree
Dt_t	*sh_bltin_tree(void)
{
	return(sh.bltin_tree);
}
