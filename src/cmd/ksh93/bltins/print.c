/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1982-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * echo [arg...]
 * print [-nrps] [-f format] [-u filenum] [arg...]
 * printf  format [arg...]
 *
 *   David Korn
 *   AT&T Labs
 *   research!dgk
 *
 */

#include	"defs.h"
#include	<error.h>
#include	<stak.h>
#include	"io.h"
#include	"name.h"
#include	"history.h"
#include	"builtins.h"
#include	"streval.h"
#include	<tm.h>

union types_t
{
	unsigned char	c;
	short		h;
	int		i;
	long		l;
	Sflong_t	ll;
	Sfdouble_t	ld;
	double		d;
	float		f;
	char		*s;
	int		*ip;
	char		**p;
};

#define fmtre(x)	(x)


    struct printf
    {
	Sffmt_t		hdr;
	int		argsize;
	int		intvar;
	char		**nextarg;
	char		cescape;
	char		err;
	Shell_t		*sh;
    };
#if SFIO_VERSION >= 19980401L
    static int		extend(Sfio_t*,void*, Sffmt_t*);
#else
    static int		getarg(Sfio_t*,void*, Sffmt_t*);
    static int		extend(Sfio_t*, void*, int, Sffmt_t*);
#endif
    static const char   preformat[] = "";
static char		*genformat(char*);
static int		fmtvecho(const char*, struct printf*);

struct print
{
	Shell_t         *sh;
	const char	*options;
	char		raw;
	char		echon;
};

static char 	*nullarg;

/*
 * Need to handle write failures to avoid locking output pool
 */
static int outexceptf(Sfio_t* iop, int mode, void* data, Sfdisc_t* dp)
{
	if(mode==SF_DPOP || mode==SF_FINAL)
		free((void*)dp);
	else if(mode==SF_WRITE)
	{
		int save = errno;
		sfpurge(iop);
		sfpool(iop,NIL(Sfio_t*),SF_WRITE);
		errno = save;
		errormsg(SH_DICT,ERROR_system(1),e_badwrite,sffileno(iop));
	}
	return(0);
}

#ifndef	SHOPT_ECHOPRINT
   int    B_echo(int argc, char *argv[],void *extra)
   {
	static char bsd_univ;
	struct print prdata;
	prdata.options = sh_optecho+5;
	prdata.raw = prdata.echon = 0;
	prdata.sh = (Shell_t*)extra;
	NOT_USED(argc);
	/* This mess is because /bin/echo on BSD is different */
	if(!prdata.sh->universe)
	{
		register char *universe;
		if(universe=astconf("_AST_UNIVERSE",0,0))
			bsd_univ = (strcmp(universe,"ucb")==0);
		prdata.sh->universe = 1;
	}
	if(!bsd_univ)
		return(b_print(0,argv,&prdata));
	prdata.options = sh_optecho;
	prdata.raw = 1;
	if(argv[1] && strcmp(argv[1],"-n")==0)
		prdata.echon = 1;
	return(b_print(0,argv+prdata.echon,&prdata));
   }
#endif /* SHOPT_ECHOPRINT */

int    b_printf(int argc, char *argv[],void *extra)
{
	struct print prdata;
	NOT_USED(argc);
	prdata.sh = (Shell_t*)extra;
	prdata.options = sh_optprintf;
	return(b_print(-1,argv,&prdata));
}

/*
 * argc==0 when called from echo
 * argc==-1 when called from printf
 */

