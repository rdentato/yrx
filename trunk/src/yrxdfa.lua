-- .:Title    Yrx - compile
-- .:Author   Remo Dentato
-- .:Date     March 2007
-- .:Abstract:
-- ..

-- .>
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
-- ..


local pkgname = "yrxdfa"
module(pkgname,package.seeall)
yrx=yrxdfa

local function yrxerror(msg,expr,i)
  error("YRX ERROR: " .. msg .. " @" .. (expr or '') .. ":".. (i or ''))
end

local function renum(dfa)
  local l,arc,t,n
  local st = {}
 
  t = dfa.state+1
  st[0] = dfa.state
  st[1] = t
  for l,arc in ipairs(dfa.arcs) do
    if not st[arc.to] then
      t = t + 1
      st[arc.to] = t
    end
    arc.to = st[arc.to]
    arc.from = st[arc.from]
    assert(arc.from)
  end 
  t = 0
  dfa.stndx = {}
  for l,arc in ipairs(dfa.arcs) do
    arc.to = arc.to - dfa.state
    arc.from = arc.from - dfa.state
    if dfa.stndx[arc.from] then
       dfa.stndx[arc.from].num = dfa.stndx[arc.from].num +1
     else
       dfa.stndx[arc.from] = { first= l, num=1 }
     end
    if arc.to > t then t = arc.to end
  end 
  dfa.state = t+1
  return dfa
end

yrx.fail  =  0
yrx.eof   = -1
yrx.exit  = -2
yrx.enter = -3

local function gethex(s,i)
  local c,t,n
  
  n=''
  i=i+1 ;  c = string.sub(s,i,i)
  if string.find(c,"[0-9A-Fa-f]") then
    n = c
    i = i+1 ;  c = string.sub(s,i,i)
  end
  if string.find(c,"[0-9A-Fa-f]") then
    n = n .. c
    i = i+1 
  end
  i=i-1
  c = string.char(tonumber('0' .. n,16))
  
  return c,i
end

local function getoct(s,i)
  local c,t,n
  
  n = string.sub(s,i,i)
  i=i+1 ;  c = string.sub(s,i,i)
  if string.find(c,"[0-7]") then
    n = n .. c
    i = i+1 ;  c = string.sub(s,i,i)
  end
  if string.find(c,"[0-7]") then
    n = n .. c
    i = i+1 
  end
  i=i-1
  c = string.char(tonumber('0' .. n,8))
 
  return c,i
end

local function pushonce(stack,val)
  stack.seen = stack.seen or {}
  
  if not stack.seen[val] then 
    table.insert(stack,val)
    stack.seen[val] = true
  end
end

local function pop(stack)
  if #stack > 0 then
    return table.remove(stack)
  end
  return nil
end


--   y....y xxxxx
--    expr  11111 FIN  31
--    expr  11110 MRK  30
--    expr  0.... BEG
--    expr  1.... END

local function tagcode(tagname,expr,n)
  local tag = n
  if tagname == "BEG" then     tag = n
  elseif tagname == "END" then tag = 16+n
  elseif tagname == "FIN" then tag = 31
  elseif tagname == "MRK" then tag = 30
  end
  return tag + expr * 32
end

function tagdecode(tag)
  local name, expr, n  
  expr = math.floor(tag / 32)
  tag = tag - (expr *32)
  if tag == 30 then
    name = "MRK"
  elseif tag == 31 then
    name = "FIN"
  elseif tag > 16 then
    name = "END"; n = tag-16
  else
    name = "BEG"; n = tag
  end

  return name, expr,n
end

function tagname(tag)
  local name, expr, n  
  
  name,expr,n = tagdecode(tag)
  
  expr = tostring(expr)
  if n then name = name .. tostring(n)
  end
  
  return name .. "_" .. expr  
end




--[[

== Adding arcs
^^^^^^^^^^^^^^

  The Finite Automaton is represented with a list of arcs
(transitions) that is hold in the {arcs[]} table.

  To avoid adding the same arc twice, the table {arcsndx[]}
is indexed with an arc /signature/ (the concatenation of
from, to and the label).
--]]
local function arcsig(from,to,label)
  return from .. "," .. to .. "," .. label
