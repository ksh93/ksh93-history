/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1992-2001 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*******************************************************************/
#pragma prototyped

/*
 * print the tail of one or more files
 *
 *   David Korn
 *   Glenn Fowler
 */

static const char usage[] =
"+[-?\n@(#)$Id: tail (AT&T Labs Research) 2001-09-06 $\n]"
USAGE_LICENSE
"[+NAME?tail - output trailing portion of one or more files ]"
"[+DESCRIPTION?\btail\b copies one or more input files to standard output "
	"starting at a designated point for each file.  Copying starts "
	"at the point indicated by the options and is unlimited in size.]"
"[+?By default a header of the form \b==> \b\afilename\a\b <==\b "
	"is output before all but the first file but this can be changed "
	"with the \b-q\b and \b-v\b options.]"
"[+?If no \afile\a is given, or if the \afile\a is \b-\b, \btail\b "
	"copies from standard input. The start of the file is defined "
	"as the current offset.]"
"[+?The option argument for \b-c\b can optionally be "
	"followed by one of the following characters to specify a different "
	"unit other than a single byte:]{"
		"[+b?512 bytes.]"
		"[+k?1-kilobyte.]"
		"[+m?1-megabyte.]"
	"}"
"[+?For backwards compatibility, \b-\b\anumber\a  is equivalent to "
	"\b-n\b \anumber\a and \b+\b\anumber\a is equivalent to "
	"\b-n -\b\anumber\a.]"

"[n:lines]:[lines:=10?Copy \alines\a lines from each file.  A negative value "
	"for \alines\a indicates an offset from the start of the file.]"
"[c:bytes]:[chars?Copy \achars\a bytes from each file.  A negative value "
	"for \achars\a indicates an offset from the start of the file.]"
"[f:forever|follow?Loop forever trying to read more characters as the "
	"end of each file to copy new data. Ignored if reading from a pipe "
	"or fifo.]"
"[q:quiet|silent?Never ouput filename headers.]"
"[r:reverse?Output lines in reverse order.]"
"[t:timeout?Exit after \atimeout\a elapses with no additional \b--forever\b "
	"output. There is no timeout by default. The default \atimeout\a "
	"unit is seconds. \atimeout\a may be a catenation of 1 or more "
	"integers, each followed by a 1 character suffix. The suffix may be "
	"omitted from the last integer, in which case it is interpreted as "
	"seconds. The supported suffixes are:]:[timeout]{"
		"[+s?seconds]"
		"[+m?minutes]"
		"[+h?hours]"
		"[+d?days]"
		"[+w?weeks]"
		"[+M?months]"
		"[+y?years]"
		"[+S?scores]"
	"}"
"[v:verbose?Always ouput filename headers.]"
"\n"
"\n[file ...]\n"
"\n"
"[+EXIT STATUS?]{"
	"[+0?All files copied successfully.]"
	"[+>0?One or more files did not copy.]"
"}"
"[+SEE ALSO?\bcat\b(1), \btail\b(1), \brev\b(1)]"
;

#include <cmdlib.h>
#include <ctype.h>
#include <ls.h>

#define F_FLAG		1
#define R_FLAG		2
#define N_FLAG		4

#define NOW		(unsigned long)time(NiL)

typedef struct
{
	char*		name;
	Sfio_t*		sp;
	size_t		size;
	size_t		last;
	int		warn;
} Tail_t;

/*
 * if file is seekable, position file to tail location and return offset
 * otherwise, return -1
 */

static Sfoff_t
tailpos(register Sfio_t* fp, register long nitems, int delim)
{
	register size_t		n;
	register Sfoff_t	offset;
	register Sfoff_t	first;
	register Sfoff_t	last;
	register char*		s;
	register char*		t;

	if ((first = sfseek(fp, (Sfoff_t)0, SEEK_CUR)) < 0)
		return -1;
	last = sfsize(fp);
	if (delim < 0)
	{
		if ((offset = last - nitems) < first)
			return first;
		return offset;
	}
	if ((offset = last - SF_BUFSIZE) < first)
		offset = first;
	for (;;)
	{
		sfseek(fp, offset, SEEK_SET);
		n = last - offset;
		if (!(s = sfreserve(fp, n, 1)))
			return -1;
		t = s + n;
		while (t > s)
			if (*--t == delim && nitems-- <= 0)
			{
				sfread(fp, s, 0);
				return offset + (t - s) + 1;
			}
		sfread(fp, s, 0);
		if (offset == first)
			break;
		last = offset;
		if ((offset = last - SF_BUFSIZE) < first)
			offset = first;
	}
	return first;
}

/*
 * this code handles tail from a pipe without any size limits
 */

