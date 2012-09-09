%option noyywrap
%option prefix="ci_" outfile="lex.yy.c"
%{
/* Interactive mode Bison.
   Copyright (C) 2006 Satya Kiran Popuri.

   This file is part of Interactive mode Bison, an extension of GNU Bison.

   Interactive mode Bison is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Interactive mode Bison is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bison; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#include <stdio.h>
#include <readline/readline.h>

#include "ci.h"

command cur_command;

#define YY_USER_INIT					\
  do							\
    {							\
      init_command();					\
    }							\
  while (0)
%}

DIGIT	[0-9]
ID	[A-Za-z_][A-Za-z0-9_]*

PATH	[./]*[A-Za-z0-9./]*
%x CMD_ARGS
%%
q	|
exit	|
quit	{ return CMD_QUIT; }
s	|
step	{ cur_command.code = CMD_STEP; return;		 } 
n	|
next	{ cur_command.code = CMD_NEXT;  return; 	 }
rule	{ cur_command.code = CMD_RULE;  BEGIN(CMD_ARGS); }
state	{ cur_command.code = CMD_STATE; BEGIN(CMD_ARGS); }
lexer	{ cur_command.code = CMD_LEXER; BEGIN(CMD_ARGS); }
test	{ cur_command.code = CMD_TEST;	BEGIN(CMD_ARGS); }
token	{ cur_command.code = CMD_TOKEN; BEGIN(CMD_ARGS); }
stack	{ cur_command.code = CMD_STACK; return;		 }
undo	{ cur_command.code = CMD_UNDO;  return;		 }
redo	{ cur_command.code = CMD_REDO;	return;		 }
reset	{ cur_command.code = CMD_RESET; return;		 }
break	{ cur_command.code = CMD_BREAK; BEGIN(CMD_ARGS); }
help	{ cur_command.code = CMD_HELP;  return;		 } 

<CMD_ARGS>
{
	{DIGIT}+		{	if (cur_command.ctr_num_args > MAX_ARGS){
						BEGIN(INITIAL);
						return ILLEGAL_ARGS;
					}
					else
						cur_command.num_args[cur_command.ctr_num_args++] = atoi(yytext); 
				}
	{ID}			| 
	{PATH}			{	if (cur_command.ctr_str_args > MAX_ARGS){
						BEGIN(INITIAL);
						return ILLEGAL_ARGS;
					}
					else
						cur_command.str_args[cur_command.ctr_str_args++] = strdup(yytext); 
				}
	[ \t]+
	.			{ BEGIN(INITIAL); return ILLEGAL_ARGS; }
	<<EOF>>			{ BEGIN(INITIAL); }
}

.	{ return CMD_UNKNOWN; } 
%%

void init_command()
{
  int i;
  cur_command.code = 0;
  cur_command.ctr_num_args = 0;
  cur_command.ctr_str_args = 0;

  for (i=0; i<MAX_ARGS; i++)
  {
    cur_command.num_args[i]=0;
    cur_command.str_args[i]=NULL;
  }
  
}
