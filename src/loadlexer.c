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
#include <dlfcn.h>

#include "system.h"
#include "loadlexer.h"

typedef int (*lex_func)(); /* yylex */
lex_func yylex_ptr;

typedef void (*restart_func)(FILE *);
restart_func yyrestart_ptr;

char *token_text = NULL;
char **ytext;
FILE **yin;

int load_lexer(char *libname)
{
  void *handle;
  char *func_name = "yylex";
  char *restart_func_name = "yyrestart";
  
  int result;
  int mode = RTLD_LAZY;
  
  if ( (handle = dlopen(libname,mode)) == NULL )
  {
    fprintf(stderr,"ERROR loading lexer: %s\n", dlerror());
    return -1;
  }
  
  if ( (yylex_ptr = (lex_func)dlsym(handle,func_name)) == NULL )
  {
    fprintf(stderr,"yylex not found!: %s\n", dlerror());
    return -1;
  }
  
  if( (yyrestart_ptr = (restart_func)dlsym(handle,restart_func_name)) == NULL)
  {
    fprintf(stderr,"yyrestart not found!: %s\n", dlerror());
    return -1;
  }
  
  if ( (yin = (FILE **)dlsym(handle, "yyin")) ==NULL)
  {
    fprintf(stderr,"yyin not found:%s\n", dlerror());
    return -1;
  }

  if((ytext = (char **)dlsym(handle,"yytext")) == NULL)
  {
    fprintf(stderr,"yytext not found:%s\n",dlerror());
    return -1;
  }
  
  
  return 1;
}

int set_lex_in(const char *path)
{
 
  if(*yin)
    fclose(*yin);
  
  *yin = fopen(path,"r");
  if (*yin == NULL)
  {
    fprintf(stderr,"Cannot open file:%s\n", path);
    return -1;
  }

  return 1;
}

void reset_lexer(char *path)
{
  FILE *fp;
  if(*yin)
    fclose(*yin);
  
  fp = fopen(path, "r");
  (*yyrestart_ptr)(fp);
}
  
int call_lex()
{
  int token = (*yylex_ptr)();
	
  if(token_text != NULL)
    free(token_text);

  if(token == 0)
    token_text = "<<EOF>>";
  else
    token_text = strdup(*ytext);

  return token;
}

int yywrap()
{
	return 1;
}
