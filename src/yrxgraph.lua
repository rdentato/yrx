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

local pkgname = "yrxgraph"

module(pkgname,package.seeall)

yrx = require("yrxdfa")
local cltab = { ["\""] = "\\\"", ["\\"] = "\\\\", ["{"] = "\\\\x7B", ["}"] = "\\\\x7D" }
local function clean(s)
  return (string.gsub(s,"([\"\\{}])",cltab))
end

local function dfadot(dfa)

  local arc,n,j,k
  local state = -1  
  n=1
  io.write("// Generated by YRX \n\n")
  
  -- while dfa.rxs[n] do
  --   io.write(string.format("// %2d: %s\n",n,dfa.rxs[n]))
  --   n=n+1 
  -- end
  
  io.write("\n")
  io.write("digraph finite_state_machine {\n")
  io.write("\trankdir=LR;\n")
  --io.write("\tnode [shape = doublecircle];\n")
  --io.write("\t0 [label = \"0\" ]; \n")
  io.write("\tnode [shape = circle];\n")
  
  for n,arc in ipairs(dfa.arcs) do
    if arc.to == 0 then
      io.write("\t",arc.from," [shape=box, style=rounded, peripheries=2, label=\"",arc.from)
    else
      io.write("\t",arc.from," -> ",arc.to," [ label = \"")
      io.write(clean(yrx.lbl2str(arc)))
    end
    string.gsub(arc.tagsig, "%d+", function (t)
                                 io.write(", ",yrx.tagname(t))
                               end)
    io.write("\" ] ;\n")
  end
  io.write("}\n")
end

local function dfavcg(dfa)

  local arc,n
  local state = -1  
  n=1
  io.write([[
graph: {
  node_alignment: center
  orientation: left_to_right
  node.shape: circle
  display_edge_labels: yes
  splines: yes
  finetuning: yes
]])
    
  for n,arc in ipairs(dfa.arcs) do
    if state ~= arc.from then
      state = arc.from
      io.write('  node: { title: "',state,'" label: "',state)
      if arc.to == 0 then
        string.gsub(arc.tagsig, "%d+", function (t)
                                 io.write(", ",yrx.tagname(t))
                               end)        io.write('" shape: box')
      else
        io.write('"')
      end
      io.write(' }\n')
    end
  end
  for n,arc in ipairs(dfa.arcs) do
    if arc.to ~= 0 then
      io.write('  edge: { sourcename: "',arc.from,'" targetname: "',arc.to,'" label: "')
      io.write(clean(yrx.lbl2str(arc.label)))
      string.gsub(arc.tagsig, "%d+", function (t)
                                 io.write(", ",yrx.tagname(t))
                               end)
      io.write('" }\n')
    end
  end
  io.write("}\n")
end

-- == Export to udg format
local function dfaudg(dfa)

  local arc,n,cnt
  local state = -1  
  n=1
  io.write("[")
  
  for n,arc in ipairs(dfa.arcs) do
    if arc.from ~= state then
      if state ~= -1 then
        io.write("]\n  )),")
      end
      state = arc.from
      io.write('\n  l("',state,'",n("node",[a("OBJECT","',state)
      if arc.to == 0 then
        string.gsub(arc.tagsig, "%d+", function (t)
                                 io.write(", ",yrx.tagname(t))
                               end)
      end
      io.write('")')
      if arc.to ~= 0 then
        io.write(',a("_GO","circle")')    
      end
      io.write('],\n      [')
      cnt = 0
    end
    if arc.to ~= 0 then
      if cnt ~= 0 then io.write(",\n      ") end
      cnt = cnt +1
      io.write('l("',arc.from,">",arc.to,'",e("edge",[a("EDGEPATTERN","single;solid;0.5;1"),a("OBJECT","')
      io.write(clean(yrx.lbl2str(arc.label)))
      string.gsub(arc.tagsig, "%d+", function (t)
                                 io.write(", ",yrx.tagname(t))
                               end)
      io.write('")],r("',arc.to,'")))')
    end
  end
  io.write("]\n  ))\n]\n")
end

local function dfagraph(rxs,graph,opts)
  local dfa,err,l

  dfa,err = yrx.dfa(rxs)
  if dfa then
    graph(dfa)
  end
  return err
end

yrx["-graph"] = function (rxs,opts)
  if rxs == 0 then
    return [[
-g[raph]:(dot|udg|vcg) [ptrns]
        generate a graph for the dfa from patterns
          dot    Graphiviz format
          udg    uDraw format
          vcg    VCG/aiSee format
]]
  end
  if opts["-graph"] == "dot" then
    return dfagraph(rxs,dfadot,opts)
  elseif opts["-graph"] == "udg" then
    return dfagraph(rxs,dfaudg,opts)
  elseif opts["-graph"] == "vcg" then
    return dfagraph(rxs,dfavcg,opts)
  else
    return "Invalid graph format"
  end
end

yrx["-g"] = function (rxs,opts)
  if rxs ~= 0 then
    opts["-graph"]=opts["-g"]
    return yrx["-graph"](rxs,opts)
  end
end

package.loaded[pkgname] = yrx
return yrx

