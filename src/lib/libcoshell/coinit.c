/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2003 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * return job initialization commands
 */

#include "colib.h"

#include <fs3d.h>
#include <ls.h>

/*
 * add n to the export list
 * old!=0 formats in old style
 * if n prefixed by % then coquote conversion enabled
 */

static void
putexport(Sfio_t* sp, char* n, int old)
{
	int	cvt;
	char*	v;

	if (cvt = *n == '%') n++;
	if (old) cvt = 0;
	if ((v = getenv(n)) && *v)
	{
		if (!old) sfprintf(sp, "\\\n");
		sfprintf(sp, " %s='", n);
		coquote(sp, v, cvt);
		sfputc(sp, '\'');
		if (old) sfprintf(sp, "\nexport %s\n", n);
	}
}

/*
 * return job initialization commands
 */

char*
coinit(int flags)
{
	register char*	s;
	int		n;
	int		m;
	int		old;
	int		sync;
	char*		t;
	long		p;
	Sfio_t*		sp;
	Sfio_t*		tp;
	struct stat	st;

	static char*	init;
	static int	mask;

	static dev_t	pwd_dev;
	static ino_t	pwd_ino;

	sync = 0;

	/*
	 * pwd
	 */

	if (stat(".", &st)) return(0);
	if (!state.pwd || st.st_ino != pwd_ino || st.st_dev != pwd_dev)
	{
		pwd_dev = st.st_dev;
		pwd_ino = st.st_ino;
		if (state.pwd) free(state.pwd);
		if (!(state.pwd = getcwd(NiL, 0)))
		{
			if (errno != EINVAL || !(state.pwd = newof(0, char, PATH_MAX, 0)))
				return(0);
			if (!getcwd(state.pwd, PATH_MAX))
			{
				free(state.pwd);
				state.pwd = 0;
				return(0);
			}
		}
		if (!(flags & CO_INIT)) sync = 1;
	}

	/*
	 * umask
	 */

	umask(n = umask(mask));
	if (mask != n)
	{
		mask = n;
		if (!(flags & CO_INIT)) sync = 1;
	}
	if (!init || sync)
	{
		/*
		 * coexport[] vars
		 */

		if (!(sp = sfstropen())) return(0);
		old = !(flags & (CO_KSH|CO_SERVER));
		if (!old) sfprintf(sp, "export");
		if (sync)
		{
			if (flags & CO_EXPORT)
				s = "(*)";
			else
			{
				for (n = 0; s = coexport[n]; n++)
					putexport(sp, s, old);
				s = getenv(coexport[0]);
			}
			if (s)
			{
				if (*s == '(')
				{
					register char**	ep = environ;
					register char*	e;
					char*		v;
					char*		es;
					char*		xs;

					if (v = strchr(s, ':')) *v = 0;
					while (e = *ep++)
						if ((t = strsubmatch(e, s, 1)) && (*t == '=' || !*t && (t = strchr(e, '='))))
						{
							m = t - e;
							if (!strneq(e, "PATH=", 5) && !strneq(e, "_=", 2))
							{
								for (n = 0; xs = coexport[n]; n++)
								{
									es = e;
									while (*xs && *es == *xs)
									{
										es++;
										xs++;
									}
									if (*es == '=' && !*xs) break;
								}
								if (!xs)
								{
									if (!old) sfprintf(sp, "\\\n");
									sfprintf(sp, " %-.*s='", m, e);
									coquote(sp, e + m + 1, 0);
									sfputc(sp, '\'');
									if (old) sfprintf(sp, "\nexport %-.*s\n", m, e);
								}
							}
						}
					if (v)
					{
						*v++ = ':';
						s = v;
					}
				}
				if (*s) for (;;)
				{
					if (t = strchr(s, ':')) *t = 0;
					putexport(sp, s, old);
					if (!(s = t)) break;
					*s++ = ':';
				}
			}
		}

		/*
		 * PATH
		 */

		sfprintf(sp, " PATH='");
		n = PATH_MAX;
		if (!(t = sfstrrsrv(sp, n)))
		{
		bad:
			sfclose(sp);
			return(0);
		}
		t += n / 2;
		if (!(flags & CO_CROSS) && !pathpath(t, "ignore", NiL, PATH_ABSOLUTE|PATH_REGULAR|PATH_EXECUTE) && pathpath(t, "bin/ignore", "", PATH_ABSOLUTE|PATH_REGULAR|PATH_EXECUTE))
		{
			*strrchr(t, '/') = 0;
			sfputc(sp, ':');
			coquote(sp, t, !old);
			sfputc(sp, ':');
			s = pathbin();
		}
		else
		{
			s = pathbin();
			if (!sync && (*s == ':' || *s == '.' && *(s + 1) == ':'))
			{
				sfstrset(sp, 0);
				goto done;
			}
			if (!(flags & CO_CROSS)) sfputc(sp, ':');
		}
		if (*s == ':') s++;
		else if (*s == '.' && *(s + 1) == ':') s += 2;
		if (!(flags & CO_CROSS))
			tp = 0;
		else if (tp = sfstropen())
		{
			while (n = *s++)
			{
				if (n == ':')
				{
					while (*s == ':')
						s++;
					if (!*s)
						break;
					if (*s == '.')
					{
						if (!*(s + 1))
							break;
						if (*(s + 1) == ':')
						{
							s++;
							continue;
						}
					}
				}
				sfputc(tp, n);
			}
			s = sfstruse(tp);
		}
		coquote(sp, s, !old);
		if (tp)
			sfstrclose(tp);
		sfputc(sp, '\'');
		if (old) sfprintf(sp, "\nexport PATH");
		sfputc(sp, '\n');
		if (sync)
		{
			/*
			 * VPATH
			 */

			p = sfstrtell(sp);
			sfprintf(sp, "vpath ");
			n = PATH_MAX;
			if (fs3d(FS3D_TEST)) for (;;)
			{
				if (!(t = sfstrrsrv(sp, n))) goto bad;
				if ((m = mount(NiL, t, FS3D_GET|FS3D_ALL|FS3D_SIZE(n), NiL)) > 0) m = n;
				else
				{
					if (!m) sfstrrel(sp, strlen(t));
					break;
				}
			}
			else
			{
				m = 0;
				sfprintf(sp, "- /#option/2d");
			}
			if (m) sfstrset(sp, p);
			else sfprintf(sp, " 2>/dev/null || :\n");
			sfprintf(sp, "umask 0%o\ncd '%s'\n", mask, state.pwd);
		}
	done:
		if (!(flags & CO_SERVER))
		{
			sfprintf(sp, "%s%s=%05d${!%s-$$}\n", old ? "" : "export ", CO_ENV_TEMP, getpid(), (flags & CO_OSH) ? "" : ":");
			if (old) sfprintf(sp, "export %s\n", CO_ENV_TEMP);
		}
		sfputc(sp, 0);
		n = sfstrtell(sp);
		if (init) free(init);
		if (!(init = newof(0, char, n, 1))) goto bad;
		memcpy(init, sfstrbase(sp), n);
		sfstrclose(sp);
	}
	else if (!init && (init = newof(0, char, 1, 0))) *init = 0;
	return(init);
}
