/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1985-2000 AT&T Corp.              *
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
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*               Phong Vo <kpv@research.att.com>                *
*                                                              *
***************************************************************/
#pragma prototyped

/*
 * posix regex executor
 * single sized-record interface
 */

#include "reglib.h"

#define BEG_ALT		1	/* beginning of an alt			*/
#define BEG_ONE		2	/* beginning of one iteration of a rep	*/
#define BEG_REP		3	/* beginning of a repetition		*/
#define BEG_SUB		4	/* beginning of a subexpression		*/
#define END_ANY		5	/* end of any of above			*/

/*
 * returns from parse(). seemingly one might better handle
 * BAD by longjmp, but that would not work with threads
 */

#define NONE		0	/* no parse found			*/
#define GOOD		1	/* some parse was found			*/
#define BEST		2	/* an unbeatable parse was found	*/
#define BAD		3	/* error ocurred			*/

/*
 * REG_SHELL_DOT test
 */

#define LEADING(e,s)	((e)->leading&&*(s)=='.'&&((s)==(e)->beg||*((s)-1)==(e)->explicit))

/*
 * Pos_t is for comparing parses. An entry is made in the
 * array at the beginning and at the end of each Group_t,
 * each iteration in a Group_t, and each Binary_t.
 */

typedef struct
{
	unsigned char*	p;		/* where in string		*/
	short		serial;		/* preorder subpattern number	*/
	short		be;		/* which end of pair		*/
} Pos_t;

/* ===== begin library support ===== */

#define vector(t,v,i)	(((i)<(v)->max)?(t*)((v)->vec+(i)*(v)->siz):(t*)vecseek(&(v),i))

static Vector_t*
vecopen(int inc, int siz)
{
	Vector_t*	v;
	Stk_t*		sp;

	if (inc <= 0)
		inc = 16;
	if (!(sp = stkopen(STK_SMALL|STK_NULL)))
		return 0;
	if (!(v = (Vector_t*)stkseek(sp, sizeof(Vector_t) + inc * siz)))
	{
		stkclose(sp);
		return 0;
	}
	v->stk = sp;
	v->vec = (char*)v + sizeof(Vector_t);
	v->max = v->inc = inc;
	v->siz = siz;
	v->cur = 0;
	return v;
}

static void*
vecseek(Vector_t** p, int index)
{
	Vector_t*	v = *p;

	if (index >= v->max)
	{
		while ((v->max += v->inc) <= index);
		if (!(v = (Vector_t*)stkseek(v->stk, sizeof(Vector_t) + v->max * v->siz)))
			return 0;
		*p = v;
		v->vec = (char*)v + sizeof(Vector_t);
	}
	return v->vec + index * v->siz;
}

static void
vecclose(Vector_t* v)
{
	if (v)
		stkclose(v->stk);
}

typedef struct
{
	Stk_pos_t	pos;
	char		data[1];
} Stk_frame_t;

#define stknew(s,p)	((p)->offset=stktell(s),(p)->base=stkfreeze(s,0))
#define stkold(s,p)	stkset(s,(p)->base,(p)->offset)

#define stkframe(s)	(*((Stk_frame_t**)(s)->next-1))
#define stkdata(s,t)	((t*)stkframe(s)->data)
#define stkpop(s)	stkold(s,&(stkframe(s)->pos))

static void*
stkpush(Stk_t* sp, size_t size)
{
	Stk_frame_t*	f;
	Stk_pos_t	p;

	stknew(sp, &p);
	size = sizeof(Stk_frame_t) + sizeof(size_t) + size - 1;
	if (!(f = (Stk_frame_t*)stkalloc(sp, sizeof(Stk_frame_t) + sizeof(Stk_frame_t*) + size - 1)))
		return 0;
	f->pos = p;
	stkframe(sp) = f;
	return f->data;
}

/* ===== end library support ===== */

/*
 * Match_frame_t is for saving and restoring match records
 * around alternate attempts, so that fossils will not be
 * left in the match array.  These are the only entries in
 * the match array that are not otherwise guaranteed to
 * have current data in them when they get used.
 */

