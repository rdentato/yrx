/* 
**  (C) 2006 by Remo Dentato (rdentato@users.sourceforge.net)
**
** Permission to use, copy, modify and distribute this code and
** its documentation for any purpose is hereby granted without
** fee, provided that the above copyright notice, or equivalent
** attribution acknowledgement, appears in all copies and
** supporting documentation.
** 
** Copyright holder makes no representations about the suitability
** of this software for any purpose. It is provided "as is" without
** express or implied warranty.
**
*/
#ifdef UTEST
#define RX_MAXNFA 40
#endif

#include "rx_.h"
#include <setjmp.h>

jmp_buf compile_fail;


/*              -= COMPILING REGULAR EXPRESSION =-
**
**   Regular expressions are compiled into a list of instructions
** for a non-deterministic finite automata (NFA) interpreter (in
** <rx_exec.c>). 
**
**   The compiled regular expression ends with a '|\0|' and has
** no embedded zeros, thus it can be safely treated as a C string.
**
**   See the <rx_exec.c> file to understand how a string is matched
** against a compiled regex.
**
**   The |rx| command supplied in the distribution (see <rx_main.c>)
** shows, in binary and symbolic form, the compiled regular
** expression.
**
*/


/* Converts an ASCII xdigit to its hex value */
static unsigned char hex(int n) {
  return ((n & 0x40)? (n & 0x5F) -'A'+10 : n-'0');
}

/* = Captures
     ~~~~~~~~
*/

static unsigned char def_capt=0;   /* defined captures */
static unsigned char num_capt=0;  

/* == Nested Captures
**
** To handle nested captures we need a stack. This simple stack 
** implementation uses:
**   - an array ({capt_stack}) to hold values,
**   - a variable for the stack pointer ({capt_stack_count})
**   - three macros ({push()} and  {pop()} to add/remove values 
**     and {empty()} for emptyness check).
**
** BEWARE: This is a simplistic implementation, its correctness
**         depends on its usage!!! Not for reuse.
*/

static unsigned char capt_stack[10];
static unsigned char capt_stack_count=0;

static unsigned int  alt_stack[64];
static unsigned char alt_stack_count=0;
static unsigned int  alt_cur_label=1;

#define push(_s,_v) (_s[_s##_count++]=_v)
#define pop(_s)     (_s[--_s##_count])
#define top(_s,_n)  (_s[_s##_count-1-_n])
#define empty(_s)   (_s##_count==0)


/* = Regex pointers
     ~~~~~~~~~~~~~~
**   Original Oz's implementation used global variables to 
** keep the values needed while compiling the regular expression.
** I found this makes the code harder to understand and maintain
** and decided to keep everything in a structure whose pointer
** is passed from function to function.
**   You may notice that there are very few things we need to 
** keep track of:
**
**   [{cur}] is the current position in the compiled regexp.
**           It points to the first empty slot in the nfa array.
**
**   [{last}] points to the last opcode we stored in the nfa array.
**
**   [{first}] points to the beginnng of the nfa array. It's used
**           in the {store()} function to check that the limit
**           {RX_MAXNFA} defined in <rx.h> is respected.
**
**   [{casesensitive}] is a flag that indicates if case matters
**           in string comparisons. Defaults to 1 (true).
*/

typedef struct {
  unsigned char *cur;   
  unsigned char *lastop;
  unsigned char *first;
           char *error;
  unsigned short len;
  unsigned short maxlen;
  unsigned char  casesensitive;
} rexptrs;

#define error(s) ( r->error=(s), longjmp(compile_fail,1))

/* = Store NFA
     ~~~~~~~~~
**   The NFA is built in an array of unsigned char. Every function
*+ that add something to the NFA is named as |store|/xxx/ (e.g.
** {storeop()}, {storeclo()}, etc.) and uses {store()} to actually
** write into the array.
**   The {store} function checks for buffer overflow if more than
** {RX_MAXNFA} chars are needed.
**
**   Note that all |store|/xxx/ functions return a pointer to the
** current status.
*/

static rexptrs *store(rexptrs *r, unsigned char c)
{ if (r->len >= RX_MAXNFA-1) {
    r->first[RX_MAXNFA-1]='\0';
    error("ERR101: Regular expression too long");
  }
  r->len++;
  *(r->cur++)=c;
  /* *(r->cur) = END; */
  return r;
}


