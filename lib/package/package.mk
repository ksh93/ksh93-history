/*
 * source and binary package support
 *
 * @(#)package.mk (AT&T Labs Research) 2000-10-31
 *
 * usage:
 *
 *	cd $INSTALLROOT/lib/package
 *	nmake -f name [ closure ] [ exp|lcl|pkg|rpm ] [ base|delta ] type
 *
 * where:
 *
 *	name	package description file or component
 *
 *	type	source	build source archive, generates
 *			$(PACKAGEDIR)/name.version.release.suffix
 *		binary	build binary archive, generates
 *			$(PACKAGEDIR)/name.version.hosttype.release.suffix
 *
 * NOTE: $(PACKAGEDIR) is in the lowest view and is shared among all views
 *
 * generated archive member files are $(PACKAGEROOT) relative
 *
 * main assertion:
 *
 *	NAME [ covers ... ] [ name=value ] :PACKAGE: component ...
 *		[ promo ]
 *
 * option variables, shown with default values
 *
 *	format=tgz
 *		archive format
 *
 *	version=YYYY-MM-DD
 *		package version
 *
 *	release=NNNN
 *		package release (delta number)
 *
 * NOTE: the Makerules.mk :PACKAGE: operator defers to :package: when
 *	 a target is specified
 */

closure =
format = tgz
index =
init = INIT
licenses = ast gnu
name =
ratz = ratz
style = tgz
suffix = tgz
type =
version = $("":T=R%Y-%m-%d)
release = 0000

PACKAGEROOT = $(VROOT:T=F:P=L*:N!=*/arch/+([!/]):O=1)
PACKAGESRC = $(PACKAGEROOT)/lib/package
PACKAGEBIN = $(INSTALLROOT)/lib/package
PACKAGEDIR = $(PACKAGESRC)/$(style)
INSTALLOFFSET = $(INSTALLROOT:C%$(PACKAGEROOT)/%%)

