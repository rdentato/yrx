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


-- == Export dfa to asm

local pkgname = "yrxasm"

module(pkgname,package.seeall)

yrx = require("yrxdfa")


local asmop = {}

-- JUMP on

asmop["JOF"] =   64   -- 0 100 0 000  0x40
asmop["JEQ"] =   65   -- 0 100 0 001  0x41
asmop["JNE"] =   66   -- 0 100 0 010  0x42
asmop["JGT"] =   67   -- 0 100 0 011  0x43
asmop["JGE"] =   68   -- 0 100 0 100  0x44
asmop["JLT"] =   69   -- 0 100 0 101  0x45
asmop["JLE"] =   70   -- 0 100 0 110  0x46
asmop["JMP"] =   71   -- 0 100 0 111  0x47
                 
asmop["FEQ"] =   73   -- 0 100 1 001  0x49
asmop["FNE"] =   74   -- 0 100 1 010  0x4A
asmop["FGT"] =   75   -- 0 100 1 011  0x4B
asmop["FGE"] =   76   -- 0 100 1 100  0x4C
asmop["FLT"] =   77   -- 0 100 1 101  0x4D
asmop["FLE"] =   78   -- 0 100 1 110  0x4E
asmop["RET"] =   79   -- 0 100 1 111  0x4F

asmop["NOP"] =   48   -- 0 011  0000  0x30
asmop["NRX"] =   49   -- 0 011  0001  0x31
asmop["GET"] =   50   -- 0 011  0010  0x32
asmop["MRK"] =   51   -- 0 011  0011  0x33
asmop["CMP"] =   52   -- 0 011  0100  0x34
asmop["MTC"] =   53   -- 0 011  0101  0x35
asmop["NCP"] =   54   -- 0 011  0110  0x36
asmop["DIM"] =   55   -- 0 011  0111  0x37

-- BEGIN CAPTURE

asmop["CB0"] =   96   -- 0 110 0000  0x60
asmop["CB1"] =   97   -- 0 110 0001  0x61
asmop["CB2"] =   98   -- 0 110 0010  0x62
asmop["CB3"] =   99   -- 0 110 0011  0x63
asmop["CB4"] =  100   -- 0 110 0100  0x64
asmop["CB5"] =  101   -- 0 110 0101  0x65
asmop["CB6"] =  102   -- 0 110 0110  0x66
asmop["CB7"] =  103   -- 0 110 0111  0x67
asmop["CB8"] =  104   -- 0 110 1000  0x68
asmop["CB9"] =  105   -- 0 110 1001  0x69
                      --        |__|
                      --          \____ capture #

-- END CAPTURE

asmop["CE0"] =  112   -- 0 111 0000  0x70
asmop["CE1"] =  113   -- 0 111 0001  0x71
asmop["CE2"] =  114   -- 0 111 0010  0x72
asmop["CE3"] =  115   -- 0 111 0011  0x73
asmop["CE4"] =  116   -- 0 111 0100  0x74
asmop["CE5"] =  117   -- 0 111 0101  0x75
asmop["CE6"] =  118   -- 0 111 0110  0x76
asmop["CE7"] =  119   -- 0 111 0111  0x77
asmop["CE8"] =  120   -- 0 111 1000  0x78
asmop["CE9"] =  121   -- 0 111 1001  0x79
                      --        |__|
                      --          \____ capture #



local function oplen(op)
  local n=2
  local c=string.sub(op,1,1)
  
  if c == "J" then n = 3 
  elseif c == "F" then n = 1
  elseif op == "DIM" then n = 3
  elseif op == "NOP" then n = 1
  elseif op == "NCP" then n = 1
  elseif op == "NRX" then n = 1
  elseif op == "GET" then n = 1
  elseif op == "RET" then n = 1
  elseif op == "XXX" then n = 0
  end
  return n
end

local function obj(dfa)
  local op,arg,lbl
  local i, step
  local k=1
  local opcode, argcode
  
  for i,step in ipairs(dfa.pgm) do
    op  = step.op
    arg = step.arg
    lbl = step.lbl or ""
    
    opcode = asmop[op]
    if arg then
      argcode = arg
      if op == "NCP" or op == "NRX" then 
        opcode = argcode
        argcode = nil
      end
      if string.sub(op,1,1) == "J" then 
        argcode = dfa.pgm[dfa.lbls[arg]].pc
      end
      step.argcode = argcode
    end
    step.opcode  = opcode
    assert(opcode ~= nil,"invalid opcode")
  end
  return dfa
end