/* == Store op
      ~~~~~~~~
**   An /operation/ is what will be interpreted by the {rx_exec}
** function.
**   We need to keep track of the last operation since some
** |store| function needs to now (see {storech} for an example).
*/

#define storeop(_r,_c) ((_r)->lastop = (_r)->cur,store(_r,_c))

/* == Store strings
      ~~~~~~~~~~~~~
*/

static rexptrs *storech(rexptrs *r, unsigned char c)
{
  unsigned char op;
  
  if (c == 0) return storeop(r,ZERO);
  
  /*  If we weren't storing chars or if already stored 31 chars
  ** we have to start a new string.
  **  We are limited to 31 char since the opcode for STR is:
  ** |010 xxxxx| i.e. we only have 5 bits to store the string length.
  */
  op = *(r->lastop);
  if (( optype(op) != STR) || ( op == NOTCHR) || ( STR_len(op) >= 30))  {
    storeop(r,STR);
  }
  
  *(r->lastop) = *(r->lastop) + 1; /* incr str length */
  if (r->casesensitive == 0) c = tolower(c);
  return store(r,c);
}

/* = Character class
     ~~~~~~~~~~~~~~~ 
**   Many regular expression routines (like grep and the original
** regex routines from Ozan Yigit) use a bitmap to represent a
** character class. This is recognized as the most effective way
** to represent sets of integers.
**
**   To represent all the 256 characters used in the ISO-8859-x
** character encodings (the ones rx is intended to work with)
** one needs 32 bytes, most of which will be 0 or 1.
** This seemed to me an unacceptable waste of space so I devised
** a scheme to compact the bitmap.
**
**   As always we trade space for speed and matching against a 
** character class is now (hopefully only a little bit) slower
** than using a full bitmap. You are encouraged to use character
** class recognizers like |\l| or |\d| rather than the character
** class [a-z] or [0-9].
**   
**
** BEWARE: The code /assumes/ that char is at least 8 bits long
**         which should be guaranteed by the latest C standards.
*/

static short setbit(unsigned short num, unsigned char *bits)
{ static unsigned short numbits;
  if (bits) {
    if (! (bits[num >> 3] & (1 << (num & 0x07)))) {
      bits[num >> 3] |= (1 << (num & 0x07));
      ++numbits;
    }
    num = numbits;
  }
  else {
    num = numbits;
    numbits = 0;
  }
  return num;
}

static short setcls(unsigned short num, unsigned char *cls)
{ 
  static unsigned char numcls;
  if (cls) {
    *cls |= (1<< ((num) & 0x07));
    num = ++numcls;
  }
  else {
    num = numcls;
    numcls = 0;
  }
  return num;
}
/*
static unsigned char tstbit(unsigned char num,unsigned char *bits)
{ return bits[num >> 3] & (1 << (num & 0x07)); }
*/

#define tstbit(_n,_b) ((_b)[(_n) >> 3] & (1 << ((_n) & 0x07)))


