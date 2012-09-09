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
#include <stdlib.h>
#include <string.h>

#include "stack.h"

stack *
create_stack()
{
  stack *s = (stack *)malloc(sizeof(stack));
  INIT_LIST_HEAD(&(s->stack_head.list));
  return s;
}

/* Push these values on top of stack */
void 
stack_push(stack *s, int state, int token, char *text)
{
  stack_node *sn = (stack_node *)malloc(sizeof(stack_node));
  sn->state = state;
  sn->token = token;
  sn->text  = strdup(text);

  /* Push this stack_node on top of list */
  list_add(&(sn->list),&(s->stack_head.list));

  s->stack_top = list_entry(s->stack_head.list.next,stack_node,list);

}

stack_node *
stack_pop(stack *s)
{
  /* Delete stack top and return it */

  stack_node *retval = s->stack_top;
  list_del(&(s->stack_top->list));
  s->stack_top = list_entry(s->stack_head.list.next,stack_node,list);
  
  return retval;
}

int 
stack_peek(stack *s)
{
  return s->stack_top->state;
}

void 
stack_print(stack *s, int field)
{
  struct list_head *pos;
  list_for_each_prev(pos,&s->stack_head.list){
  stack_node *n = list_entry(pos,stack_node,list);
  switch(field){
    case PR_STATE: printf("%d ",n->state);
		   break;
    case PR_TEXT: printf("%s ", n->text);
		  break;
    }
  }
  printf("\n");
}
	
void 
free_stack(stack *s)
{
  free(s);
}