static void
pipetail(Sfio_t* infile, Sfio_t* outfile, int nitems, int delim)
{
	register Sfio_t*	out;
	register int		n = (2 * SF_BUFSIZE);
	register int		nleft = nitems;
	register int		fno = 0;
	Sfoff_t			offset[2];
	Sfio_t*			tmp[2];

	if (delim < 0 && nitems < n)
		n = nitems;
	out = tmp[0] = sftmp(n);
	tmp[1] = sftmp(n);
	offset[0] = offset[1] = 0;
	while ((n = sfmove(infile, out, nleft, delim)) > 0)
	{
		offset[fno] = sftell(out);
		if ((nleft -= n) <= 0)
		{
			out = tmp[fno= !fno];
			sfseek(out, (Sfoff_t)0, SEEK_SET);
			nleft = nitems;
		}
	}
	if (nleft == nitems)
	{
		offset[fno] = 0;
		fno= !fno;
	}
	sfseek(tmp[0], (Sfoff_t)0, SEEK_SET);

	/*
	 * see whether both files are needed
	 */

	if (offset[fno])
	{
		sfseek(tmp[1], (Sfoff_t)0, SEEK_SET);
		if ((n = nitems - nleft) > 0) 
			sfmove(tmp[!fno], NiL, n, delim); 
		if ((n = offset[!fno] - sftell(tmp[!fno])) > 0)
			sfmove(tmp[!fno], outfile, n, -1); 
	}
	else
		fno = !fno;
	sfmove(tmp[fno], outfile, offset[fno], -1); 
	sfclose(tmp[0]);
	sfclose(tmp[1]);
}