typedef struct
{
	size_t			size;
	regmatch_t*		match;
	regmatch_t		save[1];
} Match_frame_t;

#define matchframe	stkdata(stkstd,Match_frame_t)
#define matchpush(e,x)	((x)->re.group.number?_matchpush(e,x):0)
#define matchcopy(e,x)	((x)->re.group.number?memcpy(matchframe->match,matchframe->save,matchframe->size):(void*)0)
#define matchpop(e,x)	((x)->re.group.number?memcpy(matchframe->match,matchframe->save,matchframe->size),stkpop(stkstd):(void*)0)

#define pospop(e)	(--(e)->pos->cur)

/*
 * allocate a frame and push a match onto the stack
 */

static int
_matchpush(Env_t* env, Rex_t* rex)
{
	Match_frame_t*	f;
	regmatch_t*	m;
	regmatch_t*	e;
	regmatch_t*	s;
	int		num;

	if (rex->re.group.number <= 0 || (num = rex->re.group.last - rex->re.group.number + 1) <= 0)
		num = 0;
	if (!(f = (Match_frame_t*)stkpush(stkstd, sizeof(Match_frame_t) + (num - 1) * sizeof(regmatch_t))))
	{
		env->error = REG_ESPACE;
		return 1;
	}
	f->size = num * sizeof(regmatch_t);
	f->match = m = env->match + rex->re.group.number;
	e = m + num;
	s = f->save;
	while (m < e)
	{
		*s++ = *m;
		*m++ = state.nomatch;
	}
	return 0;
}

/*
 * allocate a frame and push a pos onto the stack
 */

static int
pospush(Env_t* env, Rex_t* rex, unsigned char* p, int be)
{
	Pos_t*	pos;

	if (!(pos = vector(Pos_t, env->pos, env->pos->cur)))
	{
		env->error = REG_ESPACE;
		return 1;
	}
	pos->serial = rex->serial;
	pos->p = p;
	pos->be = be;
	env->pos->cur++;
	return 0;
}

/*
 * two matches are known to have the same length
 * os is start of old pos array, ns is start of new,
 * oend and nend are end+1 pointers to ends of arrays.
 * oe and ne are ends (not end+1) of subarrays.
 * returns 1 if new is better, -1 if old, else 0.
 */

static int
better(Pos_t* os, Pos_t* ns, Pos_t* oend, Pos_t* nend)
{
	Pos_t*	oe;
	Pos_t*	ne;
	int	k;
	int	n;

	for (;;)
	{
		if (ns >= nend)
			return os < oend;
		if (os >= oend)
		{
			if (os->be != BEG_ALT)
				/* can control get here? */
				abort();
			return -1;
		}
		if (ns->serial > os->serial)
			return -1;
		if (os->serial > ns->serial)
			abort();
		if (ns->p > os->p)
			/* can control get here? */
			return 1;
		if (os->p > ns->p)
			return -1;
		oe = os;
		n = oe->serial;
		k = 0;
		for (;;)
			if ((++oe)->serial == n)
			{
				if (oe->be != END_ANY)
					k++;
				else if (k-- <= 0)
					break;
			}
		ne = ns;
		n = ne->serial;
		k = 0;
		for (;;)
			if ((++ne)->serial == n)
			{
				if (ne->be != END_ANY)
					k++;
				else if (k-- <= 0)
					break;
			}
		if (ne->p > oe->p)
			return 1;
		if (oe->p > ne->p)
			return -1;
		if (k = better(os + 1, ns + 1, oe, ne))
			return k;
		os = oe + 1;
		ns = ne + 1;
	}
}

#define follow(e,r,c,s)	((r)->next?parse(e,(r)->next,c,s):parse(e,c,0,s))

static int		parse(Env_t*, Rex_t*, Rex_t*, unsigned char*);

