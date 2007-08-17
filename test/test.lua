
yrx = require("yrxl")

  local t = 1

function xx(a)
  local s,err
  local t=45
  s,err = yrx.scanner {
       {"p(ip)(po).*" ,function (a,b,c)
                   print(t,"Ciao Pippo ", "("..a..")","("..b..")","("..c..")")
                   --return "ok, got it"
                 end },
       {"(%w)(%d*)(%l*)"  ,function (name,_,digits,letters)
                    print(t,"Look: ".. name,_,'('..digits..')','('..letters..')')
         
                             end },
       --{"."     ,function (name)
       --              print(t,"Discard "..name)
       --            end },
       {yrx.eof ,function () 
                   print(t,"Game over, man")
                 end }
      }
   if err then print(err); os.exit(-1) end
   -- f=io.open("x.l","wb")
   -- print(s[0])
   -- f:write(string.dump(s[0]))
   -- f:close()
   --yrx.scansave(s)
   return s(a)
end

 print("-> ",xx(arg[1]))
                 
                      
--xx()
