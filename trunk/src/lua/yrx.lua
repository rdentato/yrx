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

local function usage()
  local fmt
  local out
  local use =[[
  usage: yrx [opts] [args]
    Options:
      -i:infile       take input from the specified file 
      -o:outfile      output goes to the specified file
      -a:outfile      output is appended to the specified file
      --              stop processing options
]]
  out={}
  for fmt in pairs(yrx) do 
    if string.sub(fmt,1,1) == "-" then
      out[#out+1] = yrx[fmt](0)
    end
  end
  table.sort(out)
  return use .. "      " .. table.concat(out,'\n      ')

end

--[ [ 
yrx = require("yrxgraph")
yrx = require("yrxasm")
yrx = require("yrxc")
yrx = require("yrxl")
--]]

--yrx=require("yrxlua")

opts = {}
while #arg > 0 do
  opt = nil
  optarg = nil
  string.gsub(arg[1],"^(%-[^:=]+)[:=]?(.*)",
      function (a,b)
        opt=a; optarg = b or ""
      end,1)
  if opt == nil then break  end
  table.remove(arg,1)
  if opt == "--" then break end

  if opt == '-o' or opt == '-a' then
    if opt == '-a' then opt = "a" else opt = "w"  end
    outf = io.open(optarg,opt)
    if outf then 
      io.output(outf)
    else
      err = "YRX ERROR: Unable to open '" .. optarg .. "' for output"
    end
  elseif opt == '-i' then
    inf = io.open(optarg,"r")
    if inf then 
      io.input(inf)
    else
      err = "YRX ERROR: Unable to open '" .. optarg .. "' for input"
    end
  else
    if yrx[opt] then  out = yrx[opt]  end
    opts[opt] = optarg
  end
end

if not err then
  if out then
    err = out(arg,opts)
  else
    err = usage()
  end
end

if err then  io.stderr:write(err) end

if outf then outf:close() end
if inf then inf:close() end