static int
parserep(Env_t* env, Rex_t* rex, Rex_t* cont, unsigned char* s, int n)
{
	int	i;
	int	r = NONE;

	if (n < rex->hi)
	{
		Rex_t	catcher;

		catcher.type = REX_REP_CATCH;
		catcher.serial = rex->serial;
		catcher.re.rep_catch.ref = rex;
		catcher.re.rep_catch.cont = cont;
		catcher.re.rep_catch.beg = s;
		catcher.re.rep_catch.n = n + 1;
		catcher.next = rex->next;
		if (env->stack)
		{
			if (matchpush(env, rex))
				return BAD;
			if (pospush(env, rex, s, BEG_ONE))	
				return BAD;
		}
		r = parse(env, rex->re.group.expr.rex, &catcher, s);
		if (env->stack)
		{
			pospop(env);
			matchpop(env, rex);
		}
		if (r == BEST || r == BAD)
			return r;
	}
	if (n < rex->lo)
		return r;
	if (env->stack && pospush(env, rex, s, END_ANY))
		return BAD;
	i = follow(env, rex, cont, s);
	if (env->stack)
		pospop(env);
	return i == NONE ? r : i;
}

static int
parsetrie(Env_t* env, Trie_node_t* x, Rex_t* rex, Rex_t* cont, unsigned char* s)
{
	unsigned char*	p = env->map;
	int		r;
	int		i;

	for (;;)
	{
		if (s >= env->end)
			return NONE;
		while (x->c != p[*s])
			if (!(x = x->sib))
				return NONE;
		if (x->end)
			break;
		x = x->son;
		s++;
	}
	s++;
	r = x->son ? parsetrie(env, x->son, rex, cont, s) : NONE;
	if (r == BEST || r == BAD)
		return r;
	i = follow(env, rex, cont, s);
	return i == NONE ? r : i;
}

