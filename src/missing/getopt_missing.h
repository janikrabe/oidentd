/*
** getopt_missing.h - Declarations for getopt.
**
** Copyright (C) 1989-1994, 1996-1999, 2001 Free Software Foundation, Inc.
** This file is part of the GNU C Library.
**
** The GNU C Library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** The GNU C Library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with the GNU C Library; if not, write to the Free
** Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
** 02111-1307 USA.
**
** Cleanup Copyright (C) 2001-2006 Ryan McCabe <ryan@numb.org>
*/

#ifndef _GETOPT_MISSING_H
#define _GETOPT_MISSING_H

#	ifndef HAVE_GETOPT_LONG

extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;

struct option {
	const char *name;
	int has_arg;
	int *flag;
	int val;
};

/* Names for the values of the `has_arg' field of `struct option'. */

#define no_argument			0
#define required_argument	1
#define optional_argument	2

int getopt(int ___argc, char *const *___argv, const char *__shortopts);

int getopt_long(int ___argc,
				char *const *___argv,
				const char *__shortopts,
				const struct option *__longopts,
				int *__longind);

int getopt_long_only(	int ___argc,
						char *const *___argv,
						const char *__shortopts,
						const struct option *__longopts,
						int *__longind);

int _getopt_internal(	int ___argc,
						char *const *___argv,
						const char *__shortopts,
						const struct option *__longopts,
						int *__longind,
						int __long_only);
#	endif

#else
#	warning "included multiple times"
#endif
