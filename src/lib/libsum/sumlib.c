/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1996-2004 AT&T Corp.                *
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
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * man this is sum library
 */

static const char id[] = "\n@(#)$Id: sumlib (AT&T Research) 2003-12-22 $\0\n";

#define _SUM_PRIVATE_	\
			struct Method_s*	method;	\
			unsigned _ast_intmax_t	total_count;	\
			unsigned _ast_intmax_t	total_size;	\
			unsigned _ast_intmax_t	size;

#include <sum.h>
#include <ctype.h>
#include <int.h>
#include <swap.h>
#include <hashpart.h>

#define SCALE(n,m)	(((n)+(m)-1)/(m))

typedef struct Method_s
{
	const char*	match;
	const char*	description;
	const char*	options;
	Sum_t*		(*open)(const struct Method_s*, const char*);
	int		(*init)(Sum_t*);
	int		(*block)(Sum_t*, const void*, size_t);
	int		(*data)(Sum_t*, Sumdata_t*);
	int		(*print)(Sum_t*, Sfio_t*, int);
	int		(*done)(Sum_t*);
	int		scale;
} Method_t;

typedef struct Map_s
{
	const char*	match;
	const char*	description;
	const char*	map;
} Map_t;

/*
 * 16 and 32 bit common code
 */

#define _INTEGRAL_PRIVATE_ \
	unsigned _ast_int4_t	sum; \
	unsigned _ast_int4_t	total_sum;
	
typedef struct Integral_s
{
	_SUM_PUBLIC_
	_SUM_PRIVATE_
	_INTEGRAL_PRIVATE_
} Integral_t;

static Sum_t*
long_open(const Method_t* method, const char* name)
{
	Integral_t*	p;

	if (p = newof(0, Integral_t, 1, 0))
	{
		p->method = (Method_t*)method;
		p->name = name;
	}
	return (Sum_t*)p;
}

static int
long_init(Sum_t* p)
{
	((Integral_t*)p)->sum = 0;
	return 0;
}

static int
long_done(Sum_t* p)
{
	register Integral_t*	x = (Integral_t*)p;

	x->total_sum ^= (x->sum &= 0xffffffff);
	return 0;
}

static int
short_done(Sum_t* p)
{
	register Integral_t*	x = (Integral_t*)p;

	x->total_sum ^= (x->sum &= 0xffff);
	return 0;
}

static int
long_print(Sum_t* p, Sfio_t* sp, register int flags)
{
	register Integral_t*		x = (Integral_t*)p;
	register unsigned _ast_int4_t	c;
	register unsigned _ast_intmax_t	z;
	register size_t			n;

	c = (flags & SUM_TOTAL) ? x->total_sum : x->sum;
	sfprintf(sp, "%I*u", sizeof(c), c);
	if (flags & SUM_SIZE)
	{
		z = (flags & SUM_TOTAL) ? x->total_size : x->size;
		if ((flags & SUM_SCALE) && (n = x->method->scale))
			z = SCALE(z, n);
		sfprintf(sp, " %I*u", sizeof(z), z);
	}
	if (flags & SUM_TOTAL)
		sfprintf(sp, " %I*u", sizeof(x->total_count), x->total_count);
	return 0;
}

static int
long_data(Sum_t* p, Sumdata_t* data)
{
	register Integral_t*	x = (Integral_t*)p;

	data->size = sizeof(data->num);
	data->num = x->sum;
	data->buf = 0;
	return 0;
}

/*
 * att
 */

#define att_description	\
	"The system 5 release 4 checksum. This is the default for \bsum\b \
	when \bgetconf UNIVERSE\b is \batt\b. This is the only true sum; \
	all of the other methods are order dependent."
#define att_options	0
#define att_match	"att|sys5|s5|default"
#define att_open	long_open
#define att_init	long_init
#define att_print	long_print
#define att_data	long_data
#define att_scale	512

static int
att_block(register Sum_t* p, const void* s, size_t n)
{
	register unsigned _ast_int4_t	c = ((Integral_t*)p)->sum;
	register unsigned char*		b = (unsigned char*)s;
	register unsigned char*		e = b + n;

	while (b < e)
		c += *b++;
	((Integral_t*)p)->sum = c;
	return 0;
}

static int
att_done(Sum_t* p)
{
	register unsigned _ast_int4_t	c = ((Integral_t*)p)->sum;

	c = (c & 0xffff) + ((c >> 16) & 0xffff);
	c = (c & 0xffff) + (c >> 16);
	((Integral_t*)p)->sum = c & 0xffff;
	return short_done(p);
}

/*
 * ast4
 */

