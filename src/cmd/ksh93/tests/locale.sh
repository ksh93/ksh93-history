########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2009 AT&T Intellectual Property          #
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
	let Errors+=1
}
alias err_exit='err_exit $LINENO'

Command=${0##*/}
integer Errors=0

tmp=$(mktemp -dt) || { err_exit mktemp -dt failed; exit 1; }
trap "cd /; rm -rf $tmp" EXIT

# LC_ALL=debug is an ast specific debug/test locale

if	[[ "$(LC_ALL=debug $SHELL <<- \+EOF+
		x=a<1z>b<2yx>c
		print ${#x}
		+EOF+)" != 5
	]]
then	err_exit '${#x} not working with multibyte locales'
fi

export LC_ALL=C
a=$($SHELL -c '/' 2>&1 | sed -e "s,.*: *,," -e "s, *\[.*,,")
b=$($SHELL -c '(LC_ALL=debug / 2>/dev/null); /' 2>&1 | sed -e "s,.*: *,," -e "s, *\[.*,,")
[[ "$b" == "$a" ]] || err_exit "locale not restored after subshell -- expected '$a', got '$b'"
b=$($SHELL -c '(LC_ALL=debug; / 2>/dev/null); /' 2>&1 | sed -e "s,.*: *,," -e "s, *\[.*,,")
[[ "$b" == "$a" ]] || err_exit "locale not restored after subshell -- expected '$a', got '$b'"

# test shift-jis \x81\x40 ... \x81\x7E encodings
# (shift char followed by 7 bit ascii)

typeset -i16 chr
for locale in $(PATH=/bin:/usr/bin locale -a 2>/dev/null | grep -i jis)
do	export LC_ALL=$locale
	for ((chr=0x40; chr<=0x7E; chr++))
	do	c=${chr#16#}
		for s in \\x81\\x$c \\x$c
		do	b="$(printf "$s")"
			eval n=\$\'$s\'
			[[ $b == "$n" ]] || err_exit "LC_ALL=$locale printf difference for \"$s\" -- expected '$n', got '$b'"
			u=$(print -- $b)
			q=$(print -- "$b")
			[[ $u == "$q" ]] || err_exit "LC_ALL=$locale quoted print difference for \"$s\" -- $b => '$u' vs \"$b\" => '$q'"
		done
	done
done

# find a supported UTF-8 locale

punt=
for locale in $(PATH=/bin:/usr/bin locale -a 2>/dev/null | egrep '^(de|en|es|fr)_.*?(\.UTF-8)') de_DE fr_FR es_ES en_US ''
do	[[ $locale ]] || break
	while	:
	do	[[ $locale == *.UTF-8 ]] || locale=${locale%.*}.UTF-8
		if	[[ ! $($SHELL -c "LC_ALL=$locale" 2>&1) ]]
		then	[[ $locale == @(de|fr)* ]] && break 2
			[[ $punt ]] || punt=$locale
		fi
		[[ $locale == *_* ]] || break
		locale=${locale%_*}
	done
done
[[ $locale ]] || locale=$punt

[[ $locale ]] &&
{

export LC_ALL=C

# test multibyte value/trace format -- $'\303\274' is UTF-8 u-umlaut

c=$(LC_ALL=C $SHELL -c "printf $':%2s:\n' $'\303\274'")
u=$(LC_ALL=$locale $SHELL -c "printf $':%2s:\n' $'\303\274'" 2>/dev/null)
if	[[ "$c" != "$u" ]]
then	LC_ALL=$locale
	x=$'+2+ typeset item.text\
+3+ item.text=\303\274\
+4+ print -- \303\274\
\303\274\
+5+ eval $\'arr[0]=(\\n\\ttext=\\303\\274\\n)\'
+2+ arr[0].text=ü\
+6+ print -- \303\274\
ü\
+7+ eval txt=$\'(\\n\\ttext=\\303\\274\\n)\'
+2+ txt.text=\303\274\
+8+ print -- \'(\' text=$\'\\303\\274\' \')\'\
( text=\303\274 )'
	u=$(LC_ALL=$locale PS4='+$LINENO+ ' $SHELL -x -c "
		item=(typeset text)
		item.text=$'\303\274'
		print -- \"\${item.text}\"
		eval \"arr[0]=\$item\"
		print -- \"\${arr[0].text}\"
		eval \"txt=\${arr[0]}\"
		print -- \$txt
	" 2>&1)
	[[ "$u" == "$x" ]] || err_exit LC_ALL=$locale multibyte value/trace format failed

	x=$'00fc\n20ac'
	u=$(LC_ALL=$locale $SHELL -c $'printf "%04x\n" \$\'\"\303\274\"\' \$\'\"\xE2\x82\xAC\"\'')
	[[ $u == $x ]] || err_exit LC_ALL=$locale multibyte %04x printf format failed
fi

if	(( $($SHELL -c $'export LC_ALL='$locale$'; print -r "\342\202\254\342\202\254\342\202\254\342\202\254w\342\202\254\342\202\254\342\202\254\342\202\254" | wc -m' 2>/dev/null) == 10 ))
then	LC_ALL=$locale $SHELL -c b1=$'"\342\202\254\342\202\254\342\202\254\342\202\254w\342\202\254\342\202\254\342\202\254\342\202\254"; [[ ${b1:4:1} == w ]]' || err_exit 'multibyte ${var:offset:len} not working correctly'
fi

#$SHELL -c 'export LANG='$locale'; printf "\u[20ac]\u[20ac]" > $tmp/two_euro_chars.txt'
printf $'\342\202\254\342\202\254' > $tmp/two_euro_chars.txt
exp="6 2 6"
set -- $($SHELL -c "
	unset LC_CTYPE
	export LANG=$locale
	export LC_ALL=C
	command wc -C < $tmp/two_euro_chars.txt
	unset LC_ALL
	command wc -C < $tmp/two_euro_chars.txt
	export LC_ALL=C
	command wc -C < $tmp/two_euro_chars.txt
")
got=$*
[[ $got == $exp ]] || err_exit "command wc LC_ALL default failed -- expected '$exp', got '$got'"
set -- $($SHELL -c "
	if	builtin -f cmd wc 2>/dev/null
	then	unset LC_CTYPE
		export LANG=$locale
		export LC_ALL=C
		wc -C < $tmp/two_euro_chars.txt
		unset LC_ALL
		wc -C < $tmp/two_euro_chars.txt
		export LC_ALL=C
		wc -C < $tmp/two_euro_chars.txt
	fi
")
got=$*
[[ $got == $exp ]] || err_exit "builtin wc LC_ALL default failed -- expected '$exp', got '$got'"

}

# the ast debug locale has { decimal_point="," thousands_sep="." }

[[ ! $locale || $locale == e[ns]* ]] && locale=debug

#	exp		LC_ALL		LC_NUMERIC		LANG
set -- \
	2.5		''		C			$locale		\
	2.5		$locale		C			''		\
	2,5		''		$locale			C		\
	2,5		C		$locale			''		\

unset a b c
integer a b c
while	(( $# >= 4 ))
do	exp=$1
	unset H V
	typeset -A H
	typeset -a V
	[[ $2 ]] && V[0]="LC_ALL=$2;"
	[[ $3 ]] && V[1]="LC_NUMERIC=$3;"
	[[ $4 ]] && V[2]="LANG=$4;"
	for ((a = 0; a < 3; a++))
	do	for ((b = 0; b < 3; b++))
		do	if	(( b != a ))
			then	for ((c = 0; c < 3; c++))
				do	if	(( c != a && c != b ))
					then	T=${V[$a]}${V[$b]}${V[$c]}
						if	[[ ! ${H[$T]} ]]
						then	H[$T]=1
							got=$(env - $SHELL -c "${T}print \$(( $exp ))" 2>&1)
							[[ $got == $exp ]] || err_exit "${T} sequence failed -- expected '$exp', got '$got'"
						fi
					fi
				done
			fi
		done
	done
	shift 4
done

exit $Errors
