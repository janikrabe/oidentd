/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_USER = 258,
     TOK_DEFAULT = 259,
     TOK_GLOBAL = 260,
     TOK_FROM = 261,
     TOK_TO = 262,
     TOK_FPORT = 263,
     TOK_LPORT = 264,
     TOK_FORCE = 265,
     TOK_REPLY = 266,
     TOK_ALLOWDENY = 267,
     TOK_CAP = 268,
     TOK_STRING = 269,
     TOK_USERNAME = 270
   };
#endif
#define TOK_USER 258
#define TOK_DEFAULT 259
#define TOK_GLOBAL 260
#define TOK_FROM 261
#define TOK_TO 262
#define TOK_FPORT 263
#define TOK_LPORT 264
#define TOK_FORCE 265
#define TOK_REPLY 266
#define TOK_ALLOWDENY 267
#define TOK_CAP 268
#define TOK_STRING 269
#define TOK_USERNAME 270




/* Copy the first part of user declarations.  */
#line 1 "./oidentd_cfg_parse.y"

/*
** oidentd_cfg_parse.y - oidentd configuration parser.
** Copyright (C) 2001-2003 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
**
*/

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <syslog.h>
#include <pwd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <oidentd.h>
#include <oidentd_util.h>
#include <oidentd_inet_util.h>
#include <oidentd_user_db.h>
#include <oidentd_options.h>

extern struct user_info *default_user;
extern u_int32_t current_line;
extern int parser_mode;

static FILE *open_user_config(const struct passwd *pw);
static int extract_port_range(const char *token, struct port_range *range);
static void free_cap_entries(struct user_cap *free_cap);
static void yyerror(const char *err);

static struct user_info *cur_user;
static struct user_cap *cur_cap;
list_t *pref_list;

