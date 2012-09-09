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



/*
 * A Command interpreter for Bison interactive mode.
 */

#include <config.h>
#include <bitset.h>
#include "system.h"

#include <readline/readline.h>

#include "LR0.h"
#include "complain.h"
#include "conflicts.h"
#include "derives.h"
#include "files.h"
#include "getargs.h"
#include "gram.h"
#include "lalr.h"
#include "muscle_tab.h"
#include "nullable.h"
#include "output.h"
#include "print.h"
#include "print_graph.h"
#include "reader.h"
#include "reduce.h"
#include "symtab.h"
#include "tables.h"
#include "uniqstr.h"

#include "parser.h"
#include "ci.h"
#include "loadlexer.h"
#include "interactive.h"
#include "signal.h"

static bool lexer_loaded  = false;
static bool yin_set 	  = false;
static char *test_file    = NULL; /* The test input file */
static char *break_token  = NULL;
static unsigned int LAST_COMMAND = -1;
static command last_command;

static bool break_set 	  = false; /* If breakpoint is set */

bool parse_error = false, parse_accept = false; /* Error in parse or parse accepted */

static bitset shift_set;
static bitset look_ahead_set;


void process_command();
void end_interactive();
void print_info();
void help_info();
void state_print(FILE *, state *);
void core_print(FILE*, state *);
/*
 * This is a little command line interpreter routine
 */
void begin_interactive()
{
  char *cmdline;
  int result;
 
  /* For states output */
  signal(SIGINT,SIG_IGN);
  shift_set =  bitset_create (ntokens, BITSET_FIXED);
  look_ahead_set = bitset_create (ntokens, BITSET_FIXED);
      
  init_parser();

  print_info();

  while(true){
    if (parse_error || parse_accept)
      break;
    
    if(!break_set){
      init_command();
      cmdline = readline("\n(interactive) ");

      /* Break the command into keyword and arguments */

      printf("\n");
      if ( cmdline == NULL){
  	    //printf("\n");
        break;
      }

      ci__scan_string(cmdline);
      result = ci_lex();  

      if(result == CMD_UNKNOWN){
        fprintf(stderr,"Unknown Command\n");
        continue;
      }

      if(result == ILLEGAL_ARGS){
        fprintf(stderr,"Illegal arguments\n");
        continue;
      }

      if(result == CMD_QUIT)
        break;

      if(strlen(cmdline) > 0){
        LAST_COMMAND = cur_command.code;
        last_command = cur_command;
 
        add_history(cmdline);
      }
      else
      {
        if(LAST_COMMAND > 0)
          cur_command = last_command;
      }
   } /* end if !break_set */
   else
     cur_command.code = CMD_NEXT;
   
    process_command();
  } /* end while true */

  end_interactive();
}

void process_command()
{

  /** DEBUGGING ONLY
  int i;
  printf("Command code: %d\n", cur_command.code);

  for(i=0; i< cur_command.ctr_num_args; ++i)
  	printf("Command num_arg[%d]: %d\n", i, cur_command.num_args[i]);

  for (i=0; i < cur_command.ctr_str_args; ++i)
  	printf("Command str_arg[%d]: %s\n", i, cur_command.str_args[i]);

  */
  int action=0, result=0, nxt_tok=0;
  
  switch(cur_command.code)
  {
	case CMD_NEXT:
		action = yyparse();
		switch(action)
		{
			case GET_TOKEN_ACTION:
					    if(lexer_loaded && yin_set){
					      nxt_tok = call_lex();
					      printf("Reading a token...next token is: %s (%s)\n",token_text, symbols[token_translations[nxt_tok]]->tag);
					      next_token(nxt_tok,token_text);
					    }
					    else
					      printf("Parser needs a token to proceed. use token command.\n");

					    if(break_set){
					      if(strcmp(break_token,token_text) == 0)
						      break_set = false;
					    }
					    break;
					    
			case SHIFT_ACTION:  printf("Token is shifted. Entering state %d\n", get_cur_state());
					    print_stack();
					    break;
			case REDUCE_ACTION: printf("Reduced by rule #%d\n", get_reduce_rule());
					    rule_print(&rules[get_reduce_rule()],stdout);
					    print_stack();
					    break;
			case ERROR_ACTION:  printf("Parser error!\n");
					    parse_error = true;
					    break;
			case ACCEPT_ACTION: printf("String is accepted.\n");
					    parse_accept = true;
					    break;
		}
		break;
	case CMD_TOKEN: 
		if(cur_command.ctr_str_args > 0 ){
		  next_token(find_token(cur_command.str_args[0]),"");
		  printf("%s will be parsed next.\n", cur_command.str_args[0]);
		}
		else
		  fprintf(stderr,"You must supply a token number.\n");
		break;
	case CMD_STACK: 
		print_stack ();
		break;
	case CMD_RULE:
		if(cur_command.ctr_num_args >0 && cur_command.num_args[0] < nrules)
		  rule_print(&rules[cur_command.num_args[0]],stdout);
		else
		  fprintf(stderr,"Please specifiy a rule number (0 - %d)\n",nrules-1);

		printf("\n");
		break;
	case CMD_STATE:
		if(cur_command.ctr_num_args > 0 && cur_command.num_args[0] < nstates)
		  state_print(stdout,states[cur_command.num_args[0]]);
		else
		  fprintf(stderr,"Please supply a state number (0 - %d)\n", nstates-1);
		printf("\n");
		break;
	case CMD_LEXER:
		/* Load the lexer */
		if(lexer_loaded){
		  fprintf(stderr,"Lexer is already loaded.\n");
		  break;
		}
		if(cur_command.ctr_str_args > 0){
		  result = load_lexer(cur_command.str_args[0]);
		  if(result > 0){
		    lexer_loaded = true;
		    printf("Lexer loaded successfully.\n");
		  }
		}
		else
		  fprintf(stderr,"Please supply a valid lexer module\n");
		break;
	case CMD_TEST:
		if(!lexer_loaded){
		  fprintf(stderr,"Load a lexer first. Otherwise supply tokens manually.\n");
		  break;
		}
		if(yin_set){
		  fprintf(stderr,"Test file %s was already opened...ready to test.\n",test_file);
		  break;
		}
		if(cur_command.ctr_str_args > 0){
		  if(test_file)
		    free(test_file);
		  test_file = strdup(cur_command.str_args[0]);
		  result = set_lex_in(test_file);
		  if(result > 0){
		    yin_set = true;
		    fprintf(stdout,"Opening file...ready to test.\n");
		  }
		}
		else
		  fprintf(stderr, "You must supply tokens manually.");
		break;
	case CMD_UNDO:	
		break;
	case CMD_REDO:  
		break;

	case CMD_BREAK:
		if(cur_command.ctr_str_args > 0){
		  if(break_token)
		    free(break_token);
		  break_token = strdup(cur_command.str_args[0]);
		  break_set = true;
		}
		else
		  printf("Please supply a token to break on");
		break;
		
	case CMD_RESET:
		reset_parser();
		if(test_file)
		  reset_lexer(test_file);
		break;
	
	case CMD_HELP:
		help_info();
  }
		  
}


