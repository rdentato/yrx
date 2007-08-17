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


-- == Convert asm to Lua

local pkgname = "yrxl"
module(pkgname,package.seeall)
dbg = false

yrx = require("yrxasm")

local function luafun(dfa,actions)
  local i,t,c,n,mtc
  local fun={}
  local get=true

  fun[#fun+1] = "  local s,i = ...\n"
  fun[#fun+1] = "  local c\n"
  fun[#fun+1] = "  local match = 0 \n"
  fun[#fun+1] = "  local match_pos\n"
  fun[#fun+1] = "  local start_pos\n"
  fun[#fun+1] = "  local capt = {}\n"
  fun[#fun+1] = "  local fnz = {}\n\n"
  

  fun[#fun+1] = "  i = i or 1\n"
  fun[#fun+1] = "  start_pos = i\n"
  fun[#fun+1] = "  s = s or ''\n"
  
  fun[#fun+1] = "  function fnz.S1(s,i)\n"
  if dbg then fun[#fun+1] = " io.stderr:write(\"S1(\\\"\",s,\"\\\",\",i,\")\\n\")\n" end
  for i = 1,#dfa.pgm do
    t = dfa.pgm[i].lbl     
    if t then
      if dfa.pgm[i-1].op ~= "JMP" and dfa.pgm[i-1].op ~= "RET" then
        fun[#fun+1] = "    return fnz." .. t .. "(s,i)\n  end\n\n"
      end
      fun[#fun+1] = "  function fnz." .. t .. "(s,i)\n"
      if dbg then fun[#fun+1] = " io.stderr:write(\"" .. t .. "(\\\"\",s,\"\\\",\",i,\")\\n\",c)\n" end
    end
    t = dfa.pgm[i]

    if t.op == "CMP" then
      if t.arg <= 32 then
        c = "'\\" .. tostring(t.arg) .. "'"
      else
        c = string.format("%q",string.char(t.arg))
      end
    elseif t.op == "JGE" then
      fun[#fun+1] = "    if c >= " .. c .. " then return fnz." .. t.arg .. "(s,i) end\n"
    elseif t.op == "JLE" then
      fun[#fun+1] = "    if c <= " .. c .. " then return fnz." .. t.arg .. "(s,i) end\n"
    elseif t.op == "JNE" then
      fun[#fun+1] = "    if c ~= " .. c .. " then return fnz." .. t.arg .. "(s,i) end\n"
    elseif t.op == "JEQ" then
      fun[#fun+1] = "    if c == " .. c .. " then return fnz." .. t.arg .. "(s,i) end\n"
    elseif t.op == "JGT" then
      fun[#fun+1] = "    if c >  " .. c .. " then return fnz." .. t.arg .. "(s,i) end\n"
    elseif t.op == "JLT" then
      fun[#fun+1] = "    if c <  " .. c .. " then return fnz." .. t.arg .. "(s,i) end\n"
    elseif t.op == "FGE" then
      fun[#fun+1] = "    if c >= " .. c .. " then return fnz.fail(s,i) end\n"
    elseif t.op == "FLE" then
      fun[#fun+1] = "    if c <= " .. c .. " then return fnz.fail(s,i) end\n"
    elseif t.op == "FNE" then
      fun[#fun+1] = "    if c ~= " .. c .. " then return fnz.fail(s,i) end\n"
    elseif t.op == "FEQ" then
      fun[#fun+1] = "    if c == " .. c .. " then return fnz.fail(s,i) end\n"
    elseif t.op == "FGT" then
      fun[#fun+1] = "    if c >  " .. c .. " then return fnz.fail(s,i) end\n"
    elseif t.op == "FLT" then
      fun[#fun+1] = "    if c <  " .. c .. " then return fnz.fail(s,i) end\n"
    elseif t.op == "GET" then
      fun[#fun+1] = "    c = string.sub(s,i,i);\n"
      fun[#fun+1] = "    if c == \"\" then\n"
      if not mtc and get then
        fun[#fun+1] = "      match = -1 \n" ; get = false
      end
      fun[#fun+1] = "      return fnz.fail(s,i)\n"
      fun[#fun+1] = "    end\n"
      fun[#fun+1] = "    i=i+1\n"
    elseif t.op == "RET" then
      mtc = nil
      fun[#fun+1] = "    return fnz.fail(s,i)\n  end\n\n"
    elseif t.op == "JMP" then
      fun[#fun+1] = "    return fnz." .. t.arg .. "(s,i)\n  end\n\n"
    elseif t.op == "MTC" then
      if not mtc then
        mtc = t.arg
        fun[#fun+1] = "    match = " .. mtc .. "\n"
        fun[#fun+1] = "    match_pos = i \n"
      end
    elseif string.sub(t.op,1,2) == "CB" then
        k = tonumber(string.sub(t.op,3,3)) * 2 - 1
        k = k + t.arg *20
        fun[#fun+1] = "    capt[" .. k .. "] = i-1\n"
    elseif string.sub(t.op,1,2) == "CE" then
        k = tonumber(string.sub(t.op,3,3)) * 2 - 1
        k = k + t.arg *20 +1
        fun[#fun+1] = "    capt[" .. k .. "] = i-1\n"
    end
    if t.op ~= "MTC" then mtc = nil end
  end
  
  fun[#fun+1] = [[
  function fnz.fail(s,i)
    local f
    if match <= 0 then
      match_pos = start_pos
    end
    return match, match_pos
  end
]]
  
  fun[#fun+1] = [[
  
  match,match_pos = fnz.S1(s,i) 
  
  local res = {}
  if match > 0 then
    res[1] = string.sub(s,i,match_pos-1)
    k = match * 20 + 1
    for i=1,9 do
      if capt[k] then
        --print("oo",capt[k],(capt[k+1] or match_pos)-1)
        res[i+1] = string.sub(s,capt[k],(capt[k+1] or match_pos)-1)
      else
        res[i+1] = "" 
      end
      k = k+2
    end
  end
  
  return match,(match_pos or i),res
]]

  fun = table.concat(fun)
  if false then
    t=io.open("match.lua","w")
    t:write(fun)
    t:close()
  end
  return fun
end


yrx.scanner_mt = {
   __call = function(f, s, i,...)
              local err
              local ret = 1
              local capt
              i = i or 1
              s = s or ""
              while err == nil and ret > 0 do
                ret,i,capt = f[0](s,i)
                if capt[1] == "" then ret = 0 end -- Empty matches are discarded!
                if ret ~= 0 then
                  err = f[ret](unpack(capt))
                end
              end
              return i,err
            end
}

function yrx.scanner(stab)
  local rxs = {}
  local ftab = {}
  local dfa,err
  local i,t,k
  local err

  k = 1
  for i,t in pairs(stab) do
    if type(t[1]) == "number" then
      ftab[t[1]] = t[2]
    else
      ftab[k] = t[2]
      rxs[k]  = string.gsub(t[1],"%%(.)","\\%1")
      k = k+1
    end
  end

  if ftab[yrx.eof] == nil then 
    ftab[yrx.eof] = function () end
  end
  
  dfa,err = yrx.dfa(rxs)
  if dfa then
    yrx.asm(dfa)
    if dfa.arcs[1].to == 0 then error("A pattern matches the empty string!") end
    ftab[0],err = loadstring(luafun(dfa),"scan")
    if ftab[0] then  
      setmetatable(ftab,yrx.scanner_mt)
      return ftab
    end
  end
  return nil,err    
end  
 
function yrx.scansave(scanner,fname)
  local k,f
  local fun = {}
  fname = fname or "scanner.lua"
  fun[#fun+1]="local f={}\n"
  for k,f in pairs(scanner) do
    t = string.format("%q",string.dump(f))
    t = string.gsub(t,"\26","\\26") -- get rid of possible 0x1A issues
    fun[#fun+1] = "f["..k.."] = loadstring(" .. t .. ",\"\")"
  end
  
  f = getmetatable(scanner).__call
  t = string.format("%q",string.dump(f))
  t = string.gsub(t,"\26","\\26") -- get rid of possible 0x1A issues
  
  fun[#fun+1] = "\nsetmetatable(f,{_call = loadstring(" .. t .. ",\"\")})"
  
  fun[#fun+1] = "return f\n"
  
  f = assert(io.open(fname,"wb"))
  f:write(table.concat(fun,"\n"))
  f:close()
end

function yrx.yl()
  local buf
  local sc_outyl,sc_inyl
  local st = "out"
  local scan = {}
  local i
  local function wrt(a) io.write("<",a,">") end 
  
  
  scan["out"] , err = yrx.scanner {
                   {yrx.eof,function() return "yrxend" end },
                   {'%s+', wrt },
                   {'[^%-"]+', wrt },
                   {"%Q" , wrt },
                   {"%-%-%[%[%s*YYSWITCH%s*%(%s*(%w+)%s*%).-\n?", 
                          function (a,b) io.write(a); return b end },
                   {"%-%-[^%[].-\n?" , wrt },
                   {'.', wrt }
                }
  if err then error(err) end
  
  scan["in"] , err = yrx.scanner {
                   {yrx.eof,function() return "yrxend" end },
                   {"%s*%-%-%y*END%s*]]",
                        function (a)
                          wrt(a)
                          return "out"
                        end },
                        
                   {"%s*(%Q)%s*:%s*" ,
                        function (a,b)
                          wrt(a)
                          rx.nrx = rx.nrx + 1
                          rx.crx = rx.nrx
                          rx[rx.crx] = b
                          act[rx.crx] = ""
                        end },
                        
                   {"%s*(FAIL|ELSE)%s*:%s*" ,
                        function (a,b)
                          wrt(a)
                          rx.crx = 0
                          act[rx.crx] = ""
                        end },
                        
                   {"%s*EOF%s*:%s*" ,
                        function (a,b)
                          wrt(a)
                          rx.crx = -1
                          act[rx.crx] = ""
                        end },
                        
                   {'[^EF"%-]+', function (a)
                           wrt(a)
                           if act[rx.crx] then act[rx.crx] = act[rx.crx] .. a end
                         end},
                }
  if err then error(err) end
  
  scan["chkcode"] , err = yrx.scanner {
                   {yrx.eof,function() return "yrxend" end },
                   {"%s*%-%-%[%[%s*GENERATED CODE.-]].-%n?", function() return "del" end},
                   {yrx.fail,function() return "nodel" end }
                }
  if err then error(err) end
  
  scan["delcode"] , err = yrx.scanner {
                   {yrx.eof,function() return "yrxend" end },
                   {"%s*%-%-%[%[%y*END%y*]]%y*", function() return "in" end},
                }
  if err then error(err) end
  
  buf = io.read("*all")
  
  i=1
  while st ~= "yrxend" do
    i,st = scan["out"](buf,i)
    if st ~= "yrxend" then
      rx = {} ; rx.nrx = 0; rx.crx = 0; act = {}
      i,st = scan["in"](buf,i)
    end
    if st ~= "yrxend" then
      -- remove previously generated code (if any)
  io.write("<<",i,">>")
      i,st = scan["chkcode"](buf,i)
      if st == "del" then
        i,st = scan["delcode"](buf,i)
      end
    end
  end 
end

yrx["-lua"] = function (rxs,opts)
  local dfa,err
      
  if rxs == 0 then
    return [[
-lua [ptrns]
        dump as lua function named fname (yylexer by default)
]]
  end
    
  dfa,err = yrx.dfa(rxs)
  if dfa then
    yrx.asm(dfa)
    io.write(luafun(dfa))
  end
  return err
end

yrx["-yl"] = function (rxs,opts)
  local dfa,err
      
  if rxs == 0 then
    return [[
-yl [ptrns]
        parse a .yl file
]]
  end
  
  yrx.yl(rxs,opts)  
  
  return err
end


package.loaded[pkgname] = yrx
package.loaded["yrxlua"] = yrx  -- for when we create a single module
return yrx
