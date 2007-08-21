import std.file;
import std.stdio;

class Label {
public:
   ubyte bmap[32];
   
private:   
   char[] thechr(int c) {
     char[] s;
     char t;
     
     c &= 255;

     if (c < 32 || c > 126 || c == '\\' || c == '"' || c == '\'') {
       s ~= "\\x";
       t  = c >> 4;  t += (t > 9? 'A'-10 : '0');
       s ~= t;
       t  = c & 0xF; t += (t > 9? 'A'-10 : '0');
       s ~= t;
     }
     else {
       s ~= cast(char)c;
     }
     return s;
   }
   
public:   
   void clr(ubyte c) { bmap[c>>3] &= ~(1<<(c&7));  }
   void set(ubyte c) { bmap[c>>3] |=  (1<<(c&7));  }
   bool tst(ubyte c) { return cast(bool)(bmap[c>>3] & (1<<(c&7)));  }

   void zro() { for(int i; i<32; i++) bmap[i] = 0; }
   void neg() { for(int i; i<32; i++) bmap[i] ^= 0xFF; }
   
   Label cpy() {
     Label ll = new Label;
     for(int i; i<32; i++) ll.bmap[i] = bmap[i]; 
     return ll;
   }
   
   void minus(Label l) {
     for(int i; i<32; i++) bmap[i] &= ~l.bmap[i]; 
   }
   
   void plus(Label l) {
     for(int i; i<32; i++) bmap[i] |= l.bmap[i]; 
   }
   
   char[] tostring() {
     char []s;
     int a,b;
     int i;
     
     a=0; b=0;
     while (a <= 255) {
       while (a <= 255 && !tst(a)) a++;
       b=a+1;
       while (b <= 255 && tst(b)) b++;
       b=b-1;
       if (a <= 255) {
         s ~= thechr(a) ~ thechr(b);
         a = b+1;
       }
     }   
     return s;
   }
   
   unittest {
     Label l = new Label;
     writef("UNIT TEST 100 -- Label\n");
     assert( !l.tst('A'),"101 - Unexpected set bit");
     l.set('A');
     assert( l.tst('A'),"102 - Unexpected unset bit");
     l.set('B');  l.set('C');
     l.set('a');  l.set('b');  l.set('c');
     l.set('\t'); l.set('\n'); l.set('\r');
     l.set(11); l.set(12);
     writef("Current l: %s\n",l.tostring());
     l.neg();
     writef("Current l: %s\n",l.tostring());
     writef("UNIT TEST END\n");
   }
}


class TagLst {
  ushort[] tags;
  
}

class Arc {
   ushort from;
   ushort to;
   Label  lbl;
   TagLst tags;
   
   this (ushort from, ushort to, Label l, TagLst t) {
   }
   
   this (ushort from, ushort to, Label l) {
     return this(from, to, l, new TagLst);
   }
   
}


int main (char[][] args)
{
  Label l = new Label();

  for (int i = 1; i < args.length; ++i)  {
  	writef("%d\t%d %s\t%s\n",i, i & 1, args[i],l.tostring());
  }
  return(0);
}
  