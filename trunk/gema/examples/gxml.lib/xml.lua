! .T XML Parsing
-- .A Remo Dentato
-- .D 15 Nov 2003
-- .I xml_g

-- .% Introduction
--    This is an event-driven parser a-la SAX. Each tag is recognized and the
-- appropriate handler is invoked.

-- .% XML handlers
--    These are the handlers that can be defined:
--
--  .[xml.chars(chars)] Called whenever a block of text is encountered. '|CDATA|
--                      sections and any sequence of characters different from
--                      '|<| and '|&| are considered as block of text.
--                      No guarantees on where the block of text will be split,
--                      so, do not assume it's the end of a word or a line.
--                      '|chars| will contain the block of text.

--  .[xml.startdoc()]   Signals that the document is just started.

--  .[xml.enddoc()]     Signals that the document is just ended. It's called when
--                      the end of the file is reached, not at the closure of the 
--                      root element.

--  .[xml.proc(dir)]    A process directive ('|<? ... ?>|). No split is done.

--  .[xml.opentag(name)] A start tag is encountered. You can access its attributes
--                      through the '|xml.attrs()| function. If you need them later
--                      you must save them yourself.
--                      The tag name is pushed in a stack so that an history of the
--                      elements encountered is kept. To access that history you 
--                      use the '|xml.elements()| function._
--                      If a function exists with assigned to '|xml.open.name| 
--                      (i.e. '|xml.open["name"]|) it is called instead.

--  .[xml.closetag(name)] Before this handler is called, the tag will be popped from 
--                      the stack. If the close tag does not match with the tag on
--                      stack, an error is issued._
--                      If a function exists with assigned to '|xml.close.name| 
--                      (i.e. '|xml.close["name"]|) it is called instead.

--  .[xml.entref(entity)] An entity ('|&'/entity/;|) has been encountered. If you
--                      call '|xml.entity('/entity/')| you get the corresponding
--                      value (see next section).
--
--  .[xml.comment(comment)] Called upon a comment ('|<---- ... -->|). The whole comment
--                      text is passed to the handler.

--  .[xml.doctype(root,dtd)] A '|<--DOCTYPE ...>| tag has been encountered. The name
--                      of the root element and the whole dtd (or reference) is
--                      returned to the handler.

--  .[xml.error(line,col,msg)] If an error is encountered, this handler will be called

-- .% XML functions
--
--   To access xml objects you can use the following functions.
--
--   .[xml.entity(ent,val)] Get the value of an entity if '|ent| is known (e.g. '|&amp;|)
--                      or it is a character entity (e.g. '|&#32;| or '|&#x20;|); nil
--                      otherwise._
--                      You can change the value associated to an entity providing a value
--                      for the '|val| argument. If you provide '|""|, the entity will be 
--                      deleted.
--
--   .[xml.attr(name)]  There are three ways of calling this function:
--
--                      .[] '|xml.attr("attrname")| _
--                          If you provide a string, it is assumed to be the
--                          name of an attribute. The corresponding value is returned
--                          or '|nil| if the attribute is not found.
--
--                      .[] '|xml.attr(number)| _
--                          If you provide a number '/n/, the name of the '/n/th
--                          attribute is returned (or '|nil| if '/n/ is too big).
--
--                      .[] '|xml.attr()| _
--                          If no argument is provided, the number of defined
--                          attributes is returned.
--
--                    _ Attributes are not saved and will be rewritten when another
--                      tag will be encountered. If you need to preserve them you must
--                      provide an ad-hoc mechanism.

--                    _ This function can be used only inside the '|xml.opentag()| handler
--                      otherwise its behavior is undefined.

--   .[xml.line()]      The current line number in the XML file.
--   .[xml.column()]    The current col number in the XML file.
--   .[xml.elements(n)] Access the elements in the stack. Use 0 to access the top element,
--                      1 for the one right below and so on.

-- .% Lua functions

  xml={}
  xml.vtag={n=0}
  xml.vstk={n=0}
  xml.open={}
  xml.close={}

-- .%% Entities

  xml.ents={}

-- .{ Default entities
--   Every XML parser must be aware of these:
  xml.ents["apos"]="'"
  xml.ents["quot"]="\""
  xml.ents["gt"]=">"
  xml.ents["lt"]="<"
  xml.ents["amp"]="&"
  
-- I also addedd these:
  xml.ents["ETH"]="Ð"
  xml.ents["eth"]="ð"
  xml.ents["Auml"]="Ä"
  xml.ents["Euml"]="Ë"
  xml.ents["Iuml"]="Ï"
  xml.ents["Ouml"]="Ö"
  xml.ents["Uuml"]="Ü"
  xml.ents["auml"]="ä"
  xml.ents["euml"]="ë"
  xml.ents["iuml"]="ï"
  xml.ents["ouml"]="ö"
  xml.ents["uuml"]="ü"
  xml.ents["yuml"]="ÿ"
  xml.ents["AElig"]="Æ"
  xml.ents["Acirc"]="Â"
  xml.ents["Aring"]="Å"
  xml.ents["Ecirc"]="Ê"
  xml.ents["Icirc"]="Î"
  xml.ents["Ocirc"]="Ô"
  xml.ents["THORN"]="Þ"
  xml.ents["acirc"]="â"
  xml.ents["Acirc"]="Â"
  xml.ents["aelig"]="æ"
  xml.ents["aring"]="å"
  xml.ents["ecirc"]="ê"
  xml.ents["icirc"]="î"
  xml.ents["ocirc"]="ô"
  xml.ents["szlig"]="ß"
  xml.ents["thorn"]="þ"
  xml.ents["ucirc"]="û"
  xml.ents["Aacute"]="Á"
  xml.ents["Agrave"]="À"
  xml.ents["Atilde"]="Ã"
  xml.ents["Ccedil"]="Ç"
  xml.ents["Eacute"]="É"
  xml.ents["Egrave"]="È"
  xml.ents["Iacute"]="Í"
  xml.ents["Igrave"]="Ì"
  xml.ents["Ntilde"]="Ñ"
  xml.ents["Oacute"]="Ó"
  xml.ents["Ograve"]="Ò"
  xml.ents["Oslash"]="Ø"
  xml.ents["Otilde"]="Õ"
  xml.ents["Uacute"]="Ú"
  xml.ents["Ugrave"]="Ù"
  xml.ents["Yacute"]="Ý"
  xml.ents["aacute"]="á"
  xml.ents["agrave"]="à"
  xml.ents["atilde"]="ã"
  xml.ents["ccedil"]="ç"
  xml.ents["eacute"]="é"
  xml.ents["egrave"]="è"
  xml.ents["iacute"]="í"
  xml.ents["igrave"]="ì"
  xml.ents["ntilde"]="ñ"
  xml.ents["oacute"]="ó"
  xml.ents["ograve"]="ò"
  xml.ents["oslash"]="ø"
  xml.ents["otilde"]="õ"
  xml.ents["uacute"]="ú"
  xml.ents["ugrave"]="ù"
  xml.ents["yacute"]="ý"

    
-- }

--   Entities values can be retrieved through the '{xml.entity()=} function.

  function xml.entity(ent,val)
    local n=nil
    local b=10
    local t=2

    if val then
      if val == "" then
        val=nil
      end
      xml.ents[ent]=val
      return val
    end
        
    if xml.ents[ent] then 
      return(xml.ents[ent])
    elseif string.sub(ent,1,1) == "#" then
      -- .{ Check for hex or dec values
      if string.sub(ent,2,2) == "x" then
        b=16 ; t=3
      end
      -- .}
      n=tonumber(string.sub(ent,t),b)
      -- .{ only consider char in the 0..255 range
      if n and (n > 0) and (n<256) then
        return(string.char(n))
      end
      -- .}
    end
    
    return(nil)
  end

  function xml.tagstart(name)
    local f=xml.open[name] or xml.ss
    f(name)
  end
  
  function xml.tagend(name)  
    local f=xml.close[name] or xml.ee
    f(name)
  end

-- .%% Start tag
--   Push the tag name in a stack before executing the handler.
--   The '|xml.open| table is searched first

  function xml.st(tag)
    table.insert(xml.vstk,tag)
    xml.tagstart(tag)
  end
    
-- .%% End tag
--   Pop the tag name form the stack and choke on misalignments
-- (or call the '|endtag| handler if the close tag matches)!

  function xml.et(tag)
    local n=xml.vstk.n
    
    if (n == 0) or (xml.vstk[n] ~= tag) then
      xml.er(xml.line(),xml.column(),"Mismatched tag <"..(tag or "??").."> ("..xml.vstk[n]..")")
    else
      table.remove(xml.vstk)
    end
    xml.tagend(tag)
  end

-- .%% Empty tag
--   An empty tag '|<x/>| is treated exactly as if it was '|<x></x>|

  function xml.xt(name)
    xml.tagstart(tag)
    xml.tagend(tag)
  end

-- .%% Attributes !attrs
--   The '{xml.attr()} function allows you to access the attributes of
-- the current element. 
--   Attributes are stored in the '{vtag=} table in pairs:
--
-- .f
--             0     1      2      3      4      5  
--          +-----+------+------+------+------+----
--    vtag  |     | name | val  | name | val  | 
--          +-----+------+------+------+------+-----
--           ____/ \___________/ \___________/ \_____
--        tagname     attr. 1       attr. 2       attr.
-- ..

  function xml.attr(name)
    local i
    
    -- .{ get '/i/th attribute name
    i=tonumber(name) or 0
    if i > 0 then
      return xml.vtag[(i-1)*2+1]
    end
    -- .}

    -- .{ map '|name| '$->$ '|value|
    if name then
      for i=1,xml.vtag.n,2 do
        if xml.vtag[i] == name then
          return xml.vtag[i+1]
        end
      end
      return nil
    end
    -- .}
    
    -- .{ return number of attributes
    return math.floor(xml.vtag.n / 2)
    -- .}
  end
  
  function xml.tagname()
    return xml.vtag[0]
  end

-- .%% Current values

-- .{ col/row
  function xml.line() return(xml.l) end
  function xml.column() return(xml.c) end
-- .}

-- .{ Element
--   The '/n/th element from the top of the stack (0=top).
  function xml.elements(n)
    return(xml.vstk[xml.vstk.n-(tonumber(n) or 0)])
  end
-- .}

-- .%% End of document
--   At the end of the document check if the stack is empty;
-- if not, there must be some missing close tag.

  function xml.ed()
    local n=xml.vstk.n
    if xml.vstk.n > 0 then
      xml.er(xml.line(),xml.column(),"Unbalanced tag at EOF ("..(xml.vstk[n] or "??") ..")")
    end
    xml.dd()
  end
        
-- .%% Error handler
--   The default error handler prints of stderr the error message.

  function xml.err(line,col,msg)
    print("ERR",line,col,msg)
  end

-- .%% Null handler
--   A "do nothing" function used as default handler

  function xml.nul() end

-- .%% Handlers check
--  Be sure that each handler is a valid function.
--  The gema rules will call 
  
  function xml.chk()
    xml.ch = xml.chars    or xml.nul
    xml.sd = xml.startdoc or xml.nul
    xml.dd = xml.enddoc   or xml.nul
    xml.sp = xml.space    or xml.nul
    xml.pr = xml.proc     or xml.nul
    xml.ss = xml.opentag  or xml.nul
    xml.ee = xml.closetag or xml.nul
    xml.en = xml.entref   or xml.nul
    xml.er = xml.error    or xml.err
    xml.cm = xml.comment  or xml.nul
    xml.dt = xml.doctype  or xml.nul
  end

function xml.parse(fname)
  local eof=nil
  
-- open file

  xml.file=io.open(fname,"r")
  if not (xml.file) then
    xml.er(0,0,"Input file \""..fname.."\" not found")
    return 100
  end

-- Init
  xml.chk()
  xml.vtag[0]=""
  xml.l=0
  xml.c=0
  xml.vtag.n=0

-- Start document
  xml.sd()

  xml.outbuf=""

  xml.state="text"
  
  for line in xml.file:lines() do

    while line do
      if xml.state == "text" then
        line=string.gsub(line,"^([^<&]+)(.*)$",function (s,t) xml.ch(s); return t end)
        if string
        if line == "" then
          xml.ch("\n")
          line = nil
        end
      elseif xml.stat="tag" then
      end
    end
  
    
  end  
  
  
@set{x;\#}

! .%% Initialize lua functions
\A=@lua{xml.chk()\;xml.sd()}@start{}
\Z=@start{}@lua{xml.ed()}

! .%% Entities

\&*\;\G=@lua{xml.en("*")}

! .%% Text
<txt>=@lua{xml.ch(@esc{$0})}

txt:\P\&=@terminate
txt:\P\<=@terminate

! .%% Process directives
!   Here we check for '|<? ... ?>| tags

\<\?<tag>\?\>\G=@lua{xml.pr("$t")}

! .%% Comments
!  The whole comment is passed back to lua
\<\!--<U>--\W\>\G=@lua{xml.cm(@esc{$1})}

! .%% Character Data
\<\![CDATA[*]]\W\>\G=@start{}@lua{xml.ch(@esc{$1})}


! .%% Document type
!   To have a validating parser we have to understand the definitions
! in the DOCTYPE element or reference them through an external DTD.
! 
!   For now, let's have a non-validating parser. Skip DOCTYPE entirely
! but retain root element name in '{t} and pass the rest to lua (just
! in case someone needs it).

\<\!DOCTYPE\W<dtype>=@lua{xml.dt("$t",@esc{$1})}

dtype:\A<I>=@start{$1}
dtype:[*]\>\G=*@end
dtype:\>=@end

! .%% Tags
!  According to XML specification (see '?http://www.w3.org?) an element
! name can only start with a letter, an underscore or a colon. 

@set-parm{idchars;-.:}

\<\/<tag>=@lua{xml.et("$t")}

\<<tag>=@lua{xml.$xt("$t")}

! .{ Get the tag name
tag:\A.=@terr{Invalid tag name}@end
tag:\A<N1>=@terr{Invalid tag name}@end
tag:\A-=@terr{Invalid tag name}@end

tag:\A<I>=@start{$1}

tag:\A?=@terr{Invalid tag name}@end
! .}

! .{ Get the attlist
tag:\W<I>\="<u>"=@attr{$1 $2}
tag:\W<I>\='<u>'=@attr{$1 $2}

attr:<I> <U>=@lua{table.insert(xml.vtag,"$1")\;table.insert(xml.vtag,@esc{$2})}@end
! .}

! .{ Check for the end of tag
!  If it's an empty element, the variable '|x| is set to "'|e|"
tag:\W\/\>=@set{x;x}@end
tag:\W\>=@end
! .}

tag:=@terr{Unexpected character}

terr:*=@lua{xml.er(@line,@column,"*")}@end

start:*=@lua{xml.vtag[0]="$1"\;\
             xml.l=@line\;\
             xml.c=@column\;\
             xml.vtag.n=0}\
             @set{x;s}@set{t;$1}@end

esc:\A="
esc:\n=\\\n
esc:"=\\"
esc:'=\\'
esc:\\=\\\\
esc:\Z="

end
