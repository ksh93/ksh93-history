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
 * export [-p] [arg...]
 * readonly [-p] [arg...]
 * typeset [options]  [arg...]
 * alias [-ptx] [arg...]
 * unalias [arg...]
 * builtin [-sd] [-f file] [name...]
 * set [options] [name...]
 * unset [-fnv] [name...]
 *
 *   David Korn
 *   AT&T Labs
 *   research!dgk
 *
 */

#include	"defs.h"
#include	<error.h>
#include	"path.h"
#include	"name.h"
#include	"history.h"
#include	"builtins.h"
#include	"variables.h"
#ifdef _hdr_dlldefs
#   include	<dlldefs.h>
#else
    /* use these prototyped because dlhdr.h may be wrong */
    extern void	*dlopen(const char*,int);
    extern void	*dlsym(void*, const char*);
    extern char	*dlerror(void);
#   define DL_MODE	1
#endif

struct tdata
{
	Shell_t *sh;
	int     argnum;
	int     aflag;
	char    *prefix;
	Sfio_t  *outfile;
	int     scanmask;
	Dt_t 	*scanroot;
	char    **argnam;
};


static int	print_namval(Sfio_t*, Namval_t*, int, struct tdata*);
static void	print_attribute(Namval_t*,void*);
static void	print_all(Sfio_t*, Dt_t*, struct tdata*);
static void	print_scan(Sfio_t*, int, Dt_t*, int, struct tdata*t);
static int	b_unall(int, char**, Dt_t*, Shell_t*);
static int	b_common(char**, int, Dt_t*, struct tdata*);
static void	pushname(Namval_t*,void*);
static void(*nullscan)(Namval_t*,void*);

static char *get_tree(Namval_t*, Namfun_t *);
static void put_tree(Namval_t*, const char*, int,Namfun_t*);

#undef nv_name

static const Namdisc_t treedisc =
{
	0,
	put_tree,
	get_tree
};


/*
 * Note export and readonly are the same
 */
#if 0
    /* for the dictionary generator */
    int    b_export(int argc,char *argv[],void *extra){}
#endif
int    b_readonly(int argc,char *argv[],void *extra)
{
	register int flag;
	char *command = argv[0];
	struct tdata tdata;
	NOT_USED(argc);
	tdata.sh = (Shell_t*)extra;
	tdata.aflag = '-';
	tdata.argnum = 0;
	tdata.prefix = 0;
	while((flag = optget(argv,*command=='e'?sh_optexport:sh_optreadonly))) switch(flag)
	{
		case 'p':
			tdata.prefix = command;
			break;
		case ':':
			errormsg(SH_DICT,2, "%s", opt_info.arg);
			break;
		case '?':
			errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
			return(2);
	}
	if(error_info.errors)
		errormsg(SH_DICT,ERROR_usage(2),optusage(NIL(char*)));
	argv += (opt_info.index-1);
	if(*command=='r')
		flag = (NV_ASSIGN|NV_RDONLY|NV_VARNAME);
	else
		flag = (NV_ASSIGN|NV_EXPORT|NV_IDENT);
	if(*argv)
		return(b_common(argv,flag,tdata.sh->var_tree, &tdata));
	print_scan(sfstdout,flag,tdata.sh->var_tree,tdata.aflag=='+', &tdata);
	return(0);
}


int    b_alias(int argc,register char *argv[],void *extra)
{
	register unsigned flag = NV_ARRAY|NV_NOSCOPE|NV_ASSIGN;
	register Dt_t *troot;
	register int n;
	struct tdata tdata;
	NOT_USED(argc);
	tdata.sh = (Shell_t*)extra;
	tdata.prefix=0;
	troot = tdata.sh->alias_tree;
	if(*argv[0]=='h')
		flag = NV_TAGGED;
	if(argv[1])
	{
		opt_info.offset = 0;
		opt_info.index = 1;
		*opt_info.option = 0;
		tdata.argnum = 0;
		tdata.aflag = *argv[1];
		while((n = optget(argv,sh_optalias))) switch(n)
		{
		    case 'p':
			tdata.prefix = argv[0];
			break;
		    case 't':
			flag |= NV_TAGGED;
			break;
		    case 'x':
			flag |= NV_EXPORT;
			break;
		    case ':':
			errormsg(SH_DICT,2, opt_info.arg);
			break;
		    case '?':
			errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
			return(2);
		}
		if(error_info.errors)
			errormsg(SH_DICT,ERROR_usage(2),"%s",optusage(NIL(char*)));
		argv += (opt_info.index-1);
		if(flag&NV_TAGGED)
		{
			if(argv[1] && strcmp(argv[1],"-r")==0)
			{
				/* hack to handle hash -r */
				nv_putval(PATHNOD,nv_getval(PATHNOD),NV_RDONLY);
				return(0);
			}
			troot = tdata.sh->track_tree;
		}
	}
	return(b_common(argv,flag,troot,&tdata));
}


