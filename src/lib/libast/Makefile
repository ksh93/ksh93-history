/*
 * ast library
 */

:PACKAGE: ast

LICENSE = since=1985,author=gsf+dgk+kpv

ID = ast

HOSTTYPE == "$(CC.HOSTTYPE)"

CCFLAGS = $(CC.OPTIMIZE) -D_BLD_ast $(CC.DLL)

.SOURCE : cdt comp dir disc hash man misc path port preroot \
		regex sfio stdio string tm uwin vec vmalloc

.SOURCE.h : include comp

PARANOID ==
DEBUG ==
__OBSOLETE__ == 20010101

HEADERSRC = ast.h ast_dir.h ast_std.h ast_version.h \
	ccode.h cdt.h debug.h error.h find.h ftw.h ftwalk.h fts.h \
	fs3d.h getopt.h glob.h hash.h hashkey.h hashpart.h int.h ls.h \
	magic.h mc.h mime.h mnt.h modecanon.h modex.h namval.h option.h \
	proc.h re.h re_comp.h regex.h regexp.h sfio.h sfio_t.h sfdisc.h \
	sfstr.h stack.h stak.h stk.h swap.h tar.h times.h tm.h \
	tok.h usage.h vdb.h vecargs.h vmalloc.h wait.h wordexp.h \
	bytesex.h endian.h

HEADERGEN = align.h iconv.h limits.h nl_types.h preroot.h sig.h \
	$(ID)_botch.h $(ID)_ccode.h $(ID)_common.h $(ID)_fcntl.h \
	$(ID)_fs.h $(ID)_hdr.h $(ID)_lib.h $(ID)_mode.h $(ID)_param.h \
	$(ID)_time.h $(ID)_tty.h $(ID)_types.h $(ID)_unistd.h \
	$(ID)_wait.h

HEADEROPT = fmtmsg.h fnmatch.h libgen.h syslog.h

