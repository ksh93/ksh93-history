################################################################
#                                                              #
#           This software is part of the ast package           #
#              Copyright (c) 1982-2000 AT&T Corp.              #
#      and it may only be used by you under license from       #
#                     AT&T Corp. ("AT&T")                      #
#       A copy of the Source Code Agreement is available       #
#              at the AT&T Internet web site URL               #
#                                                              #
#     http://www.research.att.com/sw/license/ast-open.html     #
#                                                              #
#     If you received this software without first entering     #
#       into a license with AT&T, you have an infringing       #
#           copy and cannot use it without violating           #
#             AT&T's intellectual property rights.             #
#                                                              #
#               This software was created by the               #
#               Network Services Research Center               #
#                      AT&T Labs Research                      #
#                       Florham Park NJ                        #
#                                                              #
#              David Korn <dgk@research.att.com>               #
#                                                              #
################################################################
function err_exit
{
	print -u2 -n "\t"
	print -u2 -r $Command: "$@"
	let Errors+=1
}

Command=$0
integer Errors=0
if	[[ $( ${SHELL-ksh} -s hello<<-\!
		print $1
		!
	    ) != hello ]]
then	err_exit "${SHELL-ksh} -s not working"
fi
x=$(
	set -e
	false && print bad
	print good
)
if	[[ $x != good ]]
then	err_exit 'sh -e not workuing'
fi
exit $((Errors))