int    b_typeset(int argc,register char *argv[],void *extra)
{
	register int flag = NV_VARNAME|NV_ASSIGN;
	register int n;
	struct tdata tdata;
	Dt_t *troot;
	int isfloat = 0;
	NOT_USED(argc);
	tdata.sh = (Shell_t*)extra;
	tdata.aflag = tdata.argnum  = 0;
	tdata.prefix = 0;
	troot = tdata.sh->var_tree;
	while((n = optget(argv,sh_opttypeset)))
	{
		switch(n)
		{
			case 'A':
				flag |= NV_ARRAY;
				break;
			case 'E':
				/* The following is for ksh88 compatibility */
				if(opt_info.offset && !strchr(argv[opt_info.index],'E'))
				{
					tdata.argnum = (int)opt_info.num;
					break;
				}
			case 'F':
				if(!opt_info.arg || (tdata.argnum = opt_info.num) <0)
					tdata.argnum = 10;
				isfloat = 1;
				if(n=='E')
					flag |= NV_EXPNOTE;
				break;
			case 'n':
				flag &= ~NV_VARNAME;
				flag |= (NV_REF|NV_IDENT);
				break;
			case 'H':
				flag |= NV_HOST;
				break;
#ifdef SHOPT_OO
			case 'C':
				tdata.prefix = opt_info.arg;
				flag |= NV_IMPORT;
				break;
#endif /* SHOPT_OO */
			case 'L':
				if(tdata.argnum==0)
					tdata.argnum = (int)opt_info.num;
				if(tdata.argnum < 0)
					errormsg(SH_DICT,ERROR_exit(1), e_badfield, tdata.argnum);
				flag &= ~NV_RJUST;
				flag |= NV_LJUST;
				break;
			case 'Z':
				flag |= NV_ZFILL;
				/* FALL THRU*/
			case 'R':
				if(tdata.argnum==0)
					tdata.argnum = (int)opt_info.num;
				if(tdata.argnum < 0)
					errormsg(SH_DICT,ERROR_exit(1), e_badfield, tdata.argnum);
				flag &= ~NV_LJUST;
				flag |= NV_RJUST;
				break;
			case 'f':
				flag &= ~(NV_VARNAME|NV_ASSIGN);
				troot = tdata.sh->fun_tree;
				break;
			case 'i':
				if(!opt_info.arg || (tdata.argnum = opt_info.num) <0)
					tdata.argnum = 10;
				flag |= NV_INTEGER;
				break;
			case 'l':
				flag |= NV_UTOL;
				break;
			case 'p':
				tdata.prefix = argv[0];
				continue;
			case 'r':
				flag |= NV_RDONLY;
				break;
			case 't':
				flag |= NV_TAGGED;
				break;
			case 'u':
				flag |= NV_LTOU;
				break;
			case 'x':
				flag &= ~NV_VARNAME;
				flag |= (NV_EXPORT|NV_IDENT);
				break;
			case ':':
				errormsg(SH_DICT,2, opt_info.arg);
				break;
			case '?':
				errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
				return(2);
		}
		if(tdata.aflag==0)
			tdata.aflag = *opt_info.option;
	}
	argv += opt_info.index;
	/* handle argument of + and - specially */
	if(*argv && argv[0][1]==0 && (*argv[0]=='+' || *argv[0]=='-'))
		tdata.aflag = *argv[0];
	else
		argv--;
	if((flag&NV_INTEGER) && (flag&(NV_LJUST|NV_RJUST|NV_ZFILL)))
		error_info.errors++;
	if(troot==tdata.sh->fun_tree && ((isfloat || flag&~(NV_FUNCT|NV_TAGGED|NV_EXPORT|NV_LTOU))))
		error_info.errors++;
	if(error_info.errors)
		errormsg(SH_DICT,ERROR_usage(2),"%s", optusage(NIL(char*)));
	if(isfloat)
		flag |= NV_INTEGER|NV_DOUBLE;
	if(tdata.sh->fn_depth)
		flag |= NV_NOSCOPE;
	return(b_common(argv,flag,troot,&tdata));
}

