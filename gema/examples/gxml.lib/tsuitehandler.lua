
-- .%% Writing

function fn_write(str)
  if vgb_skip then return end
  
  if vgs_file and vgs_string then
    xml.error(xml.line(),xml.column(),"Possibly unclosed tag\n")
    os.exit(1)
  end

  if vgs_file then
    vgs_file:write(str)
  elseif vgs_string then
    vgs_string=vgs_string .. str
  end
end

function fn_openout()
  if vgb_skip then return end

  if vgs_file then
    xml.error(xml.line(),xml.column(),"Output file already open!\n")
    os.exit(1)
  end
  vgs_file=io.open(vgs_runname,"a")
  if not vgs_file then
    xml.error(xml.line(),xml.column(),"Cannot generate test script file\n")
    os.exit(1)
  end
end

vgt_skips={n=0}

function fn_chkskip(sys)
  local vlb_skip=nil

  if sys then
    vlb_skip = xml.elements(1) and vgt_skips[vgt_skips.n] or ((sys ~= "") and (sys ~= vgs_sys))
    table.insert(vgt_skips,vlb_skip)
  else
    table.remove(vgt_skips)
  end
  vgb_skip=vgt_skips[vgt_skips.n]
end

-- .%% <testsuite>

function xml.open.testsuite()
  vgs_file=nil
  vgs_string=nil
  vgi_test=1
  vgs_runname="unnamed"
  vgb_skip=nil
  vgs_sys=gel.get("s")
end

function xml.close.testsuite()
  if vgs_file then
    vgs_file:close()
    vgs_file=nil
  end
  os.exit(0)
end

-- .%% <run>

function xml.open.run()
  if not vgb_skip then
    fn_openout()
    vgb_chk=((xml.attr("chk") or "") == "yes")
    xml.entity("tst",xml.attr("tst") or "")
    xml.entity("res",xml.attr("res") or "")
  end
end

function xml.close.run(name)
  local vls_chk=xml.entity("chk")
  if name == "run" and vgb_chk and vls_chk and not vgb_skip then
    vls_chk=string.gsub(vls_chk,"&([^;]+);",xml.entity);
    fn_write(vls_chk.."\n")
  end
  if vgs_file then
    vgs_file:close()
    vgs_file=nil
  end
end

-- .%% <file>

function xml.close.file(name)
  xml.close.run(name)
end

function xml.open.file(name)
  local n
  if not vgb_skip then 
    if vgs_file then
      xml.error(xml.line(),xml.column(),"Output file already open\n")
      os.exit(1)
    end
    n=xml.attr("f")
    if not n then
      xml.error(xml.line(),xml.column(),"Unspecified output file name\n")
      os.exit(1)
    end
    vgs_file = io.open(n,"w")
    if not vgs_file then
      xml.error(xml.line(),xml.column(),"Cannot generate file \"" .. n .. "\"\n")
      os.exit(1)
    end
  end
end

-- .%% <test>

function xml.open.test(name)
  xml.entity("desc","")
  xml.entity("obj","")
  if not vgb_skip then
    xml.entity("num",string.format("%03d",vgi_test))
    vgi_test=vgi_test+1
  end
end

function xml.close.test(name)
  vgs_string = nil
  xml.entity("num","")
  if vgs_file then
    vgs_file:close()
    vgs_file=nil
  end
end

-- .%% <name>

function xml.close.name(name)
  if not vgb_skip then
    vgs_runname=vgs_string
    os.remove(vgs_runname)
  end
  vgs_string = nil
end
  
-- .%% <def>

function xml.open.def(name)
  if not vgb_skip then 
    vgs_def = xml.attr("name")
  end
  if vgs_def then
    vgs_string=""
  end
end

function xml.close.def(name)
  if vgs_def and not vgb_skip then
    xml.entity(vgs_def,vgs_string)
  end
  vgs_string = nil
  vgs_def=nil
end

-- .%% <???>

function xml.opentag(name)
  fn_chkskip(xml.attr("sys") or "")
  if name == "desc" or
     name == "obj"  or
     name == "chk"  or
     name == "name" then  
      vgs_string = ""
  end
end

function xml.closetag(name)
  fn_chkskip()
  if not vgb_skip then
    if name == "desc" or name == "obj" or name == "chk" then  
      xml.entity(name,vgs_string)
    end
  end
  vgs_string = nil
end

-- .%% Entities

function xml.entref(ent)
  local s
  s=xml.entity(ent)
  if s then 
    fn_write(s)
  else
    fn_write("&"..ent..";?")
  end
end

function xml.chars(str)
  fn_write(str)
end

function xml.error(line,col,msg)
  io.write("err: ",line,",",col," ",msg,"\n")
end