int    b_print(int argc, char *argv[], void *extra)
{
	register Sfio_t *outfile;
	register int exitval=0,n, fd = 1;
	register Shell_t *shp = (Shell_t*)extra;
	const char *options, *msg = e_file+4;
	char *format = 0;
	int sflag = 0, nflag, rflag;
	if(argc>0)
	{
		options = sh_optprint;
		nflag = rflag = 0;
		format = 0;
	}
	else
	{
		struct print *pp = (struct print*)extra;
		shp = pp->sh;
		options = pp->options;
		if(argc==0)
		{
			nflag = pp->echon;
			rflag = pp->raw;
			argv++;
			goto skip;
		}
	}
	while((n = optget(argv,options))) switch(n)
	{
		case 'n':
			nflag++;
			break;
		case 'p':
			fd = shp->coutpipe;
			msg = e_query;
			break;
		case 'f':
			format = opt_info.arg;
			break;
		case 's':
			/* print to history file */
			if(!sh_histinit())
				errormsg(SH_DICT,ERROR_system(1),e_history);
			fd = sffileno(shp->hist_ptr->histfp);
			sh_onstate(SH_HISTORY);
			sflag++;
			break;
		case 'e':
			rflag = 0;
			break;
		case 'r':
			rflag = 1;
			break;
		case 'u':
			fd = (int)strtol(opt_info.arg,&opt_info.arg,10);
			if(*opt_info.arg)
				fd = -1;
			else if(fd<0 || fd >= shp->lim.open_max)
				fd = -1;
			else if(sh_inuse(fd) || (shp->hist_ptr && fd==sffileno(shp->hist_ptr->histfp)))
				fd = -1;
			break;
		case ':':
			/* The following is for backward compatibility */
#if OPT_VERSION >= 19990123
			if(strcmp(opt_info.name,"-R")==0)
#else
			if(strcmp(opt_info.option,"-R")==0)
#endif
			{
				rflag = 1;
				if(error_info.errors==0)
				{
					argv += opt_info.index+1;
					/* special case test for -Rn */
					if(strchr(argv[-1],'n'))
						nflag++;
					if(*argv && strcmp(*argv,"-n")==0)
					{

						nflag++;
						argv++;
					}
					goto skip2;
				}
			}
			else
				errormsg(SH_DICT,2, opt_info.arg);
			break;
		case '?':
			errormsg(SH_DICT,ERROR_usage(2), "%s", opt_info.arg);
			break;
	}
	argv += opt_info.index;
	if(error_info.errors || (argc<0 && !(format = *argv++)))
		errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
skip:
	if(format)
		format = genformat(format);
	/* handle special case of '-' operand for print */
	if(argc>0 && *argv && strcmp(*argv,"-")==0 && strcmp(argv[-1],"--"))
		argv++;
skip2:
	if(fd < 0)
	{
		errno = EBADF;
		n = 0;
	}
	else if(!(n=shp->fdstatus[fd]))
		n = sh_iocheckfd(fd);
	if(!(n&IOWRITE))
	{
		/* don't print error message for stdout for compatibility */
		if(fd==1)
			return(1);
		errormsg(SH_DICT,ERROR_system(1),msg);
	}
	if(!(outfile=shp->sftable[fd]))
	{
		Sfdisc_t *dp;
		sh_onstate(SH_NOTRACK);
		n = SF_WRITE|((n&IOREAD)?SF_READ:0);
		shp->sftable[fd] = outfile = sfnew(NIL(Sfio_t*),shp->outbuff,IOBSIZE,fd,n);
		sh_offstate(SH_NOTRACK);
		sfpool(outfile,shp->outpool,SF_WRITE);
		if(dp = new_of(Sfdisc_t,0))
		{
			dp->exceptf = outexceptf;
			dp->seekf = 0;
			dp->writef = 0;
			dp->readf = 0;
			sfdisc(outfile,dp);
		}
	}
	/* turn off share to guarantee atomic writes for printf */
	n = sfset(outfile,SF_SHARE|SF_PUBLIC,0);
	if(format)
	{
		/* printf style print */
		Sfio_t *pool;
		struct printf pdata;
		pdata.sh = shp;
		pdata.err = 0;
		pdata.cescape = 0;
		memset(&pdata, 0, sizeof(pdata));
#if SFIO_VERSION >= 19970311L
		pdata.hdr.version = SFIO_VERSION;
#endif
		pdata.hdr.extf = extend;
#if SFIO_VERSION < 19980401L
		pdata.hdr.argf = getarg;
		pdata.hdr.form = format;
#endif
		pdata.nextarg = argv;
		sh_offstate(SH_STOPOK);
		pool=sfpool(sfstderr,NIL(Sfio_t*),SF_WRITE);
		do
		{
			if(shp->trapnote&SH_SIGSET)
				break;
#if SFIO_VERSION >= 19980401L
			pdata.hdr.form = format;
#endif
			sfprintf(outfile,"%!",&pdata);
		} while(*pdata.nextarg && pdata.nextarg!=argv);
		if(pdata.nextarg == &nullarg && pdata.argsize>0)
			sfwrite(outfile,stakptr(staktell()),pdata.argsize);
		sfpool(sfstderr,pool,SF_WRITE);
		exitval = pdata.err;
	}
	else
	{
		/* echo style print */
		if(sh_echolist(outfile,rflag,argv) && !nflag)
			sfputc(outfile,'\n');
	}
	if(sflag)
	{
		hist_flush(shp->hist_ptr);
		sh_offstate(SH_HISTORY);
	}
	else if(n&SF_SHARE)
	{
		sfset(outfile,SF_SHARE|SF_PUBLIC,1);
		sfsync(outfile);
	}
	return(exitval);
}

