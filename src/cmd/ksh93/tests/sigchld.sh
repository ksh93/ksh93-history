#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#

#
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# ident	"%Z%%M%	%I%	%E% SMI"
#

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
# EOF.