static int storeccl(rexptrs *r,unsigned char *str)
{
  static unsigned char bmp[32];
  unsigned char  ccl[2];
  unsigned char *s=str;
  unsigned short chr=0;
  unsigned char  seq=0x00;
  unsigned char  last=0;
  unsigned char  bit=0x01;
  unsigned char  t=0;
  unsigned short numbits=0;
  unsigned short numcls=0;
  
  /* The internal representation of a CCL is: 
  ** :  1yxxxxxx 1zzzzzzz  .......
  ** :   |\____/  |||||||  \_____/
  ** :   |   |    |||||||     \______ compressed bitmap
  ** :   |   |    |||||||
  ** :   |   |    ||||||\____________ isupper()
  ** :   |   |    |||||\_____________ islower()
  ** :   |   |    ||||\______________ isalnum()
  ** :   |   |    |||\_______________ isdigit()
  ** :   |   |    ||\________________ isxdigit)
  ** :   |   |    |\_________________ isspace()
  ** :   |   |    \__________________ ispunct()
  ** :   |   |
  ** :   |   \_______________________ length of compressed bitmap
  ** :   |
  ** :   \___________________________ negates the character class
  ** :
  **
  **   Character class recognizers such as |\l| follow the current
  ** locale (if set by |setlocale()|), that's why they are stored
  ** explicitly: they depend on the locale of the machine where
  ** they are executed rather than on the locale of the machine
  ** they have been compiled on!
  */ 
  
  ccl[0] = CCL;       /* Set an empty isa byte */
  ccl[1] = 0x80;
  
  if (*str == '^') { 
    *ccl ^= 0x40;      /* Set the negate bit */
    str++;
  }
  
  /* {{ STEP 1: Create the bitmap  
  **   The first step is to set a bit in a full bitmap
  ** representation (held in the {bmp} array) for each character
  ** included in the class.
  */
  chr = 0xFF;
  last=0xFF;
  t=0;
  memset(bmp,'\0',32);    /* empty the bitmap */
  setbit(0,NULL);         /* reset bit counter */
  setcls(0,NULL);         /* reset class counter */
    
  if (*str == ']') { 
    setbit((chr = *str),bmp); /* If ']' is to be included it must */
    str++;                    /* be the first character.*/
  }
  
  while (*str && *str != ']') {
    /* To include a '|-|' in the class put it as the last character */
    if (*str == '-' && t == 0) {
      t=1;  /* next chr will be the second in an interval */
    }
    else {
      last = chr; chr=0x00;
      if (*str == '\\' && str[1]) {
        /* Manage escape sequence. There is no mechanism in place to
        ** ensure that these escape sequence are in sync with those
        ** specified in the {storeesc()} function below. If you extend
        ** them here, please verify the consistency with the others!
        **
        */
        switch (*++str) {
            case 'b': setbit((chr = '\b'),bmp);          break;
            case 'n': setbit((chr = '\n'),bmp);          break;
            case 'f': setbit((chr = '\f'),bmp);          break;
            case 'r': setbit((chr = '\r'),bmp);          break;
            case 't': setbit((chr = '\t'),bmp);          break;
            case 'v': setbit((chr = '\v'),bmp);          break;
            case 'e': setbit((chr = '\e'),bmp);          break;
            case '0': 
            case 'z': setbit((chr = '\0'),bmp);          break;
            
            case 'l': setcls(ANYLWR, ccl+1);   break; 
               
            /* To save a bit, |\a| is considered equal to the
            ** union of |\l| and |\u|
            */
            case 'a': setcls(ANYLWR,ccl+1);
            case 'u': setcls(ANYUPR,ccl+1);   break;
                      
            case 'd': setcls(DIGIT ,ccl+1);   break;    
            case 'h': setcls(XDIGIT,ccl+1);   break;    
            case 's': setcls(SPACE ,ccl+1);   break;    
            case 'p': setcls(PUNCT ,ccl+1);   break;    
            
            case 'w': setbit('_',bmp);    
            case 'q': setcls(ALNUM  ,ccl+1);   break;    
            
            case 'c': for (chr=1; chr<8; chr++)
                        setbit(chr,bmp);                 break;
                        
            case 'y': setbit(' ',bmp); setbit('\t',bmp); break;
            
            case 'x': chr = 'x';
                      if (isxdigit(str[1])) {
                        chr = hex(*++str);
                        if (isxdigit(str[1]))
                          chr = (chr<<4) | hex(*++str); 
                      }
                      setbit(chr,bmp);
                      break;
            
            default : /* treat unknown sequence as literals */
                      setbit((chr = *str),bmp); break;
        }
      }
      else
        setbit((chr = *str),bmp);    /* plain char */
        
      if (t == 1) {
        t=0;
        while (++last < chr) setbit(last,bmp);
      }
    }
    str++;
  }
  if (t == 1) setbit('-',bmp); /* the '-' we found was a real '-'! */
  
  if (*str == ']') str++;   /* get rid of the closing ']' */
  else {
    error("ERR102: Invalid character class definition");
  }

  /* }} */
  
  /* The following code prints the bitmap on stderr. I left it
  ** here just in case you were interested.
  */
  #if 0
  t=0;
  fprintf(stderr,"%02X\n",ccl[1]);
  while (t<32) {
    fprintf(stderr,"0x%02X ",bmp[t++]);
    if ((t & 0x07) == 0) fputc('\n',stderr);
  }
  fflush(stderr);
  #endif
  
  /* SPECIAL CASE!
  ** If the class contains only one character OR only a class
  ** specifier use a more compact representation. Also if the class
  ** contains all the characters, it is replaced by the op ANY.  
  */
  numbits = setbit(0,NULL);
  numcls  = setcls(0,NULL);
  /*fprintf(stderr,"++ %d,%d\n",numbits,numcls);*/ 
  if (numbits == 256) {
    storeop(r,ANY); /* any character is specified! */
    numbits = 0;
  }
  else if (numcls == 0) { /* There is no class specifier (\l,\d, ...) */
    if (numbits == 1) {   /* and only one character!                  */
      /* find stored char */
      for (chr=0; chr < 256 && !tstbit(chr,bmp); chr++) ;
      if (*ccl & 0x40) {
        storeop(r,NOTCHR); /* everything but that char */
        store(r,chr);
      } else {
        storech(r,chr); /* it's a single char! */
      }
      numbits = 0;
    }
  } 
  else if (numbits == 0) {
    if (numcls == 1) {
      chr = 0;
      while ((chr < 8) && ((ccl[1] & (1<<chr)) == 0)) chr++;
      chr += 0xB0;
      if (*ccl & 0x40) chr |= 0x40;
      storeop(r,chr); /* it's a single class! */
      numcls = 0;
    }
  }
  
  if ((numcls > 0) || (numbits > 0)) {
    storeop(r,ccl[0]);
    store(r, ccl[1]);
    
      
    /*  STEP 2: Compress the bitmap <:compress>
    **   This is the trickiest part. It is inspired to the RLE
    ** (run length encoding) compression techniques.
    **   Sequences of 7 equal bits are grouped and replaced by
    ** a counter. There are three type of bytes:
    **
    **   [10xxxxxx] a sequence of n 7-bits sequences set to 0;
    **   [11xxxxxx] a sequence of n 7-bits sequences set to 1;
    **   [0xxxxxxx] 7 bits with mixed 0's and 1's.
    **
    **   This scheme has the property that it produces no null
    ** bytes. In fact a byte |00000000| is a sequence of seven
    ** bits set to 0 and, hence, represented as |10000001|.
    **
    **   Note that in the worst case this scheme occupies more
    ** than the orignal bitmap! Including a character every 7
    ** leads to a 37 bytes long representation instead of 32.
    ** Such a case is so uncommon in practice that we can safely
    ** ignore it. Most of the times the character class will
    ** have long sequences of 0 (for characters not in the class)
    ** and 1 (for intervals of characters), hence the saving.
    **
    */
    
    chr=0;  /* the character to check the presence for */
    seq=0;  /* the bit sequence examined so far */
    bit=1;  /* the bit to set in {seq} if {chr} is present */
    last=0;    /* last compressed sequence */
    while(chr < 256) {
      if (tstbit(chr,bmp) && !rx_isinccl_class(chr,ccl)) {
        /*   The character {chr} is in the bitmap and is not 
        ** already included in a class recognizer.
        */
        seq |= bit;
      }
      chr++;
      
      /* Note that bit is always equal to |(1 << (chr % 7))| */
      bit = bit<<1;
      
      if (((chr % 7)==0) || (chr==256)) {
        /*   Every 7 bits the sequence is examined to see if it can
        ** be compressed (i.e. has all bits to 1 or 0).
        **   The variable {last} holds the last counter for the
        ** compressed sequences. Its value can have three forms:
        **
        **   [000000000] Uncompressed sequences
        **   [10xx xxxx] A sequence of compressed 0
        **   [11xx xxxx] A sequence of compressed 1
        **
        */
        t=0;
        switch (seq) {
          case 0x7F : t  =0x40;
          case 0x00 : t |=0x80;
                      /* Found a sequence of 7 equal bits!
                      ** Now {t} is |0x80| if {seq} contained all 0
                      ** or |0xC0| if {seq} contained all 1.
                      ** If {last} held a compressed sequence of the same
                      ** type, simply increment it, otherwise store it
                      ** in the nfa and reset it to the new type
                      */
                      if ((last & 0xC0) == t)
                        last++;
                      else {
                        if (last != 0) store(r,last);
                        last=t+1; /* 1 group of 7 bits */
                      }
                      break;
                      
          default   : /* Not a compressible sequence! Before storing
                      ** it in the nfa, store the last compressed 
                      ** sequence (if any).
                      */
                      if (last != 0) {
                        store(r,last); last=0;
                      }
                      store(r,seq);
                      break;
        }
        /* reset the sequence and the bit to their initial value */
        seq=0x00; bit=0x01;
      }
    }
    
    /* The last byte won't be stored if it contains only zeros.   */
    /*fprintf(stderr,"== %d\n",r->cur - r->lastop);*/
    if ((r->cur - r->lastop == 2) ||  /* there is only one byte    OR   */
        (last & 0xC0) == 0xC0)        /* there are 1's in the last byte */
      store(r, last);
    
    /*   The length of compressed bitmap is stored as the least
    ** significant 6 bits of the first byte. It is guaranteed to fit
    ** since in the worst case we'll need 37 bytes (|ceil(256/7)|).
    */
    *(r->lastop) |= (r->cur - r->lastop - 2); 
  }
  return (str-s);    /* return the amount of input consumed */
}