u_int16_t default_caps;



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 60 "./oidentd_cfg_parse.y"
typedef union YYSTYPE {
	int value;
	char *string;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 169 "y.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 181 "y.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   85

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  18
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  26
/* YYNRULES -- Number of rules. */
#define YYNRULES  47
/* YYNRULES -- Number of states. */
#define YYNSTATES  87

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   270

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    16,     2,    17,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    10,    13,    15,    17,
      18,    24,    26,    28,    29,    36,    38,    41,    42,    45,
      50,    55,    60,    66,    68,    70,    73,    75,    77,    80,
      83,    86,    89,    92,    94,    97,   101,   104,   107,   109,
     112,   117,   122,   127,   133,   136,   139,   141
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      19,     0,    -1,    -1,    19,    20,    -1,    22,    -1,    -1,
      21,    41,    -1,    26,    -1,    23,    -1,    -1,     4,    24,
      16,    28,    17,    -1,    15,    -1,    14,    -1,    -1,     3,
      25,    27,    16,    28,    17,    -1,    29,    -1,    28,    29,
      -1,    -1,    30,    31,    -1,     4,    16,    38,    17,    -1,
      33,    16,    38,    17,    -1,    32,    16,    38,    17,    -1,
      33,    32,    16,    38,    17,    -1,    36,    -1,    37,    -1,
      36,    37,    -1,    34,    -1,    35,    -1,    34,    35,    -1,
       7,    14,    -1,     8,    14,    -1,     6,    14,    -1,     9,
      14,    -1,    40,    -1,    38,    40,    -1,    10,    11,    14,
      -1,    39,    14,    -1,    10,    13,    -1,    39,    -1,    12,
      13,    -1,     5,    16,    43,    17,    -1,    33,    16,    43,
      17,    -1,    32,    16,    43,    17,    -1,    33,    32,    16,
      43,    17,    -1,    11,    14,    -1,    42,    14,    -1,    13,
      -1,    42,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,    83,    83,    86,    90,    92,    92,   107,   109,   113,
     113,   125,   127,   131,   131,   164,   166,   170,   170,   179,
     184,   186,   188,   192,   194,   196,   200,   202,   204,   208,
     227,   244,   263,   280,   282,   286,   294,   302,   307,   309,
     320,   322,   324,   326,   330,   337,   347,   357
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_USER", "TOK_DEFAULT", "TOK_GLOBAL", 
  "TOK_FROM", "TOK_TO", "TOK_FPORT", "TOK_LPORT", "TOK_FORCE", 
  "TOK_REPLY", "TOK_ALLOWDENY", "TOK_CAP", "TOK_STRING", "TOK_USERNAME", 
  "'{'", "'}'", "$accept", "program", "parse_global", "@1", "user_rule", 
  "default_statement", "@2", "user_spec", "user_statement", "@3", 
  "target_rule", "target_statement", "@4", "range_rule", "src_rule", 
  "dest_rule", "to_statement", "fport_statement", "from_statement", 
  "lport_statement", "cap_rule", "force_reply", "cap_statement", 
  "user_range_rule", "user_reply", "user_cap_rule", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   123,   125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    18,    19,    19,    20,    21,    20,    22,    22,    24,
      23,    25,    25,    27,    26,    28,    28,    30,    29,    31,
      31,    31,    31,    32,    32,    32,    33,    33,    33,    34,
      35,    36,    37,    38,    38,    39,    39,    40,    40,    40,
      41,    41,    41,    41,    42,    42,    43,    43
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     2,     1,     0,     2,     1,     1,     0,
       5,     1,     1,     0,     6,     1,     2,     0,     2,     4,
       4,     4,     5,     1,     1,     2,     1,     1,     2,     2,
       2,     2,     2,     1,     2,     3,     2,     2,     1,     2,
       4,     4,     4,     5,     2,     2,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,     5,     1,     0,     9,     3,     0,     4,     8,     7,
      12,    11,    13,     0,     0,     0,     0,     0,     0,     0,
       0,    26,    27,    23,    24,     6,     0,    17,     0,    31,
      29,    30,    32,     0,     0,     0,    28,    25,    17,    17,
      15,     0,     0,    46,    47,     0,     0,     0,     0,    17,
      10,    16,     0,    18,     0,     0,    44,    45,    40,    42,
      41,     0,    14,     0,     0,     0,     0,    43,     0,     0,
       0,    38,    33,     0,     0,     0,     0,    37,    39,    19,
      34,    36,    21,    20,     0,    35,    22
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     1,     5,     6,     7,     8,    13,    12,     9,    26,
      39,    40,    41,    53,    19,    20,    21,    22,    23,    24,
      70,    71,    72,    25,    44,    45
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -68
static const yysigned_char yypact[] =
{
     -68,    49,   -68,    -1,   -68,   -68,    39,   -68,   -68,   -68,
     -68,   -68,   -68,    11,    15,    23,    36,    37,    48,    20,
       2,    51,   -68,    52,   -68,   -68,    50,   -68,    43,   -68,
     -68,   -68,   -68,    43,    43,    53,   -68,   -68,   -68,    46,
     -68,    34,    54,   -68,    56,    55,    57,    58,    43,    59,
     -68,   -68,    61,   -68,    62,     3,   -68,   -68,   -68,   -68,
     -68,    63,   -68,    45,    45,    45,    65,   -68,    47,    60,
      12,    68,   -68,    13,    16,    45,    69,   -68,   -68,   -68,
     -68,   -68,   -68,   -68,    22,   -68,   -68
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -68,   -68,   -68,   -68,   -68,   -68,   -68,   -68,   -68,   -68,
      26,   -29,   -68,   -68,   -20,    24,   -68,    64,   -68,    44,
     -60,   -68,   -67,   -68,   -68,   -32
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      35,    46,    47,    80,    73,    74,    80,    80,    15,    15,
      51,    18,    18,    10,    11,    84,    61,    80,    34,    65,
      51,    54,    68,    68,    69,    69,    68,    27,    69,    79,
      82,    28,    68,    83,    69,    66,    33,    29,    52,    86,
      15,    16,    17,    18,    14,    15,    16,    17,    18,     2,
      30,    31,     3,     4,    42,    68,    43,    69,    76,    17,
      77,    18,    32,    50,    49,    55,    38,    37,    56,    48,
      57,     0,    58,    78,    59,    60,    62,    63,    64,     0,
      67,    75,    81,    85,     0,    36
};

static const yysigned_char yycheck[] =
{
      20,    33,    34,    70,    64,    65,    73,    74,     6,     6,
      39,     9,     9,    14,    15,    75,    48,    84,    16,    16,
      49,    41,    10,    10,    12,    12,    10,    16,    12,    17,
      17,    16,    10,    17,    12,    55,    16,    14,     4,    17,
       6,     7,     8,     9,     5,     6,     7,     8,     9,     0,
      14,    14,     3,     4,    11,    10,    13,    12,    11,     8,
      13,     9,    14,    17,    38,    41,    16,    23,    14,    16,
      14,    -1,    17,    13,    17,    17,    17,    16,    16,    -1,
      17,    16,    14,    14,    -1,    21
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    19,     0,     3,     4,    20,    21,    22,    23,    26,
      14,    15,    25,    24,     5,     6,     7,     8,     9,    32,
      33,    34,    35,    36,    37,    41,    27,    16,    16,    14,
      14,    14,    14,    16,    16,    32,    35,    37,    16,    28,
      29,    30,    11,    13,    42,    43,    43,    43,    16,    28,
      17,    29,     4,    31,    32,    33,    14,    14,    17,    17,
      17,    43,    17,    16,    16,    16,    32,    17,    10,    12,
      38,    39,    40,    38,    38,    16,    11,    13,    13,    17,
      40,    14,    17,    17,    38,    14,    17
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:
#line 92 "./oidentd_cfg_parse.y"
    {
		if (parser_mode != PARSE_USER) {
			o_log(NORMAL, "%s",
				"This construct is valid only for user configuration files");
			YYABORT;
		}

		cur_cap = xcalloc(1, sizeof(struct user_cap));
		cur_cap->caps = default_caps;
	}
    break;

  case 6:
#line 101 "./oidentd_cfg_parse.y"
    {
		list_prepend(&pref_list, cur_cap);
	}
    break;

  case 9:
#line 113 "./oidentd_cfg_parse.y"
    {
		if (parser_mode != PARSE_SYSTEM)
			YYABORT;

		cur_user = xmalloc(sizeof(struct user_info));
		cur_user->cap_list = NULL;

		user_db_set_default(cur_user);
	}
    break;

  case 13:
#line 131 "./oidentd_cfg_parse.y"
    {
		if (parser_mode != PARSE_SYSTEM) {
			free(yyvsp[0].string);
			YYABORT;
		}

		cur_user = xmalloc(sizeof(struct user_info));
		cur_user->cap_list = NULL;

		if (find_user(yyvsp[0].string, &cur_user->user) == -1) {
			o_log(NORMAL, "[line %u] Invalid user: \"%s\"", current_line, yyvsp[0].string);
			free(yyvsp[0].string);
			free_cap_entries(cur_cap);
			YYABORT;
		}

		if (user_db_lookup(cur_user->user) != NULL) {
			o_log(NORMAL,
				"[line %u] User \"%s\" already has a capability entry",
				current_line, yyvsp[0].string);
			free(yyvsp[0].string);
			free_cap_entries(cur_cap);
			YYABORT;
		}

		free(yyvsp[0].string);
	}
    break;

  case 14:
#line 158 "./oidentd_cfg_parse.y"
    {
		user_db_add(cur_user);
	}
    break;

  case 17:
#line 170 "./oidentd_cfg_parse.y"
    {
		cur_cap = xcalloc(1, sizeof(struct user_cap));
		cur_cap->caps = default_caps;
	}
    break;

  case 18:
#line 173 "./oidentd_cfg_parse.y"
    {
		list_prepend(&cur_user->cap_list, cur_cap);
	}
    break;

  case 19:
#line 179 "./oidentd_cfg_parse.y"
    {
		if (cur_user == default_user)
			default_caps = cur_cap->caps;
	}
    break;

  case 29:
#line 208 "./oidentd_cfg_parse.y"
    {
		cur_cap->dest = xmalloc(sizeof(struct sockaddr_storage));

		if (get_addr(yyvsp[0].string, cur_cap->dest) == -1) {
			if (parser_mode == PARSE_SYSTEM) {
				o_log(NORMAL, "[line %u]: Bad address: \"%s\"",
					current_line, yyvsp[0].string);
			}

			free(yyvsp[0].string);
			free_cap_entries(cur_cap);
			YYABORT;
		}

		free(yyvsp[0].string);
	}
    break;

  case 30:
#line 227 "./oidentd_cfg_parse.y"
    {
		cur_cap->fport = xmalloc(sizeof(struct port_range));

		if (extract_port_range(yyvsp[0].string, cur_cap->fport) == -1) {
			if (parser_mode == PARSE_SYSTEM)
				o_log(NORMAL, "[line %u] Bad port: \"%s\"", current_line, yyvsp[0].string);

			free(yyvsp[0].string);
			free_cap_entries(cur_cap);
			YYABORT;
		}

		free(yyvsp[0].string);
	}
    break;

  case 31:
#line 244 "./oidentd_cfg_parse.y"
    {
		cur_cap->src = xmalloc(sizeof(struct sockaddr_storage));

		if (get_addr(yyvsp[0].string, cur_cap->src) == -1) {
			if (parser_mode == PARSE_SYSTEM) {
				o_log(NORMAL, "[line %u]: Bad address: \"%s\"",
					current_line, yyvsp[0].string);
			}

			free(yyvsp[0].string);
			free_cap_entries(cur_cap);
			YYABORT;
		}

		free(yyvsp[0].string);
	}
    break;

  case 32:
#line 263 "./oidentd_cfg_parse.y"
    {
		cur_cap->lport = xmalloc(sizeof(struct port_range));

		if (extract_port_range(yyvsp[0].string, cur_cap->lport) == -1) {
			if (parser_mode == PARSE_SYSTEM)
				o_log(NORMAL, "[line %u] Bad port: \"%s\"", current_line, yyvsp[0].string);

			free(yyvsp[0].string);
			free_cap_entries(cur_cap);
			YYABORT;
		}

		free(yyvsp[0].string);
	}
    break;

  case 35:
#line 286 "./oidentd_cfg_parse.y"
    {
		cur_cap->caps = CAP_REPLY;
		cur_cap->action = ACTION_FORCE;
		cur_cap->force_data = xrealloc(cur_cap->force_data,
			++cur_cap->num_replies * sizeof(u_char *));
		cur_cap->force_data[cur_cap->num_replies - 1] = yyvsp[0].string;
	}
    break;

  case 36:
#line 294 "./oidentd_cfg_parse.y"
    {
		cur_cap->force_data = xrealloc(cur_cap->force_data,
			++cur_cap->num_replies * sizeof(u_char *));
		cur_cap->force_data[cur_cap->num_replies - 1] = yyvsp[0].string;
	}
    break;

  case 37:
#line 302 "./oidentd_cfg_parse.y"
    {
		cur_cap->caps = yyvsp[0].value;
		cur_cap->action = ACTION_FORCE;
	}
    break;

  case 39:
#line 309 "./oidentd_cfg_parse.y"
    {
		if (yyvsp[-1].value == ACTION_ALLOW)
			cur_cap->caps |= yyvsp[0].value;
		else
			cur_cap->caps &= ~yyvsp[0].value;

		cur_cap->action = yyvsp[-1].value;
	}
    break;

  case 44:
#line 330 "./oidentd_cfg_parse.y"
    {
		cur_cap->caps = CAP_REPLY;
		cur_cap->force_data = xrealloc(cur_cap->force_data,
			++cur_cap->num_replies * sizeof(u_char *));
		cur_cap->force_data[cur_cap->num_replies - 1] = yyvsp[0].string;
	}
    break;

  case 45:
#line 337 "./oidentd_cfg_parse.y"
    {
		if (cur_cap->num_replies < MAX_RANDOM_REPLIES) {
			cur_cap->force_data = xrealloc(cur_cap->force_data,
				++cur_cap->num_replies * sizeof(u_char *));
			cur_cap->force_data[cur_cap->num_replies - 1] = yyvsp[0].string;
		}
	}
    break;

  case 46:
#line 347 "./oidentd_cfg_parse.y"
    {
		if (yyvsp[0].value == CAP_SPOOF || yyvsp[0].value == CAP_SPOOF_ALL || yyvsp[0].value == CAP_SPOOF_PRIVPORT)
		{
			free_cap_entries(cur_cap);
			YYABORT;
		}

		cur_cap->caps = yyvsp[0].value;
	}
    break;


    }

/* Line 991 of yacc.c.  */
#line 1370 "y.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__)
  __attribute__ ((__unused__))
#endif


  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 360 "./oidentd_cfg_parse.y"


/*
** Read in the system-wide configuration file.
*/

int read_config(const char *path) {
	FILE *fp;
	int ret;

	fp = fopen(path, "r");
	if (fp == NULL) {
		if (errno == ENOENT) {
			/*
			** If a configuration file is specified on the
			** command line, return an error if it can't be opened,
			** even if it doesn't exist.
			*/

			if (!strcmp(path, CONFFILE)) {
				struct user_info *temp_default;

				temp_default = user_db_create_default();
				user_db_set_default(temp_default);
				return (0);
			}
		}

		o_log(NORMAL, "Error opening config file: %s: %s",
			path, strerror(errno));
		return (-1);
	}

	yyrestart(fp);
	current_line = 1;
	parser_mode = PARSE_SYSTEM;
	ret = yyparse();

	fclose(fp);

	/*
	** Make sure there's a default to fall back on.
	*/

	if (default_user == NULL) {
		struct user_info *temp_default;

		temp_default = user_db_create_default();
		user_db_set_default(temp_default);
	}

	return (ret);
}

/*
** Open the user's ~/.oidentd_config file for reading
** by the parser.
*/

static FILE *open_user_config(const struct passwd *pw) {
	FILE *fp;

	fp = safe_open(pw, USER_CONF);
	if (fp == NULL)
		return (NULL);

	yyrestart(fp);
	current_line = 1;
	parser_mode = PARSE_USER;

	return (fp);
}

/*
** Read in a user's $HOME/.oidentd.conf file.
*/

list_t *user_db_get_pref_list(const struct passwd *pw) {
	FILE *fp;
	int ret;

	fp = open_user_config(pw);
	if (fp == NULL)
		return (NULL);

	cur_cap = NULL;
	pref_list = NULL;

	ret = yyparse();
	fclose(fp);

	if (ret != 0) {
		list_destroy(pref_list, user_db_cap_destroy_data);
		return (NULL);
	}

	return (pref_list);
}

static void yyerror(const char *err) {

	if (parser_mode == PARSE_USER)
		free_cap_entries(cur_cap);
	else
		o_log(NORMAL, "[line %u] %s", current_line, err);
}

/*
** Extract a port range from a token.
*/

static int extract_port_range(const char *token, struct port_range *range) {
	char *p;

	p = strchr(token, ':');
	if (p != NULL)
		*p++ = '\0';

	if (*token == '\0')
		range->min = PORT_MIN;
	else if (get_port(token, &range->min) == -1)
		return (-1);

	if (p == NULL)
		range->max = range->min;
	else {
		if (*p == '\0')
			range->max = PORT_MAX;
		else if (get_port(p, &range->max) == -1)
			return (-1);
	}

	return (0);
}

static void free_cap_entries(struct user_cap *free_cap) {
	user_db_cap_destroy_data(free_cap);

	if (free_cap != cur_cap)
		free(cur_cap);

	free(free_cap);
	free(cur_user);

	cur_cap = NULL;
	cur_user = NULL;
	pref_list = NULL;
}

