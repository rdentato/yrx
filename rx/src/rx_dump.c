/*  YRX - rx_dump.c
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
** This code is a derivative work of the PUBLIC DOMAIN regex routines
** by Ozan Yigit: http://www.cs.yorku.ca/~oz/
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
    else 
      fprintf(f,"%c",c);
}

/* = Symbolic dump
**   ~~~~~~~~~~~~~
**   This is intended mainly to help debugging regular expressions.
** The function prints a human readable version of a compiled regular
** expression.
*/

#define incrnfa()  ((*nfa)? ++nfa : nfa)
#define addnfa(_x) do { int _y=_x; if ((nfa+_y) <maxnfa) nfa+=_y; } while (0)

void rx_symdump(FILE *f,unsigned char * nfa)
{
  int n;
  int back = 0;
  unsigned char *maxnfa;
  
  if (nfa != NULL) {
    maxnfa = nfa+strlen(nfa);
    while (*nfa != END) {
      fprintf(f,"%p",nfa);
      
      switch (*nfa) {
        case ANY   : fprintf(f,"\tANY\n");            break;
        case REPT0 : fprintf(f,"\tREPEAT 0..n\n");  break;
        case REPT1 : fprintf(f,"\tREPEAT 1..n\n");  break;
        case OPT   : fprintf(f,"\tREPEAT 0..1\n");     break;
        case REPT  : incrnfa();
                     n=*nfa;
                     if (n>200)
                       n=0;
                     fprintf(f,"\tREPEAT %d..",n);
                     n=*(incrnfa());
                     if (n>200)
                       n=0;
                     if (n>0)
                       fprintf(f,"%d", n);
                     else
                       fprintf(f,"n");
                     fprintf(f,"\n");
                     break;
#if 0    
        case BOW   : fprintf(f,"\tBOW\n");            break;
        case EOW   : fprintf(f,"\tEOW\n");            break;
#endif
        case BOL   : fprintf(f,"\tBOL\n");            break;
        case EOL   : fprintf(f,"\tEOL\n");            break;
        case WORDC : fprintf(f,"\tWORDC\n");          break;
        case SPACE : fprintf(f,"\tSPACE\n");          break;
        case SPCS  : fprintf(f,"\tSPACES\n");         break;
        case IDENT : fprintf(f,"\tIDENT\n");          break;
        case DIGIT : fprintf(f,"\tDIGIT\n");          break;
        case XDIGIT: fprintf(f,"\tXDIGIT\n");         break;
        case ALNUM : fprintf(f,"\tALNUM\n");          break;
        case ALPHA : fprintf(f,"\tALPHA\n");          break;
        case ANYUPR: fprintf(f,"\tANYUPR\n");         break;
        case ANYLWR: fprintf(f,"\tANYLWR\n");         break;
        case CTRL  : fprintf(f,"\tCTRL\n");           break;
        case PUNCT : fprintf(f,"\tPUNCT\n");          break;
        case SPCTAB: fprintf(f,"\tSPCTAB\n");         break;
        case ZERO  : fprintf(f,"\tZERO\n");           break;
        case QSTR  : fprintf(f,"\tQSTR\n");           break;
        case EMPTY : fprintf(f,"\tEMPTY\n");          break;
        case NINT  : fprintf(f,"\tINTEGER\n");        break;
        case NHEX  : fprintf(f,"\tHEXINTEGER\n");     break;
        case NFLOAT: fprintf(f,"\tFLOATINGPOINT\n");  break;
        case ESCAPE: fprintf(f,"\tESCAPE\t");
                     if (*(incrnfa()) == ' ') fprintf(f,"OFF");
                     else outsymc(f,*nfa);
                     putc('\n',f);
                     break;
        case BRACED: fprintf(f,"\tBRACED\t");
                     outsymc (f,*(incrnfa()));
                     outsymc (f,*(incrnfa()));
                     putc('\n',f);
                     break;
        case CASE  : fprintf(f,"\tCASE\n");           break;
        
        case FAIL:   fprintf(f,"\tFAIL\n");         break;
        case FAILALL:fprintf(f,"\tFAIL\tALL\n");         break;
        case PEEKED: fprintf(f,"\tPEEKED\n");    break;
        case ONFEND: fprintf(f,"\tONFAIL\tEND\n");    break;
        
        case BKMAX : n = *(incrnfa());
                     if (n == 255)
                        fprintf(f,"\tBKWARD\n");
                     else
                        fprintf(f,"\tBKMAX\t%d\n",n);
                     back = 1;
                     break;

        case MIN   : fprintf(f,"\tMIN\t%d\n",*(incrnfa()));
                     break;
                     
        case MATCH:  fprintf(f,"\tMATCH\n");           break;
        case PATTERN: n=((nfa[1] & 0x7F) << 7) + (nfa[2] & 0x7F); 
                     fprintf(f,"\tPATTERN\t%p\t;+%d\n",nfa+2+n,n);
                     incrnfa();incrnfa();
                     break;
                     
        case NOTCHR :
                  fprintf(f,"\tNOTCHR\t");
                  outsymc(f,nfa[1]);
                  addnfa(CCL_len(*nfa)+1);
                  fprintf(f,"\n");
                  break;
                  
        case NOTCLS :
                  fprintf(f,"\tNOTCLS\n");
                  break;

         default: switch (optype(*nfa)) {
                                             
                case GOTO    : if ((*nfa & 0xF0) == ONFAIL)
                                 fprintf(f,"\tONFAIL");
                               else 
                                 fprintf(f,"\tGOTO");
                               n = jmparg(nfa);
                               fprintf(f,"\t%p",(back)?nfa - n +2 : nfa + n+2);
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
                              case CAPT: fprintf(f,"\tCAPT"); break;
                              case BOC : fprintf(f,"\tBOC"); break;
                              case EOC : fprintf(f,"\tEOC"); break;
                            }
                            fprintf(f," %d\n",CAPT_num(*nfa));
                            break;
            
                case CCL  :  fprintf(f,"\tCCL ");
                             for (n=0;n<256;n++) {
                               if (rx_isinccl(n,nfa)) outsymc(f,n);
                             }
                             addnfa(CCL_len(*nfa)+1);
                             fprintf(f,"\n");
                             break;
                   }
                   break;
      }
      incrnfa();
    }
  }
  fprintf(f,"%p\tEND\n",nfa);
}

/* Since NFAs do not contain embedded zeros, they could be treated as
** regular C strings.
** Nonetheless they have to be printed char by char to correctly handle
** control and non-ascii characters (as well as quotes and backslash!).
*/
void rx_hexdump(FILE *f, unsigned char * nfa)
{
  while (*nfa != END) {
    outc(f,*nfa++);
  }
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