local function out(dfa)
 local pgm = dfa.pgm
 local op,arg,lbl
 local i, step
 local k=1
 local opcode, argcode
 
 obj(dfa)

 for i,step in ipairs(pgm) do
   op  = step.op
   arg = step.arg
   lbl = step.lbl or ""
   
   
   if op == "CMP" and (32 < arg and arg < 127) and arg ~= 39 and arg ~= 92 then
     arg = "'" .. string.char(arg) .. "'"
   elseif op == "FNS" then 
     arg = string.format("%q",arg)
   end
   
   io.write(string.format("%05d ",i))
   io.write(string.format("%04X  %02X ",step.pc,(step.opcode or 0)))
   if step.argcode then
     if string.sub(op,1,1) == "J" or op == "DIM" then 
       io.write(string.format("%04X",step.argcode))
     else
       io.write(string.format("%02X  ",step.argcode))     
     end
   else
     io.write("    ")     
   end
   io.write(string.format("      %-10s %s ",lbl,op),arg or "")
   if op == "MTC" then op = "CE0" end
   if string.sub(op,1,1) == "C" and  string.sub(op,2,2) ~= "M" then
     k = 2 * tonumber(string.sub(op,3,3))
     if string.sub(op,2,2) == "E" then k = k+1 end
     io.write("\t;  [",dfa.capt[arg-1]+k,"]")
   end
   if string.sub(op,1,1) == "J" then
     io.write(string.format("\t;  %d",(step.argcode or step.pc) - step.pc))
   end
   io.write("\n")
 end
end