/*
 * echo the argument list onto <outfile>
 * if <raw> is non-zero then \ is not a special character.
 * returns 0 for \c otherwise 1.
 */

int sh_echolist(Sfio_t *outfile, int raw, char *argv[])
{
	register char	*cp;
	register int	n;
	struct printf pdata;
	pdata.cescape = 0;
	pdata.err = 0;
	while(!pdata.cescape && (cp= *argv++))
	{
		if(!raw  && (n=fmtvecho(cp,&pdata))>=0)
		{
			if(n)
				sfwrite(outfile,stakptr(staktell()),n);
		}
		else
			sfputr(outfile,cp,-1);
		if(*argv)
			sfputc(outfile,' ');
		sh_sigcheck();
	}
	return(!pdata.cescape);
}

/*
 * modified version of stresc for generating formats
 */
static char strformat(char *s)
{
        register char*  t;
        register int    c;
        char*           b;
        char*           p;

        b = t = s;
        for (;;)
        {
                switch (c = *s++)
                {
                    case '\\':
			if(*s==0)
				break;
                        c = chresc(s - 1, &p);
			if(c=='%')
				*t++ = '%';
			else if(c==0)
			{
				*t++ = '%';
				c = 'Z';
			}
                        s = p;
                        break;
                    case 0:
                        *t = 0;
                        return(t - b);
                }
                *t++ = c;
        }
}


static char *genformat(char *format)
{
	register char *fp;
	stakseek(0);
	stakputs(preformat);
	stakputs(format);
	fp = (char*)stakfreeze(1);
	strformat(fp+sizeof(preformat)-1);
	return(fp);
}

#if SFIO_VERSION >= 19980401L

static char *fmthtml(const char *string)
{
	register const char *cp = string;
	register int c, offset = staktell();
	while(c= *(unsigned char*)cp++)
	{
#ifdef SHOPT_MULTIBYTE
		register int s;
		if((s=mblen(cp-1,MB_CUR_MAX)) > 1)
		{
			cp += (s-1);
			continue;
		}
#endif /* SHOPT_MULTIBYTE */
		if(c=='<')
			stakputs("&lt;");
		else if(c=='>')
			stakputs("&gt;");
		else if(c=='&')
			stakputs("&amp;");
		else if(c=='"')
			stakputs("&quot;");
		else if(c=='\'')
			stakputs("&apos;");
		else if(c==' ')
			stakputs("&nbsp;");
		else if(!isprint(c) && c!='\n' && c!='\r')
			sfprintf(stkstd,"&#%X;",c);
		else
			stakputc(c);
	}
	stakputc(0);
	return(stakptr(offset));
}

