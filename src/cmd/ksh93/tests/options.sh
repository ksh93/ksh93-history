########################################################################
#                                                                      #
#               This software is part of the ast package               #
#                  Copyright (c) 1982-2004 AT&T Corp.                  #
#                      and is licensed under the                       #
#                  Common Public License, Version 1.0                  #
#                            by AT&T Corp.                             #
#                                                                      #
#                A copy of the License is available at                 #
#            http://www.opensource.org/licenses/cpl1.0.txt             #
#         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         #
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
	print -u2 -r $Command[$1]: "${@:2}"
	let Errors+=1
}
alias err_exit='err_exit $LINENO'

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
[[ $($SHELL -D -c 'print hi; print $"hello"') == '"hello"' ]] || err_exit 'ksh -D not working'

tmp=/tmp/ksh$$
mkdir $tmp
rc=$tmp/.kshrc
print $'function env_hit\n{\n\tprint OK\n}' > $rc

export ENV=$rc
[[ $(print env_hit | $SHELL 2>&1) == "OK" ]] &&
	err_exit 'non-interactive shell reads $ENV file'
[[ $(print env_hit | $SHELL -E 2>&1) == "OK" ]] ||
	err_exit '-E ignores $ENV file'
[[ $(print env_hit | $SHELL +E 2>&1) == "OK" ]] &&
	err_exit '+E reads $ENV file'
[[ $(print env_hit | $SHELL --rc 2>&1) == "OK" ]] ||
	err_exit '--rc ignores $ENV file'
[[ $(print env_hit | $SHELL --norc 2>&1) == "OK" ]] &&
	err_exit '--norc reads $ENV file'

export ENV=
[[ $(print env_hit | HOME=$tmp $SHELL 2>&1) == "OK" ]] &&
	err_exit 'non-interactive shell reads $HOME/.kshrc file'
[[ $(print env_hit | HOME=$tmp $SHELL -E 2>&1) == "OK" ]] &&
	err_exit '-E ignores empty $ENV'
[[ $(print env_hit | HOME=$tmp $SHELL +E 2>&1) == "OK" ]] &&
	err_exit '+E reads $HOME/.kshrc file'
[[ $(print env_hit | HOME=$tmp $SHELL --rc 2>&1) == "OK" ]] &&
	err_exit '--rc ignores empty $ENV'
[[ $(print env_hit | HOME=$tmp $SHELL --norc 2>&1) == "OK" ]] &&
	err_exit '--norc reads $HOME/.kshrc file'

unset ENV
[[ $(print env_hit | HOME=$tmp $SHELL 2>&1) == "OK" ]] &&
	err_exit 'non-interactive shell reads $HOME/.kshrc file'
[[ $(print env_hit | HOME=$tmp $SHELL -E 2>&1) == "OK" ]] ||
	err_exit '-E ignores $HOME/.kshrc file'
[[ $(print env_hit | HOME=$tmp $SHELL +E 2>&1) == "OK" ]] &&
	err_exit '+E reads $HOME/.kshrc file'
[[ $(print env_hit | HOME=$tmp $SHELL --rc 2>&1) == "OK" ]] ||
	err_exit '--rc ignores $HOME/.kshrc file'
[[ $(print env_hit | HOME=$tmp $SHELL --norc 2>&1) == "OK" ]] &&
	err_exit '--norc reads $HOME/.kshrc file'

rm -rf $tmp

if	command set -G 2> /dev/null
then	mkdir /tmp/ksh$$
	cd /tmp/ksh$$
	mkdir bar foo
	> bar.c  > bam.c
	> bar/foo.c > bar/bam.c
	> foo/bam.c
	set -- **.c
	[[ $* == 'bam.c bar.c' ]] || err_exit '**.c not working with -G option'
	set -- **
	[[ $* == 'bam.c bar bar.c bar/bam.c bar/foo.c foo foo/bam.c' ]] || err_exit '** not working with -G option'
	set -- **/*.c
	[[ $* == 'bam.c bar.c bar/bam.c bar/foo.c foo/bam.c' ]] || err_exit '**/*.c not working with -G option'
	set -- **/bam.c
	[[ $* == 'bam.c bar/bam.c foo/bam.c' ]] || err_exit '**/bam.c not working with -G option'
	cd ~-
	rm -rf /tmp/ksh$$
fi
exit $((Errors))
