// import std.file;
import std.stdio;
import std.string;

enum { LBL_BMP=0,LBL_QUOTED, LBL_INTEGER, LBL_ESCAPED, LBL_NEWLINE, LBL_EMPTY, LBL_NOTGREEDY };

class Label {
public:
   ubyte bmap[32];
   ushort type;

private:
   char[] thechr(int c) {
     char[] s;
     char t;

     c &= 255;

     if (c < 32 || c > 126 || c == '\\' || c == '"' || c == '\'' || c==']' || c=='-') {
       s ~= "\\x" ~ format("%02X",c); 
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

  this () {}
  this (char []s) {
    bool negate = false;
    bool range = false;
    char last;
    int c;
    int i=0;
    int n=s.length;
    
    type = LBL_EMPTY;
    if (n > 0) {
      type = LBL_BMP;
      
      if (s[i] == '[') i++;
      if (s[i] == '^') {i++; negate = true;}
      
      while (type == LBL_BMP && i<n && s[i] != ']') {
        c = s[i];
        if (c == '-')
          range = true;
        else if (c == '.') {
          zro(); neg(); break;
        }
        else {
          if (c == '\\' && i < n-1) {
            c=-1;
            i++;
            switch (s[i]) {
              case 'd': for (last='0'; last <= '9'; last++) set(last);
                        break; 
              case 'u': for (last='A'; last <= 'Z'; last++) set(last);
                        break; 
              case 'w': for (last='0'; last <= '9'; last++) set(last);
              case 'a': for (last='A'; last <= 'Z'; last++) set(last);
              case 'l': for (last='a'; last <= 'z'; last++) set(last);
                        break; 
                        
              case 'D': for (last=0; last < '0'; last++) set(last);
                        for (last='9'+1; last <= 255; last++) set(last);
                        break; 

              case 'n': c='\n'; break;
              case 'r': c='\r'; break;
              case 'b': c='\b'; break;
              case 't': c='\t'; break;
              case 'v': c='\v'; break;
              case 'f': c='\f'; break;
      
              case 'e': type = LBL_ESCAPED; break;
              case 'N': type = LBL_NEWLINE; break;
              case 'Q': type = LBL_QUOTED; break;
              case 'I': type = LBL_INTEGER; break;
              
              default : c = s[i];
            
            }
          }
          if (c >= 0) {
            if (range) {
               while (++last < c) {
                 set(last);
               }
            }
            last = c;
            set(last);
          }
          range = false;
        }
        i++;
      }
      if (range) set('-');
      if (negate) neg();
    }
  }
  
  Label cpy() {
    Label ll = new Label;
    for(int i; i<32; i++) ll.bmap[i] = bmap[i];
    ll.type = type;
    return ll;
  }

  void minus(Label l) {
    for(int i; i<32; i++) bmap[i] &= ~l.bmap[i];
  }

  void plus(Label l) {
    for(int i; i<32; i++) bmap[i] |= l.bmap[i];
  }
  
  bool isEmpty() {
    ubyte e = 0;
    for(int i; i<32; i++) e |= bmap[i];
    return e == 0;
  }

  bool isEq(Label l) {
    return cmp(l) == 0;
  }

  int cmp(Label l) {
    int e = 0;
    for(int i=31; e == 0 && i>0; i--) e = bmap[i] - l.bmap[i];
    return e;
  }  
  
  char[] tostring() {
    char []s;
    int a,b;
    int i;
    switch (type) {
      case LBL_BMP: a=0; b=0; s = "[";
                    while (a <= 255) {
                      while (a <= 255 && !tst(a)) a++;
                      b=a+1;
                      while (b <= 255 && tst(b)) b++;
                      b=b-1;
                      if (a <= 255) {
                        if (a == b) {
                          s ~= thechr(a);
                        } else {
                          s ~= thechr(a) ~ "-" ~ thechr(b);
                        }
                        a = b+1;
                      }
                    }
                    s ~= "]";
                    break;
                    
      case LBL_ESCAPED: s="\\e"; break;
      case LBL_QUOTED:  s="\\Q"; break;
      case LBL_INTEGER: s="\\I"; break;
      case LBL_NEWLINE: s="\\N"; break;
      case LBL_EMPTY:   s=""; break;
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
    l = new Label("[xyZz92-6]");
    writef("Current l: %s\n",l.tostring());
    l = new Label("[^x2-6yZz9]");
    writef("Current l: %s\n",l.tostring());
    l = new Label(".");
    writef("Current l: %s\n",l.tostring());
    l = new Label("\\e");
    writef("Current l: %s\n",l.tostring());
    writef("UNIT TEST END\n");
  }
}


class TagLst {
  ushort[] tags;
  char[] tagsig;

  this() {}
  this(ushort t) { add(t); }
  
  this(ushort[] t) { add(t); }
  
  void add(ushort t) { // Let's use a linear search for now ...
    int i,j;
    i = tags.length;
    j = i;
    while (i>0 && tags[i-1] > t) i--;
    if (i==0) {
      tags = t ~ tags;
    }
    else if (tags[i-1] != t) {
      tags = tags[0..i] ~ t ~ tags[i..length];
    }
    tagsig = null;
  }
  
  void add(ushort[] t) {
    for (int i=0;i<t.length;i++)
      add(t[i]);
  }
 
  void add(TagLst t) { add(t.tags); }

  char[] tostring() {
    char [] s;
    if (tagsig.length == 0) {
      for (int i=0;i<tags.length;i++) {
        s ~= format("%d,",tags[i]);
      }
      tagsig = s;
    }
    return tagsig;
  }
  
  unittest {
   writef("UNIT TEST 200 -- TagList\n");
   
   TagLst tl = new TagLst;
   TagLst t2 = new TagLst;

   writef("Current tl: %s\n",tl.tostring());
   
   tl.add(100);
   writef("Current tl: %s\n",tl.tostring());
   tl.add(20);
   tl.add(130);
   tl.add(100);
   tl.add(2);
   writef("Current tl: %s\n",tl.tostring());
   t2.tags.length = tl.tags.length;
   for (int i=0;i<tl.tags.length;i++) {
     t2.tags[i] = 132-i;
   }
   tl.add(t2);
   writef("Current tl: %s\n",tl.tostring());
   writef("Current tl: %s\n",tl.tagsig);
   
   writef("UNIT TEST END\n");
  }
}

class Arc {
  ushort from;
  ushort to;
  Label  lbl;
  TagLst tags;

  this (ushort f, ushort t, Label l, TagLst tl) {
    from = f; to = t; lbl = l ; tags = tl;
  }

  this (ushort f, ushort t, Label l) { this(f, t, l, null); }
 
}

class Graph {
  Arc[][] states;
  int[]   narcs;
  ushort  curstate;   
  uint    nstates;
  
  this() {
    states.length = 100;
    narcs.length = states.length;
    states[0] = null; // the final state
    curstate = 1;
  }

  Arc addarc(ushort f, ushort t, char[] l,ushort tg)
  {
    Arc a;
    a = addarc(f, t, l);
    a.tags = new TagLst(tg);
    return a;
  }
  
  Arc addarc(ushort f, ushort t, char[] l,ushort[] tl)
  {
    Arc a;
    a = addarc(f, t, l);
    a.tags = new TagLst(tl);
    return a;
  }
  
  Arc addarc(ushort f, ushort t, char[] l,TagLst tl)
  {
    Arc a;
    a = addarc(f, t, l);
    a.tags = new TagLst(tl.tags);
    return a;
  }
  
  Arc addarc(ushort f, ushort t, char[] l)
  {
    return addarc(f, t, new Label(l));         
  }
  
  Arc addarc(ushort f, ushort t, Label l)
  {
    int j=0;
    Arc a;
    
    if (f == 0 || (f == t && l.isEmpty)) return null;
    if (f > states.length) {
      states.length = states.length + 100;
      narcs.length = states.length;
    }

    if (f > nstates) nstates = f;
    if (t > nstates) nstates = t;
            
    if (states[f].length == narcs[f])
      states[f].length = states[f].length + 50;

    a = new Arc(f,t,l);
    states[f][narcs[f]] = a;    
    narcs[f]++;
    return a;
  }
  
  void dump() {
    ushort state=1;
    int j;
    Arc a;
    
    while (state <= nstates) {
      for (j=0; j < narcs[state]; j++) {
        a=states[state][j];
        writef("%3d -> %-3d %s\n",a.from,a.to,a.lbl.tostring());
      } 
      state++;
    }
  }
  
  unittest {
    writef("UNIT TEST 300 -- Graph\n");
    Graph nfa = new Graph; 
    nfa.addarc(1,2,"p");
    nfa.addarc(1,2,"\\l");
    nfa.addarc(2,3,"\\e");
    nfa.addarc(2,3,"\\N");
    nfa.addarc(3,0,"d");
    nfa.dump();
    writef("UNIT TEST END\n");
  }
}

class rxparser {
 
  Graph parse(Graph nfa, char[] rx, int nrx)
  {
     if (nfa == null) {nfa = new Graph; }
     i = 0; m=rx.length;
     expr(nfa,rx,1);
     return nfa;
  }
  
  private:

  /* 
       expr ::= { term }.
       
       term ::= "(" [ expr { altx } ] ")" ["?"] |
                "\\E" escaped |
                cclass ["+" | "-" | "*" | "?"].
       
       altx ::=  "|" expr .
       
       cclass ::= "[" escaped {escaped} "]" |
                 escaped.
       
       escaped ::= "\\x" hexdigit [hexdigit] |
                   "\\" octdigit [octdigit] [octdigit]|
                   "\\" spchr | ^spchr
                   
       
       spchr ::= ":" | "|" | "*" | "+" | "-" | "?" | "(" | ")"
       
       
       hexdigit ::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" | "0" |
                    "A" | "B" | "C" | "D" | "E" | "F" |
                    "a" | "b" | "c" | "d" | "e" | "f" .
       
       
       octdigit ::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" .
*/

  int i;
  int m;
  
  int expr(Graph nfa, char []rx, int state)
  {
     int j = state;
     while ( j = term(nfa,rx,j)) state = j;
     return state;
  }
  
  int term(Graph nfa, char []rx, int state)
  {
     int j = state;
     char[] l;
     int c;
     Arc a;
     
     l = cclass(rx);
     if (l.length > 0) {
       a=nfa.addarc(state,state+1,l);

       switch (peek(rx)) {
         case '-' :  a.lbl.type == LBL_NOTGREEDY;
         case '*' :  nfa.addarc(state,state+1,"");
         case '+'    nfa.addarc(state+1,state+1,a.lbl.cpy());
                     i=i+1;
                     break;
                     
         case '?'    nfa.addarc(state,state+1,"");
                     i=i+1;
                     break;
       }
       state = state +1;
       
       return state;
     }
     return 0;
  }
  
  char[] cclass(rx)
  {
     char[] l;
     if (rx[i] == '\\') {
       i = i+1;
       if (i < m) 
        return null;
     }
     return l; 
  }
  
  int peek(rx) 
  {
    if (i >= m) return -1;
    return rx[i];
  }
  
  int next(rx) 
  {
    if (i >= m) return -1;
    return rx[i++]; 
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