#define ast4_description \
	"The \bast\b 128 bit PRNG hash generated by catenating 4 separate 32 \
	bit PNRG hashes. The block count is not printed."
#define ast4_options	0
#define ast4_match	"ast4|32x4|tw"
#define ast4_done	long_done
#define ast4_scale	0

typedef struct Ast4_sum_s
{
	int_4		sum0;
	int_4		sum1;
	int_4		sum2;
	int_4		sum3;
} Ast4_sum_t;

typedef struct Ast4_s
{
	_SUM_PUBLIC_
	_SUM_PRIVATE_
	Ast4_sum_t	cur;
	Ast4_sum_t	tot;
	unsigned char	buf[sizeof(Ast4_sum_t)];
} Ast4_t;

static int
ast4_init(Sum_t* p)
{
	register Ast4_t*	a = (Ast4_t*)p;

	a->tot.sum0 ^= a->cur.sum0;
	a->cur.sum0 = 0;
	a->tot.sum1 ^= a->cur.sum1;
	a->cur.sum1 = 0;
	a->tot.sum2 ^= a->cur.sum2;
	a->cur.sum2 = 0;
	a->tot.sum3 ^= a->cur.sum3;
	a->cur.sum3 = 0;
	return 0;
}

static Sum_t*
ast4_open(const Method_t* method, const char* name)
{
	Ast4_t*	p;

	if (p = newof(0, Ast4_t, 1, 0))
	{
		p->method = (Method_t*)method;
		p->name = name;
	}
	return (Sum_t*)p;
}

static int
ast4_block(Sum_t* p, const void* s, size_t n)
{
	register Ast4_sum_t*	a = &((Ast4_t*)p)->cur;
	register unsigned char*	b = (unsigned char*)s;
	register unsigned char*	e = b + n;
	register int		c;

	while (b < e)
	{
		c = *b++;
		a->sum0 = a->sum0 * 0x63c63cd9 + 0x9c39c33d + c;
		a->sum1 = a->sum1 * 0x00000011 + 0x00017cfb + c;
		a->sum2 = a->sum2 * 0x12345679 + 0x3ade68b1 + c;
		a->sum3 = a->sum3 * 0xf1eac01d + 0xcafe10af + c;
	}
	return 0;
}

static int
ast4_print(Sum_t* p, Sfio_t* sp, int flags)
{
	register Ast4_sum_t*	a;

	a = (flags & SUM_TOTAL) ? &((Ast4_t*)p)->tot : &((Ast4_t*)p)->cur;
	sfprintf(sp, "%06..64u%06..64u%06..64u%06..64u", a->sum0, a->sum1, a->sum2, a->sum3);
	return 0;
}

static int
ast4_data(Sum_t* p, Sumdata_t* data)
{
	data->size = sizeof(((Ast4_t*)p)->cur);
	data->num = 0;
#if _ast_intswap
	swapmem(_ast_intswap, data->buf = ((Ast4_t*)p)->buf, &((Ast4_t*)p)->cur, sizeof(((Ast4_t*)p)->cur));
#else
	data->buf = &((Ast4_t*)p)->cur;
#endif
	return 0;
}

/*
 * bsd
 */

#define bsd_description \
	"The BSD checksum."
#define bsd_options	0
#define bsd_match	"bsd|ucb"
#define bsd_open	long_open
#define bsd_init	long_init
#define bsd_done	short_done
#define bsd_print	long_print
#define bsd_data	long_data
#define bsd_scale	1024

static int
bsd_block(register Sum_t* p, const void* s, size_t n)
{
	register unsigned _ast_int4_t	c = ((Integral_t*)p)->sum;
	register unsigned char*		b = (unsigned char*)s;
	register unsigned char*		e = b + n;

	while (b < e)
		c = ((c >> 1) + *b++ + ((c & 01) ? 0x8000 : 0)) & 0xffff;
	((Integral_t*)p)->sum = c;
	return 0;
}

#define crc_description \
	"32 bit CRC (cyclic redundancy check)."
