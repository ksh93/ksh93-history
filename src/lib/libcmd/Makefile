/*
 * command library -- commands small enough to be builtins
 */

:PACKAGE: ast

LICENSE = since=1992,author=gsf+dgk

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

CP = $(STDCP|"cp")
CHMOD = $(STDCHMOD|"chmod")

HOSTTYPE == "$(CC.HOSTTYPE)"

cmd 1.2 :LIBRARY: RELEASE cmdinit.c \
	cmd.h rev.h wc.h \
	basename.c cat.c chgrp.c chmod.c chown.c cmp.c comm.c \
	cp.c cut.c dirname.c date.c expr.c fds.c fmt.c fold.c \
	getconf.c head.c id.c join.c ln.c logname.c mkdir.c \
	mkfifo.c mv.c paste.c pathchk.c rev.c rm.c rmdir.c \
	stty.c sync.c tail.c tee.c tty.c uname.c uniq.c wc.c \
	revlib.c wclib.c \
	-lfsg

$(INCLUDEDIR) :INSTALLPROTO: cmd.h cmdext.h cmdlist.h

"win32*" :NOOPTIMIZE: id.c

cmdext.h : $(*$(*$(*cmd)):N=*.c)
	{
	cat <<!
	$("#")pragma prototyped
	$("/")*
	$(" ")* -lcmd extern function prototypes
	$(" ")*/
	
	!
	$(SED) \
		-e '/^b_[a-z_]*(/!d' \
		-e 's/^b_//' \
		-e 's/(.*//' \
		-e 's/.*/extern int	b_&(int, char**, void*);/' \
		$(*) |
	$(SORT) -u
	} > 1.$(tmp).h
	if	$(CMP) $(CMPFLAGS) 1.$(tmp).h $(<)
	then	$(RM) $(RMFLAGS) 1.$(tmp).h
	else	$(MV) 1.$(tmp).h $(<)
	fi

cmdlist.h : $(*$(*$(*cmd)):N=*.c)
	{
	cat <<!
	$("#")pragma prototyped
	$("/")*
	$(" ")* -lcmd function list -- define your own CMDLIST()
	$(" ")*/
	
	!
	$(SED) \
		-e '/^b_[a-z_]*(/!d' \
		-e 's/^b_//' \
		-e 's/(.*//' \
		-e 's/.*/CMDLIST(&)/' \
		$(*) |
	$(SORT) -u
	} > 1.$(tmp).h
	if	$(CMP) $(CMPFLAGS) 1.$(tmp).h $(<)
	then	$(RM) $(RMFLAGS) 1.$(tmp).h
	else	$(MV) 1.$(tmp).h $(<)
	fi

if CC.HOSTTYPE == "sun4"
	pathsetlink == symlink	/* ld.so workaround */
end

ignore dlldefs.h /* prevents bootstrap double build */
