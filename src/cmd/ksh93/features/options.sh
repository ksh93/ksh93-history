####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1982-2001 AT&T Corp.                #
#        and it may only be used by you under license from         #
#                       AT&T Corp. ("AT&T")                        #
#         A copy of the Source Code Agreement is available         #
#                at the AT&T Internet web site URL                 #
#                                                                  #
#       http://www.research.att.com/sw/license/ast-open.html       #
#                                                                  #
#        If you have copied this software without agreeing         #
#        to the terms of the license you are infringing on         #
#           the license and copyright and are violating            #
#               AT&T's intellectual property rights.               #
#                                                                  #
#                 This software was created by the                 #
#                 Network Services Research Center                 #
#                        AT&T Labs Research                        #
#                         Florham Park NJ                          #
#                                                                  #
#                David Korn <dgk@research.att.com>                 #
####################################################################
: check for shell magic #!
: include OPTIONS
eval $1
shift
OPTIONS=$1
cat > /tmp/file$$ <<!
#! /bin/echo
exit 1
!
chmod 755 /tmp/file$$
if	/tmp/file$$ > /dev/null
then	echo "#define SHELLMAGIC	1"
fi
rm -f /tmp/file$$
: see whether or not in the ucb universe
if	test -f /bin/universe && univ=`/bin/universe` > /dev/null 2>&1
then	if	test ucb = "$univ"
	then	echo "#define SHOPT_UCB	1"
	fi
fi
if	test -d /dev/fd
then	echo "#define SHOPT_DEVFD	1"
fi

: get the option settings from the options file
. $OPTIONS
for i in ACCT ACCTFILE BRACEPAT CRNL DYNAMIC ECHOPRINT ESH FS_3D \
	JOBS KIA MULTIBYTE NAMESPAXE NOCASE OLDTERMIO OO P_SUID RAWONLY \
	SEVENBIT SPAWN SUID_EXEC TIMEOUT VSH
do	: This could be done with eval, but eval broken in some shells
	j=0
	case $i in
	ACCT)		j=$ACCT;;
	ACCTFILE)	j=$ACCTFILE;;
	BRACEPAT)	j=$BRACEPAT;;
	CRNL)		j=$CRNL;;
	DYNAMIC)	j=$DYNAMIC;;
	ECHOPRINT)	j=$ECHOPRINT;;
	ESH)		j=$ESH;;
	FS_3D)		j=$FS_3D;;
	JOBS)		j=$JOBS;;
	KIA)		j=$KIA;;
	MULTIBYTE)	j=$MULTIBYTE
			: check for ebcidic
			case  `echo a | tr a '\012' | wc -l` in
			*1*)	j=0;;
			esac
			;;
	NAMESPACE)		j=$NAMESPACE;;
	NOCASE)		j=$NOCASE;;
	OLDTERMIO)	echo "#include <sys/termios.h>" > /tmp/dummy$$.c
			echo "#include <sys/termio.h>" >>/tmp/dummy$$.c
			if	$cc -E /tmp/dummy$$.c > /dev/null 2>&1
			then	j=$OLDTERMIO
			fi
			rm -f dummy$$.c;;
	OO)		j=$OO;;
	P_SUID)		j=$P_SUID;;
	RAWONLY)	j=$RAWONLY;;
	SEVENBIT)	j=$SEVENBIT;;
	SPAWN)		j=$SPAWN;;
	SUID_EXEC)	j=$SUID_EXEC;;
	TIMEOUT)	j=$TIMEOUT;;
	VSH)		j=$VSH;;
	esac
	case $j in
	0|"")	;;
	*)	echo "#define SHOPT_$i	$j" ;;
	esac
done
cat <<\!
#if (MB_LEN_MAX-1)<=0 || !defined(_lib_mbtowc)
#   undef SHOPT_MULTIBYTE
#endif
!