end

local function addarc(nfa,from,to,label,tag,pos)
  local arc
  local ndx
  
  -- Final states are marked by a transition to 
  -- state 0 labeled with the |FINAL| string
  -- and tagged with |FIN_|/x/ where /x/ is the number of
  -- expression matched
  
  ndx = arcsig(from,to,label)
  arc = nfa.arcsndx[ndx]
  if arc == nil then  -- Create a new arc
    arc = {from=from, to=to, label=label, tags={}}
    nfa.arcsndx[ndx] = arc
    table.insert(nfa.arcs, (pos or #nfa.arcs+1), arc)
  end
  if tag then -- Add {tag} to the arc
    arc.tags[tag] = tag
  end
  return arc
end




local nolbl = {["*"] = true, ["("] = true, [")"] = true, ["-"] = true,
               ["?"] = true, ["+"] = true, ["|"] = true, [""]  = true}
               
local function getlabel(rx,n,i)
  local c,lbl,j
  
  c = string.sub(rx,i,i)
  
  if nolbl[c] then yrxerror("Unexpected character '"..c.."'",n,i) end
  if c == '\\' then
    i=i+1
    c = string.sub(rx,i,i) 
    if c == 'x' then
      lbl,i = gethex(rx,i)
    elseif '0' <= c and c <= '7' then
      lbl,i = getoct(rx,i)
    elseif c == 'b' then lbl = '\b'
    elseif c == 't' then lbl = '\t'
    elseif c == 'n' then lbl = '\n'
    elseif c == 'f' then lbl = '\f'
    elseif c == 'v' then lbl = '\v'
    elseif c == 'r' then lbl = '\r'
    else 
      lbl = '\\' .. c
    end
  elseif c == '^' then
      lbl = '\2'
  elseif c == '$' then
      lbl = '\3'
  elseif c == '[' then
    j = i
    while c ~= ']' do
      if c == '' then yrxerror("Unterminated character class",n,i) end
      if c == '\\' then i=i+1 end
      i=i+1
      c = string.sub(rx,i,i)
    end
    if j+1 == i then yrxerror("Empty character class",n,i) end
    lbl = string.sub(rx,j,i)
  else 
    lbl = c
  end
  if lbl then
    i=i+1
    if lbl == '-' then lbl = '\-' end
  end
  return lbl,i
end


local function nongreedy(l,rx,i)
  local c
  c = string.sub(rx,i,i)
  while c == ')' or c == '(' do
    i=i+1
    c = string.sub(rx,i,i)
  end

  if c == "" then
    c = "?"
  else
    c,i = getlabel(rx,n,i)
    l = "-" .. string.format("%03d",string.len(c)) .. c .. l
    c = "*"
  end
  return l,c
end

--[[ = Expression parsing
  The {parse()} function creates the NFA corresponding to
a single expression.
  If an NFA is passed as first argument the new arcs will
be appended to state 1 and states will be numbered properly,
otherwise a new NFA is created.
--]]

local function parse(nfa, rx, n)

  local i=1
  local k,c,j,f,_
  local from,to,tag,l
  local capt = {}
  local case = 's'
  local escchr = '\\\\'
  
  capt.stack= {}
  capt.max = 1
  capt.beg = {}
  capt.alt = {}
    
  if nfa == nil then  -- Create a new NFA
    nfa = {}
    nfa.arcs = {}
    nfa.arcsndx = {}
    nfa.state = 1
  end
  
  c = string.sub(rx,i,i)
  from = 1
  while c ~= "" do
    nfa.state = nfa.state+1
    to = nfa.state
    if c == '(' then
      if #capt.stack == 9 then
         yrxerror("Too many captures",n,i)
      end
      tag = tagcode("BEG",n, capt.max)
      capt.stack[#capt.stack+1]=capt.max
      capt.beg[capt.max] = to
      capt.alt[capt.max] = {}
      capt.max = capt.max +1
      addarc(nfa,from,to, '', tag)
      i=i+1
    elseif c == ')' then
      j = capt.stack[#capt.stack]
      if j == nil then
         yrxerror("Unexpected character ')'",n,i)
      end
      if #capt.alt[j] then
        addarc(nfa,from,to,'')
        for f = 1,#capt.alt[j] do
          addarc(nfa,capt.alt[j][f],to,'')
        end
        from = to
        nfa.state = nfa.state+1
        to = nfa.state
      end
      capt.stack[#capt.stack]=nil
      tag = tagcode("END",n, j)
      addarc(nfa,from,to, '', tag)
            
      i=i+1
      c = string.sub(rx,i,i) 
      if c == '?'  then
        i = i+1
        addarc(nfa,capt.beg[j],from, '')
      end
    elseif c == '|' then
      j = capt.stack[#capt.stack]
      if j == nil then yrxerror("Unexpected character '|'",n,i)  end
      table.insert(capt.alt[j], from)
      nfa.state = nfa.state - 1
      to = capt.beg[j]
      i = i+1
    --elseif c == '?' or c == '*' or c == '+' or c == '-' then
    --  yrxerror("Unexpected character '"..c.."'",n,i)
    else
      l,i=getlabel(rx,n,i)
      c = string.sub(rx,i,i) 
      
      if l == '' then
        addarc(nfa,from,to, '',tag)
      elseif l == '\\:' then
        addarc(nfa,from,to, '', tagcode("MRK",n)) 
      elseif l == '\\I' then
        addarc(nfa,from,nfa.state, "[+-]" )
        addarc(nfa,from,nfa.state+1, "\\d" )
        addarc(nfa,nfa.state,nfa.state+1, "\\d" )
        addarc(nfa,nfa.state+1,nfa.state+1, "\\d" )
        nfa.state = nfa.state+1
        to = nfa.state
        if c == '?' then addarc(nfa,from,to, '', nil) ; i=i+1  end
      elseif l == '\\N' then
        addarc(nfa,from,nfa.state, '\n')
        --addarc(nfa,from,nfa.state, '\0')
        addarc(nfa,from,nfa.state+1, '\r')
        addarc(nfa,nfa.state+1,nfa.state, '\n')
        addarc(nfa,nfa.state+1,nfa.state+2, '')
        addarc(nfa,nfa.state,nfa.state+2, '')
        nfa.state = nfa.state + 2
        to = nfa.state
        if c == '?' then addarc(nfa,from,to, '', nil) ; i=i+1  end
      elseif l == '\\Q' then
        addarc(nfa,from,nfa.state, '"')
        addarc(nfa,nfa.state,nfa.state+1, escchr)
        addarc(nfa,nfa.state+1,nfa.state, '.')
        addarc(nfa,nfa.state,nfa.state, '[^"' .. escchr .. ']')
        addarc(nfa,nfa.state,nfa.state+4, '"')
        
        addarc(nfa,from,nfa.state+2, '\'')
        addarc(nfa,nfa.state+2,nfa.state+3,  escchr)
        addarc(nfa,nfa.state+3,nfa.state+2, '.')
        addarc(nfa,nfa.state+2,nfa.state+2, '[^\''..escchr..']')
        addarc(nfa,nfa.state+2,nfa.state+4, '\'')
        nfa.state = nfa.state + 4
        to = nfa.state
        if c == '?' then addarc(nfa,from,to, '', nil) ; i=i+1  end
      elseif l == '\\E' then
        if c == '\\' then
        end
        escchr = c
        i=i+1
      elseif l == '\\e' then
        addarc(nfa,from,to, escchr )
        addarc(nfa,to,nfa.state+1, "." )
        nfa.state = nfa.state+1
        to = nfa.state
        l= '[^' .. escchr .. ']'
        if c == '-' then
          l,c=nongreedy(l,rx,i+1) 
        end
        if c == '?' then
          addarc(nfa,from,to, '')
          i=i+1
        elseif c == '+' then
          addarc(nfa,to,to, l)
          addarc(nfa,to,to-1, escchr)
          i=i+1
        elseif c == '*' then
          addarc(nfa,from,to, '')
          addarc(nfa,to,to, l)
          addarc(nfa,to,to-1, escchr)
          i=i+1
        end
        addarc(nfa,from,to, l)
      else     
        if c == '-' then
          l,c = nongreedy(l,rx,i+1)
        end
        if c == '*' then
          addarc(nfa,from,to, '')
          addarc(nfa,to,to, l)
          i=i+1
        elseif c == '+' then
          addarc(nfa,to,to, l)
          i=i+1
        elseif c == '?' then
          addarc(nfa,from,to, '')
          i=i+1
        end
        addarc(nfa,from,to, l, nil) 
      end
    end
    c = string.sub(rx,i,i)
    from = to
  end
  j=capt.stack[#capt.stack]
  if j then
    yrxerror("Unclosed capture " .. j,n)
  end
  addarc(nfa,nfa.state,0, "FINAL",  tagcode("FIN",n))
  nfa.ncp = nfa.ncp or {}
  nfa.ncp[n] = capt.max
  return nfa
end

-- Merge arcs


local function addlbl(dest,l,arcnum,excl)
  dest[l]= dest[l] or {}
  if not (excl and excl[l]) then
    --print("//$1",excl)
    table.insert(dest[l],arcnum)
  end
end

local function addrange(dest,a,z,arcnum,excl)
  local i
  for i=string.byte(a),string.byte(z) do 
    --print("//$2",excl)
    addlbl(dest,string.char(i),arcnum,excl)
  end
end

local function nxtchr(c)
  local i = string.byte(c)+1
  if i > 255 then i = 255 end
  return string.char(i)
end

local function prvchr(c)
  local i = string.byte(c)-1
  if i < 0 then i = 0 end
  return string.char(i)
end

local function thechr(i)
  if i > 255 then i = 255 end
  if i < 0 then i = 0 end
  return string.char(i)
end

local function expandcls(dest,label,arcnum,excl)
  local c,i,l,t,neg,rng
  neg = false
  rng = false
  t = {}
  i = 2
  c = string.sub(label,i,i)
  if c == '^' then
    neg = true
    i = i+1
    c = string.sub(label,i,i)
  end
  while c ~= ']' do
    if c == '-' then
      rng = true
    else
      if c == '\\' then
        i = i+1
        c = string.sub(label,i,i)
        if c == 'x' then
          c,i = gethex(label,i)
        elseif '0' <= c and c <= '7' then
          c,i = getoct(label,i)
        elseif c == 'b' then c = '\b'
        elseif c == 't' then c = '\t'
        elseif c == 'n' then c = '\n'
        elseif c == 'f' then c = '\f'
        elseif c == 'v' then c = '\v'
        elseif c == 'r' then c = '\r'
        end
      end
      if rng then
        if l == nil then t[string.byte('-')] = true; l=c end
        for j = string.byte(l),string.byte(c)-1 do
          t[j] = true
        end
        rng = false
        l = nil
      end
      t[string.byte(c)] = true
      l = c
    end
    i = i+1
    c = string.sub(label,i,i)
  end
  if rng then t[string.byte('-')] = true; end
  for c=0,255 do
    if (t[c] and not neg) or (not t[c] and neg) then
      addlbl(dest,string.char(c),arcnum,excl) 
    end    
  end
end

local function expandesc(dest,label,arcnum,excl)
  local c
  
  c=string.sub(label,2,2)
  if c == 'l' then
    addrange(dest,'a','z',arcnum,excl)
  elseif c == 'L' then
    addrange(dest,thechr(0),prvchr('a'),arcnum,excl)
    addrange(dest,nxtchr('z'),thechr(255),arcnum,excl)
  elseif c == 'u' then
    addrange(dest,'A','Z',arcnum,excl)
  elseif c == 'U' then
    addrange(dest,thechr(0),prvchr('A'),arcnum,excl)
    addrange(dest,nxtchr('Z'),thechr(255),arcnum,excl)
  elseif c == 'a' then
    addrange(dest,'a','z',arcnum,excl)
    addrange(dest,'A','Z',arcnum,excl)
  elseif c == 'A' then
    addrange(dest,thechr(0),prvchr('A'),arcnum,excl)
    addrange(dest,nxtchr('Z'),prvchr('a'),arcnum,excl)
    addrange(dest,nxtchr('z'),thechr(255),arcnum,excl)
  elseif c == 'w' then
    addrange(dest,'0','9',arcnum,excl)
    addrange(dest,'a','z',arcnum,excl)
    addrange(dest,'A','Z',arcnum,excl)
    addlbl(dest,'_',arcnum)
  elseif c == 'W' then
    addrange(dest,thechr(0),prvchr('0'),arcnum,excl)
    addrange(dest,nxtchr('9'),prvchr('A'),arcnum,excl)
    addrange(dest,nxtchr('Z'),prvchr('_'),arcnum,excl)
    addrange(dest,nxtchr('_'),prvchr('a'),arcnum,excl)
    addrange(dest,nxtchr('z'),thechr(255),arcnum,excl)
  elseif c == 'd' then
    addrange(dest,'0','9',arcnum,excl)
  elseif c == 'D' then
    addrange(dest,thechr(0),prvchr('0'),arcnum,excl)
    addrange(dest,nxtchr('9'),thechr(255),arcnum,excl)
  elseif c == 'h' then
    addrange(dest,'0','9',arcnum,excl)
    addrange(dest,'a','f',arcnum,excl)
    addrange(dest,'A','F',arcnum,excl)
  elseif c == 'H' then
    addrange(dest,thechr(0),prvchr('0'),arcnum,excl)
    addrange(dest,nxtchr('9'),prvchr('A'),arcnum,excl)
    addrange(dest,nxtchr('F'),prvchr('a'),arcnum,excl)
    addrange(dest,nxtchr('f'),thechr(255),arcnum,excl)
  elseif c == 'o' then
    addrange(dest,'0','7',arcnum,excl)
  elseif c == 'O' then
    addrange(dest,thechr(0),prvchr('0'),arcnum,excl)
    addrange(dest,nxtchr('7'),thechr(255),arcnum,excl)
  elseif c == 's' then
    addrange(dest,thechr(8),thechr(13),arcnum,excl)
    addlbl(dest,' ',arcnum,excl)
  elseif c == 'S' then
    addrange(dest,thechr(0),thechr(7),arcnum,excl)
    addrange(dest,thechr(14),prvchr(' '),arcnum,excl)
    addrange(dest,nxtchr(' '),thechr(255),arcnum,excl)
  elseif c == 'y' then
    addlbl(dest,' ',arcnum,excl)
    addlbl(dest,'\t',arcnum,excl)
  elseif c == 'Y' then
    addrange(dest,thechr(0),prvchr('\t'),arcnum,excl)
    addrange(dest,nxtchr('\t'),prvchr(' '),arcnum,excl)
    addrange(dest,nxtchr(' '),thechr(255),arcnum,excl)
  elseif c == 'b' then
    addlbl(dest,'\b',arcnum,excl)
  elseif c == 't' then
    addlbl(dest,'\t',arcnum,excl)
  elseif c == 'n' then
    addlbl(dest,'\n',arcnum,excl)
  elseif c == 'f' then
    addlbl(dest,'\f',arcnum,excl)
  elseif c == 'v' then
    addlbl(dest,'\v',arcnum,excl)
  elseif c == 'r' then
    addlbl(dest,'\r',arcnum,excl)
  else 
    addlbl(dest,c,arcnum,excl)
  end    
end

local function expandlbl(dest,label,arcnum)
  local c,i,excl,l
  
  c=string.sub(label,1,1)
  --print("// =",c,label)  
  --os.exit(2)
  -- Non greedy operator - is represented with a composite label:
  --  |-xxxxl1l2|
  -- where the 4 characters xxxxx are the ASCII representation of
  -- the length of l1
  if c == "-" then
     excl = {}
     i=5+tonumber(string.sub(label,2,4),10)
     l=string.sub(label,5,i-1)
     label = string.sub(label,i,-1)
     --print("// £",i,l,label)  
     expandlbl(excl,l,arcnum)
  end
  
  c=string.sub(label,1,1)
  if c == '\\' then
    if label == '\\e' or label == '\\Q' or label == '\\I' or
       label == '\\:' or label == '\\E' or label == '\\N' then
      yrxerror("Invalid use of '"..label.."'")
    end
    expandesc(dest,label,arcnum,excl)
  elseif c == '[' then
    expandcls(dest,label,arcnum,excl)
  elseif c == '.' then
    addrange(dest,thechr(0),thechr(255),arcnum,excl)
  else
    addlbl(dest,label,arcnum,excl)
  end
end

-- create a unique signature for a set of tags
local function tagsig(tags)
  local t
  local tl = {}
  
  for t in pairs(tags) do tl[#tl+1] = t end
  table.sort(tl)
  return table.concat(tl,",")
end

local function compresslbl(lbls)
  local i,j,l,ll
  
  if #lbls == 0 then return nil end
  
  ll = {}
  
  table.sort(lbls)
  lbls[#lbls+1] = 255+2
  lbls[#lbls+1] = 255+4
  
  ll[1] = lbls[1]
  ll[2] = ll[1]  
  i=2
  j=2
  while lbls[j] <= 255 do
    while lbls[j] == (lbls[j-1] + 1) do
      j = j+1
    end
    ll[i] = lbls[j-1]
    if lbls[j] <= 255 then
      ll[i+1] = lbls[j]
    end
    i=i+2
    ll[i] = ll[i-1]
    j=j+1
  end
  --ll=table.concat(ll)
  return ll
end

local function compressarcs(arcs)
  local s,t,i,a,l,k
  local cmpr = {}
  local tags = {}
  local from
  
  -- arcs that have the same from, to and tags, can be merged
  -- in a single arc having a set of labels instead of a single
  -- label
  
  for i,a in pairs(arcs) do
    from = a.from
    if a.label == "FINAL" then 
      a.label = ""
      for t,s in pairs(a.tags) do
        if (t - math.floor(t / 32) * 32) ~= 31 then
          a.tags[t] = nil
        end
      end
    end
    s = tagsig(a.tags)
    tags[s] = a.tags
    cmpr[a.to] = cmpr[a.to] or {}
    cmpr[a.to][s] = cmpr[a.to][s] or {}
    if a.label ~= "" then 
      table.insert(cmpr[a.to][s], string.byte(a.label))
    end
  end
  
  arcs = {}
  for t,s in pairs(cmpr) do
    for i,a in pairs(s) do
      l = compresslbl(a) 
      if l == nil then
        table.insert(arcs,{from=from,to=t, tagsig=i}) --tags=tags[i],
      else
        for k=1,#l,2 do
          table.insert(arcs,{from=from, to=t, lmin = l[k], lmax = l[k+1], tagsig=i}) --tags=tags[i],
        end
      end
    end
  end  
  table.sort(arcs,function (a,b)
                    return a.to == 0 or 
                          (b.to ~= 0 and a.lmax < b.lmin)
                  end)
  return arcs
end

local function chkmerged(nfa,set)
  local _,i,t,j,a
  local to={}
   
  for i in pairs(set) do
    table.insert(to,i)
  end
  
  table.sort(to)
  t = table.concat(to,",")
   
  if nfa.merged[t] then 
    return nfa.merged[t]
  end
  
  nfa.state = nfa.state+1
  nfa.merged[t] = nfa.state
  nfa.invmrg[nfa.state] = to 
  
  nfa.fromtab[nfa.state] = #nfa.arcs+1
  
  -- set arcs
  for _,i in ipairs(to) do
    j = nfa.fromtab[i];
    a = nfa.arcs[j];
    while a.from == i do
      a = addarc(nfa,nfa.state,a.to,a.label)
      for t in pairs(nfa.arcs[j].tags) do a.tags[t] = t end
      j = j+1
      a = nfa.arcs[j];
    end
  end
  return nfa.state
end

local function merge(nfa,l,set)
  local arc,tag 
  local to, _i, i, _j, j
  local a
  
  -- We need to create a new arc since the original arc
  -- could have been labeled with a set
  a = nfa.arcs[ set[1] ]
  arc = {from=a.from, to=a.to, label=l, tags={}}
  if #set > 1 then
    to = {}
    for _i,i in ipairs(set) do
      a=nfa.arcs[i]
      if nfa.invmrg[a.to] then
        for _j,j in ipairs(nfa.invmrg[a.to]) do
          to[j] = true;
        end
      else
        to[a.to] = true;
      end
      for tag in pairs(a.tags) do arc.tags[tag] = tag end
    end
    arc.to = chkmerged(nfa,to)
  else
    for tag in pairs(a.tags) do arc.tags[tag] = tag end
  end
  return arc
end

local function mergearcs(nfa)
  local stack = {}
  local from
  local i
  local arc
  local dest    
  local l,t
  local newarcs = {}
  local tmparcs
  
  nfa.merged = {} -- maps a set of states to the state they were merged to
  nfa.invmrg = {} -- maps a state to the states it is a merge of;
  
  pushonce(stack,1)
  from = pop(stack)
  nfa.states = {}
  while from do
    i = nfa.fromtab[from]
    arc = nfa.arcs[i]
    nfa.states[from] = #newarcs + 1
    
    dest = {}
    tmparcs = {}
    -- the table {dest} holds, for each char, the set of
    -- arcs exit from {from}labeled with that char
    while arc and arc.from == from do
       expandlbl(dest,arc.label,i)        
       i = i+1
       arc = nfa.arcs[i]
    end
    -- create the new list of arcs
    for l,t in pairs(dest) do 
      arc = merge(nfa,l,t)
      table.insert(tmparcs,arc)
      pushonce(stack,arc.to)
    end
    
    -- and squeeze together the arcs with the same from, to, tag  
    for i,t in ipairs(compressarcs(tmparcs)) do
      table.insert(newarcs,t)
      -- ensure that transition to 0 (final state) is the first
      -- transition listed.
      if t.to == 0 and nfa.states[from] ~= #newarcs then
        l= newarcs[nfa.states[from]]
        newarcs[nfa.states[from]] = t
        newarcs[#newarcs] = l
      end
    end
    from = pop(stack)  
  end
  nfa.arcs = newarcs
  nfa.merged = nil
  nfa.invmrg = nil
  nfa.fromtab = nil
  nfa.states = nil
  
  return nfa
end


--[[ = Determinize
  This step transform the NFA to a DFA by firstly removing
epsilon transitions ({removeeps()}) and then merging
transitions from a state that have the same label {mergearcs()}.
--]]

-- == Removing eps-transitions

local function delarc(nfa,i)
  local arc = nfa.arcs[i]
  nfa.arcsndx[arcsig(arc.from,arc.to, arc.label)]= nil
  table.remove(nfa.arcs, i)
  return arc
end

local function copyarcs(nfa,i,j)
  local arc, src, new

  arc = delarc(nfa,i)
  src = nfa.arcs[j]
  while src.from == arc.to do
    new = addarc(nfa,arc.from, src.to,src.label,nil,i)
    for t in pairs(arc.tags) do new.tags[t] = t end
    for t in pairs(src.tags) do new.tags[t] = t end

    i=i+1 
    j=j+1
    src = nfa.arcs[j]
  end
  
  return i-2
end

--   This function also creates {fromtab} array that groups together
-- arcs exiting from the same state. It will be used in the {mergearcs}
-- step

local function removeeps(nfa)
  local i=1
  local fromtab = {}
  local tags
  
  -- Sorting by "from" in reversed order allows us to navigate
  -- from the last state to the first one.
  table.sort(nfa.arcs,function (a,b)
                        if a.from  ~= b.from  then return a.from > b.from end      -- descending
                        if a.label ~= b.label then return a.label < b.label end    -- ascending
                        if a.to    ~= b.to    then return a.to > b.to end          -- descending
                        return false
                      end)
  
  arc = nfa.arcs[1]
  i=1
  while arc do
    -- set a pointer to the first arc from each state
    fromtab[arc.from] = fromtab[arc.from] or i
    if arc.label == '' then
      i = copyarcs(nfa,i,fromtab[arc.to])
    else
      i = i+1
    end
    arc = nfa.arcs[i]
  end
  nfa.fromtab = fromtab
  
  return nfa
end


local function determinize(nfa)
  removeeps(nfa)
  nfa = mergearcs(nfa)
  nfa.arcsndx = nil
  renum(nfa)
  return nfa 
end


local chrtab = {}

chrtab[string.byte('"')] = true
chrtab[string.byte('\\')] = true
chrtab[string.byte('-')] = true
chrtab[string.byte(']')] = true
chrtab[string.byte('[')] = true
chrtab[string.byte('\'')] = true

local function chr(c)
  if c < 33 or 127 < c or chrtab[c] then
    return string.format("\\x%02X",c)
  else
    return string.char(c)
  end
end

function lbl2str(arc)
  local i,c,l
  
  if arc.to == 0 then return '     ' end
  
  if i == 0 then return '      ' end
  if arc.lmin  == arc.lmax then return " '" .. chr(arc.lmin) .. "' " end

  return "[" .. chr(arc.lmin) .. '-' .. chr(arc.lmax) .. "]"
end

--[[
= Compiling Expressions
=======================

  This is the main function. Given a list of expressions it
returns the corresponding DFA. 

--]]

local function docompile(rxs)
  local nfa
  local i=1

  if rxs == nil or #rxs == 0 then yrxerror( "No expression to compile") end
  while rxs[i] do
    nfa = parse(nfa,rxs[i],i)
    i=i+1
    if i > 128 then yrxerror("Too many expressions (max is 128)") end
  end
  nfa.nrx = #rxs  
  return determinize(nfa)
end


function yrx.dfa (rxs)
  local res,dfa
  
  if not rxs or #rxs == 0 then
    rxs = rxs or {}
    l = io.read("*line")
    while l do
      rxs[#rxs+1] = l
      l = io.read("*line")
    end
  end
  
  res,dfa=pcall(docompile,rxs)
  if res then return dfa end
  return nil,dfa
end

--[[ yrx CLI --]]
yrx["-dump"] = function (rxs)
  local dfa,err,l,arc,t
  
  if rxs == 0 then
    return [[
-dump [ptrns]
        dump readable representation of the dfa
]]
  end
  
  dfa,err = yrx.dfa(rxs)

  if dfa then
    if opts["-states"] then
      for t,l in ipairs(dfa.stndx) do
        io.write("State: ",t," first=",l.first," num = ",l.num,"\n")
      end
    else
      for l,arc in ipairs(dfa.arcs) do
        io.write(string.format("%3d -> %-3d %s ",arc.from,arc.to,yrx.lbl2str(arc)))
        string.gsub(arc.tagsig, "%d+", function (t)
                                 io.write(", ",yrx.tagname(t))
                               end)
        io.write('\n')
      end
    end
  end
  return err
end


function yrx.dfasave(dfa)
  local k,v
  local j,l
  
  fname = fname or "yrx.dfa"
  yrx.renum(dfa)
  io.write("local dfa={}\n")
  io.write("dfa.nrx = ",dfa.nrx,"\n")
  io.write("dfa.state = ",dfa.state,"\n")
  io.write("dfa.ncp = {")
  for k,v in ipairs(dfa.ncp) do
    io.write(v,",")
  end
  io.write("}\n")
  io.write("dfa.arcs = {\n") 
  for k,v in ipairs(dfa.arcs) do
     io.write("  {from = ",v.from,", to = ",v.to,", ")
     if v.label then
       io.write("label = {")
       for j,l in ipairs(v.label) do
         io.write(l,", ")
       end
       io.write("}, ")
     end
     io.write("tagsig = '",v.tagsig,"'")
     io.write("},\n")
  end
  io.write("}\n")
  io.write("return dfa\n")
end

yrx["-save"] = function (rxs)
  local k,v  
  local dfa,err
  
  if rxs == 0 then
    return [[
-save [ptrns]
        
]]
  end
    dfa,err = yrx.dfa(rxs)
  if dfa then
    yrx.dfasave(dfa)
  end
  return err
end

yrx["-lib"] = function (rxs)
  local k,v
  
  if rxs == 0 then return "" end
    
  for k,v in pairs(yrx) do
    print(k)
  end
  return 
end

return package.loaded[pkgname]