static void storejmp(rexptrs *r, unsigned char op, int offset)
{
  offset &= 0x07FF;
  storeop(r, op | (offset >> 7));
  store(r,0x80 | (offset & 0x7F));
}

#define storeonfail(r, o) storejmp(r,ONFAIL,o)
#define storegoto(r, o) storejmp(r,GOTO,o)

static unsigned char *labels[3000];
static unsigned char *startalt(rexptrs *r, unsigned char *p)
{
  labels[alt_cur_label] = r->cur+2;
  push(alt_stack,alt_cur_label++); /* label to '(XX' */
  labels[alt_cur_label] = r->cur;
  push(alt_stack,alt_cur_label++); /* label to '(' */
  push(alt_stack,alt_cur_label++); /* label to ')' */
  push(alt_stack,alt_cur_label);   /* label to '|' */
  storeonfail(r,alt_cur_label++);  
  return p;
}

static unsigned char *newalt(rexptrs *r, unsigned char *p)
{
  if (alt_stack_count == 0)
    error("ERR105: Unexpected '|'");
  storegoto(r,top(alt_stack,1));
  labels[top(alt_stack,0)] = r->cur;
  pop(alt_stack);
  push(alt_stack,alt_cur_label);
  storeonfail(r,alt_cur_label++);
  return p;
}

