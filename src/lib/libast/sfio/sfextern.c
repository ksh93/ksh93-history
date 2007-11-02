/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#include	"sfhdr.h"

/*	External variables and functions used only by Sfio
**	Written by Kiem-Phong Vo
*/

/* code to initialize mutexes */
static Vtmutex_t	Sfmutex;
static Vtonce_t		Sfonce = VTONCE_INITDATA;
static void _sfoncef()
{	(void)vtmtxopen(_Sfmutex, VT_INIT);
	(void)vtmtxopen(&_Sfpool.mutex, VT_INIT);
	(void)vtmtxopen(sfstdin->mutex, VT_INIT);
	(void)vtmtxopen(sfstdout->mutex, VT_INIT);
	(void)vtmtxopen(sfstderr->mutex, VT_INIT);
	_Sfdone = 1;
}

/* global variables used internally to the package */
Sfextern_t _Sfextern =
{	0,						/* _Sfpage	*/
	{ NIL(Sfpool_t*), 0, 0, 0, NIL(Sfio_t**) },	/* _Sfpool	*/
	NIL(int(*)_ARG_((Sfio_t*,int))),		/* _Sfpmove	*/
	NIL(Sfio_t*(*)_ARG_((Sfio_t*, Sfio_t*))),	/* _Sfstack	*/
	NIL(void(*)_ARG_((Sfio_t*, int, int))),		/* _Sfnotify	*/
	NIL(int(*)_ARG_((Sfio_t*))),			/* _Sfstdsync	*/
	{ NIL(Sfread_f),				/* _Sfudisc	*/
	  NIL(Sfwrite_f),
	  NIL(Sfseek_f),
	  NIL(Sfexcept_f),
	  NIL(Sfdisc_t*)
	},
	NIL(void(*)_ARG_((void)) ),			/* _Sfcleanup	*/
	0,						/* _Sfexiting	*/
	0,						/* _Sfdone	*/
	&Sfonce,					/* _Sfonce	*/
	_sfoncef,					/* _Sfoncef	*/
	&Sfmutex					/* _Sfmutex	*/
};

ssize_t	_Sfi = -1;		/* value for a few fast macro functions	*/
#ifdef _ast_int8_t
ssize_t _Sfmaxr = 256*1024;	/* default maximum size for a record	*/
#else
ssize_t _Sfmaxr = 16*1024;	/* default maximum size for a record	*/
#endif

#if vt_threaded
static Vtmutex_t	_Sfmtxin, _Sfmtxout, _Sfmtxerr;
#define SFMTXIN		(&_Sfmtxin)
#define SFMTXOUT	(&_Sfmtxout)
#define SFMTXERR	(&_Sfmtxerr)
#else
#define SFMTXIN		(0)
#define SFMTXOUT	(0)
#define SFMTXERR	(0)
#endif

Sfio_t	_Sfstdin  = SFNEW(NIL(char*),-1,0,
			  (SF_READ |SF_STATIC|SF_MTSAFE),NIL(Sfdisc_t*),SFMTXIN);
Sfio_t	_Sfstdout = SFNEW(NIL(char*),-1,1,
			  (SF_WRITE|SF_STATIC|SF_MTSAFE),NIL(Sfdisc_t*),SFMTXOUT);
Sfio_t	_Sfstderr = SFNEW(NIL(char*),-1,2,
			  (SF_WRITE|SF_STATIC|SF_MTSAFE),NIL(Sfdisc_t*),SFMTXERR);

#undef	sfstdin
#undef	sfstdout
#undef	sfstderr

Sfio_t*	sfstdin  = &_Sfstdin;
Sfio_t*	sfstdout = &_Sfstdout;
Sfio_t*	sfstderr = &_Sfstderr;

__EXTERN__(ssize_t,_Sfi);
__EXTERN__(Sfio_t,_Sfstdin);
__EXTERN__(Sfio_t,_Sfstdout);
__EXTERN__(Sfio_t,_Sfstderr);
__EXTERN__(Sfio_t*,sfstdin);
__EXTERN__(Sfio_t*,sfstdout);
__EXTERN__(Sfio_t*,sfstderr);
