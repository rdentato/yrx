/*
.T G&L binding
.A Remo Dentato

.% Licence

  This is the integration of the Lua5 language that is released under
the MIT licence:

.quote

Lua 5.0 license

Copyright © 2003 Tecgraf, PUC-Rio. 
Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions: 

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software. 

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE. 

..

and Gema that is placed in the public domain under its own licence:

.quote

  This file is part of "gema", the general-purpose macro translator,
  written by David N. Gray <dgray@acm.org> in 1994 and 1995.
  Adapted for the Macintosh by David A. Mundie <mundie@telerama.lm.com>.
  You may do whatever you like with this, so long as you retain
  an acknowledgment of the original source.
..

As such, this binding code is placed under the most liberal of the two.

*/
#ifdef LUA
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* GEMA includes */
#include "cstream.h"
#include "pattern.h"
#include "patimp.h"
#include "var.h"

/* Lua includes */

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

/*#include "dbg.h"*/

/* .% Initialize
*/

int gel_libopen (lua_State *L) ;
static int gel_write(lua_State *L);
static int gel_set(lua_State *L);
static int gel_incr(lua_State *L);
static int gel_decr(lua_State *L);
static int gel_push(lua_State *L);
static int gel_get(lua_State *L);
static int gel_pop(lua_State *L);
static int gel_col(lua_State *L);
static int gel_line(lua_State *L);
static int gel_parsefile(lua_State *L);
static int gel_parsestring(lua_State *L);
static int gel_rulefile(lua_State *L);
static int gel_rulestring(lua_State *L);
static int gel_ruleclear(lua_State *L);
static int gel_streamfile(lua_State *L);
static int gel_streamstring(lua_State *L);
static int gel_streamclose(lua_State *L);
static int gel_parsestream(lua_State *L);

static int gel_streameof(lua_State *L);
static int gel_streamgetc(lua_State *L);
static int gel_streampeek(lua_State *L);

static COStream vgp_outstream=NULL;
static CIStream vgp_instream=NULL;

static lua_State *L = NULL;

static const luaL_reg lualibs[] = {
  {"base", luaopen_base},
  {"table", luaopen_table},
  {"io", luaopen_io},
  {"string", luaopen_string},
  {"math", luaopen_math},
  {"debug", luaopen_debug},
  {"loadlib", luaopen_loadlib},
  {"gel", gel_libopen},
  {NULL, NULL}
};

/**/
 
static const luaL_reg gellib[] = {
  {"streampeek",   gel_streampeek},
  {"streamgetc",   gel_streamgetc},
  {"streameof",    gel_streameof},
  {"parsestream",  gel_parsestream},
  {"streamfile",   gel_streamfile},
  {"streamstring", gel_streamstring},
  {"streamclose",  gel_streamclose},
  {"rulefile",     gel_rulefile},
  {"rulestring",   gel_rulestring},
  {"ruleclear",    gel_ruleclear},
  {"parsefile",    gel_parsefile},
  {"parsestring",  gel_parsestring},
  {"write",        gel_write},
  {"incr",         gel_incr},
  {"decr",         gel_decr},
  {"set",          gel_set},
  {"get",          gel_get},
  {"push",         gel_push},
  {"pop",          gel_pop},
  {"line",         gel_line},
  {"column",       gel_col},
  {NULL, NULL}
};


/*
** Open gel library
*/
int gel_libopen (lua_State *L) 
{
  luaL_openlib(L, "gel", gellib, 0);
  return 1;
}

int gel_init(lua_State *LS)
{
  gel_libopen(LS);
  initialize_syntax();
  L=LS;
  return 0;
}

static int init_lua()
{
  const luaL_reg *lib = lualibs;

  L = lua_open();  /* create state */
  if (L) {
    for (; lib->func; lib++) {
      lib->func(L);  /* open library */
      lua_settop(L, 0);  /* discard any results */
    }
  }

  return (L == NULL);
}

int close_lua()
{
  if (L) lua_close(L);
  L=NULL;
  return 0;
}


/* .% Lua -> Gema interaction
*/

static int gel_write(lua_State *L)
{
  char *p = (char *)lua_tostring(L,1);

/*  dbg_printf(0,("write:\"%s\" to %x",p,vgp_outstream));*/
  
  if (vgp_outstream) {
    if (!p) p="nil";
    cos_puts(vgp_outstream,p);
  }
  else
   luaL_error(L,"No output stream available\n");

  return 0;
}