static int
parse(Env_t* env, Rex_t* rex, Rex_t* cont, unsigned char* s)
{
	int		i;
	int		n;
	int		r;
	int*		f;
	unsigned char*	p;
	unsigned char*	t;
	unsigned char*	b;
	unsigned char*	e;
	regmatch_t*	m;
	Trie_node_t*	x;
	Rex_t		catcher;

	for (;;)
	{
		switch (rex->type)
		{
		case REX_ALT:
			if (matchpush(env, rex))
				return BAD;
			if (pospush(env, rex, s, BEG_ALT))
				return BAD;
			catcher.type = REX_ALT_CATCH;
			catcher.serial = rex->serial;
			catcher.re.alt_catch.cont = cont;
			catcher.next = rex->next;
			r = parse(env, rex->re.group.expr.binary.left, &catcher, s);
			if (r != BEST && r != BAD)
			{
				matchcopy(env, rex);
				((Pos_t*)env->pos->vec + env->pos->cur - 1)->serial = catcher.serial = rex->re.group.expr.binary.serial;
				n = parse(env, rex->re.group.expr.binary.right, &catcher, s);
				if (n != NONE)
					r = n;
			}
			pospop(env);
			matchpop(env, rex);
			return r;
		case REX_ALT_CATCH:
			if (pospush(env, rex, s, END_ANY))
				return BAD;
			r = follow(env, rex, rex->re.alt_catch.cont, s);
			pospop(env);
			return r;
		case REX_BACK:
			m = &env->match[rex->lo];
			if (m->rm_so < 0)
				return NONE;
			t = env->beg + m->rm_so;
			i = m->rm_eo - m->rm_so;
			if (s + i > env->end)
				return NONE;
			p = env->map;
			while (--i >= 0)
				if (p[*s++] != p[*t++])
					return NONE;
			break;
		case REX_BEG:
			if ((!(env->flags & REG_NEWLINE) || s <= env->beg || *(s - 1) != '\n') && ((env->flags & REG_NOTBOL) || s != env->beg))
				return NONE;
			break;
		case REX_CLASS:
			if (LEADING(env, s))
				return NONE;
			n = rex->hi;
			if (n > env->end - s)
				n = env->end - s;
			for (i = 0; i < n; i++)
				if (!settst(rex->re.charclass, s[i]))
					n = i;
			r = NONE;
			for (s += n; n-- >= rex->lo; s--)
				switch (follow(env, rex, cont, s))
				{
				case BAD:
					return BAD;
				case BEST:
					return BEST;
				case GOOD:
					r = GOOD;
					break;
				}
			return r;
		case REX_CONJ:
			{
				Rex_t	right;

				right.type = REX_CONJ_RIGHT;
				right.re.conj_right.cont = cont;
				right.next = rex->next;
				catcher.type = REX_CONJ_LEFT;
				catcher.re.conj_left.right = rex->re.group.expr.binary.right;
				catcher.re.conj_left.cont = &right;
				catcher.re.conj_left.beg = s;
				catcher.next = 0;
				return parse(env, rex->re.group.expr.binary.left, &catcher, s);
			}
		case REX_CONJ_LEFT:
			rex->re.conj_left.cont->re.conj_right.end = s;
			cont = rex->re.conj_left.cont;
			s = rex->re.conj_left.beg;
			rex = rex->re.conj_left.right;
			continue;
		case REX_CONJ_RIGHT:
			if (rex->re.conj_right.end != s)
				return NONE;
			cont = rex->re.conj_right.cont;
			break;
		case REX_DONE:
			if (!env->stack)
				return BEST;
			n = s - env->beg;
			r = env->regex->re_nsub;
			if ((i = env->best[0].rm_eo) >= 0)
			{
				if (n < i)
					return GOOD;
				if (n == i && better((Pos_t*)env->bestpos->vec,
				   		     (Pos_t*)env->pos->vec,
				   		     (Pos_t*)env->bestpos->vec+env->bestpos->cur,
				   		     (Pos_t*)env->pos->vec+env->pos->cur) <= 0)
					return GOOD;
			}
			env->best[0].rm_eo = n;
			memcpy(&env->best[1], &env->match[1], r * sizeof(regmatch_t));
			n = env->pos->cur;
			if (!vector(Pos_t, env->bestpos, n))
			{
				env->error = REG_ESPACE;
				return BAD;
			}
			env->bestpos->cur = n;
			memcpy(env->bestpos->vec, env->pos->vec, n * sizeof(Pos_t));
			return GOOD;
		case REX_DOT:
			if (LEADING(env, s))
				return NONE;
			n = rex->hi;
			if (n > env->end - s)
				n = env->end - s;
			if (env->explicit)
				for (i = 0; i < n; i++)
					if (s[i] == env->explicit)
						n = i;
			r = NONE;
			for (s += n; n-- >= rex->lo; s--)
				switch (follow(env, rex, cont, s))
				{
				case BAD:
					return BAD;
				case BEST:
					return BEST;
				case GOOD:
					r = GOOD;
					break;
				}
			return r;
		case REX_END:
			if ((!(env->flags & REG_NEWLINE) || *s != '\n') && ((env->flags & REG_NOTEOL) || s < env->end))
				return NONE;
			break;
		case REX_GROUP:
			if (env->stack)
			{
				env->match[rex->re.group.number].rm_so = s - env->beg;
				if (pospush(env, rex, s, BEG_SUB))
					return BAD;
				catcher.re.group_catch.eo = &env->match[rex->re.group.number].rm_eo;
			}
			catcher.type = REX_GROUP_CATCH;
			catcher.serial = rex->serial;
			catcher.re.group_catch.cont = cont;
			catcher.next = rex->next;
			r = parse(env, rex->re.group.expr.rex, &catcher, s);
			if (env->stack)
			{
				pospop(env);
				env->match[rex->re.group.number].rm_so = -1;
			}
			return r;
		case REX_GROUP_CATCH:
			if (env->stack)
			{
				*rex->re.group_catch.eo = s - env->beg;
				if (pospush(env, rex, s, END_ANY))
					return BAD;
			}
			r = follow(env, rex, rex->re.group_catch.cont, s);
			if (env->stack)
			{
				pospop(env);
				*rex->re.group_catch.eo = -1;
			}
			return r;
		case REX_KMP:
			p = env->map;
			f = rex->re.string.fail;
			b = rex->re.string.base;
			n = rex->re.string.size;
			t = s;
			e = env->end;
			while (t + n <= e)
			{
				for (i = -1; t < e; t++)
				{
					while (i >= 0 && b[i+1] != p[*t])
						i = f[i];
					if (b[i+1] == p[*t])
						i++;
					if (i + 1 == n)
					{
						t++;
						if (env->stack)
							env->best[0].rm_so = t - s - n;
						switch (follow(env, rex, cont, t))
						{
						case BEST:
						case GOOD:
							return BEST;
						case BAD:
							return BAD;
						}
						t -= n - 1;
						break;
					}
				}
			}
			return NONE;
		case REX_NEG:
			i = env->end - s;
			n = ((i + 7) >> 3) + 1;
			catcher.type = REX_NEG_CATCH;
			catcher.re.neg_catch.beg = s;
			if (!(p = (unsigned char*)stkpush(stkstd, n)))
				return BAD;
			memset(catcher.re.neg_catch.index = p, 0, n);
			catcher.next = rex->next;
			if (parse(env, rex->re.group.expr.rex, &catcher, s) == BAD)
				r = BAD;
			else
			{
				r = NONE;
				for (; i >= 0; i--)
					if (!bittst(p, i))
					{
						switch (follow(env, rex, cont, s + i))
						{
						case BAD:
							r = BAD;
							break;
						case BEST:
							r = BEST;
							break;
						case GOOD:
							r = GOOD;
							/*FALLTHROUGH*/
						default:
							continue;
						}
						break;
					}
			}
			stkpop(stkstd);
			return r;
		case REX_NEG_CATCH:
			bitset(rex->re.neg_catch.index, s - rex->re.neg_catch.beg);
			return NONE;
		case REX_NULL:
			break;
		case REX_ONECHAR:
			p = env->map;
			n = rex->hi;
			if (n > env->end - s)
				n = env->end - s;
			for (i = 0; i < n; i++, s++)
				if (p[*s] != rex->re.onechar)
					break;
			for (r = NONE; i-- >= rex->lo; s--)
				switch (follow(env, rex, cont, s))
				{
				case BAD:
					return BAD;
				case BEST:
					return BEST;
				case GOOD:
					r = GOOD;
					break;
				}
			return r;
		case REX_REP:
			if (env->stack && pospush(env, rex, s, BEG_REP))
				return BAD;
			r = parserep(env, rex, cont, s, 0);
			if (env->stack)
				pospop(env);
			return r;
		case REX_REP_CATCH:
			if (env->stack && pospush(env, rex, s, END_ANY))
				return BAD;
			if (rex->re.rep_catch.beg == s && rex->re.rep_catch.n > rex->re.rep_catch.ref->lo)
			{
				/*
				 * optional empty iteration
				 */

				if (!env->stack || (env->flags & REG_EXTENDED))
					r = NONE;
				else if (pospush(env, rex, s, END_ANY))
					r = BAD;
				else
				{
					r = follow(env, rex, rex->re.rep_catch.cont, s);
					pospop(env);
				}
			}
			else r = parserep(env, rex->re.rep_catch.ref, rex->re.rep_catch.cont, s, rex->re.rep_catch.n);
			if (env->stack)
				pospop(env);
			return r;
		case REX_STRING:
			if (s + rex->re.string.size > env->end)
				return NONE;
			p = env->map;
			t = rex->re.string.base;
			while (*t)
				if (p[*s++] != *t++)
					return NONE;
			break;
		case REX_TRIE:
			if (((s + rex->re.trie.min) > env->end) || !(x = rex->re.trie.root[env->map[*s]]))
				return NONE;
			return parsetrie(env, x, rex, cont, s);
		case REX_WBEG:
			if ((!(env->flags & REG_NEWLINE) || s <= env->beg || *(s - 1) != '\n') && ((env->flags & REG_NOTBOL) || s != env->beg) && (isalnum(*(s - 1)) || *(s - 1) == '_'))
				return NONE;
			break;
		case REX_WEND:
			if ((!(env->flags & REG_NEWLINE) || *s != '\n') && ((env->flags & REG_NOTEOL) || s < env->end) && (isalnum(*s) || *s == '_'))
				return NONE;
			break;
		}
		if (!(rex = rex->next))
			rex = cont;
	}
}