#define crc_options	"\
[+polynomial?The 32 bit crc polynomial bitmask with implicit bit 32.]:[mask:=0xedb88320]\
[+done?XOR the final crc value with \anumber\a. 0xffffffff is used if \anumber\a is omitted.]:?[number:=0]\
[+init?The initial crc value. 0xffffffff is used if \anumber\a is omitted.]:?[number:=0]\
[+rotate?XOR each input character with the high order crc byte (instead of the low order).]\
[+size?Include the total number of bytes in the crc. \anumber\a, if specified, is first XOR'd into the size.]:?[number:=0]\
"
#define crc_match	"crc"
#define crc_open	crc_open
#define crc_print	long_print
#define crc_data	long_data
#define crc_scale	0

typedef unsigned _ast_int4_t Crcnum_t;

typedef struct Crc_s
{
	_SUM_PUBLIC_
	_SUM_PRIVATE_
	_INTEGRAL_PRIVATE_
	Crcnum_t		init;
	Crcnum_t		done;
	Crcnum_t		xorsize;
	Crcnum_t		tab[256];
	unsigned int		addsize;
	unsigned int		rotate;
} Crc_t;

#define CRC(p,s,c)		(s = (s >> 8) ^ (p)->tab[(s ^ (c)) & 0xff])
#define CRCROTATE(p,s,c)	(s = (s << 8) ^ (p)->tab[((s >> 24) ^ (c)) & 0xff])

static Sum_t*
crc_open(const Method_t* method, const char* name)
{
	register Crc_t*		sum;
	register const char*	s;
	register const char*	t;
	register const char*	v;
	register int		i;
	register int		j;
	Crcnum_t		polynomial;
	Crcnum_t		x;

	if (sum = newof(0, Crc_t, 1, 0))
	{
		sum->method = (Method_t*)method;
		sum->name = name;
	}
	polynomial = 0xedb88320;
	s = name;
	while (*(t = s))
	{
		for (t = s, v = 0; *s && *s != '-'; s++)
			if (*s == '=' && !v)
				v = s;
		i = (v ? v : s) - t;
		if (isdigit(*t) || v && i >= 4 && strneq(t, "poly", 4) && (t = v + 1))
			polynomial = strtoul(t, NiL, 0);
		else if (strneq(t, "done", i))
			sum->done = v ? strtoul(v + 1, NiL, 0) : ~sum->done;
		else if (strneq(t, "init", i))
			sum->init = v ? strtoul(v + 1, NiL, 0) : ~sum->init;
		else if (strneq(t, "rotate", i))
			sum->rotate = 1;
		else if (strneq(t, "size", i))
		{
			sum->addsize = 1;
			if (v)
				sum->xorsize = strtoul(v + 1, NiL, 0);
		}
		if (*s == '-')
			s++;
	}
	if (sum->rotate)
	{
		Crcnum_t	t;
		Crcnum_t	p[8];

		p[0] = polynomial;
		for (i = 1; i < 8; i++)
			p[i] = (p[i-1] << 1) ^ ((p[i-1] & 0x80000000) ? polynomial : 0);
		for (i = 0; i < elementsof(sum->tab); i++)
		{
			t = 0;
			x = i;
			for (j = 0; j < 8; j++)
			{
				if (x & 1)
					t ^= p[j];
				x >>= 1;
			}
			sum->tab[i] = t;
		}
	}
	else
	{
		for (i = 0; i < elementsof(sum->tab); i++)
		{
			x = i;
			for (j = 0; j < 8; j++)
				x = (x>>1) ^ ((x & 1) ? polynomial : 0);
			sum->tab[i] = x;
		}
	}
	return (Sum_t*)sum;
}

static int
crc_init(Sum_t* p)
{
	Crc_t*		sum = (Crc_t*)p;

	sum->sum = sum->init;
	return 0;
}

static int
crc_block(Sum_t* p, const void* s, size_t n)
{
	Crc_t*			sum = (Crc_t*)p;
	register Crcnum_t	c = sum->sum;
	register unsigned char*	b = (unsigned char*)s;
	register unsigned char*	e = b + n;

	if (sum->rotate)
		while (b < e)
			CRCROTATE(sum, c, *b++);
	else
		while (b < e)
			CRC(sum, c, *b++);
	sum->sum = c;
	return 0;
}

static int
crc_done(Sum_t* p)
{
	register Crc_t*			sum = (Crc_t*)p;
	register Crcnum_t		c;
	register unsigned _ast_intmax_t	n;
	int				i;
	int				j;

	c = sum->sum;
	if (sum->addsize)
	{
		n = sum->size ^ sum->xorsize;
		if (sum->rotate)
			while (n)
			{
				CRCROTATE(sum, c, n);
				n >>= 8;
			}
		else
			for (i = 0, j = 32; i < 4; i++)
			{
				j -= 8;
				CRC(sum, c, n >> j);
			}
	}
	sum->sum = c ^ sum->done;
	sum->total_sum ^= (sum->sum &= 0xffffffff);
	return 0;
}

#include <fnv.h>

#define prng_description \
	"32 bit PRNG (pseudo random number generator) hash."
#define prng_options	"\
[+mpy?The 32 bit PRNG multiplier.]:[number:=0x01000193]\
[+add?The 32 bit PRNG addend.]:[number:=0]\
[+init?The PRNG initial value. 0xffffffff is used if \anumber\a is omitted.]:?[number:=0x811c9dc5]\
"
#define prng_match	"prng"
#define prng_done	long_done
#define prng_print	long_print
#define prng_data	long_data
#define prng_scale	0

typedef unsigned _ast_int4_t Prngnum_t;

typedef struct Prng_s
{
	_SUM_PUBLIC_
	_SUM_PRIVATE_
	_INTEGRAL_PRIVATE_
	Prngnum_t		init;
	Prngnum_t		mpy;
	Prngnum_t		add;
} Prng_t;

static Sum_t*
prng_open(const Method_t* method, const char* name)
{
	register Prng_t*	sum;
	register const char*	s;
	register const char*	t;
	register const char*	v;
	register int		i;

	if (sum = newof(0, Prng_t, 1, 0))
	{
		sum->method = (Method_t*)method;
		sum->name = name;
	}
	s = name;
	while (*(t = s))
	{
		for (t = s, v = 0; *s && *s != '-'; s++)
			if (*s == '=' && !v)
				v = s;
		i = (v ? v : s) - t;
		if (isdigit(*t) || v && strneq(t, "mpy", i) && (t = v + 1))
			sum->mpy = strtoul(t, NiL, 0);
		else if (strneq(t, "add", i))
			sum->add = v ? strtoul(v + 1, NiL, 0) : ~sum->add;
		else if (strneq(t, "init", i))
			sum->init = v ? strtoul(v + 1, NiL, 0) : ~sum->init;
		if (*s == '-')
			s++;
	}
	if (!sum->mpy)
	{
		sum->mpy = FNV_MULT;
		if (!sum->init)
			sum->init = FNV_INIT;
	}
	return (Sum_t*)sum;
}

static int
prng_init(Sum_t* p)
{
	Prng_t*		sum = (Prng_t*)p;

	sum->sum = sum->init;
	return 0;
}

static int
prng_block(Sum_t* p, const void* s, size_t n)
{
	Prng_t*			sum = (Prng_t*)p;
	register Prngnum_t	c = sum->sum;
	register unsigned char*	b = (unsigned char*)s;
	register unsigned char*	e = b + n;

	while (b < e)
		c = c * sum->mpy + sum->add + *b++;
	sum->sum = c;
	return 0;
}

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
   rights reserved.

   License to copy and use this software is granted provided that it
   is identified as the "RSA Data Security, Inc. MD5 Message-Digest
   Method" in all material mentioning or referencing this software
   or this function.

   License is also granted to make and use derivative works provided
   that such works are identified as "derived from the RSA Data
   Security, Inc. MD5 Message-Digest Method" in all material
   mentioning or referencing the derived work.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.

   These notices must be retained in any copies of any part of this
   documentation and/or software.
 */

#define md5_description \
	"The RSA Data Security, Inc. MD5 Message-Digest Method, 1991-2, \
	used with permission. The block count is not printed."
#define md5_options	"[+(version)?md5 (RSA Data Security, Inc. MD5 Message-Digest, 1991-2) 1996-02-29]"
#define md5_match	"md5|MD5"
#define md5_scale	0

typedef unsigned int_4 UINT4;

typedef struct Md5_s
{
	_SUM_PUBLIC_
	_SUM_PRIVATE_
	UINT4		state[4];	/* state (ABCD)			*/
	UINT4		count[2];	/* # bits handled mod 2^64 (lsb)*/
	unsigned char	buffer[64];	/* input buffer			*/
	unsigned char	digest[16];	/* final digest			*/
	unsigned char	digest_sum[16]; /* sum of all digests		*/
} Md5_t;

static unsigned char	md5_pad[] =
{
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*
 * encode input into output
 * len must be a multiple of 4
 */

static void
md5_encode(register unsigned char* output, register UINT4* input, unsigned int len)
{
	register unsigned int	i;
	register unsigned int	j;

	for (i = j = 0; j < len; i++, j += 4)
	{
		output[j] = (unsigned char)(input[i] & 0xff);
		output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
		output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
		output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
	}
}

/*
 * decode input into output
 * len must be a multiple of 4
 */

static void
md5_decode(register UINT4* output, register unsigned char* input, unsigned int len)
{
	unsigned int	i;
	unsigned int	j;

	for (i = j = 0; j < len; i++, j += 4)
		output[i] = ((UINT4)input[j]) |
			    (((UINT4)input[j+1]) << 8) |
			    (((UINT4)input[j+2]) << 16) |
			    (((UINT4)input[j+3]) << 24);
}

static int
md5_init(Sum_t* p)
{
	register Md5_t*	context = (Md5_t*)p;

	context->count[0] = context->count[1] = 0;
	context->state[0] = 0x67452301;
	context->state[1] = 0xefcdab89;
	context->state[2] = 0x98badcfe;
	context->state[3] = 0x10325476;
	return 0;
}

static Sum_t*
md5_open(const Method_t* method, const char* name)
{
	Md5_t*	p;

	if (p = newof(0, Md5_t, 1, 0))
	{
		p->method = (Method_t*)method;
		p->name = name;
		md5_init((Sum_t*)p);
	}
	return (Sum_t*)p;
}

/*
 * basic MD5 step -- transforms buf based on in
 */

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

/* F, G, H and I are basic MD5 functions */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) { \
    (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
    (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
    (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
    (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
  }

static void
md5_transform(UINT4 state[4], unsigned char block[64])
{
	UINT4	a = state[0];
	UINT4	b = state[1];
	UINT4	c = state[2];
	UINT4	d = state[3];
	UINT4	x[16];

	md5_decode(x, block, 64);

	/* round 1 */
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	/* round 2 */
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG (d, a, b, c, x[10], S22, 0x02441453); /* 22 */
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	/* round 3 */
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH (b, c, d, a, x[ 6], S34, 0x04881d05); /* 44 */
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

	/* round 4 */
	II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}

static int
md5_block(Sum_t* p, const void* s, size_t inputLen)
{
	register Md5_t*	context = (Md5_t*)p;
	unsigned char*	input = (unsigned char*)s;
	unsigned int	i;
	unsigned int	index;
	unsigned int	partLen;

	/* compute number of bytes mod 64 */
	index = (unsigned int)((context->count[0] >> 3) & 0x3f);

	/* update number of bits */
	if ((context->count[0] += ((UINT4)inputLen << 3)) < ((UINT4)inputLen << 3))
		context->count[1]++;
	context->count[1] += ((UINT4)inputLen >> 29);
	partLen = 64 - index;

	/* transform as many times as possible */
	if (inputLen >= partLen)
	{
		memcpy(&context->buffer[index], input, partLen);
		md5_transform(context->state, context->buffer);
		for (i = partLen; i + 63 < inputLen; i += 64)
			md5_transform(context->state, &input[i]);
		index = 0;
	}
	else
		i = 0;

	/* buffer remaining input */
	memcpy(&context->buffer[index], &input[i], inputLen - i);

	return 0;
}

static int
md5_done(Sum_t* p)
{
	register Md5_t*	context = (Md5_t*)p;
	unsigned char	bits[8];
	unsigned int	index;
	unsigned int	padLen;

	/* save number of bits */
	md5_encode(bits, context->count, sizeof(bits));

	/* pad out to 56 mod 64 */
	index = (unsigned int)((context->count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	md5_block(p, md5_pad, padLen);

	/* append length (before padding) */
	md5_block(p, bits, sizeof(bits));

	/* store state in digest */
	md5_encode(context->digest, context->state, sizeof(context->digest));

	/* accumulate the digests */
	for (index = 0; index < elementsof(context->digest); index++)
		context->digest_sum[index] ^= context->digest[index];

	return 0;
}

static int
md5_print(Sum_t* p, Sfio_t* sp, register int flags)
{
	register Md5_t*		x = (Md5_t*)p;
	register unsigned char*	d;
	register int		n;

	d = (flags & SUM_TOTAL) ? x->digest_sum : x->digest;
	for (n = 0; n < elementsof(x->digest); n++)
		sfprintf(sp, "%02x", d[n]);
	return 0;
}

static int
md5_data(Sum_t* p, Sumdata_t* data)
{
	register Md5_t*		x = (Md5_t*)p;

	data->size = elementsof(x->digest);
	data->num = 0;
	data->buf = x->digest;
	return 0;
}

/*
 * FIPS 180-1 compliant SHA-1 implementation,
 * by Christophe Devine <devine@cr0.net>;
 * this program is licensed under the GPL.
 */

#define sha1_description "FIPS 180-1 SHA-1 secure hash algorithm 1."
#define sha1_options	"[+(version)?sha1 (FIPS 180-1) 1993-05-11]\
			 [+(author)?Christophe Devine <devine@cr0.net>]"
#define sha1_match	"sha1|SHA1|sha|sha-1"
#define sha1_scale	0

#define uint8		unsigned _ast_int1_t
#define uint32		unsigned _ast_int4_t

#define sha1_padding	md5_pad

typedef struct Sha1_s
{
	_SUM_PUBLIC_
	_SUM_PRIVATE_
	uint32	total[2];
	uint32	state[5];
	uint8	buffer[64];
	uint8	digest[20];
	uint8	digest_sum[20];
} Sha1_t;

#define GET_UINT32(n,b,i)                                       \
{                                                               \
    (n) = (uint32) ((uint8 *) b)[(i)+3]                         \
      | (((uint32) ((uint8 *) b)[(i)+2]) <<  8)                 \
      | (((uint32) ((uint8 *) b)[(i)+1]) << 16)                 \
      | (((uint32) ((uint8 *) b)[(i)]  ) << 24);                \
}

#define PUT_UINT32(n,b,i)                                       \
{                                                               \
    (((uint8 *) b)[(i)+3]) = (uint8) (((n)      ) & 0xFF);      \
    (((uint8 *) b)[(i)+2]) = (uint8) (((n) >>  8) & 0xFF);      \
    (((uint8 *) b)[(i)+1]) = (uint8) (((n) >> 16) & 0xFF);      \
    (((uint8 *) b)[(i)]  ) = (uint8) (((n) >> 24) & 0xFF);      \
}

static void
sha1_process(Sha1_t* sha, uint8 data[64] )
{
    uint32 temp, A, B, C, D, E, W[16];

    GET_UINT32( W[0],  data,  0 );
    GET_UINT32( W[1],  data,  4 );
    GET_UINT32( W[2],  data,  8 );
    GET_UINT32( W[3],  data, 12 );
    GET_UINT32( W[4],  data, 16 );
    GET_UINT32( W[5],  data, 20 );
    GET_UINT32( W[6],  data, 24 );
    GET_UINT32( W[7],  data, 28 );
    GET_UINT32( W[8],  data, 32 );
    GET_UINT32( W[9],  data, 36 );
    GET_UINT32( W[10], data, 40 );
    GET_UINT32( W[11], data, 44 );
    GET_UINT32( W[12], data, 48 );
    GET_UINT32( W[13], data, 52 );
    GET_UINT32( W[14], data, 56 );
    GET_UINT32( W[15], data, 60 );

#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define R(t)                                            \
(                                                       \
    temp = W[(t -  3) & 0x0F] ^ W[(t - 8) & 0x0F] ^     \
           W[(t - 14) & 0x0F] ^ W[ t      & 0x0F],      \
    ( W[t & 0x0F] = S(temp,1) )                         \
)

#define P(a,b,c,d,e,x)                                  \
{                                                       \
    e += S(a,5) + F(b,c,d) + K + x; b = S(b,30);        \
}

    A = sha->state[0];
    B = sha->state[1];
    C = sha->state[2];
    D = sha->state[3];
    E = sha->state[4];

#undef	F

#define F(x,y,z) (z ^ (x & (y ^ z)))
#define K 0x5A827999

    P( A, B, C, D, E, W[0]  );
    P( E, A, B, C, D, W[1]  );
    P( D, E, A, B, C, W[2]  );
    P( C, D, E, A, B, W[3]  );
    P( B, C, D, E, A, W[4]  );
    P( A, B, C, D, E, W[5]  );
    P( E, A, B, C, D, W[6]  );
    P( D, E, A, B, C, W[7]  );
    P( C, D, E, A, B, W[8]  );
    P( B, C, D, E, A, W[9]  );
    P( A, B, C, D, E, W[10] );
    P( E, A, B, C, D, W[11] );
    P( D, E, A, B, C, W[12] );
    P( C, D, E, A, B, W[13] );
    P( B, C, D, E, A, W[14] );
    P( A, B, C, D, E, W[15] );
    P( E, A, B, C, D, R(16) );
    P( D, E, A, B, C, R(17) );
    P( C, D, E, A, B, R(18) );
    P( B, C, D, E, A, R(19) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0x6ED9EBA1

    P( A, B, C, D, E, R(20) );
    P( E, A, B, C, D, R(21) );
    P( D, E, A, B, C, R(22) );
    P( C, D, E, A, B, R(23) );
    P( B, C, D, E, A, R(24) );
    P( A, B, C, D, E, R(25) );
    P( E, A, B, C, D, R(26) );
    P( D, E, A, B, C, R(27) );
    P( C, D, E, A, B, R(28) );
    P( B, C, D, E, A, R(29) );
    P( A, B, C, D, E, R(30) );
    P( E, A, B, C, D, R(31) );
    P( D, E, A, B, C, R(32) );
    P( C, D, E, A, B, R(33) );
    P( B, C, D, E, A, R(34) );
    P( A, B, C, D, E, R(35) );
    P( E, A, B, C, D, R(36) );
    P( D, E, A, B, C, R(37) );
    P( C, D, E, A, B, R(38) );
    P( B, C, D, E, A, R(39) );

#undef K
#undef F

#define F(x,y,z) ((x & y) | (z & (x | y)))
#define K 0x8F1BBCDC

    P( A, B, C, D, E, R(40) );
    P( E, A, B, C, D, R(41) );
    P( D, E, A, B, C, R(42) );
    P( C, D, E, A, B, R(43) );
    P( B, C, D, E, A, R(44) );
    P( A, B, C, D, E, R(45) );
    P( E, A, B, C, D, R(46) );
    P( D, E, A, B, C, R(47) );
    P( C, D, E, A, B, R(48) );
    P( B, C, D, E, A, R(49) );
    P( A, B, C, D, E, R(50) );
    P( E, A, B, C, D, R(51) );
    P( D, E, A, B, C, R(52) );
    P( C, D, E, A, B, R(53) );
    P( B, C, D, E, A, R(54) );
    P( A, B, C, D, E, R(55) );
    P( E, A, B, C, D, R(56) );
    P( D, E, A, B, C, R(57) );
    P( C, D, E, A, B, R(58) );
    P( B, C, D, E, A, R(59) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0xCA62C1D6

    P( A, B, C, D, E, R(60) );
    P( E, A, B, C, D, R(61) );
    P( D, E, A, B, C, R(62) );
    P( C, D, E, A, B, R(63) );
    P( B, C, D, E, A, R(64) );
    P( A, B, C, D, E, R(65) );
    P( E, A, B, C, D, R(66) );
    P( D, E, A, B, C, R(67) );
    P( C, D, E, A, B, R(68) );
    P( B, C, D, E, A, R(69) );
    P( A, B, C, D, E, R(70) );
    P( E, A, B, C, D, R(71) );
    P( D, E, A, B, C, R(72) );
    P( C, D, E, A, B, R(73) );
    P( B, C, D, E, A, R(74) );
    P( A, B, C, D, E, R(75) );
    P( E, A, B, C, D, R(76) );
    P( D, E, A, B, C, R(77) );
    P( C, D, E, A, B, R(78) );
    P( B, C, D, E, A, R(79) );

#undef K
#undef F

    sha->state[0] += A;
    sha->state[1] += B;
    sha->state[2] += C;
    sha->state[3] += D;
    sha->state[4] += E;
}

static int
sha1_block(register Sum_t* p, const void* s, size_t length)
{
    Sha1_t*	sha = (Sha1_t*)p;
    uint8*	input = (uint8*)s;
    uint32	left, fill;

    if( ! length ) return 0;

    left = ( sha->total[0] >> 3 ) & 0x3F;
    fill = 64 - left;

    sha->total[0] += length <<  3;
    sha->total[1] += length >> 29;

    sha->total[0] &= 0xFFFFFFFF;
    sha->total[1] += sha->total[0] < length << 3;

    if( left && length >= fill )
    {
        memcpy( (void *) (sha->buffer + left), (void *) input, fill );
        sha1_process( sha, sha->buffer );
        length -= fill;
        input  += fill;
        left = 0;
    }

    while( length >= 64 )
    {
        sha1_process( sha, input );
        length -= 64;
        input  += 64;
    }

    if( length )
        memcpy( (void *) (sha->buffer + left), (void *) input, length );

    return 0;
}

static int
sha1_init(Sum_t* p)
{
	register Sha1_t*	sha = (Sha1_t*)p;

	sha->total[0] = sha->total[1] = 0;
	sha->state[0] = 0x67452301;
	sha->state[1] = 0xEFCDAB89;
	sha->state[2] = 0x98BADCFE;
	sha->state[3] = 0x10325476;
	sha->state[4] = 0xC3D2E1F0;

	return 0;
}

static Sum_t*
sha1_open(const Method_t* method, const char* name)
{
	Sha1_t*	sha;

	if (sha = newof(0, Sha1_t, 1, 0))
	{
		sha->method = (Method_t*)method;
		sha->name = name;
		sha1_init((Sum_t*)sha);
	}
	return (Sum_t*)sha;
}

static int
sha1_done(Sum_t* p)
{
    Sha1_t*	sha = (Sha1_t*)p;
    uint32	last, padn;
    uint8	msglen[8];

    PUT_UINT32( sha->total[1], msglen, 0 );
    PUT_UINT32( sha->total[0], msglen, 4 );

    last = ( sha->total[0] >> 3 ) & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    sha1_block( p, sha1_padding, padn );
    sha1_block( p, msglen, 8 );

    PUT_UINT32( sha->state[0], sha->digest,  0 );
    PUT_UINT32( sha->state[1], sha->digest,  4 );
    PUT_UINT32( sha->state[2], sha->digest,  8 );
    PUT_UINT32( sha->state[3], sha->digest, 12 );
    PUT_UINT32( sha->state[4], sha->digest, 16 );

    /* accumulate the digests */
    for (last = 0; last < elementsof(sha->digest); last++)
	sha->digest_sum[last] ^= sha->digest[last];
    return 0;
}

static int
sha1_print(Sum_t* p, Sfio_t* sp, register int flags)
{
	register Sha1_t*	sha = (Sha1_t*)p;
	register unsigned char*	d;
	register int		n;

	d = (flags & SUM_TOTAL) ? sha->digest_sum : sha->digest;
	for (n = 0; n < elementsof(sha->digest); n++)
		sfprintf(sp, "%02x", d[n]);
	return 0;
}

static int
sha1_data(Sum_t* p, Sumdata_t* data)
{
	register Sha1_t*	sha = (Sha1_t*)p;

	data->size = elementsof(sha->digest);
	data->num = 0;
	data->buf = sha->digest;
	return 0;
}

/*
 * now the library interface
 */

#define METHOD(x)	x##_match,x##_description,x##_options,x##_open,x##_init,x##_block,x##_data,x##_print,x##_done,x##_scale

static const Method_t	methods[] =
{
	METHOD(att),
	METHOD(ast4),
	METHOD(bsd),
	METHOD(crc),
	METHOD(md5),
	METHOD(prng),
	METHOD(sha1),
};

static const Map_t	maps[] =
{
	{
		"posix|cksum|std|standard",
		"The posix 1003.2-1992 32 bit crc checksum. This is the"
		" default \bcksum\b(1)  method.",
		"crc-0x04c11db7-rotate-done-size"
	},
	{
		"zip",
		"The \bzip\b(1) crc.",
		"crc-0xedb88320-init-done"
	},
	{
		"fddi",
		"The FDDI crc.",
		"crc-0xedb88320-size=0xcc55cc55"
	},
	{
		"fnv|fnv1",
		"The Fowler-Noll-Vo 32 bit PRNG hash with non-zero"
		" initializer (FNV-1).",
		"prng-0x01000193-init=0x811c9dc5"
	},
	{
		"ast|strsum",
		"The \bast\b \bstrsum\b(3) PRNG hash.",
		"prng-0x63c63cd9-add=0x9c39c33d"
	},
};

/*
 * open sum method name
 */

Sum_t*
sumopen(register const char* name)
{
	register int	n;
	char		pat[256];

	if (!name || !name[0] || name[0] == '-' && !name[1])
		name = "default";
	for (n = 0; n < elementsof(maps); n++)
	{
		sfsprintf(pat, sizeof(pat), "*@(%s)*", maps[n].match);
		if (strmatch(name, pat))
		{
			name = maps[n].map;
			break;
		}
	}
	for (n = 0; n < elementsof(methods); n++)
	{
		sfsprintf(pat, sizeof(pat), "*@(%s)*", methods[n].match);
		if (strmatch(name, pat))
			return (*methods[n].open)(&methods[n], name);
	}
	return 0;
}

/*
 * initialize for a new run of blocks
 */

int
suminit(Sum_t* p)
{
	p->size = 0;
	return (*p->method->init)(p);
}

/*
 * compute the running sum on buf
 */

int
sumblock(Sum_t* p, const void* buf, size_t siz)
{
	p->size += siz;
	return (*p->method->block)(p, buf, siz);
}

/*
 * done with this run of blocks
 */

int
sumdone(Sum_t* p)
{
	p->total_count++;
	p->total_size += p->size;
	return (*p->method->done)(p);
}

/*
 * print the sum [size] on sp
 */

int
sumprint(Sum_t* p, Sfio_t* sp, int flags)
{
	return (*p->method->print)(p, sp, flags);
}

/*
 * return the current sum (internal) data
 */

int
sumdata(Sum_t* p, Sumdata_t* d)
{
	return (*p->method->data)(p, d);
}

/*
 * close an open sum handle
 */

int
sumclose(Sum_t* p)
{
	free(p);
	return 0;
}

/*
 * print the checksum method optget(3) usage on sp and return the length
 */

int
sumusage(Sfio_t* sp)
{
	register int	i;
	register int	n;

	for (i = n = 0; i < elementsof(methods); i++)
	{
		n += sfprintf(sp, "[+%s?%s]", methods[i].match, methods[i].description);
		if (methods[i].options)
			n += sfprintf(sp, "{\n%s\n}", methods[i].options);
	}
	for (i = 0; i < elementsof(maps); i++)
		n += sfprintf(sp, "[+%s?%s Shorthand for \b%s\b.]", maps[i].match, maps[i].description, maps[i].map);
	return n;
}
