/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1992-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * tail [-frqv] [-c number] [-n number] [file ...]
 *
 * print the tail of one or more files
 *
 *   David Korn
 */

static const char usage[] =
"[-?\n@(#)tail (AT&T Labs Research) 1999-04-10\n]"
USAGE_LICENSE
"[+NAME?tail - output trailing portion of one or more files ]"
"[+DESCRIPTION?\btail\b copies one or more input files to standard output "
	"starting at a designated point for each file.  Copying starts "
	"at the point indicated by the options and is unlimited in size.]"
"[+?By default a header of the form \b==> \b\afilename\a\b <==\b "
	"is output before all but the first file but this can be changed "
	"with the \b-q\b and \b-v\b options.]"
"[+?If no \afile\a is given, or if the \afile\a is \b-\b, \btail\b "
	"copies from standard input.   The start of the file is defined "
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
"[f:follow?Loop forever trying to read more characters as the "
	"end of the file to copy new data.  Only a single file can be "
	"specified.  Ignored if reading from a pipe or fifo.]"
"[q:quiet|silent?Never ouput filename headers.]"
"[r:reverse?Output lines in reverse order.]"
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
#define LINE_AVE	128

/*
 * If file is seekable, position file to tail location and return offset
 * Otherwise, return -1
 */
static Sfoff_t tailpos(register Sfio_t *fp, register long nitems, int delim)
{
	register int		nleft;
	register int		n;
	register Sfoff_t	offset;
	register Sfoff_t	first;
	register Sfoff_t	last;

	if ((first=sfseek(fp, (Sfoff_t)0, SEEK_CUR)) < 0)
		return (Sfoff_t)-1;
	last = sfsize(fp);
	if (delim < 0)
	{
		if ((offset = last - nitems) < first)
			return first;
		return offset;
	}
	nleft = nitems;
	if ((offset = last - nitems * LINE_AVE) < first)
		offset = first;
	while (offset >= first)
	{
		sfseek(fp, offset, SEEK_SET);
		n = sfmove(fp, NiL, SF_UNBOUND, delim);
		if (n > nitems)
		{
			sfseek(fp, offset, SEEK_SET);
			sfmove(fp, NiL, n-nitems, delim); 
			return sftell(fp);
		}
		if (n == 0)
			offset -=  SF_BUFSIZE;
		else
		{
			nleft = nitems - n;
			n = 1 + (last - offset) / (nitems - nleft);
			offset -= (nleft + 1) * n;
		}
	}
	return first;
}

/*
 * This code handles tail from a pipe without any size limits
 */
static void pipetail(Sfio_t *infile, Sfio_t *outfile, int nitems, int delim)
{
	register Sfio_t *out;
	register int n=(2*SF_BUFSIZE), nleft=nitems, fno=0;
	Sfoff_t offset[2];
	Sfio_t *tmp[2];
	if (delim<0 && nitems < n)
		n = nitems;
	out = tmp[0] = sftmp(n);
	tmp[1] = sftmp(n);
	offset[0] = offset[1] = 0;
	while ((n=sfmove(infile, out, nleft, delim)) >0)
	{
		offset[fno] = sftell(out);
		if ((nleft-=n) <=0)
		{
			out = tmp[fno= !fno];
			sfseek(out, (Sfoff_t)0, SEEK_SET);
			nleft = nitems;
		}
	}
	if (nleft==nitems)
	{
		offset[fno]=0;
		fno= !fno;
	}
	sfseek(tmp[0], (Sfoff_t)0, SEEK_SET);
	/* see whether both files are needed */
	if (offset[fno])
	{
		sfseek(tmp[1], (Sfoff_t)0, SEEK_SET);
		if ((n=nitems-nleft)>0) 
			sfmove(tmp[!fno], NiL, n, delim); 
		if ((n=offset[!fno]-sftell(tmp[!fno])) > 0)
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
	static const char header_fmt[] = "\n==> %s <==\n";
	register Sfio_t*	fp;
	register int		n;
	register int		delim = '\n';
	register int		flags = 0;
	char*			cp;
	Sfoff_t			offset;
	long			number = 10;
	struct stat		st;
	int			header = 1;
	const char		*format = header_fmt+1;

	cmdinit(argv, context);
	while (n = optget(argv, usage)) switch (n)
	{
	    case 'q':
		header = argc;
		break;
	    case 'v':
		header = 0;
		break;
	    case 'r':
		flags |= R_FLAG;
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
		flags |= N_FLAG;
		cp = opt_info.arg;
		number = strtol(cp, &cp, 10);
		if (n=='c' && *cp=='f')
		{
			cp++;
			flags |= F_FLAG;
		}
		if (*cp)
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
		cp = argv[opt_info.index];
		number = strtol(cp, &cp, 10);
		if (cp!=argv[opt_info.index])
			flags |= N_FLAG;
		while (n = *cp++)
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
			opt_info.offset = (cp-1) - argv[opt_info.index];
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
			error(2, "r option requires line mode");
		else if (!(flags & N_FLAG))
			number = 0;
		flags &= ~F_FLAG;
	}
	if (error_info.errors)
		error(ERROR_usage(2), "%s", optusage(NiL));
	if(cp = *argv)
		argv++;
	do
	{
		if(!cp || streq(cp,"-"))
		{
			fp = sfstdin;
			sfset(fp, SF_SHARE, 1);
		}
		else if (!(fp = sfopen(NiL, cp, "r")))
		{
			error(ERROR_system(3), "%s: cannot open", cp);
			error_info.errors = 1;
			continue;
		}
		if(argc>header)
			sfprintf(sfstdout,format,cp);
		format = header_fmt;
		if (number<=0)
		{
			if ((number = -number) > 1)
				sfmove(fp, NiL, number - 1, delim);
			if (flags&R_FLAG)
				rev_line(fp, sfstdout, sfseek(fp, (Sfoff_t)0, SEEK_CUR));
			else
				sfmove(fp, sfstdout, SF_UNBOUND, -1);
		}
		else
		{
			if ((offset=tailpos(fp, number, delim))>=0)
			{
				if (flags&R_FLAG)
					rev_line(fp, sfstdout, offset);
				else
				{
					sfseek(fp, offset, SEEK_SET);
					sfmove(fp, sfstdout, SF_UNBOUND, -1);
				}
			}
			else
			{
				Sfio_t *out = sfstdout;
				if (flags&R_FLAG)
					out = sftmp(4*SF_BUFSIZE);
				pipetail(fp, out, number, delim);
				if (flags&R_FLAG)
				{
					sfseek(out, (Sfoff_t)0, SEEK_SET);
					rev_line(out, sfstdout, (Sfoff_t)0);
					sfclose(out);
				}
				flags = 0;
			}
		}
		if (flags&F_FLAG)
		{
			register char *bufp;
			while (1)
			{
				offset = sftell(fp);
				sfsync(sfstdout);
				sleep(1);
				if (offset > 0 && !fstat(sffileno(fp), &st) && st.st_size < offset)
					sfseek(fp, 0, SEEK_SET);
				if ((bufp = sfreserve(fp, 0, 0))&&(n=sfvalue(fp))>0)
				{
					sfwrite(sfstdout, bufp, n);
					sfread(fp, bufp, n);
				}
			}
		}
		if (fp != sfstdin)
			sfclose(fp);
	}
	while(cp= *argv++);
	return 0;
}