$(ID) 5.4 :LIBRARY: README RELEASE \
	LIBAST.3 ast.3 cdt.3 chr.3 compat.3 error.3 find.3 fmt.3 \
	fmtls.3 fs3d.3 ftwalk.3 getcwd.3 hash.3 iblocks.3 int.3 \
	magic.3 mem.3 mime.3 modecanon.3 optget.3 path.3 preroot.3 \
	proc.3 re.3 setenviron.3 sfdisc.3 sfio.3 sig.3 spawnveg.3 \
	stak.3 stk.3 strcopy.3 strdup.3 strelapsed.3 strerror.3 stresc.3 \
	streval.3 strgid.3 strmatch.3 stropt.3 strperm.3 strsignal.3 \
	strsort.3 strtape.3 strton.3 struid.3 swap.3 tab.3 tm.3 \
	tok.3 touch.3 vecargs.3 vmalloc.3 \
	$(HEADERSRC) \
	state.c \
	dirlib.h opendir.c readdir.c rewinddir.c seekdir.c telldir.c \
	getcwd.c fastfind.c \
	hashlib.h hashalloc.c hashdump.c hashfree.c hashlast.c \
	hashlook.c hashscan.c hashsize.c hashview.c hashwalk.c \
	memhash.c memsum.c strhash.c strkey.c strsum.c \
	stracmp.c strnacmp.c \
	_ccmaps.c ccmapc.c ccmapcpy.c ccmapid.c ccmaps.c ccnative.c \
	chresc.c chrtoi.c streval.c strexpr.c strmatch.c strcopy.c \
	modelib.h modei.c modex.c strmode.c \
	strlook.c strsearch.c strpsearch.c \
	stresc.c stropt.c strtape.c tok.c tokline.c tokscan.c \
	pathaccess.c pathcat.c pathcanon.c pathcheck.c pathpath.c \
	pathfind.c pathkey.c pathprobe.c pathrepl.c \
	pathtemp.c pathtmp.c pathstat.c \
	pathgetlink.c pathsetlink.c pathbin.c pathshell.c pathcd.c \
	fs3d.c ftwalk.c ftwflags.c fts.c setenviron.c \
	optget.c optjoin.c strsort.c \
	magic.c mime.c mimetype.c signal.c systrace.c \
	error.c errorf.c errormsg.c errorx.c setlocale.c translate.c \
	catopen.c iconv.c mc.c \
	liberror.c libevent.c \
	fmtbase.c fmtbuf.c fmtclock.c fmtdev.c fmtelapsed.c fmterror.c \
	fmtesc.c fmtfs.c fmtident.c fmtip4.c fmtls.c fmtmatch.c fmtmode.c \
	fmtnum.c fmtperm.c fmtre.c fmttime.c fmtuid.c fmtgid.c fmtsignal.c \
	strelapsed.c strperm.c struid.c strgid.c strtoip4.c stack.c stk.c \
	swapget.c swapmem.c swapop.c swapput.c \
	sigdata.c sigcrit.c sigunblock.c \
	proclib.h procopen.c procclose.c procrun.c procfree.c \
	tmdate.c tmfix.c tmfmt.c tmform.c tmgoff.c \
	tminit.c tmleap.c tmlex.c tmlocale.c tmmake.c tmpoff.c \
	tmscan.c tmtime.c tmtype.c tmword.c tmzone.c \
	vecargs.c vecfile.c vecfree.c vecload.c vecstring.c \
	univlib.h univdata.c touch.c mnt.c \
	memccpy.c memchr.c memcmp.c memcpy.c memdup.c memmove.c memset.c \
	mkdir.c mkfifo.c mknod.c rmdir.c remove.c rename.c link.c unlink.c \
	strdup.c strchr.c strrchr.c \
	strtod.c strtol.c strtoll.c strtoul.c strtoull.c strton.c strtonll.c \
	strcasecmp.c strncasecmp.c strerror.c \
	mktemp.c tmpnam.c \
	execlp.c execve.c execvp.c execvpe.c \
	spawnlp.c spawnve.c spawnveg.c spawnvp.c spawnvpe.c \
	vfork.c killpg.c \
	hsearch.c tsearch.c \
	getlogin.c putenv.c \
	lstat.c statvfs.c \
	gross.c gross_sgi.h omitted.c \
	fakelink.h readlink.c symlink.c \
	getpgrp.c setpgid.c setsid.c waitpid.c \
	confmap.c conftab.c confstr.c pathconf.c sysconf.c \
	creat64.c fcntl.c open.c \
	atexit.c getdents.c getwd.c dup2.c errno.c \
	getpreroot.c ispreroot.c realopen.c setpreroot.c \
	getgroups.c mount.c system.c iblocks.c \
	modedata.c tmdata.c \
	memfatal.c sfkeyprintf.c sfstrtmp.c \
	sfdcold.c sfdcdio.c sfdcdos.c sfdcfilter.c sfdcseekable.c \
	sfdcslow.c sfdcsubstream.c sfdctee.c sfdcunion.c \
	sfdcmore.c sfdcprefix.c \
	libc.c \
	/* standards */ \
	basename.c closelog.c dirname.c fmtmsglib.c fnmatch.c ftw.c \
	getdate.c getsubopt.c glob.c nftw.c openlog.c re_comp.c \
	realpath.c regcmp.c regexp.c setlogmask.c strftime.c \
	strptime.c swab.c syslog.c sysloglib.h tempnam.c \
	tmpfile.c wordexp.c mktime.c \
	/* regex */ \
	reglib.h regalloc.c regclass.c regcomp.c \
	regerror.c regexec.c regfatal.c reginit.c regnexec.c \
	regsub.c regrecord.c regrexec.c \
	/* cdt */ \
	dthdr.h dtclose.c dtdisc.c dtextract.c dtflatten.c dthash.c \
	dtlist.c dtmethod.c dtopen.c dtrenew.c dtrestore.c \
	dtsize.c dtstat.c dtstrhash.c dttree.c dtview.c dtwalk.c \
	/* sfio */ \
	sfhdr.h sfdchdr.h \
	sfclose.c sfclrlock.c sfdisc.c sfdlen.c sfexcept.c \
	sfgetl.c sfgetu.c sfcvt.c sfecvt.c sffcvt.c \
	sfextern.c sffilbuf.c sfflsbuf.c sfprints.c sfgetd.c \
	sfgetr.c sfllen.c sfmode.c sfmove.c sfnew.c \
	sfpkrd.c sfnotify.c sfnputc.c sfopen.c sfpeek.c sfpoll.c \
	sfpool.c sfpopen.c sfprintf.c sfputd.c sfputl.c sfputr.c \
	sfputu.c sfrd.c sfread.c sfreserve.c sfscanf.c sfseek.c sfset.c \
	sfsetbuf.c sfsetfd.c sfsize.c sfsk.c sfstack.c sfstrtod.c sfsync.c \
	sfswap.c sftable.c sftell.c sftmp.c sfungetc.c sfvprintf.c \
	sfvscanf.c sfwr.c sfwrite.c sfpurge.c sfraise.c \
	sfgetm.c sfmutex.c sfputm.c sfresize.c \
	_sfclrerr.c _sfeof.c _sferror.c _sffileno.c \
	_sfstacked.c _sfvalue.c \
	_sfgetc.c _sfgetl.c _sfgetl2.c _sfgetu.c _sfgetu2.c \
	_sfdlen.c _sfllen.c _sfslen.c _sfulen.c \
	_sfputc.c _sfputd.c _sfputl.c _sfputm.c _sfputu.c \
	/* stdio */ \
	clearerr.c fclose.c fdopen.c feof.c ferror.c fflush.c \
	fgetc.c fgetpos.c fgets.c fileno.c fopen.c fprintf.c \
	fpurge.c fputc.c fputs.c fread.c freopen.c fscanf.c \
	fseek.c fsetpos.c ftell.c fwrite.c getc.c getchar.c \
	getw.c pclose.c popen.c printf.c putc.c putchar.c \
	puts.c putw.c rewind.c scanf.c setbuf.c setbuffer.c \
	setlinebuf.c setvbuf.c snprintf.c sprintf.c sscanf.c \
	ungetc.c vfprintf.c vfscanf.c vprintf.c vscanf.c \
	vsnprintf.c vsprintf.c vsscanf.c \
	_doprnt.c _doscan.c _stdfun.c \
	_stdopen.c _stdprintf.c _stdscanf.c _stdsprnt.c \
	_stdvbuf.c _stdvsnprnt.c _stdvsprnt.c _stdvsscn.c \
	/* ast */ \
	astcopy.c astconf.c astdynamic.c astlicense.c astquery.c astwinsize.c \
	$(CC.LD.STATIC) aststatic.c getopt.c $(CC.LD.DYNAMIC) \
	/* vmalloc */ \
	vmalloc.h vmhdr.h vmbest.c vmclear.c vmclose.c vmdcheap.c vmdebug.c \
	vmdisc.c vmexit.c vmlast.c vmopen.c vmpool.c vmprivate.c vmprofile.c \
	vmregion.c vmsegment.c vmset.c vmstat.c vmstrdup.c vmtrace.c vmwalk.c \
	malloc.c \
	/* uwin */ \
	mathimpl.h rlib.h ufc-crypt.h \
	a64l.c acosh.c asinh.c atanh.c cbrt.c crypt.c crypt_util.c erf.c \
	err.c exp.c exp__E.c expm1.c gamma.c getpass.c lgamma.c log.c log1p.c \
	log__L.c rand48.c random.c rcmd.c rint.c support.c \
	/* obsolete */ \
	obsolete/mematoe.c obsolete/memetoa.c \
	/* misc */ \
	atmain.C \
	-liconv /* please OSF, this *has* to be wrong */

