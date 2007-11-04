// import std.file;
import std.stdio;
import std.string;
import std.ctype;
import std.c.stdio;
import std.gc;

enum { LBL_BMP=0,LBL_QUOTED, LBL_INTEGER, LBL_ESCAPED, LBL_NEWLINE, LBL_EMPTY, LBL_NOTGREEDY };

class Label {
public:
   ushort bmap[16];
   ushort type;

private:
   char[] thechr(int c) {
     char[] s;
     char t;

     c &= 255;

     if (c < 32    || c > 126  || c == '\\' || c == '"' ||
         c == '\'' || c == '[' || c == ']'  || c == '-' ) {
       s ~= "\\x" ~ format("%02X",c); 
     }
     else {
       s ~= cast(char)c;
     }
     return s;
   }

public:
  void   clr(ushort c) { bmap[c>>4] &= ~(1<<(c&0xF));  }
  void   set(ushort c) { bmap[c>>4] |=  (1<<(c&0xF));  }
  ushort tst(ushort c) { return (bmap[c>>4] & (1<<(c&0xF)));  }

  void zro() { for(byte i; i<16; i++) bmap[i] = 0; }
  void neg() { for(byte i; i<16; i++) bmap[i] ^= 0xFFFF; }

  this () {}
  this (char []s) {
    bool negate = false;
    bool range = false;
    int last;
    int c;
    int i=0;
    int n=s.length-1;
    int h;
    Label ll = new Label;
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
            ll.zro();
            switch (s[++i]) {
              case 'h','H': for (last='A'; last <= 'F'; last++) ll.set(last);
                            for (last='a'; last <= 'f'; last++) ll.set(last);
              case 'd','D': for (last='0'; last <= '9'; last++) ll.set(last);
                            break; 
                        
              case 'o','O': for (last='0'; last <= '7'; last++) ll.set(last);
                            break; 
                            
              case 'u','U': for (last='A'; last <= 'Z'; last++) ll.set(last);
                            break; 
                        
              case 'w','W': ll.set('_');
                            for (last='0'; last <= '9'; last++) ll.set(last);
              case 'a','A': for (last='A'; last <= 'Z'; last++) ll.set(last);
              case 'l','L': for (last='a'; last <= 'z'; last++) ll.set(last);
                            break; 

              case 's','S': for (last=8; last <= 13; last++) ll.set(last);
                            ll.set(' ');
                            break;
                        
              case 'y','Y': ll.set(' ');ll.set('\t');
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
            if (range)
              while (++last < c) set(last);
 
            last = c;
            set(last);
          }
          else {
           if (isupper(s[i])) ll.neg();
           or(ll);
           ll.zro();
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
    for(byte i; i<16; i++) ll.bmap[i] = bmap[i];
    ll.type = type;
    return ll;
  }

  void minus(Label l) {
    for(byte i; i<16; i++) bmap[i] &= ~l.bmap[i];
  }

  void or(Label l) {
    for(byte i; i<16; i++) bmap[i] |= l.bmap[i];
  }
  
  void and(Label l) {
    for(byte i; i<16; i++) bmap[i] &= l.bmap[i];
  }
  
  bool isEmpty() {
    int i;
    if (type == LBL_EMPTY) return true;    
    while (i<16) if (bmap[i++] != 0) return false;
    type = LBL_EMPTY;
    return true;
  }

  bool isIn(Label l) {
    for(byte i; i<16; i++) if ((bmap[i] | l.bmap[i]) != bmap[i]) return false ;
    return true;
  }
  
  bool isEq(Label l) {
    return cmp(l) == 0;
  }

  int cmp(Label l) {
    int e = 0;
    for(byte i=15; e == 0 && i>0; i--) e = bmap[i] - l.bmap[i];
    return e;
  }  
  
  int addch(char [] s,int i,char c)
  {
    int k;
    if (c < 32    || c > 126  || c == '\\' || c == '"' ||
        c == '\'' || c == '[' || c == ']'  || c == '-' ) {
      s[i++] = '\\';  s[i++] = 'x';

      k = c >> 4;
      if (k < 10) k += '0';
      else k += 'A'-10;
      s[i++] = k ;
      
      k = c & 0x0F;
      if (k < 10) k += '0';
      else k += 'A'-10;
      s[i++] = k;
    }
    else s[i++] = c;
      
    return i;
  }
  
  char[] tostring() {
    char []s;
    short a,b;
    short i;
    
    s.length = 80;
    
    a=0; b=0; s[i++] = '[';
    while (a <= 255) {
      if ((a & 0x0F) == 0)
        while ((a <= 255) && (bmap[a>>4] == 0)) a += 16;
      while (a <= 255 && !tst(a)) a++;
      b=a+1;
      while (b <= 255 && tst(b)) b++;
      b=b-1;
      if (a <= 255) {
        if (s.length < (i+8)) s.length = s.length + 80;
        
        i = addch(s,i,a);
        if (a != b) {
          if (b>a+1) s[i++] = '-';
          i = addch(s,i,b);
        }
        a = b+1;
      }
    }
    s[i++] = ']';
    if (type == LBL_NOTGREEDY) s[i++] = '-';
    s.length=i;
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
  ushort  ntags;

  this() {}
  this(ushort t) { add(t); }
  this(TagLst t) { if (t) add(t.tags); }
  
  this(ushort[] t) { add(t); }
  
  void add(ushort t) { // Let's use a linear search for now ...
    int i,j;
    i = ntags-1;
    while (i>=0 && tags[i] != t) i--;
    if (i<0) {
      if (ntags >= tags.length) tags.length = ntags +10;
      tags[ntags++] = t;
    }
    //writef("at (%s) ",tostring());
  }
  
  void add(ushort[] t) {
    if (t) {
      for (int i=0;i<t.length;i++)
        add(t[i]);
    }
  }
 
  void add(TagLst t) { if (t) add(t.tags); }

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
  //ushort from;
  ushort to;
  Label  lbl;
  TagLst tags;

  this (ushort f, ushort t, Label l, TagLst tl) {
    //from = f;
     to = t; lbl = l ; tags = tl;
  }

  this (ushort f, ushort t, Label l) { this(f, t, l, null); }
  
  char[] lblstr() {
    return lbl.tostring();
  }
  
  char[] tagstr() {
    if (!tags) return "";
    return tags.tostring();
  }
  
  void addtags(TagLst tl)
  {
    if (!tags) tags = new TagLst(tl);
    else tags.add(tl);
  }
  
  void addtags(ushort[] tl)
  {
    if (!tags) tags = new TagLst(tl);
    else tags.add(tl);
  }
  
  void addtags(ushort tg)
  {
    if (!tags) tags = new TagLst(tg);
    else tags.add(tg);
  }
}

class Graph {
  Arc[][] states;
  int[]    narcs;
  ushort   neps;
  ushort  curstate;   
  uint    nstates;
    
  this() {
    states.length = 10;
    narcs.length = states.length;
    states[0] = null; // the final state
    curstate = 1;
  }

  ushort nextstate() { 
    
    ++curstate;
    if (curstate >= states.length) {
      states.length = curstate + 10;
      narcs.length = states.length;
      writef(narcs.length);
    }

    return curstate;
  }
  
  ushort peeknextstate() { return curstate +1; }
  
  Arc addarc(ushort f, ushort t, char[] l,ushort tg)
  {
    return addarc(f, t, new Label(l),new TagLst(tg));
  }
  
  Arc addarc(ushort f, ushort t, char[] l,ushort[] tl)
  {
    return addarc(f, t, new Label(l),new TagLst(tl));
  }
  
  Arc addarc(ushort f, ushort t, char[] l,TagLst tl)
  {
    return addarc(f, t, new Label(l),tl);
  }
  
  Arc addarc(ushort f, ushort t, char[] l)
  {
    return addarc(f, t, new Label(l),null);
  }
  
  Arc addarc(ushort f, ushort t, Label l)
  {
    return addarc(f, t, l,null);
  }
  
  Arc addarc(ushort f, ushort t, Label l,TagLst tl)
  {
    Arc a;
    
    if (f == 0 || (f == t && l.isEmpty)) return null;
    
    if (f > nstates || t > nstates) nstates = t;
    
    a = getarc(f,t,l);
    
    if (!a) {
      a = new Arc(f,t,l);
      if (states[f].length <= narcs[f]) {
        states[f].length = narcs[f] + 10;
      }
      states[f][narcs[f]] = a;    
      narcs[f]++;
        //writef("oo %d %d %d %s\n",f,states[f].length,narcs[f],a.lblstr());
    }
    
    if (tl) a.addtags(tl);
    
    //writef("ADDARC: %d %d %s %s\n",a.from,a.to,a.lbl.tostring,(a.tags?a.tags.tostring():""));
    
    return a;
  }
   
  void dump() {
    ushort state=1;
    int j;
    Arc a;
    int stkptr;
    ushort[] stk;
    bool[] psh;
   
    stk.length = nstates+1;
    psh.length = nstates+1;
    
    stk[stkptr++] = 1;
    psh[0] = true;
    
    while (stkptr > 0) {
      state=stk[--stkptr];
      for (j=0; j < narcs[state]; j++) {
        a=states[state][j];
        writef("%3d -> %-3d %s %s\n",state,a.to,a.lblstr(),a.tagstr());
        if (!psh[a.to]) { stk[stkptr++] = a.to; psh[a.to] = true; }
      } 
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
  
private:
 
  Arc getarc(ushort from, ushort to, Label l)
  {
    Arc a;
    int i;
    
    for (i=0; i<narcs[from]; i++) {
      a = states[from][i];
      if ((a.to == to) && (a.lbl.isEq(l)))
        return a;
    }
    return null;
  }
  
  
  
private:
 
  void copyarcs(ushort from,ushort of,TagLst epstags)
  {
    Arc a;
    int j;
    TagLst t;
    for (j = narcs[of]-1; j >= 0; j--) {
      a = states[of][j];
      t = new TagLst(epstags);
      t.add(a.tags); //writef("ca (%d,%d,%d) ",of,narcs[of],j);
      addarc(from,a.to,a.lbl.cpy(),t);
    } 
  }
  
  void delarc(ushort state,int k)
  {
    narcs[state]--;
    if (k < narcs[state])
       states[state][k] = states[state][narcs[state]];   
  }
  
  void removeeps(ushort state)
  {
    Arc a;
    int j;
    
    while (j < narcs[state]) {
      a=states[state][j];
      if (a.lbl.isEmpty()) {
        if (a.to == 0) {
          if (j > 0) {
            states[state][j] = states[state][0];
            states[state][0] = a;
          }
          j++;
        }
        else{
          copyarcs(state,a.to,a.tags);
          delarc(state,j);
        }
      }
      else j++;
    }
  }
  
  // v = a intersect b
  // a = a minus v
  // b = b minus v
  Label intersect(Label a, Label b, )
  {
    Label v = a.cpy();
    v.and(b);
    //writef("a:%s b:%s v:%s\n",a.tostring(),b.tostring(),v.tostring());
    a.minus(v);
    b.minus(v);
    return v;
  }

  enum {STK_INIT, STK_PUSH, STK_POP,STK_CLR};
  
  ushort stack(ushort op, ushort val = 0)
  {
    static bool[] psh;
    static ushort[] stk;
    static int stkptr;
    
    //writef("STACK: %d %d %d %d\n",op,val,stkptr,(stkptr>0)?stk[stkptr-1]:0 );
    
    if (op == STK_CLR) {
      stkptr = 0;
      psh = null;
      stk = null;
      return 0;
    }
    
    if (op == STK_POP) {
      if (stkptr == 0) return 0;
      val = stk[--stkptr];
      return val;
    }
    
    if (val == 0) {
      if (psh.length <100) psh.length = 100;
      psh[0] = true;
      if (stk.length <100) stk.length = 100;
      stkptr = 0;
      return 0;
    }    
    
    if (psh.length < val) psh.length = psh.length +100;
    if (stk.length < stkptr-1) stk.length = stk.length +100;
    if (!psh[val]) {
      stk[stkptr++] = val;
      psh[val] = true;
    }
    return val;
  }

ushort[][] mrgd;
ushort mrgd_start;
      
  ushort merge(ushort a, ushort b)
  {
     ushort t,a_j,b_k,n;
   
     ushort[]alst;
     ushort[]blst;
     ushort[]tlst;
     
     //writef("MERGE: %d %d = ",a,b);
     
     if (mrgd.length == 0) {
       mrgd.length = 10;
       mrgd_start = curstate;
     }
     
     if (a>b){t=a;a=b;b=t;}
     if (mrgd.length < (b - mrgd_start)) { mrgd.length =  (b- mrgd_start) + 10; }
     
     if (a <= mrgd_start) {
       alst.length=1;
       alst[0] = a;
     } 
     else alst = mrgd[a-mrgd_start];
     
     if (b <= mrgd_start) {
       blst.length=1;
       blst[0] = b;
     } 
     else blst = mrgd[b-mrgd_start];
  
     if (alst.length < blst.length) {
       t = b; b = a; a = t;
       tlst = blst; blst = alst; alst = tlst;
     }

     do {
       while (a_j < alst.length && alst[a_j] < blst[b_k]) a_j++;
       
       if (a_j == alst.length) {
         alst ~= blst[b_k..$];
         b_k = blst.length;
         n++;
       }
       else {
         if (alst[a_j] != blst[b_k]) {       
           alst = alst[0..a_j+1] ~ blst[b_k] ~ alst[a_j+1 .. length];
           n++;
         }
         a_j++; 
         b_k++;
       }
       
     } while (b_k < blst.length);

     //writef("(a=");
     //for (int k=0;k<alst.length ;k++) writef("%d ",alst[k]);
     //writef(")(b=");
     //for (int k=0;k<blst.length ;k++) writef("%d ",blst[k]);
     //writef(")");
     
     t = a;
     if (n>0) {
       t = curstate - mrgd_start;
       while (t > 0 && n > 0) {
         if (mrgd[t].length == alst.length) {
           a_j =0;
           while (a_j < alst.length && mrgd[t][a_j] == alst[a_j]) a_j++;
           if (a_j == alst.length) break;
           t--;
         } 
         else t--; 
       }
       if (t == 0) {
         t = nextstate();
         mrgd[t-mrgd_start] = alst;
         addarc(t,a,"");
         addarc(t,b,"");
       }
       else t += mrgd_start;
     }
     
     //writef("%d\n",t);
     //if (curstate > 8) throw new Exception("PIOPO" ~ format("%d %d",n,t));
     

     return t;
  }
  
public:   
 
  void determinize()
  {
    Graph dfa = new Graph;
    
    ushort state=1;
    int j,k;
    Arc a,b,c;
    int stkptr;
    Label l,al,bl;
    ushort to;
    Arc[] s;
    
    stack(STK_PUSH,0);
    stack(STK_PUSH,1);
    state = stack(STK_POP);

    while (state > 0) {
      removeeps(state);
      //k = (states[state][0].to == 0)? 1: 0;
      s = states[state];
      for (j=0; j < narcs[state]; j++) {
        a=s[j]; al=a.lbl; 
        for (k=j+1; k < narcs[state]; k++) {
          b=s[k]; bl=b.lbl;
          //writef("State:%d   j:%d   k:%d ",state,j,k);
          //l=intersect(a.lbl,b.lbl);
          l=al.cpy();
          l.and(bl);
          al.minus(l);
          bl.minus(l);
          
          //writef("a:%s b:%s v:%s\n",a.lbl.tostring(),b.lbl.tostring(),l.tostring());
          if (!l.isEmpty()) {
            to = merge(a.to,b.to);
            c = addarc(state,to,l,a.tags);
            c.addtags(b.tags);
            if (bl.isEmpty()) { delarc(state,k);}
            if (al.isEmpty()) { delarc(state,j); k = narcs[state]; }
          }
        }
      } 
      //k = (states[state][0].to == 0)? 1: 0;
      for (j=0; j < narcs[state]; j++) {
        a = states[state][j];
        stack(STK_PUSH,a.to);
      }
      state = stack(STK_POP);
    }
    stack(STK_CLR);
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
     if (peek(0) >= 0) err("Unexpected character '" ~ r[i] ~ "'");
     nfa.addarc(state,0,"",tag_code(TAG_FIN,n));
     return nfa;
  }
  
  private:

  /* 
       <expr>     ::= <term>+

       <term>     ::= \(<expr> ('|' <expr>)*\)\?? |
                      \\E<escaped> | \\: |
                      \[eNQI][\+\-\*\?]? |
                      <cclass>[\+\-\*\?]?

       <cclass>   ::= \[\e*\] | <escaped>

       <escaped>  ::= \\x\h?\h? | \\\o\o?\o? |
                      \\. | [^\|\*\+\-\?\(\)]
           
  */

  int expr(Graph nfa, ushort state)
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
     ushort to;
     ushort t1;
     Arc a;
     
     c = peek(0);
     
     if ( c < 0) return 0;
     
     if ( c == '(' ) {
       ushort[] st;
       
       c = p++;
       if (c>120)
         err("Too many captures");
       i++;
       to = nfa.nextstate();
       nfa.addarc(state,to,"",tag_code(TAG_CB|c,n));
       state = to;
       t1 = expr(nfa,state);
       while (t1 > 0) {
         st ~= t1;
         if (peek(0) != '|') break;
         i++;
         t1 = expr(nfa,state);
       }
       if (peek(0) != ')') err("Unclosed capture");
       i++;
       to = nfa.nextstate();
       for (t1 = 0; t1 <st.length; t1++)
         nfa.addarc(st[t1],to,"",tag_code(TAG_CE|c,n));
       if (peek(0)== '?') {
          i++;
          nfa.addarc(state,to,"",tag_code(TAG_CE|c,n));
       }
       return to;
     }
          
     if ( c == '\\') {
       switch (peek(1)) {
         case 'E' :  e = escaped();
                     i = i+2;
                     return state;
                     
         case ':' :  to = nfa.nextstate();
                     nfa.addarc(state,to,"",tag_code(TAG_MRK,n));
                     //nfa.pushtag(nfa.peeknextstate(),tag_code(TAG_MRK,n));
                     i = i+2;
                     return to;

         case 'e' :  t1 = nfa.nextstate();
                     to = nfa.nextstate();
                     a = nfa.addarc(state,to,format("[^%s]",e));
                     nfa.addarc(state,t1,e);
                     nfa.addarc(t1,to,".");
                     switch (peek(2)) {
                       case '-' :  if (peek(3) >=0 ) a.lbl.type = LBL_NOTGREEDY;
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
                     i = i+2;
                     return to;
                                          
         default  :  break;
       }
     }
     
     l = cclass();
     if (l.length > 0) {
       to = nfa.nextstate();
       a = nfa.addarc(state,to,l);
       
       switch (peek(0)) {
         case '-' :  if (peek(1) >=0 ) a.lbl.type = LBL_NOTGREEDY;
         case '*' :  nfa.addarc(state,to,"");
         case '+' :  nfa.addarc(to,to,a.lbl.cpy());
                     i=i+1;
                     break;
                     
         case '?' :  nfa.addarc(state,to,"");
                     i=i+1;
                     break;
                     
         default  :  break;           
       }
       
       return  to;
     }
     return 0;
  }
  
  int altx(Graph nfa, ushort state)
  {
     if (peek(0) != '|') return 0;
     i++;
     return expr(nfa,state);
  }
  
  char[] cclass()
  {
     char[] l;
     int j = i;
     if (i == m) return null;
     if (r[j] == '[') {
       j++;
       while (true) {
         //writef("-- %s\n",r[j..$]);
         if (r[j] == '\\') j+=2; 
         else if (j < m) {
           if (r[j] == ']') break;
           j++;
         }
         if (j >= m) err("Unclosed class");
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

     c = peek(0);
     
     if ( c < 0 || c == '*' || c == '+' || c == '?' || c == '-' ||
          c == '(' || c == ')' || c == ']' || c == '|') {
       return l;
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
     
     return l;
  }
  
  int peek(int j) 
  {
    if ((i+j) >= m) return -1;
    return r[i+j];
  }
  
  void err(char[] m)
  {
    throw new Exception(format("%s @%d:%d",m,i,n));
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
  char [][] rxs;


  rxs = args;
  
  if (rxs.length == 1) {
    rxs.length = 256;
    i=1;
    while ((rxs[i] = readln()) != null) {
      if (rxs[i][rxs[i].length-1] == '\n') 
        rxs[i].length = rxs[i].length-1;
      if (rxs[i].length >0)
       i++;
    }
    rxs.length = i;
  }
  
  for (i = 1; i < rxs.length; ++i)  {
    writefln(rxs[i]);
    dfa = rxp.parse(dfa,rxs[i],i);
  }
  
  if (dfa) {
    dfa.determinize();
    dfa.dump();
  }
  return(0);
}