local function optimize(dfa)

  local stk = {}
  local lbk = {}
  local k,i,n,lbl
  local newpgm  = {}
  local pgm = dfa.pgm
  local op
  
    
  lbl = "START"
  dfa.lbls = {}
  newpgm[0] = pgm[0]
  
  while lbl do
    n = dfa.lblk[lbl]
    if n then
      dfa.lblk[lbl] = nil
      -- print(lbl)
      if ((dfa.jxx[lbl] or 0) + (dfa.jmp[lbl] or 0)) == 0 then
        pgm[dfa.blks[n]].lbl = nil
      end
      for i = dfa.blks[n],dfa.blks[n+1] do
        pgm[i].pc = newpgm[#newpgm].pc + oplen(newpgm[#newpgm].op)
        newpgm[#newpgm+1] = pgm[i]

        -- {{ Re-create dfa.lbls to be used 
        lbl = pgm[i].lbl
        if lbl then dfa.lbls[lbl] = #newpgm end
        -- }}
        -- {{ Try to keep labels close to the first JMP
        op  = pgm[i].op
        lbl = pgm[i].arg
        if string.sub(op,1,1) == "J" and dfa.jmp[lbl] == nil then
          stk[#stk+1] = lbl
        end
        -- }} 
      end
      lbl = pgm[dfa.blks[n+1]].arg
      if lbl and dfa.lblk[lbl] then -- the block has not been copied yet
        newpgm[#newpgm] = nil -- remove the "JMP" instruction
        -- {{ One jmp less
        -- if no jump to this label is left we can remove it.
        -- Conditional jumps are accounted in the jxx table
        dfa.jmp[lbl] = dfa.jmp[lbl] - 1
        -- }}
      end
    else
      lbl = nil
    end  
    if lbl == nil then
      while lbl == nil and #stk > 0 do
        lbl = table.remove(stk)
        if dfa.lblk[lbl] == nil then lbl = "" end
      end
      if lbl == nil then lbl = next(dfa.lblk) end
    end
  end
  
  newpgm[1].arg = newpgm[#newpgm].pc + oplen(newpgm[#newpgm].op)

  
  dfa.pgm = newpgm
  
  dfa.lblk = nil
  dfa.blks = nil
    
  return dfa
end

local function nxtarc(dfa,n)
  n=n+1
  return n,dfa.arcs[n]
end

local function treendx(i,j,tmp,lbl)
 local k
 if i <= j then
   k = math.floor((i+j)/2)
   lbl[#lbl+1] = tmp[k]
   lbl[#lbl].lt = 0
   lbl[#lbl].gt = 0
   if k > i then lbl[#lbl].lt = 1 end
   if j > k then lbl[#lbl].gt = math.floor((j-i+1)/2+0.5) end
   treendx(i,k-1,tmp,lbl)
   treendx(k+1,j,tmp,lbl)
 end
 return lbl
end

local function curstep(dfa)
 return dfa.pgm[#dfa.pgm]
end

local function addop(dfa,op,arg)
  local n  
  
  n = dfa.pgm[#dfa.pgm].pc + oplen(dfa.pgm[#dfa.pgm].pc)
   
  dfa.pgm[#dfa.pgm + 1] = {pc = n, op = op, arg = arg}

  if string.sub(op,1,1) == "J" then
    if op == "JMP" then
      dfa.jmp[arg] = (dfa.jmp[arg] or 0) + 1
    else
      dfa.jxx[arg] = (dfa.jxx[arg] or 0) + 1
    end
  end
end

local function addtags(dfa,arc)
  if arc.tagsig ~= "" then
    string.gsub(arc.tagsig, "%d+", function (t)
                                     local tag,expr,n
                                     tag,expr,n = yrx.tagdecode(t)
                                     if tag == "FIN" then tag = "MTC"
                                     elseif tag == "BEG" and arc.to ~= 0 then
                                       tag = "CB".. n
                                     elseif tag == "END" and arc.to ~= 0 then
                                       tag = "CE" .. n
                                     elseif tag == "MRK" and arc.to ~= 0 then
                                       tag = "MRK"
                                     else
                                       tag = nil
                                     end
                                     if tag then addop(dfa,tag,expr) end
                                   end)
 end
end

local function addlbl(dfa,lbl)
  local n = #dfa.pgm+1
  local op = dfa.pgm[#dfa.pgm].op
  dfa.lbls[lbl] = n
  if dfa.linv[n] then
    dfa.leqv[lbl] = dfa.linv[n]
    lbl = dfa.leqv[lbl]
  else
    dfa.leqv[lbl] = lbl
    dfa.linv[n] = lbl
  end
  if op == "JMP" or op == "RET" then
    dfa.blks[#dfa.blks+1] = n-1
    dfa.blks[#dfa.blks+1] = n
    dfa.lblk[lbl] = #dfa.blks    
  end
  return lbl
end

-- NOTE: this has been kept linear to ease maintenance and debugging!
-- Remember gt ==  0  -> leaf
--          gt ==  1  -> only right child (>) (at next step)
--          gt ==  x  -> right at x, left at 1
local function addcmp(dfa,state,i,step)
  local pgm = dfa.pgm
  local Jlbl
  
  addlbl(dfa,"L"..state.."_"..i)
      
  if dfa.arcs[step.eq].tagsig == "" then
    Jlbl =  "S"..dfa.arcs[step.eq].to
  else
    Jlbl =  "A"..step.eq
  end
  
  if step.min == 0 and step.max == 255 then
    addop(dfa,"JMP",Jlbl)
    return
  end
  
  if step.min == 0 and step.max == 0 then
    if step.gt == 0 then
      addop(dfa,"CMP",0)
      addop(dfa,"FNE")
      addop(dfa,"JMP",Jlbl)
    elseif step.gt == 1 then
      addop(dfa,"CMP",0)
      addop(dfa,"JEQ",Jlbl)
    else
      addop(dfa,"CMP",0)
      addop(dfa,"JGT", "L"..state.."_"..i+step.gt)
      addop(dfa,"JMP",Jlbl)
    end
    return
  end
  
  if step.min == 255 and step.max == 255 then
    if step.gt == 0 then
      addop(dfa,"CMP",255)
      addop(dfa,"FNE")
      addop(dfa,"JMP",Jlbl)
    elseif step.gt == 1 then
      addop(dfa,"CMP",255)
      addop(dfa,"FLT")
      addop(dfa,"JMP",Jlbl)
    else
      addop(dfa,"CMP",255)
      addop(dfa,"JEQ",Jlbl)
    end
    return
  end
  
  if step.min == 0 then
    if step.gt == 0 then
      addop(dfa,"CMP",step.max)
      addop(dfa,"FGT")
      addop(dfa,"JMP",Jlbl)
    elseif step.gt == 1 then
      addop(dfa,"CMP",step.max)
      addop(dfa,"JLE",Jlbl)
    else
      addop(dfa,"CMP",step.max)
      addop(dfa,"JGT", "L"..state.."_"..i+step.gt)
      addop(dfa,"JMP",Jlbl)
    end
    return
  end
  
  if step.max == 255 then
    if step.gt == 0 then
      addop(dfa,"CMP",step.min)
      addop(dfa,"FLT")
      addop(dfa,"JMP",Jlbl)
    elseif step.gt == 1 then
      addop(dfa,"CMP",step.min)
      addop(dfa,"JGE",Jlbl)
    else
      addop(dfa,"CMP",step.min)
      addop(dfa,"JGE",Jlbl)
    end
    return
  end
  
  if step.min == step.max then
    if step.gt == 0 then
      addop(dfa,"CMP",step.min)
      addop(dfa,"FNE")
      addop(dfa,"JMP",Jlbl)
    elseif step.gt == 1 then
      addop(dfa,"CMP",step.min)
      addop(dfa,"FLT")
      addop(dfa,"JEQ",Jlbl)
    else
      addop(dfa,"CMP",step.min)
      addop(dfa,"JGT", "L"..state.."_"..i+step.gt)
      addop(dfa,"JEQ",Jlbl)
    end
    return
  end

  if step.gt == 0 then
    addop(dfa,"CMP",step.min)
    addop(dfa,"FLT")
    addop(dfa,"CMP",step.max)
    addop(dfa,"FGT")
    addop(dfa,"JMP",Jlbl)
  elseif step.gt == 1 then
    addop(dfa,"CMP",step.min)
    addop(dfa,"FLT")
    addop(dfa,"CMP",step.max)
    addop(dfa,"JLE",Jlbl)
  else
    addop(dfa,"CMP",step.max)
    addop(dfa,"JGT", "L"..state.."_"..i+step.gt)
    addop(dfa,"CMP",step.min)
    addop(dfa,"JGE",Jlbl)
  end
  return
end

function yrx.asm(dfa,doopt)
  local pgm = {}
  local lbl,tmp
  local state
  local l
  local n
  local step
  local t
  local first
  
  if doopt == nil then doopt = true end
  
  dfa.pgm  = pgm
  dfa.lbls = {}
  dfa.leqv = {}
  dfa.linv = {}
  dfa.capt = {}
  dfa.jxx  = {}
  dfa.jmp  = {}
  dfa.blks = {}
  dfa.lblk = {}

  dfa.blks[1] = 1
  dfa.lblk["START"] = 1
    
  dfa.pgm[0] = {pc=0, op="XXX"}
  
  n,arc = nxtarc(dfa,0)
  
  addop(dfa,"DIM", 0)
  addop(dfa,"NRX", dfa.nrx)
  for n=1,dfa.nrx do
    addop(dfa,"NCP", 0)
  end
  
  while arc do
    state = arc.from
    addlbl(dfa,"S"..state)
    tags = {}
  
    if arc.to == 0 then
      addtags(dfa,arc)
      n,arc = nxtarc(dfa,n)
    end
    if arc and arc.from == state then
      first = n
      addop(dfa,"GET")
      tmp = {}
      -- {{
      while arc and arc.from == state do
        if arc.lmin == nil then
          print("UNEXPECTED!!",arc.from, arc.to)
        end
        tmp[#tmp+1] = {min=arc.lmin, max=arc.lmax, eq=n}
        n,arc = nxtarc(dfa,n)
      end
      -- arrange lbl as reading on an bin tree
      for i,step in ipairs(treendx(1,#tmp,tmp,{})) do
        addcmp(dfa,state,i,step)
      end
      tmp = nil
      -- }}
      for i = first, n-1 do
        if dfa.arcs[i].tagsig ~= "" then
          addlbl(dfa,"A"..i)
          addtags(dfa,dfa.arcs[i])
          addop(dfa, "JMP", "S"..dfa.arcs[i].to)
        end
      end
    else
      addop(dfa, "RET")
    end
  end
  
  dfa.linv = nil
  dfa.leqv = nil

  dfa.pgm[1].arg = dfa.pgm[#dfa.pgm].pc + oplen(dfa.pgm[#dfa.pgm].op)
  
  -- fix capture index
  dfa.capt[0] = 0
  for i = 1,dfa.nrx do
    dfa.capt[i] = dfa.capt[i-1] + dfa.ncp[i]*2
  end
  for i=1,dfa.nrx do
    dfa.pgm[i+2].arg = dfa.ncp[i]
  end

  dfa.blks[#dfa.blks+1] = #dfa.pgm
  
  -- fix labels
  for lbl,i in pairs(dfa.lbls) do
    if ((dfa.jmp[lbl] or 0) > 0) or ((dfa.jxx[lbl] or 0) > 0) then
      dfa.pgm[i].lbl = lbl
    else
      dfa.lbls[lbl] = nil
    end
  end
  
  if doopt then optimize(dfa) end
  
  return dfa
end

local function dumpr (rxs,opts)
 local dfa,err

 dfa,err = yrx.dfa(rxs)

 if dfa then
   yrx.asm(dfa,opts["-asm"] ~= "n")
   out(dfa)
 end

 return err
end

yrx["-asm"] = function (rxs,opts)
 if rxs == 0 then
   return [[
-asm[:n]  [ptrns]
       generate ASM-like program. Withou optimization if "n" is specified
       
]]
 end
 return dumpr(rxs,opts)
end


package.loaded[pkgname] = yrx
return yrx