static int     b_common(char **argv,register int flag,Dt_t *troot,struct tdata *tp)
{
	register char *name;
	int nvflags=(flag&(NV_ARRAY|NV_NOSCOPE|NV_VARNAME|NV_IDENT|NV_ASSIGN));
	int r=0, ref=0;
	Shell_t *shp =tp->sh;
#ifdef SHOPT_OO
	char *base=0;
	if(flag&NV_IMPORT)
	{
		if(!argv[1])
			errormsg(SH_DICT,ERROR_exit(1),"requires arguments");
		if(tp->aflag!='-')
			errormsg(SH_DICT,ERROR_exit(1),"cannot be unset");
		base = tp->prefix;
		tp->prefix = 0;
		flag &= ~(NV_IMPORT|NV_ASSIGN);
	}
#endif /* SHOPT_OO */
	flag &= ~(NV_ARRAY|NV_NOSCOPE|NV_VARNAME|NV_IDENT);
	if(argv[1])
	{
		if(flag&NV_REF)
		{
			flag &= ~NV_REF;
			ref=1;
			if(tp->aflag!='-')
				nvflags |= NV_REF;
		}
		while(name = *++argv)
		{
			register unsigned newflag;
			register Namval_t *np;
			unsigned curflag;
			if(troot == shp->fun_tree)
			{
				/*
				 *functions can be exported or
				 * traced but not set
				 */
				flag &= ~NV_ASSIGN;
				if(flag&NV_LTOU)
				{
					/* Function names cannot be special builtin */
					if((np=nv_search(name,shp->bltin_tree,0)) && nv_isattr(np,BLT_SPC))
						errormsg(SH_DICT,ERROR_exit(1),e_badfun,name);
					np = nv_open(name,shp->fun_tree,NV_ARRAY|NV_IDENT|NV_NOSCOPE);
				}
				else
					np = nv_search(name,shp->fun_tree,HASH_NOSCOPE);
				if(np && ((flag&NV_LTOU) || !nv_isnull(np) || nv_isattr(np,NV_LTOU)))
				{
					if(flag==0)
					{
						print_namval(sfstdout,np,0,tp);
						continue;
					}
					if(shp->subshell)
						sh_subfork();
					if(tp->aflag=='-')
						nv_onattr(np,flag|NV_FUNCTION);
					else if(tp->aflag=='+')
						nv_offattr(np,flag);
				}
				else
					r++;
				continue;
			}
			np = nv_open(name,troot,nvflags);
			/* tracked alias */
			if(troot==shp->track_tree && tp->aflag=='-')
			{
				nv_onattr(np,NV_NOALIAS);
				path_alias(np,path_absolute(nv_name(np),NIL(char*)));
				continue;
			}
			if(flag==NV_ASSIGN && !ref && tp->aflag!='-' && !strchr(name,'='))
			{
				if(troot!=shp->var_tree && (nv_isnull(np) || !print_namval(sfstdout,np,0,tp)))
				{
					sfprintf(sfstderr,sh_translate(e_noalias),name);
					r++;
				}
				continue;
			}
			if(troot==shp->var_tree && (nvflags&NV_ARRAY))
				nv_setarray(np,nv_associative);
#ifdef SHOPT_OO
			if(base)
			{
				Namval_t *nq, *nr;
				nv_offattr(np,NV_IMPORT);
				if(!(nq=nv_search(base,troot,0)))
					errormsg(SH_DICT,ERROR_exit(1),e_badbase,base);
				/* check for loop */
				for(nr=nq; nr; nr = nv_class(nr))
				{
					if(nr==np)
						errormsg(SH_DICT,ERROR_exit(1),e_loop,base);
				}
				np->nvenv = (char*)nq;
				if(nq->nvfun && !nv_isref(nq) && (nq->nvfun)->disc->create)
					(*(nq->nvfun)->disc->create)(np,0,0,nq->nvfun);
			}
#endif /* SHOPT_OO */
			curflag = np->nvflag;
			flag &= ~NV_ASSIGN;
			if (tp->aflag == '-')
			{
				if((flag&NV_EXPORT) && strchr(nv_name(np),'.'))
					errormsg(SH_DICT,ERROR_exit(1),e_badexport,nv_name(np));
#ifdef SHOPT_BSH
				if(flag&NV_EXPORT)
					nv_offattr(np,NV_IMPORT);
#endif /* SHOPT_BSH */
				newflag = curflag;
				if(flag&~NV_NOCHANGE)
					newflag &= NV_NOCHANGE;
				newflag |= flag;
				if (flag & (NV_LJUST|NV_RJUST))
				{
					if(!(flag&NV_RJUST))
						newflag &= ~NV_RJUST;
					
					else if(!(flag&NV_LJUST))
						newflag &= ~NV_LJUST;
				}
				if (flag & NV_UTOL)
					newflag &= ~NV_LTOU;
				else if (flag & NV_LTOU)
					newflag &= ~NV_UTOL;
			}
			else
			{
				if((flag&NV_RDONLY) && (curflag&NV_RDONLY))
					errormsg(SH_DICT,ERROR_exit(1),e_readonly,nv_name(np));
				newflag = curflag & ~flag;
			}
			if (tp->aflag && (tp->argnum>0 || (curflag!=newflag)))
			{
				if(shp->subshell)
					sh_assignok(np,1);
				if(troot!=shp->var_tree)
					nv_setattr(np,newflag&~NV_ASSIGN);
				else
				{
					if(tp->argnum==1 && newflag==NV_INTEGER && nv_isattr(np,NV_INTEGER))
						tp->argnum = 10;
					nv_newattr (np, newflag&~NV_ASSIGN,tp->argnum);
				}
			}
			/* set or unset references */
			if(ref)
			{
				if(tp->aflag=='-')
					nv_setref(np);
				else
					nv_unref(np);
			}
			nv_close(np);
		}
	}
	else
	{
		if(tp->aflag)
		{
			if(troot==shp->fun_tree)
			{
				flag |= NV_FUNCTION;
				tp->prefix = 0;
			}
			else if(troot==shp->var_tree)
				flag |= (nvflags&NV_ARRAY);
			print_scan(sfstdout,flag,troot,tp->aflag=='+',tp);
		}
		else if(troot==shp->alias_tree)
			print_scan(sfstdout,0,troot,0,tp);
		else
			print_all(sfstdout,troot,tp);
	}
	return(r);
}

