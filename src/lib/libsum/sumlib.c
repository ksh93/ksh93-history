/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1996-2001 AT&T Corp.                *
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
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * man this is sum library
 */

static const char id[] = "\n@(#)$Id: sumlib (AT&T Research) 1999-11-11 $\0\n";

#define _SUM_PRIVATE_	\
			struct Algorithm*	algorithm;	\
			unsigned long		total_count;	\
			unsigned long		total_size;	\
			unsigned long		size;

#include <sum.h>
#include <int.h>
#include <hashpart.h>

#define SCALE(n,m)	(((n)+(m)-1)/(m))

typedef struct Algorithm
{
	const char*	match;
	const char*	description;
	const char*	notice;
	Sum_t*		(*open)(const struct Algorithm*, const char*);
	int		(*init)(Sum_t*);
	int		(*block)(Sum_t*, const void*, size_t);
	int		(*print)(Sum_t*, Sfio_t*, int);
	int		(*done)(Sum_t*);
	int		scale;
} Algorithm_t;

/*
 * 16 and 32 bit common code
 */

typedef struct
{
	_SUM_PUBLIC_
	_SUM_PRIVATE_
	unsigned long	sum;
	unsigned long	total_sum;
} Integral_t;

static Sum_t*
long_open(const Algorithm_t* algorithm, const char* name)
{
	Integral_t*	p;

	if (p = newof(0, Integral_t, 1, 0))
	{
		p->algorithm = (Algorithm_t*)algorithm;
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
	register Integral_t*	x = (Integral_t*)p;
	register unsigned long	c;
	register int		n;

	c = (flags & SUM_TOTAL) ? x->total_sum : x->sum;
	sfprintf(sp, "%lu", c);
	if (flags & SUM_SIZE)
	{
		c = (flags & SUM_TOTAL) ? x->total_size : x->size;
		if ((flags & SUM_SCALE) && (n = x->algorithm->scale))
			c = SCALE(c, n);
		sfprintf(sp, " %lu", c);
	}
	if (flags & SUM_TOTAL)
		sfprintf(sp, " %lu", x->total_count);
	return 0;
}

/*
 * att
 */

#define att_description	\
	"The system 5 release 4 checksum. This is the default for \bsum\b \
	when \bgetconf UNIVERSE\b is \batt\b. This is the only true sum; \
	all of the other methods are order dependent."
#define att_notice	0
#define att_match	"att|sys5|s5|default"
#define att_open	long_open
#define att_init	long_init
#define att_print	long_print
#define att_scale	512

static int
att_block(register Sum_t* p, const void* s, size_t n)
{
	register unsigned long	c = ((Integral_t*)p)->sum;
	register unsigned char*	b = (unsigned char*)s;
	register unsigned char*	e = b + n;

	while (b < e)
		c += *b++;
	((Integral_t*)p)->sum = c;
	return 0;
}

static int
att_done(Sum_t* p)
{
	register unsigned long	c = ((Integral_t*)p)->sum;

	c = (c & 0xffff) + ((c >> 16) & 0xffff);
	c = (c & 0xffff) + (c >> 16);
	((Integral_t*)p)->sum = c & 0xffff;
	return short_done(p);
}

/*
 * ast
 */

#define ast_description \
	"The \bast\b \bstrsum\b(3) PRNG hash."
#define ast_notice	0
#define ast_match	"ast"
#define ast_open	long_open
#define ast_init	long_init
#define ast_done	long_done
#define ast_print	long_print
#define ast_scale	0

static int
ast_block(register Sum_t* p, const void* s, size_t n)
{
	register unsigned long	c = ((Integral_t*)p)->sum;
	register unsigned char*	b = (unsigned char*)s;
	register unsigned char*	e = b + n;

	while (b < e)
		HASHPART(c, *b++);
	((Integral_t*)p)->sum = c;
	return 0;
}

/*
 * ast4
 */

#define ast4_description \
	"The \bast\b 128 bit PRNG hash generated by catenating 4 separate 32 \
	bit PNRG hashes. The block count is not printed."
#define ast4_notice	0
#define ast4_match	"ast4|32x4|tw"
#define ast4_done	long_done
#define ast4_scale	0

typedef struct
{
	int_4		sum0;
	int_4		sum1;
	int_4		sum2;
	int_4		sum3;
} Ast4_sum_t;

typedef struct
{
	_SUM_PUBLIC_
	_SUM_PRIVATE_
	Ast4_sum_t	cur;
	Ast4_sum_t	tot;
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
ast4_open(const Algorithm_t* algorithm, const char* name)
{
	Ast4_t*	p;

	if (p = newof(0, Ast4_t, 1, 0))
	{
		p->algorithm = (Algorithm_t*)algorithm;
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

/*
 * bsd
 */

#define bsd_description \
	"The BSD checksum."
#define bsd_notice	0
#define bsd_match	"bsd|ucb"
#define bsd_open	long_open
#define bsd_init	long_init
#define bsd_done	short_done
#define bsd_print	long_print
#define bsd_scale	1024

static int
bsd_block(register Sum_t* p, const void* s, size_t n)
{
	register unsigned long	c = ((Integral_t*)p)->sum;
	register unsigned char*	b = (unsigned char*)s;
	register unsigned char*	e = b + n;

	while (b < e)
		c = ((c >> 1) + *b++ + ((c & 01) ? 0x8000 : 0)) & 0xffff;
	((Integral_t*)p)->sum = c;
	return 0;
}

/*
 * crc -- posix 1003.2-1992
 *
 * x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1
 */

static const unsigned long	crctab[] =
{
	0x00000000,
	0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6,
	0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
	0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac,
	0x5bd4b01b, 0x569796c2, 0x52568b75, 0x6a1936c8, 0x6ed82b7f,
	0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a,
	0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58,
	0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033,
	0xa4ad16ea, 0xa06c0b5d, 0xd4326d90, 0xd0f37027, 0xddb056fe,
	0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
	0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4,
	0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5,
	0x2ac12072, 0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
	0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca, 0x7897ab07,
	0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c,
	0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1,
	0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b,
	0xbb60adfc, 0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698,
	0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d,
	0x94ea7b2a, 0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
	0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2, 0xc6bcf05f,
	0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80,
	0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
	0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a,
	0x58c1663d, 0x558240e4, 0x51435d53, 0x251d3b9e, 0x21dc2629,
	0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c,
	0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e,
	0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65,
	0xeba91bbc, 0xef68060b, 0xd727bbb6, 0xd3e6a601, 0xdea580d8,
	0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
	0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2,
	0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74,
	0x857130c3, 0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
	0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c, 0x7b827d21,
	0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a,
	0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e, 0x18197087,
	0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d,
	0x2056cd3a, 0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce,
	0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb,
	0xdbee767c, 0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
	0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4, 0x89b8fd09,
	0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf,
	0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

#define crc_description \
	"The posix 1003.2-1992 32 bit crc checksum for the polynomial \
	{ 32 26 23 22 16 12 11 10 8 7 5 4 2 1 }. This is the default \
	\bcksum\b(1)  method."
#define crc_notice	0
#define crc_match	"posix|cksum|crc|std|standard"
#define crc_open	long_open
#define crc_init	long_init
#define crc_print	long_print
#define crc_scale	0

#define CRCPART(s,c)	(s = (s << 8) ^ crctab[((s >> 24) ^ (c)) & 0xff])

static int
crc_block(register Sum_t* p, const void* s, size_t n)
{
	register unsigned long	c = ((Integral_t*)p)->sum;
	register unsigned char*	b = (unsigned char*)s;
	register unsigned char*	e = b + n;

	while (b < e)
		CRCPART(c, *b++);
	((Integral_t*)p)->sum = c;
	return 0;
}

static int
crc_done(Sum_t* p)
{
	register unsigned long	c = ((Integral_t*)p)->sum;
	register unsigned long	n = ((Integral_t*)p)->size;

	while (n)
	{
		CRCPART(c, n);
		n >>= 8;
	}
	((Integral_t*)p)->sum = ~c;
	return long_done(p);
}

/*
 * zip -- same polynomial as crctab[] except high powers in low bits
 */

static const unsigned long	ziptab[] =
{
	0x00000000,
	0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e,
	0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d,
	0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0,
	0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63,
	0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa,
	0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75,
	0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a, 0xc8d75180,
	0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87,
	0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5,
	0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01, 0x6b6b51f4,
	0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b,
	0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea,
	0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541,
	0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc,
	0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f,
	0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f, 0x5edef90e,
	0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c,
	0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b,
	0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344, 0x8708a3d2,
	0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671,
	0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8,
	0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767,
	0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c, 0x36034af6,
	0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795,
	0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b,
	0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713, 0x95bf4a82,
	0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d,
	0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8,
	0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff,
	0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee,
	0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d,
	0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c,
	0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02,
	0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

#define zip_description \
	"The \bzip\b(1) crc; uses the \bposix\b crc polynomial with the high \
	powers in the lower checksum bits."
#define zip_notice	0
#define zip_match	"zip"
#define zip_open	long_open
#define zip_print	long_print
#define zip_scale	0

#define ZIPPART(s,c)	(s = (s >> 8) ^ ziptab[(s ^ (c)) & 0xff])

static int
zip_block(register Sum_t* p, const void* s, size_t n)
{
	register unsigned long	c = ((Integral_t*)p)->sum;
	register unsigned char*	b = (unsigned char*)s;
	register unsigned char*	e = b + n;

	while (b < e)
		ZIPPART(c, *b++);
	((Integral_t*)p)->sum = c;
	return 0;
}

static int
zip_init(Sum_t* p)
{
	((Integral_t*)p)->sum = 0xffffffff;
	return 0;
}

static int
zip_done(register Sum_t* p)
{
	((Integral_t*)p)->sum = ~(((Integral_t*)p)->sum);
	return long_done(p);
}

/*
 * fddi (crc 035556101440) -- same as zip but diff init/done
 */

#define fddi_description \
	"The FDDI crc; uses the \bposix\b crc polynomial with a different \
	initial value."
#define fddi_notice	0
#define fddi_match	"fddi"
#define fddi_open	long_open
#define fddi_init	long_init
#define fddi_block	zip_block
#define fddi_print	long_print
#define fddi_scale	0

static int
fddi_done(register Sum_t* p)
{
	register int		i;
	register int		j;
	register unsigned long	n;
	unsigned char		buf[sizeof(unsigned long)];

	static unsigned long	mix[2] = { 0xcc, 0x55 };

	/*
	 * encode the length but make n==0 not 0
	 */

	n = ((Integral_t*)p)->size;
	j = CHAR_BIT * sizeof(buf);
	for (i = 0; i < sizeof(buf); i++)
	{
		j -= CHAR_BIT;
		buf[i] = (n >> j) ^ mix[i & 01];
	}
	fddi_block(p, buf, sizeof(buf));
	return long_done(p);
}

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
   rights reserved.

   License to copy and use this software is granted provided that it
   is identified as the "RSA Data Security, Inc. MD5 Message-Digest
   Algorithm" in all material mentioning or referencing this software
   or this function.

   License is also granted to make and use derivative works provided
   that such works are identified as "derived from the RSA Data
   Security, Inc. MD5 Message-Digest Algorithm" in all material
   mentioning or referencing the derived work.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.

   These notices must be retained in any copies of any part of this
   documentation and/or software.
 */

#define md5_description \
	"The RSA Data Security, Inc. MD5 Message-Digest Algorithm, 1991-2, \
	used with permission. The block count is not printed."
#define md5_notice	"\n@(#)$Id: md5 (RSA Data Security, Inc. MD5 Message-Digest, 1991-2) 1996-02-29 $\0\n"
#define md5_match	"md5|MD5"
#define md5_scale	0

typedef unsigned int_4 UINT4;

typedef struct
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
md5_open(const Algorithm_t* algorithm, const char* name)
{
	Md5_t*	p;

	if (p = newof(0, Md5_t, 1, 0))
	{
		p->algorithm = (Algorithm_t*)algorithm;
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

	/* zeroize sensitive information */
	memset(x, 0, sizeof(x));
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
	else i = 0;

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

	/* zeroize sensitive information */
	memset(context->state, 0, (char*)context->digest - (char*)context->state);

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

/*
 * fnv
 */

#define fnv_description \
	"The Fowler-Noll-Vo 32 bit PRNG hash with non-zero initializer (FNV-1)."
#define fnv_notice	0
#define fnv_match	"fnv|fnv1"
#define fnv_open	long_open
#define fnv_done	long_done
#define fnv_print	long_print
#define fnv_scale	0

static int
fnv_init(Sum_t* p)
{
	((Integral_t*)p)->sum = 0x811c9dc5;
	return 0;
}

static int
fnv_block(register Sum_t* p, const void* s, size_t n)
{
	register unsigned long	c = ((Integral_t*)p)->sum;
	register unsigned char*	b = (unsigned char*)s;
	register unsigned char*	e = b + n;

	while (b < e)
	{
		c *= 0x01000193;
		c ^= *b++;
	}
	((Integral_t*)p)->sum = c;
	return 0;
}

/*
 * now the library interface
 */

#define ALGORITHM(x)	x##_match,x##_description,x##_notice,x##_open,x##_init,x##_block,x##_print,x##_done,x##_scale

static const Algorithm_t	algorithms[] =
{
	ALGORITHM(att),
	ALGORITHM(ast4),
	ALGORITHM(ast),
	ALGORITHM(bsd),
	ALGORITHM(crc),
	ALGORITHM(zip),
	ALGORITHM(md5),
	ALGORITHM(fddi),
	ALGORITHM(fnv),
};

/*
 * open sum algorithm name
 */

Sum_t*
sumopen(register const char* name)
{
	register int	n;
	char		pat[256];

	if (!name || !name[0] || name[0] == '-' && !name[1])
		name = "default";
	for (n = 0; n < elementsof(algorithms); n++)
	{
		sfsprintf(pat, sizeof(pat), "*@(%s)*", algorithms[n].match);
		if (strmatch(name, pat))
			return (*algorithms[n].open)(&algorithms[n], name);
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
	return (*p->algorithm->init)(p);
}

/*
 * compute the running sum on buf
 */

int
sumblock(Sum_t* p, const void* buf, size_t siz)
{
	p->size += siz;
	return (*p->algorithm->block)(p, buf, siz);
}

/*
 * done with this run of blocks
 */

int
sumdone(Sum_t* p)
{
	p->total_count++;
	p->total_size += p->size;
	return (*p->algorithm->done)(p);
}

/*
 * print the sum [size] on sp
 */

int
sumprint(Sum_t* p, Sfio_t* sp, int flags)
{
	return (*p->algorithm->print)(p, sp, flags);
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
 * print the checksum algorithm optget(3) usage on sp and return the length
 */

int
sumusage(Sfio_t* sp)
{
	register int	i;
	register int	n;

	for (i = n = 0; i < elementsof(algorithms); i++)
		n += sfprintf(sp, "[+%s?%s]", algorithms[i].match, algorithms[i].description);
	return n;
}
