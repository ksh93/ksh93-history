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
*      If you have copied this software without agreeing       *
*      to the terms of the license you are infringing on       *
*         the license and copyright and are violating          *
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
 * G. S. Fowler
 * D. G. Korn
 * AT&T Labs
 *
 * arithmetic expression evaluator
 *
 * NOTE: all operands are evaluated as both the parse
 *	 and evaluation are done on the fly
 */

#include	"streval.h"
#include	<ctype.h>
#include	<error.h>
#include	"FEATURE/externs"

#ifndef ERROR_dictionary
#   define ERROR_dictionary(s)	(s)
#endif

#define MAXLEVEL	9

struct vars			 /* vars stacked per invocation		*/
{
	const char*	nextchr; /* next char in current expression	*/
	const char*	errchr;	 /* next char after error		*/
	struct lval	errmsg;	 /* error message text			*/
	const char*	errstr;  /* error string			*/
	unsigned char	paren;	 /* parenthesis level			*/
	char		isfloat; /* set when floating number		*/
	char		wascomma;/* incremented by comma operator	*/
	double		ovalue;  /* value at comma operator		*/
};

typedef double (*Math_1_f)(double);
typedef double (*Math_2_f)(double,double);

#define getchr()	(*cur.nextchr++)
#define peekchr()	(*cur.nextchr)
#define ungetchr()	(cur.nextchr--)

#if ('a'==97)	/* ASCII encodings */
#   define getop(c)	(((c) >= sizeof(strval_states))? \
			((c)=='|' ? A_OR: ((c)=='^'?A_XOR:A_REG)): \
			strval_states[(c)])
#else
#   define getop(c)	(isdigit(c)?A_DIG:((c==' '||c=='\t' ||c=='\n')?0: \
			(c=='<'?A_LT:(c=='>'?A_GT:(c=='='?A_ASSIGN: \
			(c=='+'?A_PLUS:(c=='-'?A_MINUS:(c=='*'?A_TIMES: \
			(c=='/'?A_DIV:(c=='%'?A_MOD:(c==','?A_COMMA: \
			(c=='&'?A_AND:(c=='!'?A_NOT:(c=='('?A_LPAR: \
			(c==')'?A_RPAR:(c==0?A_EOF:(c==':'?A_COLON: \
			(c=='?'?A_QUEST:(c=='|'?A_OR:(c=='^'?A_XOR: \
			(c=='.'?A_DOT:A_REG)))))))))))))))))))))
#endif

#define pushchr(s)	{struct vars old;old=cur;cur.nextchr=((char*)(s));cur.errmsg.value=0;cur.errstr=0;cur.paren=0;
#define popchr()	cur=old;}
#define seterror(msg,n)	_seterror(ERROR_dictionary(msg),n)
#define ERROR(msg)	return(seterror(msg,n))

static struct vars	cur;
static char		noassign;	/* set to skip assignment	*/
static int		level;
static double		(*convert)(const char**,struct lval*,int,double);
				/* external conversion routine		*/
static double		expr(int);	/* subexpression evaluator	*/
static double		_seterror(const char[],double);	/* set error message string	*/


/*
 * evaluate an integer arithmetic expression in s
 *
 * (double)(*convert)(char** end, struct lval* string, int type, double value)
 *     is a user supplied conversion routine that is called when unknown 
 *     chars are encountered.
 * *end points to the part to be converted and must be adjusted by convert to
 * point to the next non-converted character; if typ is ERRMSG then string
 * points to an error message string
 *
 * NOTE: (*convert)() may call strval()
 */

double strval(const char *s, char** end, double(*conv)(const char**,struct lval*,int,double))
{
	register double	n;
	int wasfloat;

	pushchr(s);
	cur.isfloat = 0;
	convert = conv;
	if(level++ >= MAXLEVEL)
		n = seterror(e_recursive,0);
	else
		n = expr(0);
	if (cur.errmsg.value)
	{
		if(cur.errstr) s = cur.errstr;
		(void)(*convert)( &s , &cur.errmsg, ERRMSG, n);
		cur.nextchr = cur.errchr;
	}
	if (end) *end = (char*)cur.nextchr;
	if(level>0) level--;
	wasfloat = cur.isfloat;
	popchr();
	cur.isfloat |= wasfloat;
	return(n);
}

/*   
 * evaluate a subexpression with precedence
 */