/*
 * man is stdio.h problematic
 * it gets included by all sorts of system headers
 * it gums up the libast works when override header
 * generation, e.g., limits.h, has stdio.h imbedded
 * the symptom is having to run `make install' with
 * -A and -t sprinkled in umpteen times until it
 * reaches steady state -- until the next stdio.h change
 *
 * this works around the problem by ignoring stdio.h
 * changes for the ast source, which shouldn't be too
 * bad since on most systems it includes sfio.h, and
 * any changes there will be seen
 *
 * also, -D_BLD_ast is explicit for the uwin bootstrap
 */

parameter	(_BLD_ast) ast_version.h
ignore		stdio.h
virtual		ast/stdio.h
scan.ignore	iffeio.h stdio.h FEATURE/stdio

if CC.HOSTTYPE == "sol*.*" || "$(-mam)" == "static*"
sfrd.o sfvprintf.o : .CC.NOOPTIMIZE
end
if CC.HOSTTYPE == "win32.*" || "$(-mam)" == "static*"
fastfind.o mc.o : .CC.NOOPTIMIZE
end

:INSTALLDIR: conf

conf :: conf.sh

conflib.h conflim.h confmap.h conftab.h confuni.h \
confmap.c conftab.c confstr.c pathconf.c sysconf.c :JOINT: \
		conf conf.tab
	$(*:O=1) $(IFFEFLAGS:V:N=-v) $(*:O>1) $(CC) $(CCFLAGS:N!=-[DIU]*)

/* NOTE: sun4 runtime link botches ro data so advertized sig_info is rw */

:READONLY: confmap.c conftab.c modedata.c /*sftable.c*/ \
	/*sigdata.c*/ tmdata.c univdata.c

:: stdgets.c stdprintf.c stdscanf.c stdvbuf.c stdsprnt.c \
	stdvsprnt.c stdvsnprnt.c stdvsscn.c stdopen.c

$(INCLUDEDIR) :INSTALLPROTO: $(HEADERSRC) $(HEADERGEN)

$(INCLUDEDIR)/stdio.h :INSTALL: FEATURE/stdio