static int extend(Sfio_t* sp, void* v, Sffmt_t* fe)
{
	char*		lastchar = "";
	register int	neg = 0;
	double		d;
	double		longmin = LONG_MIN;
	double		longmax = LONG_MAX;
	int		format = fe->fmt;
	int		n;
	union types_t*	value = (union types_t*)v;
	struct printf*	pp = (struct printf*)fe;
	register char*	argp = *pp->nextarg;

	fe->flags |= SFFMT_VALUE;
	if(!argp || format=='Z')
	{
		switch(format)
		{
		case 'c':
			value->c = 0;
			break;
		case 's':
		case 'q':
		case 'H':
		case 'P':
		case 'R':
		case 'Z':
		case 'b':
			fe->fmt = 's';
			fe->size = -1;
			fe->base = -1;
			value->s = "";
			break;
		case 'e':
		case 'f':
			value->f = 0.;
			break;
		case 'E':
		case 'G':
		case 'F':
			value->d = 0.;
			break;
		case 'n':
			value->ip = &pp->intvar;
			break;
		case 'Q':
			value->l = 0;
			break;
		case 'T':
			fe->fmt = 'd';
			value->l = time(NIL(time_t*));
			break;
		default:
			fe->fmt = 'd';
			value->l = 0;
			break;
		}
	}
	else
	{
		switch(format)
		{
		case 'p':
			value->p = (char**)strtol(argp,&lastchar,10);
			break;
		case 'n':
		{
			Namval_t *np;
			np = nv_open(argp,sh.var_tree,NV_VARNAME|NV_NOASSIGN|NV_ARRAY);
			nv_unset(np);
			nv_onattr(np,NV_INTEGER);
			if (np->nvalue.lp = new_of(long,0))
				*np->nvalue.lp = 0;
			nv_setsize(np,10);
			if(sizeof(int)==sizeof(long))
				value->ip = (int*)np->nvalue.lp;
			else
			{
				long sl = 1;
				value->ip = (int*)(((char*)np->nvalue.lp) + (*((char*)&sl) ? 0 : sizeof(int)));
			}
			nv_close(np);
			break;
		}
		case 'q':
		case 'b':
		case 's':
		case 'H':
		case 'P':
		case 'R':
			fe->fmt = 's';
			fe->size = -1;
			if(format=='s' && fe->base>=0)
			{
				value->p = pp->nextarg;
				pp->nextarg = (char**)&nullarg;
			}
			else
			{
				fe->base = -1;
				value->s = argp;
			}
			break;
		case 'c':
			if(fe->base >=0)
				value->s = argp;
			else
				value->c = *argp;
			break;
		case 'o':
		case 'x':
		case 'X':
		case 'u':
		case 'U':
			longmax = (unsigned long)ULONG_MAX;
		case '.':
			if(fe->size==2 && strchr("bcsqHPRQTZ",*fe->form))
			{
				value->l = ((unsigned char*)argp)[0];
				break;
			}
		case 'd':
		case 'D':
		case 'i':
			switch(*argp)
			{
			case '\'':
			case '"':
				value->l = ((unsigned char*)argp)[1];
				break;
			default:
				d = sh_strnum(argp,&lastchar,0);
				if(d<longmin)
				{
					errormsg(SH_DICT,ERROR_warn(0),e_overflow,argp);
					pp->err = 1;
					d = longmin;
				}
				else if(d>longmax)
				{
					errormsg(SH_DICT,ERROR_warn(0),e_overflow,argp);
					pp->err = 1;
					d = longmax;
				}
				value->l = (long)d;
				if(lastchar == *pp->nextarg)
				{
					value->l = *argp;
					lastchar = "";
				}
				break;
			}
			if(neg)
				value->l = -value->l;
			fe->size = sizeof(value->l);
			break;
		case 'e':
		case 'f':
		case 'E':
		case 'F':
		case 'G':
			value->d = sh_strnum(*pp->nextarg,&lastchar,0);
			fe->size = sizeof(value->d);
			break;
		case 'Q':
			value->l = (long)strelapsed(*pp->nextarg,&lastchar,1);
			break;
		case 'T':
			value->l = (long)tmdate(*pp->nextarg,&lastchar,NIL(time_t*));
			break;
		default:
			value->l = 0;
			fe->fmt = 'd';
			fe->size = sizeof(value->l);
			errormsg(SH_DICT,ERROR_exit(1),e_formspec,format);
			break;
		}
		if(*lastchar)
		{
			errormsg(SH_DICT,ERROR_warn(0),e_argtype,format);
			pp->err = 1;
		}
		pp->nextarg++;
	}
	switch(format)
	{
	case 'Z':
		fe->fmt = 'c';
		fe->base = -1;
		value->c = 0;
		break;
	case 'b':
		if((n=fmtvecho(value->s,pp))>=0)
		{
			if(pp->nextarg == &nullarg)
			{
				pp->argsize = n;
				return -1;
			}
			value->s = stakptr(staktell());
		}
		break;
	case 'H':
		value->s = fmthtml(value->s);
		break;
	case 'q':
		value->s = sh_fmtq(value->s);
		break;
	case 'P':
		value->s = fmtmatch(value->s);
		if(*value->s==0)
			errormsg(SH_DICT,ERROR_exit(1),e_badregexp,value->s);
		break;
	case 'R':
		value->s = fmtre(value->s);
		if(*value->s==0)
			errormsg(SH_DICT,ERROR_exit(1),e_badregexp,value->s);
		break;
	case 'Q':
		if (fe->n_str>0)
		{
			fe->fmt = 'd';
			fe->size = sizeof(value->l);
		}
		else
		{
			value->s = fmtelapsed(value->l, 1);
			fe->fmt = 's';
			fe->size = -1;
		}
		break;
	case 'T':
		if(fe->n_str>0)
		{
			n = fe->t_str[fe->n_str];
			fe->t_str[fe->n_str] = 0;
			value->s = fmttime(fe->t_str, value->l);
			fe->t_str[fe->n_str] = n;
		}
		else value->s = fmttime(NIL(char*), value->l);
		fe->fmt = 's';
		fe->size = -1;
		break;
	}
	return 0;
}
#else
static int getarg(Sfio_t *sp, void* v, Sffmt_t* fe)
{
	register char *argp = *nextarg;
	char *lastchar = "";
	register int neg = 0;
	double d, longmin= LONG_MIN, longmax=LONG_MAX;
	int size;
	int format = fe->fmt;
	union types_t *value = (union types_t*)v;
	struct printf* pp = (struct printf*)fe;
	if(!argp || format=='Z')
	{
		switch(format)
		{
			case 'c':
				value->c = 0;
				break;
			case 's':
			case 'q':
			case 'P':
			case 'R':
			case 'Z':
			case 'b':
				value->s = "";
				break;
			case 'e':
			case 'f':
				value->f = 0.;
				break;
			case 'E':
			case 'G':
			case 'F':
				value->d = 0.;
				break;
			case 'n':
			{
				value->ip = &pp->intvar;
				break;
			}
			default:
				value->l = 0;
		}
		return(1);
	}
	switch(format)
	{
		case 'p':
			value->p = (char**)strtol(argp,&lastchar,10);
			break;
		case 'n':
		{
			Namval_t *np;
			np = nv_open(argp,sh.var_tree,NV_VARNAME|NV_NOASSIGN|NV_ARRAY);
			nv_unset(np);
			nv_onattr(np,NV_INTEGER);
			if (np->nvalue.lp = new_of(long,0))
				*np->nvalue.lp = 0;
			nv_setsize(np,10);
			if(sizeof(int)==sizeof(long))
				value->ip = (int*)np->nvalue.lp;
			else
			{
				long sl = 1;
				value->ip = (int*)(((char*)np->nvalue.lp) + (*((char*)&sl) ? 0 : sizeof(int)));
			}
			nv_close(np);
			break;
		}
		case 'q':
		case 'b':
		case 's':
		case 'P':
		case 'R':
			value->s = argp;
			break;
		case 'c':
			value->c = *argp;
			break;
		case 'o':
		case 'x':
		case 'X':
		case 'u':
		case 'U':
			longmax = (unsigned long)ULONG_MAX;
		case 'd':
		case 'D':
		case 'i':
			size = sizeof(int);
			if(pp->hdr.flag=='h')
				size = sizeof(short);
			if(pp->hdr.flag=='l' || pp->hdr.flag=='L')
				size = (pp->hdr.n_flag>1?sizeof(Sfulong_t):sizeof(long));
			switch(*argp)
			{
				case '\'': case '"':
					value->l = ((unsigned char*)argp)[1];
					break;
				default:
					d = sh_strnum(argp,&lastchar,0);
					if(d<longmin)
					{
						errormsg(SH_DICT,ERROR_warn(0),e_overflow,argp);
						pp->err = 1;
						d = longmin;
					}
					else if(d>longmax)
					{
						errormsg(SH_DICT,ERROR_warn(0),e_overflow,argp);
						pp->err = 1;
						d = longmax;
					}
					value->l = (long)d;
					if(lastchar == *nextarg)
					{
						value->l = *argp;
						lastchar = "";
					}
			}
			if(neg)
				value->l = -value->l;
			if(size!=sizeof(long))
			{
				if(size==sizeof(int))
					value->i = (int)value->l;
				else if(size==sizeof(short))
					value->h = (short)value->l;
				else if(size==sizeof(Sflong_t))
					value->ll = (Sflong_t)value->l;
			}
			break;
		case 'e':
		case 'f':
		case 'E':
		case 'F':
		case 'G':
			size = sizeof(double);
			if(pp->hdr.flag=='h' || format=='f')
				size = sizeof(float);
			if((pp->hdr.flag=='l' || pp->hdr.flag=='L') && format!='f')
				size = sizeof(Sfdouble_t);
			value->d = sh_strnum(*nextarg,&lastchar,0);
			if(size!=sizeof(double))
			{
				if(size==sizeof(float))
					value->f = (float)value->d;
				if(size==sizeof(Sfdouble_t))
					value->ld = (Sfdouble_t)value->d;
			}
			break;
		default:
			value->l = 0;
			errormsg(SH_DICT,ERROR_exit(1),e_formspec,format);
	}
	if(*lastchar)
	{
		errormsg(SH_DICT,ERROR_warn(0),e_argtype,format);
		pp->err = 1;
	}
	nextarg++;
	return(1);
}