/*
 * returning REG_BADPAT or REG_ESPACE is not explicitly
 * countenanced by the standard
 */

int
regnexec(const regex_t* p, const char* s, size_t len, size_t nmatch, regmatch_t* match, int flags)
{
	register int	n;
	register int	i;
	int		j;
	int		k;
	Env_t*		env;
	Rex_t*		e;

	if (!s || !p || !(env = p->env))
		return REG_BADPAT;
	if (len < env->min)
		return REG_NOMATCH;
	env->regex = p;
	env->beg = (unsigned char*)s;
	env->end = env->beg + len;
	stknew(stkstd, &env->stk);
	env->flags &= ~REG_EXEC;
	env->flags |= (flags & REG_EXEC);
	if (env->stack = env->hard || !(env->flags & REG_NOSUB) && nmatch)
	{
		n = p->re_nsub;
		if (!(env->match = (regmatch_t*)stkpush(stkstd, 2 * (n + 1) * sizeof(regmatch_t))) ||
		    !env->pos && !(env->pos = vecopen(16, sizeof(Pos_t))) ||
		    !env->bestpos && !(env->bestpos = vecopen(16, sizeof(Pos_t))))
		{
			k = REG_ESPACE;
			goto done;
		}
		env->pos->cur = env->bestpos->cur = 0;
		env->best = &env->match[n + 1];
		env->best[0].rm_so = 0;
		env->best[0].rm_eo = -1;
		for (i = 0; i <= n; i++)
			env->match[i] = state.nomatch;
	}
	k = REG_NOMATCH;
	if ((e = env->rex)->type == REX_BM)
	{
		if (len < e->re.bm.right)
			goto done;
		else
		{
			register unsigned char*	buf = (unsigned char*)s;
			register size_t		index = e->re.bm.left + e->re.bm.size;
			register size_t		mid = len - e->re.bm.right;
			register size_t*	skip = e->re.bm.skip;
			register size_t*	fail = e->re.bm.fail;
			register Bm_mask_t**	mask = e->re.bm.mask;
			Bm_mask_t		m;

			for (;;)
			{
				while ((index += skip[buf[index]]) < mid);
				if (index < HIT)
					goto done;
				index -= HIT;
				m = mask[n = e->re.bm.size - 1][buf[index]];
				do
				{
					if (!n--)
						goto possible;
				} while (m &= mask[n][buf[--index]]);
				if ((index += fail[n + 1]) >= len)
					goto done;
			}
 possible:
			n = p->re_nsub;
			e = e->next;
		}
	}
	while (parse(env, e, &state.done, (unsigned char*)s) == NONE)
	{
		if (env->once)
			goto done;
		if ((unsigned char*)++s > env->end - env->min)
			goto done;
		if (env->stack)
			env->best[0].rm_so++;
	}
	if (env->error == REG_ESPACE)
	{
		k = REG_ESPACE;
		goto done;
	}
	if (!(env->flags & REG_NOSUB))
	{
		k = (env->flags & (REG_SHELL|REG_AUGMENTED)) == (REG_SHELL|REG_AUGMENTED);
		if (env->flags & REG_MINIMAL)
			n = (n - 1) / 2;
		for (i = j = 0; j < nmatch; i++)
			if (!i || !k || (i & 1))
			{
				if (i <= n)
					match[j++] = env->best[i];
				else
					match[j++] = state.nomatch;
			}
	}
	k = 0;
 done:
	stkold(stkstd, &env->stk);
	env->stk.base = 0;
	return k;
}

void
regfree(regex_t* p)
{
	Env_t*	env;

	if (p && (env = p->env))
	{
		p->env = 0;
		drop(env->rex);
		if (env->pos)
			vecclose(env->pos);
		if (env->bestpos)
			vecclose(env->bestpos);
		if (env->stk.base)
			stkold(stkstd, &env->stk);
		alloc(env, 0);
	}
}
