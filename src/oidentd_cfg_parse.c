/* A Bison parser, made by GNU Bison 1.875a.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

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
     TOK_STRING = 269
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




/* Copy the first part of user declarations.  */
#line 1 "oidentd_cfg_parse.y"

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
#line 60 "oidentd_cfg_parse.y"
typedef union YYSTYPE {
	int value;
	char *string;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 168 "oidentd_cfg_parse.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 180 "oidentd_cfg_parse.c"

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
#define YYLAST   87

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  17
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  25
/* YYNRULES -- Number of rules. */
#define YYNRULES  45
/* YYNRULES -- Number of states. */
#define YYNSTATES  85

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   269

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
       2,     2,     2,    15,     2,    16,     2,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    10,    13,    15,    17,
      18,    24,    25,    32,    34,    37,    38,    41,    46,    51,
      56,    62,    64,    66,    69,    71,    73,    76,    79,    82,
      85,    88,    90,    93,    97,   100,   103,   105,   108,   113,
     118,   123,   129,   132,   135,   137
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      18,     0,    -1,    -1,    18,    19,    -1,    21,    -1,    -1,
      20,    39,    -1,    24,    -1,    22,    -1,    -1,     4,    23,
      15,    26,    16,    -1,    -1,     3,    14,    25,    15,    26,
      16,    -1,    27,    -1,    26,    27,    -1,    -1,    28,    29,
      -1,     4,    15,    36,    16,    -1,    31,    15,    36,    16,
      -1,    30,    15,    36,    16,    -1,    31,    30,    15,    36,
      16,    -1,    34,    -1,    35,    -1,    34,    35,    -1,    32,
      -1,    33,    -1,    32,    33,    -1,     7,    14,    -1,     8,
      14,    -1,     6,    14,    -1,     9,    14,    -1,    38,    -1,
      36,    38,    -1,    10,    11,    14,    -1,    37,    14,    -1,
      10,    13,    -1,    37,    -1,    12,    13,    -1,     5,    15,
      41,    16,    -1,    31,    15,    41,    16,    -1,    30,    15,
      41,    16,    -1,    31,    30,    15,    41,    16,    -1,    11,
      14,    -1,    40,    14,    -1,    13,    -1,    40,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,    80,    80,    83,    87,    89,    89,   104,   106,   110,
     110,   122,   122,   155,   157,   161,   161,   170,   175,   177,
     179,   183,   185,   187,   191,   193,   195,   199,   218,   235,
     254,   271,   273,   277,   285,   293,   298,   300,   311,   313,
     315,   317,   321,   328,   338,   348
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_USER", "TOK_DEFAULT", "TOK_GLOBAL", 
  "TOK_FROM", "TOK_TO", "TOK_FPORT", "TOK_LPORT", "TOK_FORCE", 
  "TOK_REPLY", "TOK_ALLOWDENY", "TOK_CAP", "TOK_STRING", "'{'", "'}'", 
  "$accept", "program", "parse_global", "@1", "user_rule", 
  "default_statement", "@2", "user_statement", "@3", "target_rule", 
  "target_statement", "@4", "range_rule", "src_rule", "dest_rule", 
  "to_statement", "fport_statement", "from_statement", "lport_statement", 
  "cap_rule", "force_reply", "cap_statement", "user_range_rule", 
  "user_reply", "user_cap_rule", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   123,   125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    17,    18,    18,    19,    20,    19,    21,    21,    23,
      22,    25,    24,    26,    26,    28,    27,    29,    29,    29,
      29,    30,    30,    30,    31,    31,    31,    32,    33,    34,
      35,    36,    36,    37,    37,    38,    38,    38,    39,    39,
      39,    39,    40,    40,    41,    41
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     2,     1,     0,     2,     1,     1,     0,
       5,     0,     6,     1,     2,     0,     2,     4,     4,     4,
       5,     1,     1,     2,     1,     1,     2,     2,     2,     2,
       2,     1,     2,     3,     2,     2,     1,     2,     4,     4,
       4,     5,     2,     2,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,     5,     1,     0,     9,     3,     0,     4,     8,     7,
      11,     0,     0,     0,     0,     0,     0,     0,     0,    24,
      25,    21,    22,     6,     0,    15,     0,    29,    27,    28,
      30,     0,     0,     0,    26,    23,    15,    15,    13,     0,
       0,    44,    45,     0,     0,     0,     0,    15,    10,    14,
       0,    16,     0,     0,    42,    43,    38,    40,    39,     0,
      12,     0,     0,     0,     0,    41,     0,     0,     0,    36,
      31,     0,     0,     0,     0,    35,    37,    17,    32,    34,
      19,    18,     0,    33,    20
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     1,     5,     6,     7,     8,    11,     9,    24,    37,
      38,    39,    51,    17,    18,    19,    20,    21,    22,    68,
      69,    70,    23,    42,    43
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -66
static const yysigned_char yypact[] =
{
     -66,    49,   -66,    -5,   -66,   -66,    39,   -66,   -66,   -66,
     -66,    -3,    11,    25,    36,    37,    48,    19,     4,    51,
     -66,    18,   -66,   -66,    46,   -66,    43,   -66,   -66,   -66,
     -66,    43,    43,    50,   -66,   -66,   -66,    52,   -66,    34,
      53,   -66,    55,    54,    56,    57,    43,    58,   -66,   -66,
      60,   -66,    61,     5,   -66,   -66,   -66,   -66,   -66,    62,
     -66,    45,    45,    45,    64,   -66,    47,    67,    12,    63,
     -66,    13,    20,    45,    68,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,    21,   -66,   -66
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,    27,
     -29,   -66,   -66,   -18,    32,   -66,    65,   -66,    66,   -58,
     -66,   -65,   -66,   -66,   -30
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      33,    44,    45,    78,    71,    72,    78,    78,    49,    10,
      13,    13,    25,    16,    16,    82,    59,    78,    49,    32,
      63,    52,    66,    66,    67,    67,    26,    16,    77,    80,
      66,    66,    67,    67,    31,    64,    81,    84,    50,    27,
      13,    14,    15,    16,    12,    13,    14,    15,    16,     2,
      28,    29,     3,     4,    40,    66,    41,    67,    74,    15,
      75,    36,    30,    47,     0,    46,     0,    54,    48,    55,
      56,    53,    57,    58,    60,    61,    62,    79,    65,    73,
      76,     0,    83,     0,    34,     0,     0,    35
};

static const yysigned_char yycheck[] =
{
      18,    31,    32,    68,    62,    63,    71,    72,    37,    14,
       6,     6,    15,     9,     9,    73,    46,    82,    47,    15,
      15,    39,    10,    10,    12,    12,    15,     9,    16,    16,
      10,    10,    12,    12,    15,    53,    16,    16,     4,    14,
       6,     7,     8,     9,     5,     6,     7,     8,     9,     0,
      14,    14,     3,     4,    11,    10,    13,    12,    11,     8,
      13,    15,    14,    36,    -1,    15,    -1,    14,    16,    14,
      16,    39,    16,    16,    16,    15,    15,    14,    16,    15,
      13,    -1,    14,    -1,    19,    -1,    -1,    21
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    18,     0,     3,     4,    19,    20,    21,    22,    24,
      14,    23,     5,     6,     7,     8,     9,    30,    31,    32,
      33,    34,    35,    39,    25,    15,    15,    14,    14,    14,
      14,    15,    15,    30,    33,    35,    15,    26,    27,    28,
      11,    13,    40,    41,    41,    41,    15,    26,    16,    27,
       4,    29,    30,    31,    14,    14,    16,    16,    16,    41,
      16,    15,    15,    15,    30,    16,    10,    12,    36,    37,
      38,    36,    36,    15,    11,    13,    13,    16,    38,    14,
      16,    16,    36,    14,    16
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
#line 89 "oidentd_cfg_parse.y"
    {
		if (parser_mode != PARSE_USER) {
			o_log(NORMAL,
				"[line %d] This construct is valid only for user configuration files", current_line);
			YYABORT;
		}

		cur_cap = xcalloc(1, sizeof(struct user_cap));
		cur_cap->caps = default_caps;
	}
    break;

  case 6:
#line 98 "oidentd_cfg_parse.y"
    {
		list_prepend(&pref_list, cur_cap);
	}
    break;

  case 9:
#line 110 "oidentd_cfg_parse.y"
    {
		if (parser_mode != PARSE_SYSTEM)
			YYABORT;

		cur_user = xmalloc(sizeof(struct user_info));
		cur_user->cap_list = NULL;

		user_db_set_default(cur_user);
	}
    break;

  case 11:
#line 122 "oidentd_cfg_parse.y"
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

  case 12:
#line 149 "oidentd_cfg_parse.y"
    {
		user_db_add(cur_user);
	}
    break;

  case 15:
#line 161 "oidentd_cfg_parse.y"
    {
		cur_cap = xcalloc(1, sizeof(struct user_cap));
		cur_cap->caps = default_caps;
	}
    break;

  case 16:
#line 164 "oidentd_cfg_parse.y"
    {
		list_prepend(&cur_user->cap_list, cur_cap);
	}
    break;

  case 17:
#line 170 "oidentd_cfg_parse.y"
    {
		if (cur_user == default_user)
			default_caps = cur_cap->caps;
	}
    break;

  case 27:
#line 199 "oidentd_cfg_parse.y"
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

  case 28:
#line 218 "oidentd_cfg_parse.y"
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

  case 29:
#line 235 "oidentd_cfg_parse.y"
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

  case 30:
#line 254 "oidentd_cfg_parse.y"
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

  case 33:
#line 277 "oidentd_cfg_parse.y"
    {
		cur_cap->caps = CAP_REPLY;
		cur_cap->action = ACTION_FORCE;
		cur_cap->force_data = xrealloc(cur_cap->force_data,
			++cur_cap->num_replies * sizeof(u_char *));
		cur_cap->force_data[cur_cap->num_replies - 1] = yyvsp[0].string;
	}
    break;

  case 34:
#line 285 "oidentd_cfg_parse.y"
    {
		cur_cap->force_data = xrealloc(cur_cap->force_data,
			++cur_cap->num_replies * sizeof(u_char *));
		cur_cap->force_data[cur_cap->num_replies - 1] = yyvsp[0].string;
	}
    break;

  case 35:
#line 293 "oidentd_cfg_parse.y"
    {
		cur_cap->caps = yyvsp[0].value;
		cur_cap->action = ACTION_FORCE;
	}
    break;

  case 37:
#line 300 "oidentd_cfg_parse.y"
    {
		if (yyvsp[-1].value == ACTION_ALLOW)
			cur_cap->caps |= yyvsp[0].value;
		else
			cur_cap->caps &= ~yyvsp[0].value;

		cur_cap->action = yyvsp[-1].value;
	}
    break;

  case 42:
#line 321 "oidentd_cfg_parse.y"
    {
		cur_cap->caps = CAP_REPLY;
		cur_cap->force_data = xrealloc(cur_cap->force_data,
			++cur_cap->num_replies * sizeof(u_char *));
		cur_cap->force_data[cur_cap->num_replies - 1] = yyvsp[0].string;
	}
    break;

  case 43:
#line 328 "oidentd_cfg_parse.y"
    {
		if (cur_cap->num_replies < MAX_RANDOM_REPLIES) {
			cur_cap->force_data = xrealloc(cur_cap->force_data,
				++cur_cap->num_replies * sizeof(u_char *));
			cur_cap->force_data[cur_cap->num_replies - 1] = yyvsp[0].string;
		}
	}
    break;

  case 44:
#line 338 "oidentd_cfg_parse.y"
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

/* Line 999 of yacc.c.  */
#line 1368 "oidentd_cfg_parse.c"

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
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
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


#line 351 "oidentd_cfg_parse.y"


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


