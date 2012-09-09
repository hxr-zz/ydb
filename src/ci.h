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


/* A command interpreter for Bison interactive mode. Based on GNU readline. */

#define CMD_NEXT	1
#define CMD_TOKEN	2
#define CMD_STACK	3
#define CMD_RULE	4
#define CMD_STATE	5
#define CMD_LEXER	6
#define CMD_TEST	7
#define CMD_UNDO	8
#define CMD_REDO	9
#define CMD_RESET	10
#define CMD_BREAK	11
#define CMD_STEP	12

#define CMD_HELP	230
#define CMD_REPEAT	240
#define CMD_QUIT	250
#define CMD_UNKNOWN 	260
#define ILLEGAL_ARGS 	270

#define MAX_ARGS	5

typedef struct
{
  int code;
  int ctr_num_args;
  int ctr_str_args;
  char *str_args[20];
  int num_args[20];
} command;

extern command cur_command;

void init_command();
void process_command();

void interpret(void); /* Start the interpreter */