/*
 * This routine adds new % escape sequences to printf
 */
static int extend(Sfio_t* sp, void* val, int precis, Sffmt_t* fe)
{
	register int n;
	struct printf *pp = (struct printf*)fe;
	int format = fe->fmt;
	char *invalue = (char*)val;
	char **outval = &fe->t_str;
	NOT_USED(precis);
	switch(format)
	{
		case 'Z':
			*outval = invalue;
			n=1;
			break;
		case 'b':
			if((n=fmtvecho(invalue,pp))>=0)
			{
				if(nextarg == &nullarg)
				{
					*outval = 0;
					argsize = n;
					return(-1);
				}
				*outval = stakptr(staktell());
				break;
			}
			*outval = invalue;
			n = strlen(*outval);
			break;
		case 'q':
			*outval = sh_fmtq(invalue);
			n = strlen(*outval);
			break;
		case 'P':
			*outval = fmtmatch(invalue);
			if(*outval==0)
				errormsg(SH_DICT,ERROR_exit(1),e_badregexp,invalue);
			n = strlen(*outval);
			break;
		case 'R':
			*outval = fmtre(invalue);
			if(*outval==0)
				errormsg(SH_DICT,ERROR_exit(1),e_badregexp,invalue);
			n = strlen(*outval);
			break;
		default:
			*outval = 0;
			return(0);
	}
	fe->n_str = n;
	return(1);

}
#endif /* SFIO_VERSION */