int
b_tail(int argc, char** argv, void* context)
{
	static const char	header_fmt[] = "\n==> %s <==\n";

	register Sfio_t*	ip;
	register int		n;
	register int		delim = '\n';
	register int		flags = 0;
	char*			s;
	char*			t;
	char*			r;
	char*			e;
	Sfoff_t			offset;
	long			number = 10;
	unsigned long		timeout = 0;
	unsigned long		expire;
	struct stat		st;
	int			header = 1;
	int			quiet = 0;
	const char*		format = header_fmt+1;
	size_t			z;
	Sfio_t*			op;
	register Tail_t*	fp;
	register Tail_t*	ep;
	register Tail_t*	hp;
	Tail_t*			files;

	cmdinit(argv, context, ERROR_CATALOG);
	while (n = optget(argv, usage)) switch (n)
	{
	    case 'q':
		quiet = 1;
		header = argc;
		break;
	    case 'v':
		header = 0;
		break;
	    case 'r':
		flags |= R_FLAG;
		break;
	    case 't':
		timeout = strelapsed(opt_info.arg, &s, 1);
		if (*s)
			error(ERROR_exit(1), "%s: invalid elapsed time", opt_info.arg);
		break;
	    case 'f':
		flags |= F_FLAG;
		break;
	    case 'c':
		delim = -1;
		if (*opt_info.arg=='f' && opt_info.arg[1]==0)
		{
			flags |= F_FLAG;
			break;
		}
		/* Fall Thru */
	    case 'n':
	    case 'N':
		flags |= N_FLAG;
		s = opt_info.arg;
		number = strtol(s, &s, 10);
		if (n=='c' && *s=='f')
		{
			s++;
			flags |= F_FLAG;
		}
		if (*s)
		{
			error(2, "%c requires numeric argument", n);
			break;
		}
		if (*opt_info.arg=='+' || *opt_info.arg=='-')
			number = -number;
		if (opt_info.option[0]=='+')
			number = -number;
		break;
	    case ':':
		/* handle old style arguments */
		s = argv[opt_info.index];
		number = strtol(s, &s, 10);
		if (s!=argv[opt_info.index])
			flags |= N_FLAG;
		while (n = *s++)
		{
			switch(n)
			{
			    case 'r':
				flags |= R_FLAG;
				continue;
			    case 'f':
				flags |= F_FLAG;
				continue;
			    case 'b':
				number *= 512;
				delim = -1;
				continue;
			    case 'c':
				delim = -1;
				continue;
			    case 'k':
				number *= 1024;
				delim = -1;
				continue;
			    case 'l':
				delim = '\n';
				continue;
			    case 'm':
				number *= 1024 * 1024;
				delim = -1;
				continue;
			    default:
				error(2, "%s", opt_info.arg);
				break;
			}
			break;
		}
		if (n==0)
		{
			opt_info.offset = (s-1) - argv[opt_info.index];
			if (number==0 && !(flags&N_FLAG))
				number = (opt_info.option[0]=='-'?10:-10);
			number = -number;
		}
		break;
	    case '?':
		error(ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	argv += opt_info.index;
	argc -= opt_info.index;
	if (flags & R_FLAG)
	{
		if (delim < 0)
			error(2, "--reverse requires line mode");
		else if (!(flags & N_FLAG))
			number = 0;
		flags &= ~F_FLAG;
	}
	if (timeout && !(flags & F_FLAG))
	{
		timeout = 0;
		error(ERROR_warn(0), "--timeout ignored for --noforever");
	}
	if (error_info.errors)
		error(ERROR_usage(2), "%s", optusage(NiL));
	if (argc > 1 && (flags & F_FLAG))
	{
		/*
		 * multiple forever
		 */

		if (!(files = newof(0, Tail_t, argc, 0)))
			error(ERROR_system(1), "out of space [files]");
		ep = files;
		while (ep->name = *argv++)
		{
			if (streq(ep->name, "-"))
				ep->sp = sfstdin;
			else if (!(ep->sp = sfopen(NiL, ep->name, "r")))
			{
				error(ERROR_warn(0), "%s: cannot open", ep->name);
				continue;
			}
			if ((offset = tailpos(ep->sp, number, delim)) >= 0)
			{
				sfseek(ep->sp, offset, SEEK_SET);
				ep->size = offset;
				ep++;
			}
		}
		if (timeout)
			expire = NOW + timeout;
		hp = 0;
		for (;;)
		{
			if (sfsync(sfstdout))
				error(ERROR_system(1), "write error");
			sleep(1);
			n = 0;
			for (fp = files; fp < ep; fp++)
			{
				if (!fstat(sffileno(fp->sp), &st))
				{
					if (st.st_size > fp->last)
					{
						n = 1;
						fp->last = st.st_size;
						z = st.st_size - fp->size;
						if (s = sfreserve(fp->sp, z, 1))
						{
							r = 0;
							for (e = (t = s) + z; t < e; t++)
								if (*t == '\n')
									r = t;
							if (r)
							{
								if (hp != fp && !quiet)
								{
									hp = fp;
									sfprintf(sfstdout, format, fp->name);
									format = header_fmt;
								}
								z = r - s + 1;
								fp->size += z;
								sfwrite(sfstdout, s, z);
							}
							else
								z = 0;
							sfread(fp->sp, s, z);
						}
					}
				}
				else if (!fp->warn)
				{
					fp->warn = 1;
					error(ERROR_warn(0), "%s: cannot stat", fp->name);
				}
			}
			if (timeout)
			{
				if (n)
					expire = NOW + timeout;
				else if (expire < NOW)
					break;
			}
		}
	}
	else
	{
		if (s = *argv)
			argv++;
		do
		{
			if (!s || streq(s, "-"))
				ip = sfstdin;
			else if (!(ip = sfopen(NiL, s, "r")))
			{
				error(ERROR_system(0), "%s: cannot open", s);
				continue;
			}
			sfset(ip, SF_SHARE, 1);
			if (argc > header)
				sfprintf(sfstdout, format, s);
			format = header_fmt;
			if (number <= 0)
			{
				if ((number = -number) > 1)
					sfmove(ip, NiL, number - 1, delim);
				if (flags & R_FLAG)
					rev_line(ip, sfstdout, sfseek(ip, (Sfoff_t)0, SEEK_CUR));
				else
					sfmove(ip, sfstdout, SF_UNBOUND, -1);
			}
			else
			{
				if ((offset = tailpos(ip, number, delim)) >= 0)
				{
					if (flags & R_FLAG)
						rev_line(ip, sfstdout, offset);
					else
					{
						sfseek(ip, offset, SEEK_SET);
						sfmove(ip, sfstdout, SF_UNBOUND, -1);
					}
				}
				else
				{
					op = (flags & R_FLAG) ? sftmp(4*SF_BUFSIZE) : sfstdout;
					pipetail(ip, op, number, delim);
					if (flags & R_FLAG)
					{
						sfseek(op, (Sfoff_t)0, SEEK_SET);
						rev_line(op, sfstdout, (Sfoff_t)0);
						sfclose(op);
					}
					flags = 0;
				}
			}
			if (flags & F_FLAG)
			{
				if (timeout)
					expire = NOW + timeout;
				for (;;)
				{
					if (sfsync(sfstdout))
						error(ERROR_system(1), "write error");
					offset = sftell(ip);
					sleep(1);
					if (offset > 0 && !fstat(sffileno(ip), &st) && st.st_size < offset)
						sfseek(ip, 0, SEEK_SET);
					if ((s = sfreserve(ip, 0, 0)) && (z = sfvalue(ip)) > 0)
					{
						sfwrite(sfstdout, s, z);
						sfread(ip, s, z);
						if (timeout)
							expire = NOW + timeout;
					}
					else if (timeout && expire < NOW)
						break;
				}
			}
			if (ip != sfstdin)
				sfclose(ip);
		} while(s = *argv++);
	}
	if (timeout && !quiet)
		error(ERROR_warn(0), "%s timeout", fmtelapsed(timeout, 1));
	return error_info.errors != 0;
}
