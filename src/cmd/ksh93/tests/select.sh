########################################################################
#                                                                      #
#               This software is part of the ast package               #
#                  Copyright (c) 1982-2004 AT&T Corp.                  #
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
#                  David Korn <dgk@research.att.com>                   #
#                                                                      #
########################################################################
function err_exit
{
	print -u2 -n "\t"
	print -u2 -r $Command: "$@"
	let Errors+=1
}

Command=$0
integer Errors=0
trap "rm -f /tmp/Sh$$*" EXIT
PS3='ABC '

cat > /tmp/Sh$$.1 <<\!
1) foo
2) bar
3) bam
!

select i in foo bar bam
do	case $i in
	foo)	break;;
	*)	err_exit "select 1 not working"
		break;;
	esac
done 2> /dev/null <<!
1
!

unset i
select i in foo bar bam
do	case $i in
	foo)	err_exit "select foo not working" 2>&3
		break;;
	*)	if	[[ $REPLY != foo ]]
		then	err_exit "select REPLY not correct" 2>&3
		fi
		( set -u; : $i ) || err_exit "select: i not set to null" 2>&3
		break;;
	esac
done  3>&2 2> /tmp/Sh$$.2 <<!
foo
!
exit $((Errors))
