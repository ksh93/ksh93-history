####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1982-2000 AT&T Corp.                #
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
#                                                                  #
####################################################################
function err_exit
{
	print -u2 -n "\t"
	print -u2 -r $Command: "$@"
	let Errors+=1
}
#test for compound variables
Command=$0
integer Errors=0
Point=(
	float x=1. y=0.
)
eval p="$Point"
if	(( (p.x*p.x + p.y*p.y) > 1.01 ))
then	err_exit 'compound variable not working'
fi
nameref foo=p
if	[[ ${foo.x} != ${Point.x} ]]
then	err_exit 'reference to compound object not working'
fi
unset foo
rec=(
	name='Joe Blow'
	born=(
		month=jan
		integer day=16
		year=1980
	)
)
eval newrec="$rec"
if	[[ ${newrec.name} != "${rec.name}" ]]
then	err_exit 'copying a compound object not working'
fi
if	(( newrec.born.day != 16 ))
then	err_exit 'copying integer field of  compound object not working'
fi
p_t=(
        integer z=0
        typeset -A tokens
)
unset x
typeset -A x
x=( [foo]=bar )
if	[[ ${x[@]} != bar ]]
then	err_exit 'compound assignemnt of associative arrays not working'
fi
unset -n foo x
unset foo x
foo=( x=3)
nameref x=foo
if	[[ ${!x.@} != foo.x ]]
then	err_exit 'name references not expanded on prefix matching'
fi
exit $((Errors))
