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

#include "rx_.h"


/* = Printing characters
**   ~~~~~~~~~~~~~~~~~~~
**   Dumped nfa must be readable for humans and machines. These two
** little helpers below ensure that binary dumps can be safely
** included as strings in a C source file and symbolic dumps show
** control characters as "^x".
*/

static void outc(FILE *f,int c)
{
  static int ch=0;
  
    /* Avoid Backslash, control characters, non ASCII, quotes 
    ** Note that also digits are printed as escaped after an 
    ** escaped character. This is to avoid that '\12' '0' is
    ** interpreted as '\120'        
    */
    if ((c == '\\') || (c < ' ') || (c >= 0x7F) || (c == '"') ||
        ((ch == 1) && ('0' <= c) && (c <= '9'))) {
      fprintf(f,"\\%o",c);
      ch = 1;
    }
    else { 
      fprintf(f,"%c",c);
      ch = 0;
    }
}

static void outsymc(FILE *f,int c)
{
    if ((c < ' ')) 
      fprintf(f,"^%c",0x40+c); /* control character */
    else if ((c == '*')) 
      fprintf(f,"\\%03o",c); /* control character */
    else 
      fprintf(f,"%c",c);
}

/* = Symbolic dump
**   ~~~~~~~~~~~~~
**   This is intended mainly to help debugging regular expressions.
** The function prints a human readable version of a compiled regular
** expression.
*/

#define incrnfa()  (cnt++,((*nfa)? ++nfa : nfa))
#define addnfa(_x) do { int _y=_x; if ((nfa+_y) <maxnfa) {nfa+=_y; cnt+=_y;} } while (0)

void rx_dump_asm(FILE *f,unsigned char * nfa)
{
  int n;
  int back = 0;
  unsigned int cnt = 0;
  unsigned char *maxnfa;
  
  if (nfa != NULL) {
    maxnfa = nfa+strlen(nfa);
    while (*nfa != END) {
      fprintf(f,"%04X\t%02X",cnt,*nfa);
      
      switch (*nfa) {

        case BOL   : fprintf(f,"\tBOL\n");     break;
        case EOL   : fprintf(f,"\tEOL\n");     break;
        
        case SPCS  : fprintf(f,"\tWSP\n");     break;
        case IDENT : fprintf(f,"\tIDN\n");     break;
        case ZERO  : fprintf(f,"\tZRO\n");     break;
        case QSTR  : fprintf(f,"\tQST\n");     break;
        case NINT  : fprintf(f,"\tINT\n");     break;
        case NHEX  : fprintf(f,"\tHEX\n");     break;
        case NFLOAT: fprintf(f,"\tFLT\n");     break;
        case ESCOFF: fprintf(f,"\tESC\tOFF\n");     break;
        case ESCAPE: fprintf(f,"\tESC\t");
                     outsymc(f,*(incrnfa()));
                     putc('\n',f);
                     break;
        case BRACED: fprintf(f,"\tBRC\t");
                     outsymc (f,*(incrnfa()));
                     outsymc (f,*(incrnfa()));
                     putc('\n',f);
                     break;
        case CASE  : fprintf(f,"\tCSE\n");           break;
        case NEWLN : fprintf(f,"\tNLN\n");           break;
        case ESCANY: fprintf(f,"\tANE\n");           break;
        
        case FAIL:   fprintf(f,"\tFAI\n");         break;
        case FAILALL:fprintf(f,"\tFAA\n");         break;
        case PEEKED: fprintf(f,"\tRST\n");    break;
        case ONFEND: fprintf(f,"\tOFF\n");    break;
        
        case BKABS : n = 257; goto bk;
        case BACK  : n = 256; goto bk;
        case BKMAX : n = *(incrnfa());
                     fprintf(f,"%02X",n);                     
                bk : if (n == 256)
                        fprintf(f,"\tJFO\t0,");
                     else if (n == 257)
                        fprintf(f,"\tJMP\t");
                     else
                        fprintf(f,"\tJFO\t%d,",n);
                     incrnfa();
                     n = jmparg(nfa);
                     fprintf(f,"%04X",cnt - n +2);
                     fprintf(f,"\t;-%d\n",n); 
                     incrnfa();
                     break;

        case MIN   : fprintf(f,"\tMIN\t%d\n",*(incrnfa()));
                     break;
                     
        case MINANY: fprintf(f,"\tMIN\t0\n");
                     break;
                     
        case MATCH:  fprintf(f,"\tMTC\n");           break;
        case PATTERN: n=((nfa[1] & 0x7F) << 7) + (nfa[2] & 0x7F); 
                     fprintf(f,"%02X%02X\tNXT\t%04X\t;+%d\n",nfa[1],nfa[2],cnt+2+n,n);
                     incrnfa();incrnfa();
                     break;
                     
        case NOTCHR :
                  fprintf(f,"\tNCH\t");
                  outsymc(f,nfa[1]);
                  incrnfa();
                  fprintf(f,"\n");
                  break;
                  
         default: switch (optype(*nfa)) {
                                             
                case GOTO    : fprintf(f,"%02X",nfa[1]);
                               if ((*nfa & 0xF0) == ONFAIL)
                                 fprintf(f,"\tOFJ");
                               else 
                                 fprintf(f,"\tJMP");
                               n = jmparg(nfa);
                               fprintf(f,"\t%04X",(back)?cnt - n +2 : cnt + n+2);
                               fprintf(f,"\t;%c%d\n",(back? '-': '+'),n); 
                               back = 0;
                               incrnfa();
                               break; 
                            
                case STR : fprintf(f,"\tSTR\t");
                            n=STR_len(*nfa);
                            while (n) {
                              outsymc (f,*(incrnfa()));
                              n--;
                            } 
                            fputc('\n',f);
                            break;
                            
                case CAPTR : switch (CAPT_type(*nfa)) {
                              case CAPT: fprintf(f,"\tCPT"); break;
                              case BOC : fprintf(f,"\tBOC"); break;
                              case EOC : fprintf(f,"\tEOC"); break;
                            }
                            fprintf(f,"\t%d\n",CAPT_num(*nfa));
                            break;
            
                case CCL  : fprintf(f,"\t");
                            if (iscls(*nfa)) {
                              if (*nfa & 0x40) fprintf(f,"NOT ");
                              switch((*nfa & 0xBF)) {  
                                case ANY   : fprintf(f,"ANY"); break;
                                case WORDC : fprintf(f,"WRD"); break;
                                case SPACE : fprintf(f,"SPC"); break;
                                case DIGIT : fprintf(f,"DGT"); break;
                                case XDIGIT: fprintf(f,"HDG"); break;
                                case ANYUPR: fprintf(f,"UPR"); break;
                                case ANYLWR: fprintf(f,"LWR"); break;
                                case ALNUM : fprintf(f,"ALN"); break;
                                case ALPHA : fprintf(f,"ALP"); break;
                                case CTRL  : fprintf(f,"CTL"); break;
                                case PUNCT : fprintf(f,"PNC"); break;
                                case SPCTAB: fprintf(f,"STB"); break;
                                case ZERO  : fprintf(f,"ZRO"); break;
                              }
                            }
                            else {
                              fprintf(f,"CCL ");
                              for (n=0;n<256;n++) {
                                if (rx_isinccl(n,nfa)) outsymc(f,n);
                              }
                              addnfa(CCL_len(*nfa));
                            }
                            fprintf(f,"\n");
                            break;
                             
                default: fprintf(f,"0x%02X\n",*nfa);
                         break; 
          }
          break;
      }
      incrnfa();
    }
  }
  fprintf(f,"%04X\t00\tEND\n",cnt);
}

