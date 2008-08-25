########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2008 AT&T Intellectual Property          #
#                      and is licensed under the                       #
#                  Common Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
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
	print -u2 -r ${Command}[$1]: "${@:2}"
	(( Errors+=1 ))
}

alias err_exit='err_exit $LINENO'

integer Errors=0

s="$($SHELL -c '
set -o errexit
integer i

trap "print got_child" CHLD

sleep 2.0 &
sleep 4.0 &
for ((i=0 ; i < 10 ; i++)) ; do
      print $i
      sleep .5
      
      # external, non-background command for which a SIGCHLD should
      # _not_ be fired
      $SHELL -c : > /dev/null
done
print "loop finished"
wait
print "done"
' 2>&1 )" || err_exit "test loop failed."

[[ "$s" == ~(Er)$'9\nloop finished\ndone' ]] || err_exit "Expected '9\nloop fi
nished\ndone' at the end of the output, got ${s}."
[[ "$s" == ~(El)$'0\n1\n2' ]] || err_exit "Expected '0\n1\n2' as at the beginnin
g of the output, got ${s}."

integer count
(( count=$(fgrep "got_child" <<< "$s" | wc -l) )) || err_exit "counting failed."
(( count == 2 )) || err_exit "Expected count==2, got count==${count}."

exit $((Errors))