static int gel_setvar(lua_State *L,char type)
{
  const char *vlpc_var;
  const char *vlpc_val;
  
  vlpc_var=lua_tostring(L,1);
  vlpc_val=lua_tostring(L,2);
  
  if (!vlpc_var)
    luaL_error(L,"No variable name specified\n");
  else if (!vlpc_val)
    luaL_error(L,"No value specified for assignment\n");
  else {
    if (type == 's')
      set_var(vlpc_var, vlpc_val, strlen(vlpc_val));
    else
      bind_var(vlpc_var, vlpc_val, strlen(vlpc_val));
  }
    
  return 0;
}


static int gel_set(lua_State *L)
{
  return gel_setvar(L,'s');
}

static int gel_push(lua_State *L)
{
  return gel_setvar(L,'p');
}

static int gel_getvar(lua_State *L,char type)
{
  const char *vlpc_var=NULL;
  const char *vlpc_val=NULL;
  char buf[20];
  int  length;
  
  buf[0] = '.'; buf[1] = '\0';buf[2] = '\0';buf[3] = '\0';

  if (isupper(type)) {
    vlpc_var=buf;
    if (type >= 'M') {
      buf[1] = 'c';
      if (type > 'M') buf[2] = '0' + type- 'N' ;
    }
    else {
      buf[1] = 'l';
      if (type > 'A') buf[2] = '0' + type- 'B' ;
    }
  }
  else
    vlpc_var=lua_tostring(L,1);
  
  if (!vlpc_var)
    luaL_error(L,"No variable name specified\n");
  else {
    vlpc_val= get_var(vlpc_var, TRUE, &length);
    if (type == 'p' && vlpc_val)
        unbind_var(vlpc_var);
  }

  lua_settop(L,0);

  if (vlpc_val)
    lua_pushlstring(L,vlpc_val,length);
  else
    lua_pushnil(L);
  
  return 1;  
}

static int gel_get(lua_State *L)
{
  return gel_getvar(L,'g');
}

static int gel_pop(lua_State *L)
{
  return gel_getvar(L,'p');
}

static int gel_line(lua_State *L)
{
  int vli_mrk=0;
  const char *vlpc_mark=NULL;
  
  vlpc_mark=lua_tostring(L,1);
  
  if (vlpc_mark) vli_mrk= atoi(vlpc_mark) % 10 + 1;

  return gel_getvar(L,'A'+vli_mrk);
}

static int gel_col(lua_State *L)
{
  int vli_mrk=0;
  const char *vlpc_mark=NULL;
  
  vlpc_mark=lua_tostring(L,1);
  
  if (vlpc_mark) vli_mrk= atoi(vlpc_mark) % 10 + 1;
  
  return gel_getvar(L,'M'+vli_mrk);
}

static int gel_add(lua_State *L,int val)
{
  const char *vlpc_var=NULL;
  
  vlpc_var=lua_tostring(L,1);
  
  if (!vlpc_var)
    luaL_error(L,"No variable name specified\n");
  else 
    incr_var(vlpc_var, val);
    
  return 0;
}

static int gel_incr(lua_State *L)
{
  return gel_add(L,1);
}

static int gel_decr(lua_State *L)
{
  return gel_add(L,-1);
}


static char *vgpc_emptystr="";

