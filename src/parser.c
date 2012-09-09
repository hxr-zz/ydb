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

#include <config.h>
#include "system.h"
#include "gram.h"
#include "reader.h"
#include "symtab.h"
#include "tables.h"
#include "stack.h"
#include "parser.h"

/**
 * These tables have to be calculated 
 */
item_number *rhs; 
unsigned int *prhs;
unsigned int *rline;
symbol_number *r1;
unsigned int *r2;

/*
 * This is just for clear code. These tables share the same base table.
 */

int *pact;
int *pgoto;


/*
 * These define parser state 
 */
static unsigned int cur_state;
static stack  *state_stack;
static stack  *token_stack;
static unsigned int cur_token;
static char *cur_text;
static unsigned int reduce_rule;

static int empty_token=-2;

/** Local functions */
void pop_stack(int n); 
int reduce_p(rule_number r);
int default_p();

void init_parser()
{
  rule_number r;
  unsigned int i = 0;

  rhs = xnmalloc (nritems, sizeof *rhs);
  prhs = xnmalloc (nrules, sizeof *prhs);
  rline = xnmalloc (nrules, sizeof *rline);
  r1 = xnmalloc (nrules, sizeof *r1);
  r2 = xnmalloc (nrules, sizeof *r2);
  
  for (r = 0; r < nrules; ++r)
    {
      item_number *rhsp = NULL;
      /* Index of rule R in RHS. */
      prhs[r] = i;
      /* RHS of the rule R. */
      for (rhsp = rules[r].rhs; *rhsp >= 0; ++rhsp)
	rhs[i++] = *rhsp;
      /* LHS of the rule R. */
      r1[r] = rules[r].lhs->number;
      /* Length of rule R's RHS. */
      r2[r] = i - prhs[r];
      /* Separator in RHS. */
      rhs[i++] = -1;
      /* Line where rule was defined. */
      rline[r] = rules[r].location.start.line;
    }

  pact = xnmalloc(nstates,sizeof *pact);
  pgoto = xnmalloc(nvectors-nstates,sizeof *pgoto);

  for (i=0;i<nstates;i++)
	  pact[i]=base[i];

  for(i=nstates;i<nvectors;i++)
	  pgoto[i-nstates]=base[i];
  
  cur_state=0;
  cur_token=empty_token;
  reduce_rule = 0;

  state_stack = create_stack();
  token_stack = create_stack();
  stack_push(state_stack,cur_state,0,"");
}

void reset_parser()
{
  free_stack(state_stack);
  free_stack(token_stack);

  state_stack = create_stack();
  token_stack = create_stack();
  
  cur_state = 0;
  cur_token = empty_token;
  reduce_rule = 0;
  stack_push(state_stack,cur_state,0,"");
}

void next_token(int token, char *tok_text)
{
  //if(cur_token == empty_token){
    cur_token = token_translations[token];
    if(cur_text != NULL)
      free(cur_text);
    cur_text = strdup(tok_text);
  //}
}

void pop_stack(int n)
{
  int m=n;
  printf("Popping stack: ");
  for(;n>0;--n)
    free(stack_pop(state_stack));
  for(;m>0;--m){
    stack_node *m = stack_pop(token_stack);
    printf("%s\t",m->text);
    free(m);
  } 
  printf("\n\n");
}

unsigned int get_cur_state()
{
  return cur_state;
}

unsigned int get_reduce_rule()
{
  return reduce_rule;
}

/*
 * Parse one step and return action taken.
 */
int yyparse()
{

  int n = pact[cur_state];
  if(n == base_ninf)
	  return default_p();
  
  /* Check if we have a valid token */
  if (cur_token == empty_token)
	  return GET_TOKEN_ACTION;

  n += cur_token;
  if(n<0 || high < n || check[n]!=cur_token)
	  return default_p();
  
  n = table[n];
  if(n<=0)
  {
    if(n==0 || n==table_ninf)
      return error_p();
    else
      return reduce_p(-n);
  }

  if(n==final_state->number)
    return ACCEPT_ACTION;

  cur_state = n;
  stack_push(state_stack,cur_state,0,"");
  stack_push(token_stack,-1,cur_token,cur_text);

  cur_token = empty_token;
  return SHIFT_ACTION;
}

int default_p()
{
  int rule = yydefact[cur_state];
  if(rule==0)
    return error_p();
  else
    return reduce_p(rule);
}

int reduce_p(rule_number rule)
{
  reduce_rule = rule-1;

  int len = r2[rule-1];
  int lhs = r1[rule-1];

  pop_stack(len);
  
  int top_state = stack_peek(state_stack);
  cur_state = pgoto[lhs-ntokens] + top_state;
  if (0 <= cur_state && cur_state <= high && check[cur_state] == top_state)
	  cur_state = table[cur_state];
  else
	  cur_state = yydefgoto[lhs-ntokens];
  
  stack_push(state_stack,cur_state,0,"");
  stack_push(token_stack,-1,0,symbols[lhs]->tag);

  return REDUCE_ACTION;
}

int error_p()
{
  return ERROR_ACTION;
}

void print_stack()
{
  unsigned int i;
  printf("\nStacks:(states, tokens)\n");
  stack_print(state_stack,PR_STATE);
  stack_print(token_stack,PR_TEXT);
}

void free_parser()
{
	free(rhs);
	free(prhs);
	free(rline);
	free(r1);
	free(r2);

	free_stack(state_stack);
	free_stack(token_stack);
}