static unsigned char *endalt(rexptrs *r, unsigned char *p)
{
  unsigned char min,max;
  
  if (alt_stack_count == 0)
    error("ERR106: Unexpected ')'");
    
  min=0; max=0; 
  
  switch (p[1]) {
    case '<' : p+=2;
               while ('0' <= *p && *p <= '9') min = min * 10 + (*p++ - '0');
               if (*p == ',') {
                 p++;
                 while ('0' <= *p && *p <= '9') max = max * 10 + (*p++ - '0');
               }
               else max = min; 
      
               if (*p != '>' || ((max > 0) && (min > max)) ||
                   (min > 200) || (max > 200)) {
                 /*fprintf(stderr,"<> %d %d\n",min,max);*/
                 error("ERR124: Bad closure limits");
               }
               goto clo;
               
     case '*' : p++; min = 0; goto clo;           
     case '+' : p++; min = 1; goto clo;           
     case '?' : p++; max = 1; goto clo;
     
          clo : labels[top(alt_stack,1)] = r->cur;
                if (max != 1) { 
                  if (max > 0) { 
                    storeop(r,BKMAX);
                    store(r,max);
                  } 
                  else {
                    storeop(r,BACK);
                  }
                  storegoto(r,top(alt_stack,2));
                }
                else storeop(r,ONFEND);
                labels[top(alt_stack,0)] = r->cur;
                if (min > 0) {
                  storeop(r,MIN); store(r,min);
                }
                else {
                  storeop(r,MINANY);
                }
                break;
                
     case '!' : p++; 
                labels[top(alt_stack,1)] = r->cur;
                storeop(r,ONFEND);  
                storeop(r,FAIL);  
                labels[top(alt_stack,0)] = r->cur;
                break;

     case '#' : p++; 
                storeop(r,ONFEND);  
                storegoto(r,top(alt_stack,1));
                labels[top(alt_stack,0)] = r->cur;
                storeop(r,ANY);  
                storeop(r,BKABS);
                storegoto(r,top(alt_stack,2));
                labels[top(alt_stack,1)] = r->cur;
                storeop(r,MINANY);
                break;
  
     case '&' : p++;
                storeop(r,PEEKED); 
                storegoto(r,top(alt_stack,1));
                labels[top(alt_stack,0)] = r->cur;
                storeop(r,FAIL);  
                labels[top(alt_stack,1)] = r->cur;
                break;
                
     default  : storegoto(r,top(alt_stack,1));
                labels[top(alt_stack,0)] = r->cur;
                storeop(r,FAIL);  
                labels[top(alt_stack,1)] = r->cur;
                storeop(r,ONFEND);
                break;  
  }
  pop(alt_stack);
  pop(alt_stack);
  pop(alt_stack);
  pop(alt_stack);
  return p;
}