static double expr(register int precedence)
{
	register int	c, op;
	register double	n=0, x;
	int		wasop, incr=0;
	struct lval	lvalue, assignop;
	const char	*pos;
	char		invalid=0;

	while ((c=getchr()) && isspace(c));
	switch (c)
	{
	case 0:
		if(precedence>5)
			ERROR(e_moretokens);
		return(0);

	case '-':
		incr = -2;
		/* FALL THRU */
	case '+':
		incr++;
		if(c != peekchr())
		{
			/* unary plus or minus */
			n = incr*expr(2*MAXPREC-1);
			incr = 0;
		}
		else /* ++ or -- */
		{
			invalid = 1;
			getchr();
		}
		break;

	case '!':
		n = !expr(2*MAXPREC-1);
		break;
	case '~':
	{
		long nl;
		n = expr(2*MAXPREC-1);
		nl = (long)n;
		if(nl != n)
			ERROR(e_incompatible);
		else
			n = ~nl;
		break;
	}
	default:
		ungetchr();
		invalid = 1;
		break;
	}
	wasop = invalid;
	lvalue.value = 0;
	lvalue.fun = 0;
	while(1)
	{
		cur.errchr = cur.nextchr;
		c = getchr();
		if((op=getop(c))==0)
			continue;
		switch(op)
		{
			case A_EOF:
				ungetchr();
				break;
			case A_DIG:
#ifdef future
				n = c - '0';
				while((c=getchr()), isdigit(c))
					n = (10*n) + (c-'0');
				wasop = 0;
				ungetchr();
				continue;
#endif
			case A_REG:	case A_DOT:
				op = 0;
				break;
			case A_QUEST:
				if(*cur.nextchr==':')
				{
					cur.nextchr++;
					op = A_QCOLON;
				}
				break;
			case A_LT:	case A_GT: 
				if(*cur.nextchr==c)
				{
					cur.nextchr++;
					op -= 2;
					break;
				}
				/* FALL THRU */
			case A_NOT:	case A_COLON:
				c = '=';
				/* FALL THRU */
			case A_ASSIGN:
			case A_PLUS:	case A_MINUS:
			case A_OR:	case A_AND:
				if(*cur.nextchr==c)
				{
					cur.nextchr++;
					op--;
				}
		}
		assignop.value = 0;
		if(op && wasop++ && op!=A_LPAR)
			ERROR(e_synbad);
		/* check for assignment operation */
		if(peekchr()== '=' && !(strval_precedence[op]&NOASSIGN))
		{
			if(!noassign && (!lvalue.value || precedence > 2))
				ERROR(e_notlvalue);
			assignop = lvalue;
			getchr();
			c = 3;
		}
		else
		{
			c = (strval_precedence[op]&PRECMASK);
			c *= 2;
		}
		/* from here on c is the new precedence level */
		if(lvalue.value && (op!=A_ASSIGN))
		{
			if(noassign)
				n = 1;
			else
			{
				pos = cur.nextchr;
				n = (*convert)(&cur.nextchr, &lvalue, VALUE, n);
				if (cur.nextchr!=pos)
				{
					if(cur.errmsg.value = lvalue.value)
						cur.errstr = cur.nextchr;
					ERROR(e_synbad);
				}
			}
			if(cur.nextchr==0)
				ERROR(e_badnum);
			if(!(strval_precedence[op]&SEQPOINT))
				lvalue.value = 0;
			invalid = 0;
		}
		if(invalid && op>A_ASSIGN)
			ERROR(e_synbad);
		if(precedence >= c)
			goto done;
		if(strval_precedence[op]&RASSOC)
			c--;
		if(c < 2*MAXPREC && !(strval_precedence[op]&SEQPOINT))
		{
			wasop = 0;
			x = expr(c);
		}
		if((strval_precedence[op]&NOFLOAT) && !noassign && cur.isfloat)
			ERROR(e_incompatible);
		switch(op)
		{
		case A_RPAR:
			if(!cur.paren)
				ERROR(e_paren);
			if(invalid)
				ERROR(e_synbad);
			goto done;

		case A_COMMA:
			wasop = 0;
			cur.wascomma++;
			cur.ovalue = n;
			n = expr(c);
			lvalue.value = 0;
			break;

		case A_LPAR:
		{
			char	savefloat = cur.isfloat;
			int	savecomma = cur.wascomma;
			double (*fun)(double,...);
			int nargs = lvalue.nargs;
			fun = lvalue.fun;
			lvalue.fun = 0;
			cur.wascomma=0;
			cur.isfloat = 0;
			if(!invalid)
				ERROR(e_synbad);
			cur.paren++;
			n = expr(1);
			cur.paren--;
			if(fun)
			{
				if(cur.wascomma+1 != nargs)
					ERROR(e_argcount);
				if(cur.wascomma)
					n = (*((Math_2_f)fun))(cur.ovalue,n);
				else
					n = (*((Math_1_f)fun))(n);
				cur.isfloat = ((void*)fun!=(void*)floor);
			}
			cur.isfloat |= savefloat;
			cur.wascomma = savecomma;
			if (getchr() != ')')
				ERROR(e_paren);
			wasop = 0;
			break;
		}

		case A_PLUSPLUS:
			incr = 1;
			goto common;
		case A_MINUSMINUS:
			incr = -1;
		common:
			x = n;
			wasop=0;
		case A_ASSIGN:
			if(!noassign && !lvalue.value)
				ERROR(e_notlvalue);
			n = x;
			assignop = lvalue;
			lvalue.value = 0;
			break;

		case A_QUEST:
			if(!n)
				noassign++;
			x = expr(1);
			if(!n)
				noassign--;
			if(getchr()!=':')
				ERROR(e_questcolon);
			if(n)
			{
				n = x;
				noassign++;
				(void)expr(c);
				noassign--;
			}
			else
				n = expr(c);
			lvalue.value = 0;
			wasop = 0;
			break;

		case A_COLON:
			seterror(e_badcolon,n);
			break;

		case A_OR:
			n = (long)n | (long)x;
			break;

		case A_QCOLON:
		case A_OROR:
			if(n)
			{
				noassign++;
				expr(c);
				noassign--;
			}
			else
				n = expr(c);
			if(op==A_OROR)
				n = (n!=0);
			lvalue.value = 0;
			wasop=0;
			break;

		case A_XOR:
			n = (long)n ^ (long)x;
			break;

		case A_NOT:
			ERROR(e_synbad);

		case A_AND:
			n = (long)n & (long)x;
			break;

		case A_ANDAND:
			if(n==0)
			{
				noassign++;
				expr(c);
				noassign--;
			}
			else
				n = (expr(c)!=0);
			lvalue.value = 0;
			wasop=0;
			break;

		case A_EQ:
			n = n == x;
			break;

		case A_NEQ:
			n = n != x;
			break;

		case A_LT:
			n = n < x;
			break;

		case A_LSHIFT:
			n = (long)n << (long)x;
			break;

		case A_LE:
			n = n <= x;
			break;

		case A_GT:
			n = n > x;
			break;

		case A_RSHIFT:
			n = (long)n >> (long)x;
			break;

		case A_GE:
			n = n >= x;
			break;

		case A_PLUS:
			n +=  x;
			break;

		case A_MINUS:
			n -=  x;
			break;

		case A_TIMES:
			n *=  x;
			break;

		case A_DIV:
			if(x!=0)
			{
				if(cur.isfloat)
					n /=  x;
				else
					n =  (long)n / (long)x;
				break;
			}

		case A_MOD:
			if(x!=0)
				n = (long)n % (long)x;
			else if(!noassign)
				ERROR(e_divzero);
			break;

		default:
			if(!wasop)
				ERROR(e_synbad);
			wasop = 0;
			pos = --cur.nextchr;
			lvalue.isfloat = 0;
			n = (*convert)(&cur.nextchr, &lvalue, LOOKUP, n);
			if (cur.nextchr == pos)
			{
				if(cur.errmsg.value = lvalue.value)
					cur.errstr = pos;
				ERROR(e_synbad);
			}
			cur.isfloat |= lvalue.isfloat;
	
			/* check for function call */
			if(lvalue.fun)
				continue;
			/* this handles ++x and --x */
			if(!noassign && incr)
			{
				if(cur.isfloat)
					ERROR(e_incompatible);
				if(lvalue.value)
				{
					pos = cur.nextchr;
					n = (*convert)(&cur.nextchr, &lvalue, VALUE, n);
					if (cur.nextchr!=pos)
					{
						if(cur.errmsg.value = lvalue.value)
							cur.errstr=cur.nextchr;
						ERROR(e_synbad);
					}
				}
				n += incr;
				incr = 0;
				goto common;
			}
			break;
		}
		invalid = 0;
		if(!noassign && assignop.value)
		{
			/*
			 * Here the output of *convert must be reassigned to n
			 * in case a cast is done in *convert.
			 * The value of the increment must be subsequently
			 * subtracted for the postincrement return value 
			*/
			n=(*convert)(&cur.nextchr,&assignop,ASSIGN,n+incr);
			n -= incr;
		}
		incr = 0;
	}
 done:
	cur.nextchr = cur.errchr;
	return(n);
}

/*
 * set error message string
 */

static double _seterror(const char *msg, double n)
{
	if(!cur.errmsg.value)
		cur.errmsg.value = (char*)msg;
	cur.errchr = cur.nextchr;
	cur.nextchr = "";
	level = 0;
	return(n);
}

#ifdef _mem_name_exception
#undef error
    int matherr(struct exception *ep)
    {
	const char *message;
	switch(ep->type)
	{
	    case DOMAIN:
		message = ERROR_dictionary(e_domain);
		break;
	    case SING:
		message = ERROR_dictionary(e_singularity);
		break;
	    case OVERFLOW:
		message = ERROR_dictionary(e_overflow);
		break;
	    default:
		return(1);
	}
	level=0;
	errormsg(SH_DICT,ERROR_exit(1),message,ep->name);
	return(0);
    }
#endif /* _mem_name_exception */
