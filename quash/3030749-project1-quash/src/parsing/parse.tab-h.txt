/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_SRC_PARSING_PARSE_TAB_H_INCLUDED
# define YY_YY_SRC_PARSING_PARSE_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 22 "src/parsing/parse.y"

#include <stdbool.h>

#include "command.h"
#include "parsing_interface.h"
#include "parse.tab.h"
#include "memory_pool.h"

#line 58 "src/parsing/parse.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    PIPE = 258,                    /* PIPE  */
    BCKGRND = 259,                 /* BCKGRND  */
    SQUOTE = 260,                  /* SQUOTE  */
    EQUALS = 261,                  /* EQUALS  */
    REDIRIN = 262,                 /* REDIRIN  */
    REDIROUT = 263,                /* REDIROUT  */
    REDIROUTAPP = 264,             /* REDIROUTAPP  */
    END = 265,                     /* END  */
    ECHO_TOK = 266,                /* ECHO_TOK  */
    EXPORT_TOK = 267,              /* EXPORT_TOK  */
    CD_TOK = 268,                  /* CD_TOK  */
    PWD_TOK = 269,                 /* PWD_TOK  */
    JOBS_TOK = 270,                /* JOBS_TOK  */
    KILL_TOK = 271,                /* KILL_TOK  */
    EOC_TOK = 272,                 /* EOC_TOK  */
    STR = 273,                     /* STR  */
    SIM_STR = 274,                 /* SIM_STR  */
    ID = 275,                      /* ID  */
    NUM = 276,                     /* NUM  */
    EXIT_TOK = 277                 /* EXIT_TOK  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 31 "src/parsing/parse.y"

  int integer;
  char* str;
  Command cmd;
  CommandHolder holder;
  CommandHolder* holder_arr;
  CmdStrs cmd_strs;
  Cmds cmd_list;
  Redirect redirect;

#line 108 "src/parsing/parse.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (CommandHolder** __ret_cmds);


#endif /* !YY_YY_SRC_PARSING_PARSE_TAB_H_INCLUDED  */
