yrx = require "yrxlua"

mplexer,err = loadfile("mp_lex.lua")
  
if (err) then
  mplexer,err = yrx.scanner {
     { "#[^%n]*%n" ,
          function (x) print("COMMENT",x) end 
     },
     { "%$([a-z0-9_%-]+)" ,
          function (x,mname) print("MACRO",mname) end 
     },
     { "mrnd%$([a-z0-9_%-]+)(%([^%)]*%))" ,
          function (x,name,mlist) print("MACRORND",name,mlist) end 
     },
     { "m%$([a-z0-9_%-]+)(%([^%)]*%))" ,
          function (x,name,def) print("MACRODEF",name,def) end 
     },
     { "i(%d+|%l[a-z0-9])" ,
          function (x,instr) print("INSTRUMENT",instr) end 
     },
     { "=+" ,
          function (x) end 
     },
     { "%-+" ,
          function (x) end 
     },
     { "%*(%d+)" ,
          function (x) end 
     },
     { "ch(%d+)" ,
          function (x) end 
     },
     { "ctrl(%d+),(%d+)" ,
          function (x) end 
     },
     { "debug(max)?" ,
          function (x) end 
     },
     { "resolution(%d+)" ,
          function (x) end 
     },
     { "loose(%d+),(%d+|g)" ,
          function (x) end 
     },
     { "velvar(%d+),(%d+|g)" ,
          function (x) end 
     },
     { "globaloose(%d+),(%d+|g)" ,
          function (x) end 
     },
     { "globalvelvar(%d+),(%d+|g)" ,
          function (x) end 
     },
     { "duty(%d+)" ,
          function (x) end 
     },
     { "t(%I)" ,
          function (x) end 
     },
     { "soft(%d+)" ,
          function (x) end 
     },
     { "stress(%d+)" ,
          function (x) end 
     },
     { "strum(%d+),(%d+),(%d+)" ,
          function (x) end 
     },
     { "pitch(%I)" ,
          function (x) end 
     },
     { "tempo(%d+)" ,
          function (x) end 
     },
     { "velocity(%d+)" ,
          function (x) end 
     },
     { "u(%d+)" ,
          function (x) end 
     },
     { "v(%d+)" ,
          function (x) end 
     },
     { "sysex([0-9,]+)" ,
          function (x) end 
     },
     { "%[(g:)?([a-g][#b+%-]?)([^%]]*)%]" ,
          function (x) end 
     },
     { "%[(g:)?([0-9,%-]+)%]" ,
          function (x) end 
     },
     { "globalguiton" ,
          function (x) end 
     },
     { "guit(on|off)" ,
          function (x) end 
     },
     { "toms(on|off)" ,
          function (x) end 
     },
     { "tuning%[([a-g0-9,]+)%]" ,
          function (x) end 
     },
     { "%|(%d*)" ,
          function (x) end 
     },
     { "([',o]?)(([a-g])([-+#b]?)(%d*)|n(t)?([+%-]?)%d+)(/(%d*))?" ,
          function (x) end 
     },
     { "o(%d*)(/(%d+))?" ,
          function (x) end 
     },
     { "x" ,
          function (x) end 
     },
     { "p(/(%d+))?" ,
          function (x) end 
     },
     { "r(%d+)(/(%d+))?" ,
          function (x) end 
     },
     { "/+" ,
          function (x) end 
     },
     { "%\\+" ,
          function (x) end 
     },
     { "%d+" ,
          function (x) end 
     },
     { "%s+" ,
          function (x) end }
   }
   
   if err then error(err) end
  
   yrx.scansave(mplexer,"mp_lex.lua")
end 
f = io.open (arg[1]) 
assert(f,"unable to open" .. arg[1])

buf = string.lower(f:read("*all") .. "\n")
f:close()
mplexer(buf)
