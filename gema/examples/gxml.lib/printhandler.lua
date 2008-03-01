function xml.open.testsuite()
  io.write("It's a testsuite!\n")
end

function xml.close.testsuite()
  io.write("It was a testsuite!\n")
end

function xml.opentag(name)
  local n
  
  io.write("open tag: "..(name or "??"))

  for i=1,xml.attr() do
    n=xml.attr(i)
    io.write(" ["..n.."='"..xml.attr(n).."']")
  end
  
  io.write("\n")
end

function xml.closetag(name)
  io.write("close tag: "..(name or "??").."\n")
end

function xml.startdoc()
  io.write("START DOCUMENT\n")
end

function xml.enddoc()
  io.write("END DOCUMENT\n")
end

function xml.comment(str)
end

function xml.doctype(root)
  io.write("doctype is: "..(root or "??").."\n")
end

function xml.proc(name)
  local n
  io.write("process as "..(name or "??"))
  for i=1,xml.attr() do
    n=xml.attr(i) or "??"
    io.write(" ["..n.."='"..(xml.attr(n) or "!!").."']")
  end
  
  io.write("\n")
end

function xml.entref(ent)
  io.write("entity: "..(xml.entity(ent) or "??") .."\n")
end

function xml.chars(str)
  io.write("text: "..str)
  io.write("--\n")
end

function xml.error(line,col,msg)
  io.write("err: ",line,",",col," ",msg,"\n")
end