package.omit = -|*/$(init)
package.glob.all = $(INSTALLROOT)/src/($(MAKEDIRS:/:/|/G))/*/($(MAKEFILES:/:/|/G))
package.all = $(package.glob.all:P=G:W=O=$(?$(name):A=.VIRTUAL):N!=$(package.omit):T=F:$(VROOT:T=F:P=L*:C,.*,C;^&/;;,:/ /:/G))
package.glob.pkg = $(INSTALLROOT)/src/($(MAKEDIRS:/:/|/G))/($(~$(name):/ /|/G))/($(MAKEFILES:/:/|/G))
package.pkg = $(package.glob.pkg:P=G:D:N!=$(package.omit):T=F:$(VROOT:T=F:P=L*:C,.*,C;^&/;;,:/ /:/G))
package.closure = $(closure:?$(package.all)?$(package.pkg)?)

package.ini = ignore mamprobe manmake package silent
package.src.pat = $(PACKAGESRC)/($(name).(ini|lic|pkg)|($(name:N=*-*:/-.*/|/)$(licenses:/ /|/G)).lic) $(PACKAGESRC)/LICENSES/($(name)|$(licenses:/ /|/G)|$(name:N=*-*:/-.*//)|$(package.closure:B:/ /|/G))
package.src = $(package.src.pat:P=G)
package.bin = $(PACKAGEBIN)/$(name).ini

op = current
source = $(PACKAGEDIR)/$(name).$(version).$(release).$(suffix)
binary = $(PACKAGEDIR)/$(name).$(version).$(release).$(CC.HOSTTYPE).$(suffix)
old.new.source = $(PACKAGEDIR)/$(name).$(old.version).$(version).$(suffix)
new.old.source = $(PACKAGEDIR)/$(name).$(version).$(old.version).$(suffix)
old.new.binary = $(PACKAGEDIR)/$(name).$(old.version).$(version).$(CC.HOSTTYPE).$(suffix)
new.old.binary = $(PACKAGEDIR)/$(name).$(version).$(old.version).$(CC.HOSTTYPE).$(suffix)

source.list = $("$(PACKAGEDIR)/$(name).*.[0-9][0-9][0-9][0-9].$(suffix)":P=G:H>)
binary.list = $("$(PACKAGEDIR)/$(name).*.[0-9][0-9][0-9][0-9].$(CC.HOSTTYPE).$(suffix)":P=G:H>)

source.ratz = $("$(INSTALLROOT)/src/cmd/$(init)/$(ratz).c":T=F)
binary.ratz = $("$(INSTALLROOT)/src/cmd/$(init)/$(ratz)":T=F)

$(init) : .VIRTUAL $(init)

":package:" : .MAKE .OPERATOR
	local P I R V
	P := $(<:O=1)
	$(P) : $(>:V)
	if ! name
		name := $(P)
		.PACKAGE. := $(P)
		if name == "$(init)"
			package.omit = -
			package.src += $(package.ini:C,^,$(PACKAGEROOT)/bin/,) $(PACKAGESRC)/package.mk
		end
		for I $(<:O>1)
			if I == "*=*"
				eval
				$(I)
				end
			elif R = "$(I:D:B:S=.pkg:T=F)"
				if "$(I:D:B=gen/$(I:B):S=.ver:T=F)"
					covers : $(I:B)
				else
					error $(--exec:?3?1?) package $(I) must be written before $(P)
				end
			else
				version := $(I)
			end
		end
		I := $(name)
		while 1
			LICENSEFILE := $(LICENSEFILE):$(I:D=$(LIBDIR)/package:B:S=.lic)
			if I != "*-*"
				break
			end
			I := $(I:/-[^-]*$//)
		end
		export LICENSEFILE
	end
	if "$(>)"
		for I $(>:V)
			if R = "$(I:D:B:S=.pkg:T=F)"
				if I == "$(init)"
					package.omit = -
				else
					requires : $(I:B)
				end
				if V = "$(I:D:B=gen/$(I:B):S=.ver:T=F)"
					req : $(V)
				else
					error $(--exec:?3?1?) package $(I) must be written before $(P)
				end
				include $(R)
			else
				$(I) : .VIRTUAL
				if I == "/*"
					package.dir += $(I:V)
				end
			end
		end
	end
	if "$(@)"
		$(P).txt := $(@)
	else
		$(P).txt := This is the $(P) package.
	end

":INDEX:" : .MAKE .OPERATOR
	index := $(>)

base delta : .MAKE .VIRTUAL .FORCE
	op := $(<)

closure : .MAKE .VIRTUAL .FORCE
	$(<) := 1

exp lcl pkg rpm : .MAKE .VIRTUAL .FORCE
	style := $(<)

source : .source.init .source.gen .source.$$(style)

.source.init : .MAKE
	local A B D P V I
	type := source
	if name == "$(ratz)"
		suffix = c
	end
	A := $(source.list)
	B := $(A:N=*.0000.$(suffix):O=1:T=F)
	P := $(A:N=*.0000.$(suffix):O=2:T=F)
	D := $(A:N!=*.0000.$(suffix):O=1:T=F)
	if op == "delta"
		if ! B
			error 3 delta requires a base archive
		end
		base := -z $(B)
		if "$(release)" != "0000"
			D := $(release)
		else
			if D
				D := $(D:B:/.*\.0*//)
			end
			let D = D + 1
			if D > 999
				error 3 $(source:B:S): too many deltas
			end
		end
		release := $(D:F=%04d)
		source := $(B:D)/$(B:D:B:B:S=.$(release).$(suffix))
		deltaversion := $(B:B:B:/$(name).//)
		let deltasince = $(deltaversion:/.*-//) + 1
		deltasince := $(deltaversion:/[^-]*$/$(deltasince:F=%02d)/)
	elif B || op == "base"
		if op == "base"
			for I $(B) $(P)
				V := $(I:B:/$(name)\.\([^.]*\).*/\1/)
				if V != "$(version)"
					old.version := $(V)
					old.source := $(I)
					if "$(old.version)" >= "$(version)"
						error 3 $(name): previous base $(old.version) is newer than $(version)
					end
					break
				end
			end
		else
			source := $(B)
		end
		if B == "$(source)"
			if "$(B:D:B:B)" == "$(D:D:B:B)" && "$(B:B::S)" != "$(D:B::S)"
				error 3 $(B:B:S): base overwrite would invalidate delta $(D:B:S)
			end
			error 1 $(B:B:S): replacing current base
		end
	end
	version := $(source:B:B:/$(name).//)
	PACKAGEGEN := $(PACKAGESRC)/gen

.source.gen : $$(PACKAGEDIR) $$(PACKAGEGEN) $$(PACKAGEGEN)/SOURCE.html $$(PACKAGEGEN)/BINARY.html $$(PACKAGEGEN)/DETAILS.html

$$(PACKAGEDIR) $$(PACKAGEGEN) : .IGNORE
	test -d $(<) || mkdir $(<)

$$(PACKAGEGEN)/SOURCE.html : $$(INSTALLROOT)/bin/package
	package html source > $(<)

$$(PACKAGEGEN)/BINARY.html : $$(INSTALLROOT)/bin/package
	package html binary > $(<)

$$(PACKAGEGEN)/DETAILS.html : $$(INSTALLROOT)/bin/package
	package html intro > $(<)

.source.exp .source.pkg .source.rpm : .MAKE
	error 3 $(style): source package style not supported yet

.source.lcl :
	if	test '$(ratz)' != '$(name)'
	then	tmp=/tmp/pkg$(tmp)
		mkdir $tmp
		{
			integer m
			$(package.src:T=F:/.*/echo ";;;&"$("\n")/)
			set -- $(package.closure)
			for i
			do	cd $(INSTALLROOT)/$i
				$(MAKE) --noexec $(-) $(=) list.package.local
			done
			set -- $(package.dir:P=G)
			for i
			do	tw -d $i -e "action:printf(';;;%s\n',path);"
			done
		} |
		$(PAX)	--filter=- \
			$(op:N=delta:??--format=$(format)?) \
			--local \
			-wvf $(source) $(base) \
			$(VROOT:T=F:P=L*:C%.*%-s",^&/,,"%)
		test '' != '$(old.source)' &&
		$(PAX) -rf $(source) -wvf $(old.new.source) -z $(old.source) &&
		$(PAX) -rf $(old.source) -wvf $(new.old.source) -z $(source)
		rm -rf $tmp
	fi

.source.tgz :
	if	test '$(ratz)' = '$(name)'
	then	if	test '' != '$(old.source)' &&
			cmp -s $(source.ratz) $(source)
		then	: $(name) is up to date
		else	echo $(name) $(version) $(release) 1 > $(PACKAGEGEN)/$(name).ver
			: > $(PACKAGEGEN)/$(name).req
			if	test '' != '$(index)'
			then	echo '$(index)' > $(PACKAGEGEN)/$(name).inx
			fi
			{
				echo '$($(name).txt)'
				package help source
			} > $(PACKAGEGEN)/$(name).txt
			{
				echo '.xx title="$(name) package"'
				echo '.xx meta.description="$(name) package"'
				echo '.xx meta.keywords="software, package"'
				echo '.MT 4'
				echo '.TL'
				echo '$(name) package'
				echo '.H 1'
				echo '$($(name).txt)'
			} |
			$(MM2HTML) $(MM2HTMLFLAGS) -o nohtml.ident > $(PACKAGEGEN)/$(name).html
			cp $(source.ratz) $(source)
			echo local > $(source:D:B=$(name):S=.tim)
		fi
	else	tmp=/tmp/pkg$(tmp)
		mkdir $tmp
		{
			integer m
			if	test '$(init)' = '$(name)'
			then	cat > $tmp/README <<'!'
	This is a package root directory $PACKAGEROOT. All source and binary
	packages under this directory are controlled by the command
		bin/package
	Binary package files are in the install root directory $INSTALLROOT,
	named arch/`bin/package`. For more information run
		bin/package help
	!
				echo ";;;$tmp/README;README"
				cat > $tmp/Makefile <<'!'
	:MAKE:
	!
				echo ";;;$tmp/Makefile;src/Makefile"
				echo ";;;$tmp/Makefile;src/cmd/Makefile"
				echo ";;;$tmp/Makefile;src/lib/Makefile"
				cat > $tmp/Mamfile1 <<'!'
	info mam static
	note source level :MAKE: equivalent
	make install
	make all
	exec - ${MAMAKE} -r '*/*' ${MAMAKEARGS}
	done all virtual
	done install virtual
	!
				echo ";;;$tmp/Mamfile1;src/Mamfile"
				cat > $tmp/Mamfile2 <<'!'
	info mam static
	note component level :MAKE: equivalent
	make install
	make all
	exec - ${MAMAKE} -r '*' ${MAMAKEARGS}
	done all virtual
	done install virtual
	!
				echo ";;;$tmp/Mamfile2;src/cmd/Mamfile"
				echo ";;;$tmp/Mamfile2;src/lib/Mamfile"
			fi
			$(package.src:T=F:/.*/echo ";;;&"$("\n")/)
			echo $(name) $(version) $(release) 1 > $(PACKAGEGEN)/$(name).ver
			echo ";;;$(PACKAGEGEN)/$(name).ver"
			if	test '' != '$(~covers)'
			then	for i in $(~covers)
				do	echo ";;;$(PACKAGEGEN)/$i.ver"
					echo ";;;$(PACKAGEGEN)/$i.req"
				done
			fi
			sed 's,1$,0,' $(~req) < /dev/null > $(PACKAGEGEN)/$(name).req
			echo ";;;$(PACKAGEGEN)/$(name).req"
			if	test '' != '$(index)'
			then	echo '$(index)' > $(PACKAGEGEN)/$(name).inx
				echo ";;;$(PACKAGEGEN)/$(name).inx"
			fi
			{
				{
				echo '$($(name).txt)'
				if	test '' != '$(~covers)'
				then	echo "This package is a superset of the following package$(~covers:O=2:?s??): $(~covers); you won't need $(~covers:O=2:?these?this?) if you download $(name)."
				fi
				if	test '' != '$(~requires)'
				then	echo 'It requires the following package$(~requires:O=2:?s??): $(~requires).'
				fi
				} | fmt
				package help source
				package release $(name)
			} > $(PACKAGEGEN)/$(name).txt
			echo ";;;$(PACKAGEGEN)/$(name).txt"
			{
				echo '.xx title="$(name) package"'
				echo '.xx meta.description="$(name) package"'
				echo '.xx meta.keywords="software, package"'
				echo '.MT 4'
				echo '.TL'
				echo '$(name) package'
				echo '.H 1 "$(name) package"'
				echo '$($(name).txt)'
				set -- $(package.closure:C,.*,$(INSTALLROOT)/&/PROMO.mm,:T=F:D::B)
				hot=
				for i
				do	hot="$hot -e s/\\(\\<$i\\>\\)/\\\\h'0*1'\\1\\\\h'0'/"
				done
				set -- $(package.closure:B)
				if	test $# != 0
				then	echo 'Components in this package:'
					echo '.nf'
					if	test '' != "$hot"
					then	hot="sed $hot"
					else	hot=cat
					fi
					for i
					do	echo $i
					done |
					pr -6 -o4 -t |
					$hot
					echo '.fi'
				fi
				echo '.P'
				if	test '' != '$(~covers)'
				then	echo "This package is a superset of the following package$(~covers:O=2:?s??): $(~covers); you won't need $(~covers:O=2:?these?this?) if you download $(name)."
				fi
				if	test '' != '$(~requires)'
				then	echo 'It requires the following package$(~requires:O=2:?s??): $(~requires).'
				fi
				case $(name) in
				$(init))set -- $(licenses:B:S=.lic:T=F) ;;
				*)	set -- $(package.src:N=*.lic) ;;
				esac
				case $# in
				0)	;;
				*)	case $# in
					1)	echo 'The software is covered by this license:' ;;
					*)	echo 'The software is covered by these licenses:' ;;
					esac
					echo .BL
					for j
					do	i=`$(PROTO) -l $j -p -h -o type=usage /dev/null | sed -e 's,.*\[-license?\([^]]*\).*,\1,'`
						echo .LI
						echo ".xx link=\"$i\""
					done
					echo .LE
					;;
				esac
				echo 'A recent'
				echo '.xx link="release change log"'
				echo 'is also included.'
				cat $(package.closure:C,.*,$(INSTALLROOT)/&/PROMO.mm,:T=F) < /dev/null
				echo '.nf'
				echo '.H 1 "release change log"'
				package release $(name) |
				sed -e 's/:::::::: \(.*\) ::::::::/.H 2 \1/'
				echo '.fi'
			} |
			$(MM2HTML) $(MM2HTMLFLAGS) -o nohtml.ident > $(PACKAGEGEN)/$(name).html
			echo ";;;$(PACKAGEGEN)/$(name).html"
			if	test '' != '$(deltasince)'
			then	{
				echo '.xx title="$(name) package"'
				echo '.xx meta.description="$(name) package $(version) delta $(release)"'
				echo '.xx meta.keywords="software, package, delta"'
				echo '.MT 4'
				echo '.TL'
				echo '$(name) package $(deltaversion) delta $(release)'
				echo '.H 1 "$(name) package $(deltaversion) delta $(release) changes"'
				echo '.nf'
				package release $(deltasince) $(name) |
				sed -e 's/:::::::: \(.*\) ::::::::/.H 2 \1/'
				echo '.fi'
				} |
				$(MM2HTML) $(MM2HTMLFLAGS) -o nohtml.ident > $(PACKAGEGEN)/$(name).$(release).html
				echo ";;;$(PACKAGEGEN)/$(name).$(release).html"
			fi
			set -- $(package.closure)
			for i
			do	cd $(INSTALLROOT)/$i
				(( m++ ))
				$(MAKE) --noexec --force --mam=static --mismatch CC=$(CC.DIALECT:N=C++:?CC?cc?) $(=) 'dontcare test' install test > $tmp/$m.mam
				echo ";;;$tmp/$m.mam;$i/Mamfile"
				$(MAKE) --noexec $(-) $(=) list.package.$(type)
			done
			set -- $(package.dir:P=G)
			for i
			do	tw -d $i -e "action:printf(';;;%s\n',path);"
			done
		} |
		$(PAX)	--filter=- \
			$(op:N=delta:??--format=$(format)?) \
			--local \
			-wvf $(source) $(base) \
			$(VROOT:T=F:P=L*:C%.*%-s",^&/,,"%)
		echo local > $(source:D:B=$(name):S=.tim)
		test '' != '$(old.source)' &&
		$(PAX) -rf $(source) -wvf $(old.new.source) -z $(old.source) &&
		$(PAX) -rf $(old.source) -wvf $(new.old.source) -z $(source)
		rm -rf $tmp
	fi

binary : .binary.init .binary.gen .binary.$$(style)

.binary.init : .MAKE
	local A B D I P V
	type := binary
	if name == "$(ratz)"
		suffix = exe
	end
	A := $(binary.list)
	B := $(A:N=*.0000.*:O=1:T=F)
	P := $(A:N=*.0000.*:O=2:T=F)
	D := $(A:N!=*.0000.*:O=1:T=F)
	if op == "delta"
		if ! B
			error 3 delta requires a base archive
		end
		base := -z $(B)
		if "$(release)" != "0000"
			D := $(release)
		else
			if D
				D := $(D:B:/.*\.0*//)
			end
			let D = D + 1
			if D > 999
				error 3 $(binary:B:S): too many deltas
			end
		end
		release := $(D:F=%04d)
	elif B || op == "base"
		if op == "base"
			for I $(B) $(P)
				V := $(I:B:/$(name)\.\([^.]*\).*/\1/)
				if V != "$(version)"
					old.version := $(V)
					old.binary := $(I)
					if "$(old.version)" >= "$(version)"
						error 3 $(name): previous base $(old.version) is newer than $(version)
					end
					break
				end
			end
		else
			binary := $(B)
		end
		if B == "$(binary)"
			if "$(B:D:B)" == "$(D:D:B)" && "$(B:S)" != "$(D:S)"
				error 3 $(B:B:S): base overwrite would invalidate delta $(D:B:S)
			end
			error 1 $(B:B:S): replacing current base
		end
	end
	version := $(binary:B:B:/$(name).\([^.]*\).*/\1)
	PACKAGEGEN := $(PACKAGEBIN)/gen

.binary.gen : $$(PACKAGEDIR) $$(PACKAGEGEN)

.binary.exp .binary.pkg .binary.rpm : .MAKE
	error 3 $(style): binary package style not supported yet

.binary.lcl :
	if	test '$(ratz)' != '$(name)'
	then	tmp=/tmp/pkg$(tmp)
		mkdir $tmp
		{
			$(package.src:T=F:/.*/echo ";;;&"$("\n")/)
			$(package.bin:T=F:/.*/echo ";;;&"$("\n")/)
			set -- $(package.closure)
			for i
			do	cd $(INSTALLROOT)/$i
				$(MAKE) --noexec $(-) $(=) list.package.$(type) cc-
			done
		} |
		sort -u |
		$(PAX)	--filter=- \
			$(op:N=delta:??--format=$(format)?) \
			--local \
			--checksum=md5:$(PACKAGEGEN)/$(name).sum \
			--install=$(PACKAGEGEN)/$(name).ins \
			-wvf $(binary) $(base) \
			-s",^$tmp/,$(INSTALLOFFSET)/," \
			$(PACKAGEROOT:C%.*%-s",^&/,,"%)
		echo local > $(binary:D:B=$(name):S=.$(CC.HOSTTYPE).tim)
		test '' != '$(old.binary)' &&
		$(PAX) -rf $(binary) -wvf $(old.new.binary) -z $(old.binary) &&
		$(PAX) -rf $(old.binary) -wvf $(new.old.binary) -z $(binary)
		rm -rf $tmp
	fi

.binary.tgz :
	if	test '$(ratz)' = '$(name)'
	then	if	test '' != '$(old.binary)' &&
			cmp -s $(binary.ratz) $(binary)
		then	: $(name) is up to date
		else	echo $(name) $(version) $(release) 1 > $(PACKAGEGEN)/$(name).ver
			echo $(name) $(version) $(release) 1 > $(PACKAGEGEN)/$(name).ver
			: > $(PACKAGEGEN)/$(name).req
			if	test '' != '$(index)'
			then	echo '$(index)' > $(PACKAGEGEN)/$(name).inx
			fi
			{
				echo '$($(name).txt)'
				package help binary
			} > $(PACKAGEGEN)/$(name).txt
			cp $(binary.ratz) $(binary)
			echo local > $(binary:D:B=$(name):S=.$(CC.HOSTTYPE).tim)
		fi
	else	tmp=/tmp/pkg$(tmp)
		mkdir $tmp
		{
			if	test '$(init)' = '$(name)'
			then	for i in lib32 lib64
				do	if	test -d $(INSTALLROOT)/$i
					then	echo ";physical;;$(INSTALLROOT)/$i"
					fi
				done
			fi
			$(package.src:T=F:/.*/echo ";;;&"$("\n")/)
			$(package.src:T=F:N=*/LICENSES/*:B:C,.*,echo ";;;$(PACKAGESRC)/LICENSES/&;$(PACKAGEBIN)/LICENSES/&"$("\n"),)
			$(package.bin:T=F:/.*/echo ";;;&"$("\n")/)
			echo $(name) $(version) $(release) 1 > $(PACKAGEGEN)/$(name).ver
			echo ";;;$(PACKAGEGEN)/$(name).ver"
			if	test '' != '$(~covers)'
			then	for i in $(~covers)
				do	echo ";;;$(PACKAGEGEN)/$i.ver"
					echo ";;;$(PACKAGEGEN)/$i.req"
				done
			fi
			sed 's,1$,0,' $(~req) < /dev/null > $(PACKAGEGEN)/$(name).req
			echo ";;;$(PACKAGEGEN)/$(name).req"
			if	test '' != '$(index)'
			then	echo '$(index)' > $(PACKAGEGEN)/$(name).inx
				echo ";;;$(PACKAGEGEN)/$(name).inx"
			fi
			{
				{
				echo '$($(name).txt)'
				if	test '' != '$(~covers)'
				then	echo "This package is a superset of the following package$(~covers:O=2:?s??): $(~covers); you won't need $(~covers:O=2:?these?this?) if you download $(name)."
				fi
				if	test '' != '$(~requires)'
				then	echo 'It requires the following package$(~requires:O=2:?s??): $(~requires).'
				fi
				} | fmt
				package help binary
				package release $(name)
			} > $(PACKAGEGEN)/$(name).txt
			echo ";;;$(PACKAGEGEN)/$(name).txt"
			set -- $(package.closure)
			for i
			do	cd $(INSTALLROOT)/$i
				$(MAKE) --noexec $(-) $(=) list.package.$(type) cc-
			done
		} |
		sort -u |
		$(PAX)	--filter=- \
			$(op:N=delta:??--format=$(format)?) \
			--local \
			--checksum=md5:$(PACKAGEGEN)/$(name).sum \
			--install=$(PACKAGEGEN)/$(name).ins \
			-wvf $(binary) $(base) \
			-s",^$tmp/,$(INSTALLOFFSET)/," \
			$(PACKAGEROOT:C%.*%-s",^&/,,"%)
		echo local > $(binary:D:B=$(name):S=.$(CC.HOSTTYPE).tim)
		test '' != '$(old.binary)' &&
		$(PAX) -rf $(binary) -wvf $(old.new.binary) -z $(old.binary) &&
		$(PAX) -rf $(old.binary) -wvf $(new.old.binary) -z $(binary)
		rm -rf $tmp
	fi

list.install list.manifest :
	set -- $(package.closure)
	for i
	do	cd $(INSTALLROOT)/$i
		$(MAKE) --noexec $(-) $(=) $(<)
	done