static void fixalt(rexptrs *r)
{
   int   n;
   unsigned char op;
  /*
  for (n=0; n<alt_cur_label;n++) {
   fprintf(stderr,"[%d]\t%p\n",n,labels[n]);
  }
  */
  fflush(stderr);
  
  r->cur = r->first;
  if (alt_stack_count > 0)
    error("ERR116: Unclosed parenthesis");
  op = *(r->cur);
  while ( op != END) {
   /* fprintf(stderr,"+> OP %p %02X\n",r->cur,*(r->cur)); */
    
    switch (optype(op)) {
      case CAPTR   : if (op == ESCAPE) (r->cur)++;
                     break;
      case GOTO    : if (*(r->cur) == ONFEND) break;
                     n = jmparg(r->cur);
                     /* fprintf(stderr,"++ %p %p %d\n",r->cur,labels[n],labels[n] - r->cur);*/
                      
                     n  = labels[n] - r->cur;
                     if (n < 0) n = 2-n;
                     else n = n-2;
                     if (n > 0x07FF) {
                       error("ERR112: JMP argument out of range");
                     }
                     (r->cur)[0] = ((r->cur)[0] & 0xF0) | (n >> 7);
                     (r->cur)[1] = 0x80 | (n & 0x7F);
                     (r->cur)++;
                     break; 
                    
      case SINGLE : switch (op) {
                     /* arg is 2 bytes */
                     case PATTERN:
                     case BRACED: (r->cur)++;
                     
                     /* arg is 1 byte */
                     case MIN   :
                     case BKMAX : (r->cur)++;
                                  break;
                  }
                  break;
                  
      case STR :  (r->cur) += (op == NOTCHR)? 1 : STR_len(op);
                  break;
                  
      case CCL  : (r->cur) += CCL_len(op);
                  break;
    }
    (r->cur)++;
    op = *(r->cur);
  }
  
}

static unsigned char *storeclo(rexptrs *r,unsigned char clo,unsigned char *p)
{
  unsigned char *t,*q;
  unsigned short n;
  unsigned short min=0,max=0;
  
  /* The opcode for the closure is to be inserted
  ** before last opcode.
  */
  n=*(r->lastop);
  if (optype(n) == STR && n != NOTCHR) {
    /* take latest character of the string */
    if (STR_len(n) > 1) {
     *(r->lastop) = n-1;
      storeop(r,END);               /* make room */
     *(r->lastop) = *(r->lastop-1); /* move last letter of the string */
      r->lastop--;                  /* the actual op is a ... */
     *(r->lastop) = STR | 1;        /* string of lenght 1 */ 
    }
  }
  
  q = r->lastop;
  labels[alt_cur_label++] = r->lastop;
  
  storeonfail(r,alt_cur_label++);

  min = (r->lastop)[0]; max = (r->lastop)[1];

  t = r->cur;
  while (--t > q) {
    *t = *(t-2);
  }
  *t++ = min;
  *t++ = max;
  
  min=0,max=0;
  switch (*p) {
    case '<' :
               min=0;p++;
               while (min < 200 && isdigit(*p)) min=min*10 + (*p++ - '0');
               if (*p == ',') {
                 max=0; p++;
                 while (max < 200 && isdigit(*p)) max=max*10 + (*p++ - '0');  
               }
               else max = min;
               if (*p != '>' || (max > 0 && (min>max)) || (min>200) || (max>200)) {
                 error("ERR104: Bad closure limits");
               }
               goto clo;
               
    case '*' : min = 0; goto clo;
    case '+' : min = 1; goto clo;
    case '?' : max = 1; goto clo;
    
         clo : if (max != 1) { 
                  if (max > 0) { 
                    storeop(r,BKMAX);
                    store(r,max);
                  } 
                  else {
                    storeop(r,BACK);
                  }
                  storegoto(r,alt_cur_label-2);
                }
                if (max == 1)  storeop(r,ONFEND);
                labels[alt_cur_label-1] = r->cur;
                if (min > 0) {
                  storeop(r,MIN); store(r,min);
                }
                else {
                  storeop(r,MINANY);
                }
                break;
                
    case '!' :  storeop(r,ONFEND);  
                storeop(r,FAIL);  
                labels[alt_cur_label-1] = r->cur;
                break;
                
    case '#' :  storeop(r,ONFEND);  
                storegoto(r,alt_cur_label);
                labels[alt_cur_label-1] = r->cur;
                storeop(r,ANY);  
                storeop(r,BKABS);
                storegoto(r,alt_cur_label-2);
                labels[alt_cur_label++] = r->cur;
                storeop(r,MINANY);
                break;                
                
    case '&' :  storeop(r,PEEKED); 
                storegoto(r,alt_cur_label++);
                labels[alt_cur_label-2] = r->cur;
                storeop(r,FAIL);  
                labels[alt_cur_label-1] = r->cur;
                break;

  }
  return p;
}               

