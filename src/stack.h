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


/* A linked list implementation of a stack for Bison intereactive mode
 */

#define PR_STATE 1
#define PR_TOKEN 2
#define PR_TEXT  3

#include "list.h"
typedef struct
{
  int state;
  int token;
  char *text;

  struct list_head list;
  
} stack_node;

typedef struct
{
  stack_node *stack_top;
  stack_node stack_head; /* The head of the stack list */
} stack;


stack *create_stack();
void stack_push(stack *s,int state, int token, char *text);
stack_node *stack_pop();
int stack_peek(stack *s);
void stack_print(stack *s, int field);
void free_stack(stack *s);
