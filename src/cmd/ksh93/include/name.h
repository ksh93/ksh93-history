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
#ifndef _NV_PRIVATE
/*
 * This is the implementation header file for name-value pairs
 */

#define _NV_PRIVATE	\
	union Value	nvalue; 	/* value field */ \
	char		*nvenv;		/* pointer to environment name */ \
	Namfun_t	*nvfun;		/* pointer to trap functions */ \
	int		nvsize;	

#include	<ast.h>
#include	<cdt.h>
#include	"shtable.h"

/* Nodes can have all kinds of values */
union Value
{
	const char	*cp;
	int		*ip;
	char		c;
	int		i;
	unsigned	u;
	long		*lp;
	Sflong_t	*llp;	/* for long long arithmetic */
	short		s;
	double		*dp;	/* for floating point arithmetic */
	Sfdouble_t	*ldp;	/* for long floating point arithmetic */
	struct Namarray	*array;	/* for array node */
	struct Namval	*np;	/* for Namval_t node */
	Dt_t		*hp;	/* value is a dictionary */
	union Value	*up;	/* for indirect node */
	struct Ufunction *rp;	/* shell user defined functions */
	int (*bfp)(int,char*[],void*);/* builtin entry point function pointer */
};

#include	"nval.h"

/* used for arrays */
#define ARRAY_MASK	((1L<<ARRAY_BITS)-1)	/* For index values */

#define ARRAY_MAX 	(1<<16)	/* maximum number of elements in an array
				   This must be less than ARRAY_MASK */
#define ARRAY_INCR	32	/* number of elements to grow when array 
				   bound exceeded.  Must be a power of 2 */

/* These flags are used as options to array_get() */
#define ARRAY_ASSIGN	0
#define ARRAY_LOOKUP	1
#define ARRAY_DELETE	2


/* This describes a user shell function node */
struct Ufunction
{
	int	*ptree;			/* address of parse tree */
	int	lineno;			/* line number of function start */
	off_t	hoffset;		/* offset into history file */
	Namval_t *nspace;		/* pointer to name space */
};

/* attributes of Namval_t items */

/* The following attributes are for internal use */
#define NV_NOCHANGE	(NV_EXPORT|NV_IMPORT|NV_RDONLY|NV_TAGGED|NV_NOFREE)
#define NV_ATTRIBUTES	(~(NV_NOSCOPE|NV_ARRAY|NV_IDENT|NV_ASSIGN|NV_REF|NV_VARNAME))
#define NV_PARAM	(1<<14)		/* expansion use positional params */

/* This following are for use with nodes which are not name-values */
#define NV_FUNCTION	(NV_RJUST|NV_FUNCT)	/* value is shell function */
#define NV_FPOSIX	NV_LJUST		/* posix function semantics */

#define NV_NOPRINT	(NV_LTOU|NV_UTOL)	/* do not print */
#define NV_NOALIAS	(NV_NOPRINT|NV_IMPORT)
#define NV_NOEXPAND	NV_RJUST		/* do not expand alias */
#define NV_BLTIN	(NV_NOPRINT|NV_EXPORT)
#define BLT_ENV		(NV_RDONLY)		/* non-stoppable,
						 * can modify enviornment */
#define BLT_SPC		(NV_LJUST)		/* special built-ins */
#define BLT_EXIT	(NV_RJUST)		/* exit value can be > 255 */
#define BLT_DCL		(NV_TAGGED)		/* declaration command */
#define nv_isref(n)	(nv_isattr((n),NV_REF))
#define is_abuiltin(n)	(nv_isattr(n,NV_BLTIN)==NV_BLTIN)
#define is_afunction(n)	(nv_isattr(n,NV_FUNCTION)==NV_FUNCTION)
#define	nv_funtree(n)	((n)->nvalue.rp->ptree)
#define	funptr(n)	((n)->nvalue.bfp)


/* NAMNOD MACROS */
/* ... for attributes */

#define nv_onattr(n,f)	((n)->nvflag |= (f))
#define nv_setattr(n,f)	((n)->nvflag = (f))
#define nv_offattr(n,f)	((n)->nvflag &= ~(f))
#define nv_name(n)	((n)->nvname)
#define nv_context(n)	((void*)(n)->nvfun)		/* for builtins */
#define nv_table(n)	((Namval_t*)((n)->nvfun))	/* for references */
#define nv_refnode(n)	((Namval_t*)((n)->nvalue.np))	/* for references */
#ifdef SHOPT_OO
#   define nv_class(np)		(nv_isattr(np,NV_REF|NV_IMPORT)?0:(Namval_t*)((np)->nvenv))
#endif /* SHOPT_OO */

/* ... etc */

#define nv_setsize(n,s)	((n)->nvsize = (s))
#define nv_size(np)	((np)->nvsize)
#define nv_isnull(np)	(!(np)->nvalue.cp && !(np)->nvfun)

/* ...	for arrays */

#define nv_arrayptr(np)	(nv_isattr(np,NV_ARRAY)?(np)->nvalue.array:(Namarr_t*)0)
#define array_elem(ap)	((ap)->nelem&ARRAY_MASK)
#define array_assoc(ap)	((ap)->fun)

extern void		array_check(Namval_t*, int);
extern union Value	*array_find(Namval_t*, int);
extern char 		*nv_endsubscript(Namval_t*, char*, int);
extern Namfun_t 	*nv_cover(Namval_t*);
struct argnod;		/* struct not declared yet */
extern void		nv_setlist(struct argnod*, int);
extern void 		nv_scope(struct argnod*);

extern Dtdisc_t		_Nvdisc;
extern const char	e_subscript[];
extern const char	e_nullset[];
extern const char	e_notset[];
extern const char	e_noparent[];
extern const char	e_readonly[];
extern const char	e_badfield[];
extern const char	e_restricted[];
extern const char	e_ident[];
extern const char	e_varname[];
extern const char	e_funname[];
extern const char	e_noalias[];
extern const char	e_aliname[];
extern const char	e_badexport[];
extern const char	e_badref[];
extern const char	e_noref[];
extern const char	e_selfref[];
extern const char	e_envmarker[];
extern const char	e_badlocale[];
extern const char	e_loop[];
#endif /* _NV_PRIVATE */
