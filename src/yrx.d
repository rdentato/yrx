// import std.file;
import std.stdio;
import std.string;
import std.ctype;

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

     if (c < 32 || c > 126 || c == '\\' || c == '"' || c == '\'' || c=='[' || c==']' || c=='-') {
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
    int last;
    int c;
    int i=0;
    int n=s.length-1;
    int h;
  //writef("--- '%s'\n",s);
    type = LBL_BMP;
    if (n >= 0) {
      
      if (s[i] == '[') {
        i++;
        if (s[i] == '^') {i++; negate = true;}
      }
      
      while ( i <= n && s[i] != ']') {
        c = s[i];
        if (c == '-')
          range = true;
        else if (c == '.') {
          zro(); neg(); break;
        }
        else {
          if (c == '\\' && i < n) {
            c=-1;
            switch (s[++i]) {
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

              case 'U': for (last=0; last < 'A'; last++) set(last);
                        for (last='Z'+1; last <= 255; last++) set(last);
                        break; 
                        
              case 'L': for (last=0; last < 'a'; last++) set(last);
                        for (last='z'+1; last <= 255; last++) set(last);
                        break; 
                        
              case 'A': for (last=0; last < 'A'; last++) set(last);
                        for (last='Z'+1; last < 'a'; last++) set(last);
                        for (last='z'+1; last <= 255; last++) set(last);
                        break; 
                        
              case 'W': for (last=0; last < '0'; last++) set(last);
                        for (last='9'+1; last < 'A'; last++) set(last);
                        for (last='Z'+1; last < 'a'; last++) set(last);
                        for (last='z'+1; last <= 255; last++) set(last);
                        break; 
                        
              case 'n': c='\n'; break;
              case 'r': c='\r'; break;
              case 'b': c='\b'; break;
              case 't': c='\t'; break;
              case 'v': c='\v'; break;
              case 'f': c='\f'; break;

              case 'x': c = 0;
                        while (i < n)
                          c = (c<<4) | hex(s[++i]);
                        break;
                        
              case '0','1','2','3','4','5','6','7': 
                        c = oct(s[i]); 
                        while (i < n)
                          c = (c<<3) | oct(s[++i]);
                        break;
                    
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
    
    a=0; b=0; s = "[";
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
    if (type == LBL_NOTGREEDY) s ~= "-";
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
  
private:

  int hex(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('A' <= c && c <= 'F') return c - 'A'+10;
    if ('a' <= c && c <= 'f') return c - 'a'+10;
    return -1;
  }    
  
  int oct(char c) {
    if ('0' <= c && c <= '7') return c - '0';
    return -1;
  }    

}

enum {TAG_MRK = 255, TAG_FIN = 127, TAG_CB=0, TAG_CE=128};

ushort tag_code(ubyte type, ubyte nrx) {
  return (type | (cast(ushort)nrx << 8));
}

ubyte tag_nrx(ushort tag) { 
  return (tag >> 8);
}

ubyte tag_type(ushort tag) { 
  tag = tag & 0xFF;
  if (tag == TAG_MRK || tag == TAG_FIN) return tag;
  return tag & 0x80;
} 

ubyte tag_capt(ushort tag) { 
  tag = tag & 0xFF;
  if (tag == TAG_MRK || tag == TAG_FIN) return 0;
  return tag & 0x7F;
}   

char[] tag_str(ushort tag) { 
  char[] s;
  
  switch (tag_type(tag)) {
    case TAG_MRK : s = format("MRK_%d", tag_nrx(tag)); break;
    case TAG_FIN : s = format("FIN_%d", tag_nrx(tag)); break;
    case TAG_CE  : s = format("CE%d_%d", tag_capt(tag), tag_nrx(tag)); break;
    case TAG_CB  : s = format("CB%d_%d", tag_capt(tag), tag_nrx(tag)); break;    
  }
  
  return s;
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
     
    if (tags.length == 0) return "";
    
    for (int i=0;i<tags.length;i++) {
      s ~= tag_str(tags[i]) ~ ',';
    }
    return s[0 .. length-1];
  }
  
  unittest {
   writef("UNIT TEST 200 -- TagList\n");
   
   TagLst tl = new TagLst;
   TagLst t2 = new TagLst;

   writef("Current tl: %s\n",tl.tostring());
   
   tl.add(tag_code(TAG_CE|23,2));
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
  
  char[] lblstr() {
    return lbl.tostring();
  }
  
  char[] tagstr() {
    if (!tags) return "";
    return tags.tostring();
  }
 
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

  ushort nextstate() { return ++curstate; }
  
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
    if (f > states.length || t > states.length) {
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
        writef("%3d -> %-3d %s %s\n",a.from,a.to,a.lblstr(),a.tagstr());
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

class Parser {
 
  int i;
  int m;
  int n;
  char[] e;
  char[] r;
  int p;
  
  Graph parse(Graph nfa, char[] rx, int nrx)
  {  ushort state;
  
     if (!nfa) { nfa = new Graph; }
     r = rx;
     m = r.length;
     i = 0;
     n = nrx;
     e = "\\\\";
     p = 1;
     state = expr(nfa,1);
     
     nfa.addarc(state,0,"",tag_code(TAG_FIN,n));
     
     return nfa;
  }
  
  private:

  /* 
       <expr>     ::= <term>+

       <term>     ::= \(<expr><altx>*\)\?? |
                      \\E<escaped> | \\: |
                      <cclass>[\+\-\*\?]?

       <altx>     ::=  \|<expr>

       <cclass>   ::= \[\e*\] | <escaped>

       <escaped>  ::= \\x\h?\h? | \\\o\o?\o? |
                      \\. | [^\|\*\+\-\?\(\)]
           
  */

  int expr(Graph nfa, int state)
  {
     int j = state;
     int k = state;
     do {
       state = j;
       j = term(nfa,state);
     } while ( j > 0 )  ;
     return state;
  }
  
  int term(Graph nfa, ushort state)
  {
     int j = state;
     char[] l;
     int c;
     ushort to = state;
     ushort t1;
     Arc a;
     

     if ( (i < m) && (r[i] == '(')) {
        c = p++; i++;
        to = nfa.nextstate();
        nfa.addarc(state,to,"",tag_code(TAG_CB|c,n));
        state = expr(nfa,to);
        to = nfa.nextstate();
        nfa.addarc(state,to,"",tag_code(TAG_CE|c,n));
        if ((i < m) && (r[i] == ')')) {
           i++;
        }
        return to;
     }
          
     if ( (i < m) && (r[i] == '\\')) {
       switch (r[i+1]) {
         case 'E' :  e = escaped();
                     i = i+2;
                     return state;
                     
         case ':' :  to = nfa.nextstate();
                     nfa.addarc(state,to,"",tag_code(TAG_MRK,n));
                     i = i+2;
                     return to;
                     
         default  :  break;
       }
     }
     
     l = cclass();
     if (l.length > 0) {
       if (l == "\\e") {
         t1 = nfa.nextstate();
         to = nfa.nextstate();
         a = nfa.addarc(state,to,format("[^%s]",e));
         nfa.addarc(state,t1,e);
         nfa.addarc(t1,to,".");
         switch (peek()) {
           case '-' :  a.lbl.type = LBL_NOTGREEDY;
           case '*' :  nfa.addarc(state,to,"");
           case '+' :  nfa.addarc(to,to,a.lbl.cpy());
                       nfa.addarc(to,t1,e);
                       i=i+1;
                       break;
                       
           case '?' :  nfa.addarc(state,to,"");
                       i=i+1;
                       break;
           default  :  break;           
         }
       }
       else {
         to = nfa.nextstate();
         a = nfa.addarc(state,to,l);
         
         switch (peek()) {
           case '-' :  a.lbl.type = LBL_NOTGREEDY;
           case '*' :  nfa.addarc(state,to,"");
           case '+' :  nfa.addarc(to,to,a.lbl.cpy());
                       i=i+1;
                       break;
                       
           case '?' :  nfa.addarc(state,to,"");
                       i=i+1;
                       break;
           default  :  break;           
         }
       }
       state = to;
       
       return state;
     }
     return 0;
  }
  
  char[] cclass()
  {
     char[] l;
     int j = i;
     if (i == m) return null;
     if (r[i] == '[') {
       while (true) {
         if (r[j] == '\'') j++; 
         if (r[j] == ']') break; 
         j++;
       }
       l = r[i..j+1];
       i = j+1;
     }
     else l = escaped();
     
     return l; 
  }

  char[] escaped() 
  {
     char [] l;
     int c;
     ubyte s;
     int j;

     if (i < m) {
       c = r[i];
       
       if ( c == '*' || c == '+' || c == '?' || c == '-' ||
            c == '(' || c == ')' || c == '|') {
         return null;
       }
       
       j=i; 
       if (c == '\\' && j < (m-1)) {
         c = r[++j];
         if (c == 'x') {
           if (j < (m-1) && isxdigit(r[j+1])) {
             j++;
             if (j < (m-1) && isxdigit(r[j + 1])) j++;
           }
         }
         else if ('0' <= c && c <= '7') {
           c = j < (m-1)? r[j+1] : 0;
           if ('0' <= c && c <= '7') {
             j++;
             c = j < (m-1)? r[j+1] : 0;
             if ('0' <= c && c <= '7') {
               j++;
             }
           }
         }
       }
       j++;
       l = r[i..j];
       i = j;
     }
     return l;
  }
  
  int peek() 
  {
    if (i >= m) return -1;
    return r[i];
  }
      
  unittest {
    Parser p = new Parser;
    Graph dfa;
    
    assert(p!=null);
    dfa = p.parse(dfa,"[\\d\\l]",1);
    //dfa = p.parse(dfa,"cd*",1);
    if (dfa) dfa.dump();
  }

}


int main (char[][] args)
{
  int nrx;
  int i;
  Graph dfa;
  Parser rxp = new Parser;

  for (i = 1; i < args.length; ++i)  {
    dfa = rxp.parse(dfa,args[i],i);
  }
  if (dfa) dfa.dump();
  return(0);
}