/* Type = 'f' for file or 's' for string */
static int gel_parse(lua_State *L,char type)
{
  FILE *vlpF_infile =NULL;
  FILE *vlpF_outfile=NULL;

  CIStream vlp_instream =NULL;
  COStream vlp_outstream=NULL;

  void *vlp_domain=NULL;
  

  char *vlpc_in  =NULL;  
  char *vlpc_out =NULL;  
  char *vlpc_dom =NULL;  

  int vli_return=0;
  int vli_success=0;

  if (type == 'm')
	vlp_instream=lua_touserdata(L, 1);
  else
    vlpc_in =(char *)lua_tostring(L, 1);

  vlpc_out =(char *)lua_tostring(L, 3);
  vlpc_dom =(char *)lua_tostring(L, 2);

  if (!vlpc_dom) vlpc_dom=vgpc_emptystr;

  vlp_domain=get_domain(vlpc_dom);
  if (!vlp_domain) {
    luaL_error(L,"Unable to find domain \"%s\"\n",vlpc_dom);
	return 0;
  }
    
  if (vlpc_in ) {
    if (type == 'f') {
      vlpF_infile=fopen(vlpc_in ,"r");
      if (vlpF_infile) 
        vlp_instream = make_file_input_stream(vlpF_infile,vlpc_in );
      else
        luaL_error(L,"Can't open \"%s\" for reading\n",vlpc_in);
    }
    else {
      vlp_instream=make_string_input_stream(vlpc_in,0,TRUE);
    }
  }
  
  if (vlp_instream) {
    if (vlpc_out ) {
      if ((vlpc_out[0] == '-') && vlpc_out[1] == '\0')
        vlp_outstream = vgp_outstream;
      else {
	    vlpF_outfile=fopen(vlpc_out ,"w");
        if (vlpF_outfile)
          vlp_outstream = make_file_output_stream(vlpF_outfile,
                                                       vlpc_out );
        else
          luaL_error(L,"Can't open \"%s\" for reading\n",vlpc_in);
      }
    }
    else {
      vlp_outstream = make_buffer_output_stream();
      vli_return = 1;
    }
  }

  if (vlp_outstream) {
/*    dbg_printf(0,("About to translate %08X into domain %s (%08X)\n",vlp_instream,vlpc_dom,vlp_domain));*/
    vli_success = translate (vlp_instream, vlp_domain, vlp_outstream, NULL);
/*    dbg_printf(0,("Translation done (status = %d)!\n",vli_success));*/
  }

  lua_settop(L,0);

  if (vli_success && vli_return)
    lua_pushstring(L,(const char *)cos_whole_string(vlp_outstream));
  else
    lua_pushstring(L,"");
   

  if (vlp_instream && (type != 'm')) {
    cis_close(vlp_instream);
    vlpF_infile=NULL;
  }
  if (vlp_outstream && (vlp_outstream != vgp_outstream)) {
    cos_close(vlp_outstream);
    vlpF_outfile=NULL;
  }
/*  dbg_printf(0,("%x",vlpF_outfile));*/

  if (vlpF_infile) fclose(vlpF_infile);
  if (vlpF_outfile) fclose(vlpF_outfile);

  return 1;
}

static int gel_parsefile(lua_State *L)
{
  return gel_parse(L,'f');
}

static int gel_parsestring(lua_State *L)
{
  return gel_parse(L,'s');
}

static int gel_parsestream(lua_State *L)
{
  return gel_parse(L,'m');
}

static int gel_streamfile(lua_State *L)
{
  FILE *vlpF_infile =NULL;
  CIStream vlp_instream =NULL;
  char *vlpc_in  =NULL;  

  vlpc_in =(char *)lua_tostring(L, 1);
  if (vlpc_in ) {
    vlpF_infile=fopen(vlpc_in ,"r");
    if (vlpF_infile) 
      vlp_instream = make_file_input_stream(vlpF_infile,vlpc_in );
    else
      luaL_error(L,"Can't open \"%s\" for reading\n",vlpc_in);
  }
  lua_pushlightuserdata(L,vlp_instream);

  return 1;
}

static int gel_streamstring(lua_State *L)
{
  CIStream vlp_instream =NULL;
  char *vlpc_in  =NULL;  
  vlpc_in =(char *)lua_tostring(L, 1);
  if (vlpc_in)
    vlp_instream=make_string_input_stream(vlpc_in,0,TRUE);

  lua_pushlightuserdata(L,vlp_instream);
   
  return 1;
}

static int gel_streamclose(lua_State *L)
{
  CIStream vlp_instream =NULL;
  
  vlp_instream=lua_touserdata(L, 1);

  if (vlp_instream) 
    cis_close(vlp_instream);

  return 0;
}

static int gel_streameof(lua_State *L)
{
  CIStream vlp_instream =NULL;
  
  vlp_instream=lua_touserdata(L, 1);
/*  dbg_printf(0,("EOF: %x\n",vlp_instream));*/

  if (vlp_instream && cis_eof(vlp_instream))
    lua_pushnumber(L,1);
  else
    lua_pushnil(L);

  return 1;
}

