/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
/*
 * the socket part of ast_intercept.h
 */

#ifndef _AST_SOCKET_H
#define _AST_SOCKET_H	1

#if _sys_socket

#include <../include/sys/socket.h>	/* the real <sys/socket.h> */

#if _AST_INTERCEPT_IMPLEMENT < 2

#undef	accept
#define accept		ast_accept

#undef	accept4
#define accept4		ast_accept4

#undef	connect
#define connect		ast_connect

#undef	socket
#define socket		ast_socket

#undef	socketpair
#define socketpair	ast_socketpair

#endif

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int		ast_accept(int, struct sockaddr*, socklen_t*);
extern int		ast_accept4(int, struct sockaddr*, socklen_t*, int);
extern int		ast_connect(int, struct sockaddr*, socklen_t);
extern int		ast_socket(int, int, int);
extern int		ast_socketpair(int, int, int, int[2]);

#undef extern

#endif

#endif