$(HEADERGEN) :COPY: FEATURE/$$(<:B:/$(ID)_//)

iffeio.h :COPY: FEATURE/stdio.lcl

stdio.h :COPY: FEATURE/stdio

$(LIBDIR)/file/magic :INSTALL: magic.tab

:INSTALL: $(HEADEROPT:D=$(INCLUDEDIR):B:S)

$(HEADEROPT:D=$(INCLUDEDIR):B:S) : $$(<:B:S) ast_lib.h
	$(SILENT) $(GREP) -l 'define[ 	][ 	]*_[hl][di][rb]_$(<:B)' $(*:O>1) > /dev/null || {
		$(PROTO) -p $(PROTOFLAGS) $(*:O=1) > 1.$(tmp).x
		if	$(CMP) -s $(<) 1.$(tmp).x
		then	$(RM) -f 1.$(tmp).x
		else	$(MV) 1.$(tmp).x $(<)
		fi
	}

/* a few headers are problematic */

$(INCLUDEDIR)/dirent.h :INSTALL: $(INCLUDEDIR) dir64.h dirstd.h dirnsys.h dirsys.h ast_lib.h
	header=
	match=`$(SED) -e '/define[ 	]*_[lhst][idy][brsp]_[a-z]*[d6][i4]/!d' -e 's/.*define[ 	]*/./' -e 's/[ 	].*/./' $(*:N=*ast_lib.h) 2>/dev/null`
	case $match in
	*._lib_opendir.*)
		case $match in
		*._lib_stat64.*)case $match in
				*._typ_off64_t.*) header=$(*:N=*dir64.h) ;;
				esac
				;;
		*._hdr_dirent.*);;
		*._hdr_ndir.*)	header=$(*:N=*dirnsys.h) ;;
		*._sys_dir.*)	header=$(*:N=*dirsys.h) ;;
		esac
		;;
	*)	header=$(*:N=*dirstd.h)
		;;
	esac
	case $header in
	?*)	$(PROTO) -p $(PROTOFLAGS) $header > 1.$(tmp).x
		if	$(CMP) -s $(<) 1.$(tmp).x
		then	$(RM) -f 1.$(tmp).x
		else	$(MV) 1.$(tmp).x $(<)
		fi
	esac

$(INCLUDEDIR)/prototyped.h :INSTALL: $(INCLUDEDIR)
	echo "#include <../prototyped.h>" > 1.$(tmp).x
	if	$(CMP) -s $(<) 1.$(tmp).x
	then	$(RM) -f 1.$(tmp).x
	else	$(MV) 1.$(tmp).x $(<)
	fi

ast.req : .check.hdr .check.lib

/*
 * atmain.C is the only C++ and only for a few systems
 */

atmain.o : atmain.C
	ignore $(CC) -c $(CCFLAGS) $(*)
	if	test ! -f $(<)
	then	$(CP) $(*) $(*:B:S=.c)
		$(CC) -c $(CCFLAGS) $(*:B:S=.c)
	fi

/*
 * prepare for compilation by generating these headers first;
 * this helps mam by catching headers that might be indirectly
 * included on other systems; it also catches iffeio.h early in
 * the process since the ast stdio.h is useless until -last is built
 */

.check.hdr : .IGNORE .VIRTUAL iffeio.h - stdio.h

/*
 * some systems move -lc routines to -lm
 * see astmath.c for details
 */

.check.lib : .AFTER astmath.exe
	test -x $(~) ||
	echo ' -lm' >> $(<<)

astmath.exe : .DONTCARE astmath.c
	$(CC) $(CCFLAGS) -o $(*:N=*.c:B:S=.exe) $(*) 2>/dev/null ||
	rm -f $(*:N=*.c:B:S=.exe)

/* libmini.a is a bootstrap dll uwin cc -D_BLD_ast that exports part of ast */

MINI = mini

:: $(MINI).sym

if CC.HOSTTYPE == "win32*"

DLL = $(ID)$(VERSION:/[^0-9]//G)

$(MINI) : $(MINI)$$(CC.SUFFIX.SHARED)

$(MINI)$$(CC.SUFFIX.SHARED) : $(MINI).sym $(ID)
	rm -rf $(MINI).tmp
	mkdir $(MINI).tmp
	{
	echo LIBRARY $(DLL:F=%(upper)S)
	echo
	echo SECTIONS
	echo .data READ WRITE
	echo
	echo EXPORTS
	cat $(*:N=*.sym)
	} > $(MINI).tmp/$(DLL)$(CC.SUFFIX.LD:O=1)
	cd $(MINI).tmp
	$(LD) $(CCFLAGS:N=-[gG]*) $(CC.SHARED) -o $(DLL) $(DLL)$(CC.SUFFIX.LD:O=1) $(*$(DLL)$(CC.SUFFIX.DYNAMIC):C,^[^-],../&,)
	cd ..
	mv $(MINI).tmp/$(DLL)$(CC.SUFFIX.SHARED) $(<)
	rm -rf $(MINI).tmp

end

:MSGKEY: misc/magic.tab
	$(SED) \
		-e '/^#/d' \
		-e '/[^	]*		*[^	]*		*[^	]*		*./!d' \
		-e 's,^[^	]*		*[^	]*		*[^	]*		*\(.[^	]*\).*$,\1,' \
		-e 's,[\\"],\\&,g' \
		-e 's,.*,"&",' \
		$(*)