static int gel_getc(lua_State *L,char type)
{
  char vlac_ch[2];
  CIStream vlp_instream =NULL;

  vlac_ch[0]='\0';
  vlac_ch[1]='\0';

  vlp_instream=lua_touserdata(L, 1);
/*  dbg_printf(0,("CHR: %x\n",vlp_instream));*/
  
  if (vlp_instream) {
	if (type == 'g')
      vlac_ch[0] = (char)cis_getch(vlp_instream);
    else
      vlac_ch[0] = (char)cis_peek(vlp_instream);
  }
  
  if (*vlac_ch && (*vlac_ch != EOF))
    lua_pushstring(L,vlac_ch);
  else
    lua_pushnil(L);

  return 1;
}

static int gel_streampeek(lua_State *L)
{
  return gel_getc(L,'p');
}

static int gel_streamgetc(lua_State *L)
{
  return gel_getc(L,'g');
}

/*.% Rules handling
*/
static int gel_rules(lua_State *L,char type)
{
  CIStream vlp_stream=NULL;
  FILE *vlpF_file=NULL;
  char *vlpc_in=NULL;

  vlpc_in =(char *)lua_tostring(L, 1);

  if (vlpc_in) {
    if (type == 'f') {
      vlpF_file=fopen(vlpc_in,"r");
      if (vlpF_file)
        vlp_stream = make_file_input_stream(vlpF_file,vlpc_in);
      else
        luaL_error(L,"Can't open \"%s\" for reading\n",vlpc_in);
    }
    else  
      vlp_stream=make_string_input_stream(vlpc_in,0,TRUE);
  }
  else 
    luaL_error(L,"Missing filename\n");
    
  if (vlp_stream) {
    read_patterns (vlp_stream, "", FALSE);
    cis_close(vlp_stream);
    vlpF_file=NULL;
  }
  
  if (vlpF_file) fclose(vlpF_file);

  return 0;
}


static int gel_rulefile(lua_State *L)
{
  return gel_rules(L,'f');
}

static int gel_rulestring(lua_State *L)
{
  return gel_rules(L,'s');
}

extern void delete_patterns();
extern void delete_pattern();

static int gel_ruleclear(lua_State *L)
{
  const char *vlpc_dom;
  Domain vlp_dom;

  vlpc_dom =(char *)lua_tostring(L, 1);

  if (!vlpc_dom) vlpc_dom=vgpc_emptystr;

  vlp_dom=get_domain(vlpc_dom);
  if (!vlp_dom) {
    luaL_error(L,"Unable to find domain \"%s\"\n",vlpc_dom);
	return 0;
  }
  delete_patterns( &vlp_dom->patterns );
  while ( vlp_dom->init_and_final_patterns != NULL ) {
    Pattern p = vlp_dom->init_and_final_patterns;
    vlp_dom->init_and_final_patterns = p->next;
    delete_pattern(p);
  }


  return 0;
}


/* .% Calling Lua from GEMA
*/

/*
  Runs lua code, the first of the value returned in the stack, if any,
is given back to gema as result

  @lua{f(x,t)}        executes f and discard return value
  @lua{return f(x,t)} executes f and keep return value

  @lua{w[$0]=$1} 

*/



void dolua(CIStream in, COStream out,char *luacode)
{
  int status=0;
  char *p=NULL;
  
  int top;

  if (L == NULL) init_lua();
    
  if (L == NULL) {
	fprintf(stderr,"Unable to open Lua\n");
	return;
  }

  if (!luacode || !*luacode) return;
  
  top=lua_gettop(L);
  vgp_outstream = out;
  vgp_instream = in;
  
/*  dbg_printf(0,("---\ncmd: %s (stack:%d) (out: %x)\n",luacode,top,vgp_outstream));*/

  status = lua_dostring(L,luacode);

/*  dbg_printf(0,("executed: %d (stack:%d)\n",status,lua_gettop(L)));*/

  if (status != 0 && out) {
    cos_puts(out,"[Error during lua execution]");
  }
    
  if (status == 0 && lua_gettop(L) > top) {
    p=(char *)lua_tostring(L,top+1);
    
/*
#ifndef NDEBUG
    if (p) dbg_printf(0,("return: %s\n",p))
    else dbg_printf(0,("no return:\n"));
#endif
*/
    if (p && out) cos_puts(out,p);
  }
  lua_settop(L,top);
  vgp_outstream = NULL;
  
}

#endif