typedef int (*Fptr_t)(int, char*[], void*);

#define GROWLIB	4
static void **liblist;

/*
 * This allows external routines to load from the same library */
void **sh_getliblist(void)
{
	return(liblist);
}

/*
 * add change or list built-ins
 * adding builtins requires dlopen() interface
 */
int	b_builtin(int argc,char *argv[],void *extra)
{
	register char *arg=0, *name;
	register int n, r=0, flag=0;
	register Namval_t *np;
	int dlete=0;
	struct tdata tdata;
	static int maxlib, nlib;
	Fptr_t addr;
	void *library=0;
	char *errmsg;
	NOT_USED(argc);
	tdata.sh = (Shell_t*)extra;
	while (n = optget(argv,sh_optbuiltin)) switch (n)
	{
	    case 's':
		flag = BLT_SPC;
		break;
	    case 'd':
		dlete=1;
		break;
	    case 'f':
#ifdef SHOPT_DYNAMIC
		arg = opt_info.arg;
#else
		errormsg(SH_DICT,2, "adding built-ins not supported");
		error_info.errors++;
#endif /* SHOPT_DYNAMIC */
		break;
	    case ':':
		errormsg(SH_DICT,2, opt_info.arg);
		break;
	    case '?':
		errormsg(SH_DICT,ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	argv += opt_info.index;
	if(error_info.errors)
		errormsg(SH_DICT,ERROR_usage(2),"%s", optusage(NIL(char*)));
	if(arg || *argv)
	{
		if(sh_isoption(SH_RESTRICTED))
			errormsg(SH_DICT,ERROR_exit(1),e_restricted,argv[-opt_info.index]);
		if(tdata.sh->subshell)
			sh_subfork();
	}
	if(arg)
	{
#ifdef _hdr_dlldefs
		if(!(library = dllfind(arg,NIL(char*),RTLD_LAZY)))
		{
			errormsg(SH_DICT,ERROR_exit(0),"%s: cannot load library",arg);
			return(1);
		}
#else
		if(!(library = dlopen(arg,DL_MODE)))
		{
			errormsg(SH_DICT,ERROR_exit(0),"%s: %s",arg,dlerror());
			return(1);
		}
#endif
		/* 
		 * see if library is already on search list
		 * if it is, move to head of search list
		 */
		for(n=r=0; n < nlib; n++)
		{
			if(r)
				liblist[n-1] = liblist[n];
			else if(liblist[n]==library)
				r++;
		}
		if(r)
			nlib--;
		else
		{
			typedef void (*Iptr_t)(int);
			Iptr_t initfn;
			if((initfn = (Iptr_t)dlsym(library,"lib_init")))
				(*initfn)(0);
		}
		if(nlib >= maxlib)
		{
			/* add library to search list */
			maxlib += GROWLIB;
			if(nlib>0)
				liblist = (void**)realloc((void*)liblist,(maxlib+1)*sizeof(void**));
			else
				liblist = (void**)malloc((maxlib+1)*sizeof(void**));
		}
		liblist[nlib++] = library;
		liblist[nlib] = 0;
	}
	else if(*argv==0 && !dlete)
	{
		print_scan(sfstdout, flag, tdata.sh->bltin_tree, 1, &tdata);
		return(0);
	}
	r = 0;
	flag = staktell();
	while(arg = *argv)
	{
		name = path_basename(arg);
		stakputs("b_");
		stakputs(name);
		errmsg = 0;
		addr = 0;
		for(n=(nlib?nlib:dlete); --n>=0;)
		{
			/* (char*) added for some sgi-mips compilers */ 
			if(dlete || (addr = (Fptr_t)dlsym(liblist[n],stakptr(flag))))
			{
				if(np = sh_addbuiltin(arg, addr,(void*)dlete))
				{
					if(dlete || nv_isattr(np,BLT_SPC))
						errmsg = "restricted name";
				}
				break;
			}
		}
		if(!dlete && !addr)
		{
			np = sh_addbuiltin(arg, 0 ,0);
			if(np && nv_isattr(np,BLT_SPC))
				errmsg = "restricted name";
			else if(!np)
				errmsg = "not found";
		}
		if(errmsg)
		{
			errormsg(SH_DICT,ERROR_exit(0),"%s: %s",*argv,errmsg);
			r = 1;
		}
		stakseek(flag);
		argv++;
	}
	return(r);
}

int    b_set(int argc,register char *argv[],void *extra)
{
	struct tdata tdata;
	tdata.sh = (Shell_t*)extra;
	tdata.prefix=0;
	if(argv[1])
	{
		if(sh_argopts(argc,argv) < 0)
			return(2);
		sh_offstate(SH_VERBOSE|SH_MONITOR);
		sh_onstate(sh_isoption(SH_VERBOSE|SH_MONITOR));
	}
	else
		/*scan name chain and print*/
		print_scan(sfstdout,0,tdata.sh->var_tree,0,&tdata);
	return(0);
}

/*
 * The removing of Shell variable names, aliases, and functions
 * is performed here.
 * Unset functions with unset -f
 * Non-existent items being deleted give non-zero exit status
 */

int    b_unalias(int argc,register char *argv[],void *extra)
{
	Shell_t *shp = (Shell_t*)extra;
	return(b_unall(argc,argv,shp->alias_tree,shp));
}

int    b_unset(int argc,register char *argv[],void *extra)
{
	Shell_t *shp = (Shell_t*)extra;
	return(b_unall(argc,argv,shp->var_tree,shp));
}

static int b_unall(int argc, char **argv, register Dt_t *troot, Shell_t* shp)
{
	register Namval_t *np;
	register struct slnod *slp;
	register const char *name;
	register int r;
	int nflag = 0;
	int all=0;
	NOT_USED(argc);
	if(troot!=shp->var_tree && shp->subshell)
		sh_subfork();
	if(troot==shp->alias_tree)
		name = sh_optunalias;
	else
		name = sh_optunset;
	while(r = optget(argv,name)) switch(r)
	{
		case 'f':
			troot = shp->fun_tree;
			nflag = NV_NOSCOPE;
			break;
		case 'a':
			all=1;
			break;
		case 'n':
			nflag = NV_NOREF;
		case 'v':
			troot = shp->var_tree;
			break;
		case ':':
			errormsg(SH_DICT,2, opt_info.arg);
			break;
		case '?':
			errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
			return(2);
	}
	argv += opt_info.index;
	if(error_info.errors || (*argv==0 &&!all))
		errormsg(SH_DICT,ERROR_usage(2),"%s",optusage(NIL(char*)));
	r = 0;
	if(troot==shp->var_tree)
		nflag |= NV_VARNAME;
	if(all)
		dtclear(troot);
	else while(name = *argv++)
	{
		if(np=nv_open(name,troot,NV_NOADD|nflag))
		{
			if(troot!=shp->var_tree)
			{
				if(is_abuiltin(np))
				{
					r = 1;
					continue;
				}
				else if(slp=(struct slnod*)(np->nvenv))
				{
					/* free function definition */
					register char *cp= strrchr(name,'.');
					if(cp)
					{
						Namval_t *npv;
						*cp = 0;
						npv = nv_open(name,shp->var_tree,NV_ARRAY|NV_VARNAME|NV_NOADD);
						*cp++ = '.';
						if(npv)
							nv_setdisc(npv,cp,NIL(Namval_t*),(Namfun_t*)npv);
					}
					stakdelete(slp->slptr);
					np->nvenv = 0;
					dtdelete(troot,np);
					continue;
				}
			}
#ifdef apollo
			else
			{
				short namlen;
				name = nv_name(np);
				namlen =strlen(name);
				ev_$delete_var(name,&namlen);
			}
#endif /* apollo */
			if(shp->subshell)
				np=sh_assignok(np,0);
			nv_unset(np);
			nv_close(np);
		}
		else
			r = 1;
	}
	return(r);
}



/*
 * print out the name and value of a name-value pair <np>
 */

static int print_namval(Sfio_t *file,register Namval_t *np,register int flag, struct tdata *tp)
{
	register char *cp;
	sh_sigcheck();
	if(flag)
		flag = '\n';
	if(nv_isattr(np,NV_NOPRINT)==NV_NOPRINT)
	{
		if(is_abuiltin(np))
			sfputr(file,nv_name(np),'\n');
		return(0);
	}
	if(tp->prefix)
		sfputr(file,tp->prefix,' ');
	if(is_afunction(np))
	{
		if(!flag && !np->nvalue.ip)
			sfputr(file,"typeset -fu",' ');
		else if(!flag && !nv_isattr(np,NV_FPOSIX))
			sfputr(file,"function",' ');
		if(!np->nvalue.ip || np->nvalue.rp->hoffset<0)
			flag = '\n';
		sfputr(file,nv_name(np),flag?flag:-1);
		if(!flag)
		{
			sfputr(file,nv_isattr(np,NV_FPOSIX)?"()\n{":"\n{",'\n');
			hist_list(tp->sh->hist_ptr,file,np->nvalue.rp->hoffset,0,"\n");
		}
		return(nv_size(np)+1);
	}
	if(cp=nv_getval(np))
	{
		sfputr(file,nv_name(np),-1);
		if(!flag)
		{
			flag = '=';
		        if(nv_arrayptr(np))
				sfprintf(file,"[%s]", sh_fmtq(nv_getsub(np)));
		}
		sfputc(file,flag);
		if(flag != '\n')
		{
			if(nv_isref(np) && np->nvenv)
			{
				sfputr(file,sh_fmtq(cp),-1);
				sfprintf(file,"[%s]\n", sh_fmtq(np->nvenv));
			}
			else
				sfputr(file,sh_fmtq(cp),'\n');
		}
		return(1);
	}
	else if(tp->scanmask && tp->scanroot==tp->sh->var_tree)
		sfputr(file,nv_name(np),'\n');
	return(0);
}

/*
 * print attributes at all nodes
 */

static void	print_all(Sfio_t *file,Dt_t *root, struct tdata *tp)
{
	tp->outfile = file;
	nv_scan(root, print_attribute, (void*)tp, 0, 0);
}

#include	<fcin.h>
#include	"argnod.h"

/*
 * format initialization list given a list of assignments <argp>
 */
static void genvalue(struct argnod *argp, const char *prefix, int n, int indent,struct tdata *tp)
{
	register struct argnod *ap;
	register char *cp,*nextcp;
	register int m,isarray;
	int associative=0;
	Namval_t *np;
	if(n==0)
		m = strlen(prefix);
	else
		m = strchr(prefix,'.')-prefix;
	m++;
	if(tp->outfile)
	{
		sfwrite(tp->outfile,"(\n",2);
		indent++;
	}
	for(ap=argp; ap; ap=ap->argchn.ap)
	{
		if(ap->argflag==ARG_MAC)
			continue;
		cp = ap->argval+n;
		if(n==0 && cp[m-1]!='.')
		{
			ap->argflag = ARG_MAC;
			continue;
		}
		if(n && cp[m-1]==0)
			continue;
		if(n==0 || strncmp(ap->argval,prefix-n,m+n)==0)
		{
			cp +=m;
			if(nextcp=strchr(cp,'.'))
			{
				if(tp->outfile)
				{
					sfnputc(tp->outfile,'\t',indent);
					sfwrite(tp->outfile,cp,nextcp-cp);
					sfputc(tp->outfile,'=');
				}
				genvalue(argp,cp,n+m ,indent, tp);
				if(tp->outfile)
					sfputc(tp->outfile,'\n');
				continue;
			}
			ap->argflag = ARG_MAC;
			if(!(np=nv_search(ap->argval,tp->sh->var_tree,0)))
				continue;
			if(np->nvfun && !nv_isref(np) && np->nvfun->disc== &treedisc)
				continue;
			isarray=0;
			if(nv_isattr(np,NV_ARRAY))
			{
				isarray=1;
				if(array_elem(nv_arrayptr(np))==0)
					isarray=2;
				else
					nv_putsub(np,NIL(char*),ARRAY_SCAN);
				associative= nv_aindex(np)<0;
			}
			if(!tp->outfile)
			{
				nv_close(np);
				continue;
			}
			sfnputc(tp->outfile,'\t',indent);
			if(nv_isattr(np,~NV_ARRAY) || associative)
				print_attribute(np,tp);
			sfputr(tp->outfile,cp,(isarray==2?'\n':'='));
			if(isarray)
			{
				if(isarray==2)
					continue;
				sfwrite(tp->outfile,"(\n",2);
				sfnputc(tp->outfile,'\t',++indent);
			}
			while(1)
			{
				char *fmtq,*ep;
				if(isarray && associative)
				{
					sfprintf(tp->outfile,"[%s]",sh_fmtq(nv_getsub(np)));
					sfputc(tp->outfile,'=');
				}
				if(!(fmtq = sh_fmtq(nv_getval(np))))
					fmtq = "";
				else if(!associative && (ep=strchr(fmtq,'=')))
				{
					char *qp = strchr(fmtq,'\'');
					if(!qp || qp>ep)
					{
						sfwrite(tp->outfile,fmtq,ep-fmtq);
						sfputc(tp->outfile,'\\');
						fmtq = ep;
					}
				}
				sfputr(tp->outfile,fmtq,'\n');
				if(!ap || !nv_nextsub(np))
					break;
				sfnputc(tp->outfile,'\t',indent);
			}
			if(isarray)
			{
				sfnputc(tp->outfile,'\t',--indent);
				sfwrite(tp->outfile,")\n",2);
			}
		}
	}
	if(tp->outfile)
	{
		if(indent > 1)
			sfnputc(tp->outfile,'\t',indent-1);
		sfputc(tp->outfile,')');
	}
}


/*
 * walk the virtual tree and print or delete name-value pairs
 */
static char *walk_tree(register Namval_t *np, int dlete)
{
	static Sfio_t *out;
	struct tdata tdata;
	Fcin_t save;
	int savtop = staktell();
	char *savptr = stakfreeze(0);
	register struct argnod *ap; 
	struct argnod *arglist=0;
	char *name;
	char *subscript=0;
	tdata.sh = sh_getinterp();
	stakseek(ARGVAL);
	stakputs("\"${!");
	stakputs(nv_name(np));
#ifdef SHOPT_COMPOUND_ARRAY
	if(subscript = nv_getsub(np))
	{
		stakputc('[');
		stakputs(subscript);
		stakputc(']');
		stakputc('.');
	}
#endif /* SHOPT_COMPOUND_ARRAY */
	stakputs("@}\"");
	ap = (struct argnod*)stakfreeze(1);
	ap->argflag = ARG_MAC;
	fcsave(&save);
	sh_macexpand(ap,&arglist,0);
	fcrestore(&save);
	ap->argval[strlen(ap->argval)-(subscript?4:3)] = 0;
	name = (char*)&ap->argval[4];
	ap = arglist;
	if(dlete)
		tdata.outfile = 0;
	else if(!(tdata.outfile=out))
		tdata.outfile = out =  sfnew((Sfio_t*)0,(char*)0,-1,-1,SF_WRITE|SF_STRING);
	else
		sfseek(tdata.outfile,0L,SEEK_SET);
	tdata.prefix = "typeset";
	tdata.aflag = '=';
	genvalue(ap,name,0,0,&tdata);
	stakset(savptr,savtop);
	if(!tdata.outfile)
		return((char*)0);
	sfputc(out,0);
	return((char*)out->data);
}

/*
 * get discipline for compound initializations
 */
static char *get_tree(register Namval_t *np, Namfun_t *fp)
{
	NOT_USED(fp);
	return(walk_tree(np,0));
}

/*
 * put discipline for compound initializations
 */
static void put_tree(register Namval_t *np, const char *val, int flags,Namfun_t *fp)
{
	walk_tree(np,1);
	if(fp = nv_stack(np,NIL(Namfun_t*)))
	{
		free((void*)fp);
		if(np->nvalue.cp && !nv_isattr(np,NV_NOFREE))
			free((void*)np->nvalue.cp);
	}
	if(val)
		nv_putval(np,val,flags);
}

/*
 * Insert discipline to cause $x to print current tree
 */
void nv_setvtree(register Namval_t *np)
{
	register Namfun_t *nfp = newof(NIL(void*),Namfun_t,1,0);
	nfp->disc = &treedisc;
	nv_stack(np, nfp);
}

/*
 * print the attributes of name value pair give by <np>
 */
static void	print_attribute(register Namval_t *np,void *data)
{
	register const Shtable_t *tp;
	register char *cp;
	register unsigned val;
	register unsigned mask;
	register unsigned attr;
	struct tdata *dp = (struct tdata*)data;
#ifdef SHOPT_OO
	Namval_t *nq;
	char *cclass=0;
#endif /* SHOPT_OO */
	if (attr=nv_isattr(np,~NV_DEFAULT))
	{
		if((attr&NV_NOPRINT)==NV_NOPRINT)
			attr &= ~NV_NOPRINT;
		if(!attr)
			return;
		if(dp->prefix)
			sfputr(dp->outfile,dp->prefix,' ');
		for(tp = shtab_attributes; *tp->sh_name;tp++)
		{
			val = tp->sh_number;
			mask = val;
			/*
			 * the following test is needed to prevent variables
			 * with E attribute from being given the F
			 * attribute as well
			*/
			if(val==(NV_INTEGER|NV_DOUBLE) && (attr&NV_EXPNOTE))
				continue;
			if(val&NV_INTEGER)
				mask |= NV_DOUBLE;
			else if(val&NV_HOST)
				mask = NV_HOST;
			if((attr&mask)==val)
			{
				if(val==NV_ARRAY)
				{
					Namarr_t *ap = nv_arrayptr(np);
					if(array_assoc(ap))
						cp = "associative";
					else
						cp = "indexed";
					if(!dp->prefix)
						sfputr(dp->outfile,cp,' ');
					else if(*cp=='i')
						continue;
				}
				if(dp->prefix)
				{
					if(*tp->sh_name=='-')
						sfprintf(dp->outfile,"%.2s ",tp->sh_name);
				}
				else
					sfputr(dp->outfile,tp->sh_name+2,' ');
		                if ((val&(NV_LJUST|NV_RJUST|NV_ZFILL)) && !(val&NV_INTEGER) && val!=NV_HOST)
					sfprintf(dp->outfile,"%d ",nv_size(np));
			}
		        if(val == NV_INTEGER && nv_isattr(np,NV_INTEGER))
			{
				if(nv_size(np) != 10)
				{
					if(nv_isattr(np, NV_DOUBLE))
						cp = "precision";
					else
						cp = "base";
					if(!dp->prefix)
						sfputr(dp->outfile,cp,' ');
					sfprintf(dp->outfile,"%d ",nv_size(np));
				}
				break;
			}
		}
#ifdef SHOPT_OO
		if(nq=nv_class(np))
		{
			if(dp->prefix && *dp->prefix=='t')
				cclass = "-C";
			else if(!dp->prefix)
				cclass = "class";
			if(cclass)
				sfprintf(dp->outfile,"%s %s ",cclass,nv_name(nq));
		
		}
#endif /* SHOPT_OO */
		if(dp->aflag)
			return;
		sfputr(dp->outfile,nv_name(np),'\n');
	}
}

/*
 * print the nodes in tree <root> which have attributes <flag> set
 * of <option> is non-zero, no subscript or value is printed.
 */

static void print_scan(Sfio_t *file, int flag, Dt_t *root, int option,struct tdata *tp)
{
	register char **argv;
	register Namval_t *np;
	register int namec;
	Namval_t *onp = 0;
	flag &= ~NV_ASSIGN;
	tp->scanmask = flag&~NV_NOSCOPE;
	tp->scanroot = root;
	tp->outfile = file;
	if(flag&NV_INTEGER)
		tp->scanmask |= (NV_DOUBLE|NV_EXPNOTE);
	namec = nv_scan(root,nullscan,(void*)0,tp->scanmask,flag);
	argv = tp->argnam  = (char**)stakalloc((namec+1)*sizeof(char*));
	namec = nv_scan(root, pushname, (void*)tp, tp->scanmask, flag);
	strsort(argv,namec,strcoll);
	while(namec--)
	{
		if((np=nv_search(*argv++,root,0)) && np!=onp && (!nv_isnull(np) || np->nvfun || nv_isattr(np,~NV_NOFREE)))
		{
			onp = np;
			if((flag&NV_ARRAY) && nv_aindex(np)>=0)
				continue;
			if(!flag && nv_isattr(np,NV_ARRAY))
			{
				if(array_elem(nv_arrayptr(np))==0)
					continue;
				nv_putsub(np,NIL(char*),ARRAY_SCAN);
				do
				{
					print_namval(file,np,option,tp);
				}
				while(!option && nv_nextsub(np));
			}
			else
				print_namval(file,np,option,tp);
		}
	}
}

/*
 * add the name of the node to the argument list argnam
 */

static void pushname(Namval_t *np,void *data)
{
	struct tdata *tp = (struct tdata*)data;
	*tp->argnam++ = nv_name(np);
}

