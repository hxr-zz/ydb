/* Interactive mode Bison
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


/**
 * Implements an LALR parser. The tables are accessed directly from tables.h
 */

#define SHIFT_ACTION  		1
#define REDUCE_ACTION 		2
#define ACCEPT_ACTION 		3
#define ERROR_ACTION  		4
#define GET_TOKEN_ACTION	5
/** Must be called to initialize the parser */
void init_parser();

/** Supply a token to parser */
void next_token(int token, char *tok_text);

/** The parsing function */
int yyparse();

/** Various interfacing functions */
unsigned int get_cur_state();
unsigned int get_reduce_rule();
void print_stack();

void reset_parser();
/** Free the parser allocated memory */
void free_parser();
