################################################################
#                                                              #
#           This software is part of the ast package           #
#              Copyright (c) 1999-2000 AT&T Corp.              #
#      and it may only be used by you under license from       #
#                     AT&T Corp. ("AT&T")                      #
#       A copy of the Source Code Agreement is available       #
#              at the AT&T Internet web site URL               #
#                                                              #
#     http://www.research.att.com/sw/license/ast-open.html     #
#                                                              #
#      If you have copied this software without agreeing       #
#      to the terms of the license you are infringing on       #
#         the license and copyright and are violating          #
#             AT&T's intellectual property rights.             #
#                                                              #
#               This software was created by the               #
#               Network Services Research Center               #
#                      AT&T Labs Research                      #
#                       Florham Park NJ                        #
#                                                              #
#             Glenn Fowler <gsf@research.att.com>              #
#                                                              #
################################################################
# manmake - mamake wrapper to bootstrap nmake
# @(#)build (AT&T Labs Research) 1999-11-19
# this script written to make it through all sh variants
# Glenn Fowler <gsf@research.att.com>

case $-:$BASH_VERSION in
*x*:[0-9]*)	: bash set -x is broken :; set +ex ;;
esac

command=nmake

# check the args

set '' "$@" ''
shift
while	:
do	a=$1
	shift
	case $a in
	'')	break
		;;
	'error 0 $(MAKEVERSION:'*)
		exit 1
		;;
	-K|cc-*|recurse)
		continue
		;;
	esac
	set '' "$@" "$a"
	shift
done

# hope mamake can handle it

exec mamake "$@"