/* Since NFAs do not contain embedded zeros, they could be treated as
** regular C strings.
** Nonetheless they have to be printed char by char to correctly handle
** control and non-ascii characters (as well as quotes and backslash!).
*/
void rx_dump_str(FILE *f, unsigned char * nfa)
{
  while (*nfa != END) {
    outc(f,*nfa++);
  }
}

void rx_dump_num(FILE *f, unsigned char * nfa)
{
  int cnt = 0;
  do {
    fprintf(f,"%3d,",*nfa);
    if ((++cnt & 0x0F) == 0) fputc('\n',f);
  } while (*nfa++ != END) ;
}


#ifdef UTEST

#include "ut.h"

typedef void (*outfunctype)(FILE *f,int c)  ;

static char *outtest(char *fname, outfunctype func)
{
  FILE *ftst;
  int c;
  
  ftst=UTfopen(fname,"wb");
  
  for (c=0; c<=255; c++) func(ftst,c);
  fclose(ftst);
  
  UTCheckFileEqual(fname, "../test/testdata"); 
  remove(fname); 
  
  return NULL;
}


TESTCASE(outc,"Output char")
TESTRESULT(outtest("outc.chk",outc))


TESTCASE(outsymc,"Output symbolic char")
TESTRESULT(outtest("outsymc.chk",outsymc))

TESTCASE(hexdump,"Hexadecimal dump")
  FILE *ftst=NULL;
  char *fname="hexdump.chk";
  
  ftst=UTfopen(fname,"wb");

  rx_hexdump(ftst,"TestTest");
  rx_hexdump(ftst,"\377\377\12\eèéx");
  rx_hexdump(ftst,"");
  rx_hexdump(ftst,"\0aaaa");
  rx_hexdump(ftst,"bbbb\0");
  rx_hexdump(ftst,"\1\1\1\1\2\2\2\f\n\baaAAbb");
  fclose(ftst);  
  
  UTCheckFileEqual(fname, "../test/testdata"); 
  remove(fname);
TESTRESULT(NULL)

TESTCASE(symdump,"Symbolic Dump")
  FILE *ftst=NULL;
  char *fname="symdump.chk";
  
  ftst=stdout;
  ftst=UTfopen(fname,"wb");
  rx_symdump(ftst,"bab");
  rx_symdump(ftst,"@ABCDEFGHIJKLSTUVWXYZ[]^_");
  rx_symdump(ftst,"MxPKQKNKO\4\5KO\4\377KO\377\4K");
  rx_symdump(ftst," !\"#$%&'()*+,-./01234567");
  rx_symdump(ftst,"\202\200\211x\377C\376C");
  
  rx_symdump(ftst,NULL);            /* NULL */
  rx_symdump(ftst,"");              /* empty */
  rx_symdump(ftst,"dab");           /* Invalid string */
  rx_symdump(ftst,"d");             /* Invalid string */
  rx_symdump(ftst,"Pbab");          /* Invalid repetition */
  rx_symdump(ftst,"\204\200\215@"); /* Invalid class */
  rx_symdump(ftst,"\375");          /* Invalid class */
  rx_symdump(ftst,"\1cabc\2bcd");   /* Invalid opcodes */
  
  fclose(ftst);  
  UTCheckTextFileEqual(fname, "../test/testdata"); 
  remove(fname);
TESTRESULT(NULL)



#endif