void end_interactive()
{
  
  bitset_free (shift_set);
  bitset_free (look_ahead_set);
      
  free_parser();
}

void print_info()
{
  printf("\nWelcome to Bison 2.3 interactive mode. Type \"help\" for assistance.\n");
  printf("Please report bugs to spopur2@uic.edu.\n");
  
}

void
state_print (FILE *out, state *s)
{
  fputs ("\n", out);
  fprintf (out, _("state %d:"), s->number);
  fputc ('\n', out);
  core_print (out, s);
  //print_actions (out, s);
  if (s->solved_conflicts)
  {
    fputc ('\n', out);
    fputs (s->solved_conflicts, out);
  }
}

void
core_print (FILE *out, state *s)
{
  size_t i;
  item_number *sitems = s->items;
  size_t snritems = s->nitems;
  symbol *previous_lhs = NULL;

  /* Output all the items of a state, not only its kernel. 
  if (report_flag & report_itemsets)
    {
      closure (sitems, snritems);
      sitems = itemset;
      snritems = nritemset;
    }
*/
  if (!snritems)
    return;

  fputc ('\n', out);

  for (i = 0; i < snritems; i++)
    {
      item_number *sp;
      item_number *sp1;
      rule_number r;

      sp1 = sp = ritem + sitems[i];

      while (*sp >= 0)
	sp++;

      r = item_number_as_rule_number (*sp);

      rule_lhs_print (&rules[r], previous_lhs, out);
      previous_lhs = rules[r].lhs;

      for (sp = rules[r].rhs; sp < sp1; sp++)
	fprintf (out, " %s", symbols[*sp]->tag);
      fputs (" .", out);
      for (/* Nothing */; *sp >= 0; ++sp)
	fprintf (out, " %s", symbols[*sp]->tag);

      /*state_rule_look_ahead_tokens_print (s, &rules[r], out);*/
      fputc ('\n', out);
    }
}

/*
 * Find a token number corresponding to a %token definition string */
int find_token(char *token)
{
  int i;
  int sym=0;
  for(i=0;i<nsyms;++i){
    if(strcmp(symbols[i]->tag,token) == 0){
      sym = symbols[i]->user_token_number;
      break;
    }
  }
  return sym;
}

void help_info()
{
  printf("Currently available commands:\n");
  printf("\n");
  printf("quit\t\t\t- Quit interactive mode.\n");
  printf("lexer [lexer-module]\t- Load a lexical analyzer.\n");
  printf("test  [testfile    ]\t- Start single step parse with test file.\n");
  printf("next\t\t\t- Single step parse.\n");
  printf("state [stateno     ]\t- Print rules in a state with position of dot.\n");
  printf("rule  [ruleno      ]\t- Print a rule.\n");
  printf("stack\t\t\t- Print stacks.\n");
  printf("help\t\t\t- Show this help info.\n");
  printf("<Enter>\t\t\t- repeat last command.\n");
}
