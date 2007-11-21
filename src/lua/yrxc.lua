--  
--  (C) 2007 by Remo Dentato (rdentato@users.sourceforge.net)
--
-- Permission to use, copy, modify and distribute this code and
-- its documentation for any purpose is hereby granted without
-- fee, provided that the above copyright notice, or equivalent
-- attribution acknowledgement, appears in all copies and
-- supporting documentation.
-- 
-- Copyright holder makes no representations about the suitability
-- of this software for any purpose. It is provided "as is" without
-- express or implied warranty.
--


-- == Export to dot format

local pkgname = "yrxc"

module(pkgname,package.seeall)

yrx = require("yrxasm")
local cl = {}
cl[string.byte('\\')] = "'\\\\'"
cl[string.byte('\'')] = "'\\''"

local function chr(c)
   if c < 32 or c > 127 then
     return string.format("0x%02X",c)
   end
   if cl[c] then return cl[c] end
   return "'"..string.char(c).."' "
end

local capt = {}
local mrk = {}

local function captnum(capt,be,n,expr)
  local k
  
  k=2*n
  if be == 'E' then k = k+1 end
  return capt[expr-1]+k
end

function yrx.cfun(dfa,fname,actions)
  local l,t
  local state 
  local n
  local mtc
  local get
  local c
  
  actions = actions or {}

  if fname == "" then fname = nil end
  fname = fname or "yrxlex"
  
  capt[0] = 0
  for i = 1,dfa.nrx do
    capt[i] = capt[i-1] + dfa.ncp[i]*2
  end

  io.write([[
#ifndef YYGET
#define YYGET(s) (*s == '\0'? -1 : *s++)
#define YRX_UNDEFGET
#endif

]])

  io.write("yrxmatch *",fname,"(char *str)\n")
  io.write([[{
  static yrxmatch yrx_match;
  int yrx_c;
  char *yrx_match_pos;
  char *yrx_capt[]],capt[dfa.nrx],[[];
  
  yrx_match.rx = 0;
  for (yrx_c=0; yrx_c<10; yrx_c++) {yrx_match.capt[yrx_c] = NULL; yrx_match.len[yrx_c] = 0;}
  memset(yrx_capt,'\0',]],capt[dfa.nrx],[[*sizeof(char *));
  yrx_match.capt[0] = str;
  yrx_match_pos = NULL;
  
]])


  for i,step in ipairs(dfa.pgm) do
    op  = step.op

    if step.lbl then
      io.write("\n  ",step.lbl,":  /*********************/\n")
    end
    if op == "GET" then
      io.write("    yrx_c = YYGET(str);\n    if ( yrx_c == -1  ) {")
      if not get then
        io.write(" yrx_match.rx = -1;")
        get = true
      end
      io.write(" goto ret;}\n")
    elseif op == "CMP" then
      c = chr(step.arg)
    elseif op == "JMP" then
      io.write ("    goto ",step.arg,";\n")
    elseif op == "JEQ" then
      io.write ("    if ( yrx_c == ",c,") goto ",step.arg,";\n")
    elseif op == "JNE" then
      io.write ("    if ( yrx_c != ",c,") goto ",step.arg,";\n")
    elseif op == "JLE" then
      io.write ("    if ( yrx_c <= ",c,") goto ",step.arg,";\n")
    elseif op == "JGE" then
      io.write ("    if ( yrx_c >= ",c,") goto ",step.arg,";\n")
    elseif op == "JLT" then
      io.write ("    if ( yrx_c <  ",c,") goto ",step.arg,";\n")
    elseif op == "JGT" then
      io.write ("    if ( yrx_c >  ",c,") goto ",step.arg,";\n")
    elseif op == "RET" then
      if i < #dfa.pgm then
        io.write ("    goto ret;\n")
      end
    elseif op == "FEQ" then
      io.write ("    if ( yrx_c == ",c,") goto ret;\n")
    elseif op == "FNE" then
      io.write ("    if ( yrx_c != ",c,") goto ret;\n")
    elseif op == "FLE" then
      io.write ("    if ( yrx_c <= ",c,") goto ret;\n")
    elseif op == "FGE" then
      io.write ("    if ( yrx_c >= ",c,") goto ret;\n")
    elseif op == "FLT" then
      io.write ("    if ( yrx_c <  ",c,") goto ret;\n")
    elseif op == "FGT" then
      io.write ("    if ( yrx_c >  ",c,") goto ret;\n")
    elseif op == "MTC" and not mtc then
      mtc = step.arg
      io.write ("    yrx_match.rx = ",mtc,";\n")
      io.write ("    yrx_match_pos = str;\n")
      get = true
    elseif op == "MRK" then
      io.write ("    yrx_capt[",captnum(capt,'E',0,step.arg),"] = str-1;\n")
      mrk[step.arg] = true
    elseif string.sub(op,1,1) == "C" then
      io.write ("    yrx_capt[",captnum(capt,string.sub(op,2,2),string.sub(op,3,3),step.arg),"] = str-1;\n")
    end
    if op ~= "MTC" then mtc = nil end

  end  
  
  io.write("\n  ret:\n") 
  
  io.write("    yrx_match.len[0] = yrx_match_pos - yrx_match.capt[0];\n")
  io.write("    switch(yrx_match.rx) {\n")
  io.write("      case  -1:\n") 
  if actions[-1] then
    io.write("                ",actions[-1],"(&yrx_match);\n")
  end
  io.write("                break;\n\n")
  io.write("      case   0: yrx_match_pos = NULL;\n") 
  io.write("                yrx_match.capt[0] = NULL;\n")
  io.write("                yrx_match.len[0] = 0;\n")
  if actions[0] then
    io.write("                ",actions[0],"(&yrx_match);\n")
  end
  io.write("                break;\n\n")
  
  for i=1,dfa.nrx do
    if actions[i] or mrk[i] or dfa.ncp[i] > 1 then
      io.write(string.format("      case %3d:\n",i)) 
      if mrk[i] then
        io.write("                if (yrx_capt[",captnum(capt,'E',0,i),"] != NULL) yrx_match.len[0] = yrx_capt[",captnum(capt,'E',0,i),"] - yrx_match.capt[0];\n") 
      end
      if dfa.ncp[i] > 1 then
        io.write("                for (yrx_c = 1, i = ",capt[i-1]+2,"; i < ",capt[i],"; i += 2, yrx_c++) {\n")
        io.write("                  if ( yrx_capt[i] != NULL) {\n")
        io.write("                    yrx_match.capt[yrx_c] = yrx_capt[i];\n")    
        io.write("                    yrx_match.len[yrx_c] = ((yrx_capt[i+1] != NULL)? yrx_capt[i+1] : str) - yrx_capt[i];\n")    
        io.write("                  }\n")
        io.write("                }\n")
      end
      if actions[i] then
        io.write("                ",actions[i],"(&yrx_match);\n")
      end
      io.write("                break;\n\n")
    end
  end
  
  io.write("    }\n")
  io.write("\n    return &yrx_match;\n}\n")
  
  io.write([[
#ifdef YRX_UNDEFGET
#undef YYGET
#undef YRX_UNDEFGET
#endif

]])
  
  io.write("\n\n")
end

yrx["-c"] = function (rxs,opts)
  local dfa,err
      
  if rxs == 0 then
    return [[
-c:fname [ptrns]
        dump as C function named fname (yylexer by default)
]]
  end
    
  dfa,err = yrx.dfa(rxs)
  if dfa then
    yrx.asm(dfa)
    yrx.cfun(dfa,opts["-c"])
  end
  return err
end

yrx["-cy"] = function (rxs)
  local dfa,err
  local act = {}
      
  if rxs == 0 then
    return [[
-cy:fname [ptrns]
        dump as C function
]]
  end

  dfa = {}
  for i,rx in ipairs(rxs) do
    string.gsub(rx,'%s*"([^"]+)"%s*:?%s*(%w*).*',
                                          function (a,b)
                                           dfa[i] = rx
                                           if b and b~="" then act[i] = b end
                                           return ""
                                          end,1)
    string.gsub(rx,'%s*EOF%s*:?%s*(%w*).*',
                                          function (b)
                                           if b and b~="" then act[-1] = b end
                                           return ""
                                          end,1)
    string.gsub(rx,'%s*ELSE%s*:?%s*(%w*).*',
                                          function (b)
                                           if b and b~="" then act[0] = b end
                                           return ""
                                          end,1)
  end
  rxs = dfa
  dfa,err = yrx.dfa(rxs)
  if dfa then
    yrx.asm(dfa)
    yrx.cfun(dfa,opts["-cy"],act)
  end
  return err
end



package.loaded[pkgname] = yrx
return yrx