####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1985-2003 AT&T Corp.                #
#        and it may only be used by you under license from         #
#                       AT&T Corp. ("AT&T")                        #
#         A copy of the Source Code Agreement is available         #
#                at the AT&T Internet web site URL                 #
#                                                                  #
#       http://www.research.att.com/sw/license/ast-open.html       #
#                                                                  #
#    If you have copied or used this software without agreeing     #
#        to the terms of the license you are infringing on         #
#           the license and copyright and are violating            #
#               AT&T's intellectual property rights.               #
#                                                                  #
#            Information and Software Systems Research             #
#                        AT&T Labs Research                        #
#                         Florham Park NJ                          #
#                                                                  #
#               Glenn Fowler <gsf@research.att.com>                #
#                David Korn <dgk@research.att.com>                 #
#                 Phong Vo <kpv@research.att.com>                  #
#                                                                  #
####################################################################
: generate sig features
case $# in
0)	;;
*)	eval $1
	shift
	;;
esac
echo "#include <signal.h>
int xxx;" > $tmp.c
$cc -c $tmp.c >/dev/null 2>$tmp.e
echo "#pragma prototyped
#define sig_info	_sig_info_

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide kill killpg
#else
#define kill	______kill
#define killpg	______killpg
#endif
#include <signal.h>
#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide kill killpg
#else
#undef	kill
#undef	killpg
#endif
#ifndef sigmask
#define sigmask(s)	(1<<((s)-1))
#endif"
echo "#include <signal.h>
#ifdef TYPE
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
typedef TYPE (*Sig_handler_t)(ARG);
#else
typedef TYPE (*Sig_handler_t)();
#endif
#endif
Sig_handler_t f()
{
	Sig_handler_t	handler;
	handler = signal(1, SIG_IGN);
	return(handler);
}" > $tmp.c
if	$cc -c $tmp.c >/dev/null
then	:
else	e=`wc -l $tmp.e`
	i1= j1=
	for i in void int
	do	for j in int,... ... int
		do	$cc -c -DTYPE=$i -DARG=$j $tmp.c >/dev/null 2>$tmp.e || continue
			case `wc -l $tmp.e` in
			$e)	i1= j1=; break 2 ;;
			esac
			case $i1 in
			"")	i1=$i j1=$j ;;
			esac
		done
	done
	case $i1 in
	?*)	i=$i1 j=$j1 ;;
	esac
	echo "typedef $i (*Sig_handler_t)($j);"
fi
echo '

#define Handler_t		Sig_handler_t

#define SIG_REG_PENDING		(-1)
#define SIG_REG_POP		0
#define SIG_REG_EXEC		00001
#define SIG_REG_PROC		00002
#define SIG_REG_TERM		00004
#define SIG_REG_ALL		00777
#define SIG_REG_SET		01000

typedef struct
{
	char**		name;
	char**		text;
	int		sigmax;
} Sig_info_t;

extern int		kill(pid_t, int);
extern int		killpg(pid_t, int);

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif
#if !_BLD_ast && defined(__IMPORT__)
#define extern		__IMPORT__
#endif

extern Sig_info_t	sig_info;

#undef	extern

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int		sigcritical(int);
extern int		sigunblock(int);

#undef	extern'