static char *storeesc(rexptrs *r, unsigned char *p)
{
  unsigned char chr = *p;
  unsigned char op;
  
  switch (chr) {
    case '\0': storech(r,'\\');
               break;

    case 'x' : if (isxdigit(p[1])) {
                 chr = hex(*++p);
                 if (isxdigit(p[1])) {
                   chr = (chr << 4) | hex(*++p);
                 }
                 storech(r,chr);
               }
               else 
                 storech(r,*p); 
               break;
               
    case 'o' :            
    case '0' : if ('0' <= p[1] && p[1] <= '7') {
                 chr = *++p - '0';
                 if ('0' <= p[1] && p[1] <= '7') {
                   chr = (chr << 3) | (*++p - '0');
                   if ('0' <= p[1] && p[1] <= '7') {
                     chr = (chr << 3) | (*++p - '0');
                   }
                 }
                 storech(r,chr);
               }
               else 
                 storech(r,*p); 
               break;
                   
    case '1' : case '2' : case '3' : case '4' :
    case '5' : case '6' : case '7' : case '8' :
               chr -= '1';
               if (!(def_capt & (1 << chr)))  {
                 error("ERR107: Reference to an undefined capture");
               }
               storeop(r,CAPT | chr);
               break;
               
    case 'b' : storech(r,'\b');    break;
    case 'n' : storech(r,'\n');    break;
    case 'f' : storech(r,'\f');    break;
    case 'r' : storech(r,'\r');    break;
    case 't' : storech(r,'\t');    break;
    case 'v' : storech(r,'\v');    break;
        
    case 'a' : storeop(r,ALPHA);   break;
    case 'd' : storeop(r,DIGIT);   break;
    case 'u' : storeop(r,ANYUPR);  break;
    case 'l' : storeop(r,ANYLWR);  break;
    case 's' : storeop(r,SPACE);   break;
    case 'q' : storeop(r,ALNUM);   break;
    case 'h' : storeop(r,XDIGIT);  break;
    case 'w' : storeop(r,WORDC);   break;
    case 'c' : storeop(r,CTRL);    break;
    case 'p' : storeop(r,PUNCT);   break;
    case 'z' : storeop(r,ZERO);    break;
    
    case 'L' : op = NEWLN;   goto rec;
    case 'Q' : op = QSTR;   goto rec;
    case 'H' : op = NHEX;   goto rec;
    case 'F' : op = NFLOAT; goto rec;
    case 'X' : op = FAIL;   goto rec;
    case 'I' : op = IDENT;  goto rec;
    case 'N' : op = NINT;   goto rec;
         rec : storeop(r,op);
               break;
               
    case 'y' : if (p[1] == '*') {
                 storeop(r,SPCS); p++;
               }
               else   
                 storeop(r,SPCTAB);  break;
    
    case 'e' : storeop(r,ESCOFF);    break;

    case 'E' : if (p[1] == '\\') p++; 
               if (p[1] != '\0') {
                 storeop(r,ESCAPE);
                 store(r,*++p);
               }
               break;

    case 'B' : if (p[1] != '\0' && p[2] != '\0' && p[1] != p[2]) {
                 if (p[3] == '?') {
                   storeonfail(r,alt_cur_label);
                 } 
                 storeop(r,BRACED);
                 store(r,*++p);
                 store(r,*++p);
                 if (p[1] == '?') {
                   labels[alt_cur_label++] = r->cur;
                   storeop(r,ONFEND);
                   p++;
                 } 
               }
               else  error("ERR108: Invalid braced pattern");
               break;

    case 'C' : storeop(r,CASE);
               r->casesensitive ^= 1; break;
    
    default  : storech(r,*p); break;
  }
  return p;
}