/*
 * construct System V echo string out of <cp>
 * If there are not escape sequences, returns -1
 * Otherwise, puts null terminated result on stack, but doesn't freeze it
 * returns length of output.
 */

static int fmtvecho(const char *string, struct printf *pp)
{
	register const char *cp = string, *cpmax;
	register int c;
	register int offset = staktell();
#ifdef SHOPT_MULTIBYTE
	int chlen;
	if (MB_CUR_MAX > 1)
	{
		while(1)
		{
			if ((chlen = mblen(cp, MB_CUR_MAX)) > 1)
				/* Skip over multibyte characters */
				cp += chlen;
			else if((c= *cp++)==0 || c == '\\')
				break;
		}
	}
	else
#endif /* SHOPT_MULTIBYTE */
	while((c= *cp++) && (c!='\\'));
	if(c==0)
		return(-1);
	c = --cp - string;
	if(c>0)
		stakwrite((void*)string,c);
	for(; c= *cp; cp++)
	{
#ifdef SHOPT_MULTIBYTE
		if ((MB_CUR_MAX > 1) && ((chlen = mblen(cp, MB_CUR_MAX)) > 1))
		{
			stakwrite(cp,chlen);
			cp +=  (chlen-1);
			continue;
		}
#endif /* SHOPT_MULTIBYTE */
		if( c=='\\') switch(*++cp)
		{
			case 'E':
				c = ('a'==97?'\033':39); /* ASCII/EBCDIC */
				break;
			case 'a':
				c = '\a';
				break;
			case 'b':
				c = '\b';
				break;
			case 'c':
				pp->cescape++;
				pp->nextarg = (char**)&nullarg;
				goto done;
			case 'f':
				c = '\f';
				break;
			case 'n':
				c = '\n';
				break;
			case 'r':
				c = '\r';
				break;
			case 'v':
				c = '\v';
				break;
			case 't':
				c = '\t';
				break;
			case '\\':
				c = '\\';
				break;
			case '0':
				c = 0;
				cpmax = cp + 4;
				while(++cp<cpmax && *cp>='0' && *cp<='7')
				{
					c <<= 3;
					c |= (*cp-'0');
				}
			default:
				cp--;
		}
		stakputc(c);
	}
done:
	c = staktell()-offset;
	stakputc(0);
	stakseek(offset);
	return(c);
}
