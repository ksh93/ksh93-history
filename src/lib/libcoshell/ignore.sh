########################################################################
#                                                                      #
#               This software is part of the ast package               #
#                  Copyright (c) 1990-2004 AT&T Corp.                  #
#                      and is licensed under the                       #
#          Common Public License, Version 1.0 (the "License")          #
#                        by AT&T Corp. ("AT&T")                        #
#      Any use, downloading, reproduction or distribution of this      #
#      software constitutes acceptance of the License.  A copy of      #
#                     the License is available at                      #
#                                                                      #
#         http://www.research.att.com/sw/license/cpl-1.0.html          #
#         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#                 Glenn Fowler <gsf@research.att.com>                  #
#                                                                      #
########################################################################
:
#
# Glenn Fowler
# AT&T Bell Laboratories
#
# Bourne coshell support
#
# @(#)ignore (AT&T Bell Laboratories) 08/11/92
#
while	:
do	case $# in
	0)	exit 0 ;;
	esac
	case $1 in
	*=*)	case $RANDOM in
		$RANDOM)`echo $1 | sed "s/\\([^=]*\\)=\\(.*\\)/eval \\1='\\2'; export \\1/"` ;;
		*)	export "$1" ;;
		esac
		shift
		;;
	*)	break
		;;
	esac
done
"$@"
exit 0