/* = Compile
**   ~~~~~~~
**  
*/

static char *compile(const unsigned char *pat, unsigned char *nfa,
                                                          unsigned long maxlen)
{
  unsigned char *p = (unsigned char *)pat;
  static rexptrs rex;
  rexptrs *r;
  /*unsigned char c;*/
  unsigned char n;
  unsigned char bol = 0;
  unsigned char clo = 0;
  unsigned short l;
  
  r = &rex;
  
  rex.first         = nfa;
  rex.cur           = rex.first;
  rex.lastop        = rex.first;
  rex.len           = 0;
  rex.casesensitive = 1;

  *(r->cur) = END;
  r->error =NULL;
  if (setjmp(compile_fail) != 0) {
    return r->error;
  }

  num_capt = 0;
  
  storeop(r,PATTERN);
  store(r,0x80);
  store(r,0x80);
  
  while (*p) {
    /*c=0;*/
    switch (*p) {
      case '.' : storeop(r,ANY) ;
                 clo = 1;
                 break;
      
      case '^':  if (p == pat) {
                   storeop(r,BOL) ;
                   bol = 1;
                   clo = 0;
                 }
                 else {
                   storech(r,*p);
                   clo = 1; 
                 }
                 break;
      
      case '$':  if (p[1] == '\0') storeop(r,EOL);
                 else              storech(r,*p);
                 clo = 1;
                 break;

      case '{':  if (num_capt < RX_MAXCAPT) {
                   push(capt_stack,num_capt);
                   storeop(r, BOC | num_capt++);
                 }      
                 else error("ERR109: Too many captures");
                 clo = 0;
                 break;
                 
      case '}':  if (empty(capt_stack))
                   error("ERR110: No capture to close");
                 n=pop(capt_stack);
                 storeop(r, EOC | n);
                 def_capt |= (1 << n);
                 clo = 0;
                 break;    
                 
      case '+':  
      case '*':  
      case '<':  
      case '?':  
      case '!':  
      case '#':  
      case '&':  if (!clo)
                   error("ERR119: unexpected closure");
                 p = storeclo(r,*p,p);
                 clo = 0;
                 break;
                 
      case '[' : if (p[1] == '\0')
                   storech(r,*p);
                 else 
                   p += storeccl(r,p+1);
                 clo = 1;
                 break;
                 
      case '\\': switch(p[1]) {
                   case 'E':
                   case 'e':
                   case 'C': clo = 0;
                             break;
                   default : clo = 1;
                             break;
                 }
                 p = storeesc(r,p+1);
                 break;

      case '(' : p = startalt(r,p); 
                 clo = 0;
                 break;
                                    
      case '|' : p = newalt(r,p); 
                 clo = 0;
                 break;
                                    
      case ')' : p = endalt(r,p); 
                 clo = 0;
                 break;
                                    
      default :  storech(r,*p);
                 clo = 1;
                 break;
                       
    }
    p++;
    *(r->cur)   = END;
  }
  *(r->cur++) = MATCH;
  *(r->cur++) = bol ? FAILALL :FAIL;
  *(r->cur)   = END;
  
  if (capt_stack_count > 0) 
    error("ERR117: Unclosed capture"); 
    
  if (alt_cur_label > 1) {
    fixalt(r);
  }
  /* set the argument of the PATTERN op to the length of the pattern */
  l = (r->cur - r->first)-3;
  p = r->first;
  p[1] = 0x80 | (l & 0x7F) >> 7;
  p[2] = 0x80 | (l & 0x7F);  
  
  return NULL;
}

char *rx_compile(const unsigned char *pat, unsigned char *nfa)
{
  return compile(pat,nfa,RX_MAXNFA);
}

char *rx_compile_add(const unsigned char *pat, unsigned char *nfa)
{
  char *err;
  unsigned char fail=END;
  
  if (*nfa != END) {
    while (*nfa != END) {
     nfa++;
    }
    fail = *--nfa; 
  }
  err = compile(pat,nfa,RX_MAXNFA);
  
  if (err == NULL) {
    /* fix failure types (FAI or FAA) */
    while (*nfa != END) nfa++;
    if (fail != END && nfa[-1] != fail) nfa[-1] = FAIL;
  }
  
  return err;
}

