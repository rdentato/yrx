/*
**  (C) 2007 by Remo Dentato (rdentato@users.sourceforge.net)
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
*/

#include "yrxlib.h"
#include "yrx.h"

/*****/

#define MRK 0x02  /* 0 00000 10  MaRK                          */
#define MTC 0x03  /* 0 00000 11  MaTCh                         */

#define CPB 0x06  /* 0 00001 10  Capture Begin                 */
#define CPE 0x07  /* 0 00001 11  Capture End                   */

#define NRX 0x21  /* 00 1000 01  Number of Regular eXpressions */
#define CMP 0x25  /* 00 1001 01  Compare                       */
#define NCP 0x29  /* 00 1010 01  Number of Captures            */
#define GET 0x2D  /* 00 1011 01  GET next input char           */
#define NOP 0x31  /* 00 1100 01  No OPeration                  */
#define DEC 0x35  /* 00 1101 01  DECrement D                   */
#define INC 0x39  /* 00 1110 01  INCrement D                   */
#define ZRO 0x3D  /* 00 1111 01  set D to ZeRO                 */

#define RET 0x04  /* 00 0001 00  RETurn                        */
#define REQ 0x08  /* 00 0010 00  Return on EQual               */
#define RGE 0x0C  /* 00 0011 00  Return on Greater or Equal    */
#define RGT 0x10  /* 00 0100 00  Return on Greater Than        */
#define RLE 0x14  /* 00 0101 00  Return on Less or Equal       */
#define RLT 0x18  /* 00 0110 00  Return on Less Than           */
#define RNE 0x1C  /* 00 0111 00  Return on Not Equal           */

#define ONR 0x20  /* 00 1000 00  ON Return                     */
#define JMP 0x24  /* 00 1001 00  JuMP                          */
#define JEQ 0x28  /* 00 1010 00  Jump on EQual                 */
#define JGE 0x2C  /* 00 1011 00  Jump on Less Than             */
#define JGT 0x30  /* 00 1100 00  Jump on Greater or Equal      */
#define JLE 0x34  /* 00 1101 00  Jump on Not Equal             */
#define JLT 0x38  /* 00 1110 00  Jump on Less or Equal         */
#define JNE 0x3C  /* 00 1111 00  Jump on Greater Than          */

#define isjmp(x) (((x) & 0x23) == 0x20)
#define isret(x) (((x) & 0x23) == 0x00)
#define istag(x) (((x) & 0x03) >  0x01)

uint8_t nargs(uint8_t op)
{
   switch (op & 0x03) {
     case 0x00: return ((op & 0x20) ? 1 : 0);
     case 0x01: if (op & 0x20) return ((op < GET)? 1 : 0);
     default  : /* includes MTC and MRK! */
                return 2;
   } 
}

static ulv_t pgm; 
static ulv_t trg; 
static usv_t trg_L;

#define opstr(a) op[a] = #a 
static char *op[0x40];

void yrxASMInit(void)
{
  opstr(CPB);
  opstr(CPE);
  opstr(MTC);
  opstr(MRK);
  opstr(CMP);
  opstr(GET);
  opstr(NCP);
  opstr(NRX);
  opstr(NOP);
  opstr(DEC);
  opstr(INC);
  opstr(ZRO);
  opstr(REQ);
  opstr(RGE);
  opstr(RGT);
  opstr(RLE);
  opstr(RLT);
  opstr(RNE);
  opstr(RET);
  opstr(ONR);
  opstr(JMP);
  opstr(JEQ);
  opstr(JGE);
  opstr(JGT);
  opstr(JLE);
  opstr(JLT);
  opstr(JNE);
  
  pgm = ulvNew();
  if (pgm == NULL) err(934,yrxStrNoMem);
  
  trg = ulvNew(); 
  if (trg == NULL) err(935,yrxStrNoMem);
}

void yrxASMClean(void)
{
  pgm   = ulvFree(pgm); 
  trg   = ulvFree(trg); 
  trg_L = usvFree(trg_L);
}

static uint8_t *dmp_asmchr(uint8_t c)
{
  static char buf[8];
  
  if (c <= 32   || c > 126  || c == '\\' || c == '"' ||
      c == '\'') {
    sprintf(buf,"%02X",c);
  } 
  else {
    buf[0] = '\''; buf[1] = c;
    buf[2] = '\''; buf[3] = '\0';
  }
  return buf;
}

static void dmp_asmstep(uint32_t step)
{
  uint8_t opcode;
  uint32_t arg;
  uint8_t capnum;
  
  opcode  = step & 0xFF;
  arg     = step >> 8;
  
  if (opcode == CMP) {
    fprintf(yrxFileOut,"%s %s\n",op[opcode],dmp_asmchr(arg));
  }
  else if (opcode == NCP) {
    fprintf(yrxFileOut,"%s %u,%u\n",op[opcode], arg & 0xFF, arg >> 8);
  }
  else if (isjmp(opcode)) {
    switch(opcode & 0xC0) {
      case 0x40: capnum = 'A'; break;
      case 0x80: capnum = 'L'; break;
      case 0xC0: capnum = 'R'; break;
      default  : capnum = 'S'; break;
    }
    fprintf(yrxFileOut,"%s %c%u\n",op[opcode & ~0xC0],capnum,arg);
  }
  else if (istag(opcode)) {
    capnum = (opcode & 0x7C) >> 2;
    opcode = (opcode & 0x03);
    
    if (capnum > 0) opcode |= 0x04;
    
    if (capnum == 0) {
      fprintf(yrxFileOut,"%s %u",op[opcode & 0x03],arg & 0xFF);
    }
    else {
      char c = 'X';
      switch (opcode) {
        case CPB : c ='('; break;
        case CPE : c =')'; break;
      }
      fprintf(yrxFileOut,"%c%02u %u",c,capnum,arg & 0xFF);
    }
    arg = arg >> 8;
    if (arg != 0) {
      fprintf(yrxFileOut,",%u",arg);
    }
    fprintf(yrxFileOut,"\n");
  }
  else {
    switch (nargs(opcode)) {
      case 0: fprintf(yrxFileOut,"%s\n",op[opcode]); break;
      case 1: fprintf(yrxFileOut,"%s %X\n",op[opcode],arg); break;
      case 2: fprintf(yrxFileOut,"%s %X\n",op[opcode],arg); break;
    }
  }  
}

static void dumplbl(uint32_t lbl)
{
  char c = ' ';
  
  if (lbl != 0) {
    switch(lbl & 0x000000C0) {
      case 0x00000040: c = 'A'; break;
      case 0x00000080: c = 'L'; break;
      case 0x000000C0: c = 'R'; break;
      default  :       c = 'S'; break;
    }
    fprintf(yrxFileOut,"%c%-5u: ",c,lbl >> 8);
  }
  else 
    fprintf(yrxFileOut,"        ");
}

void asm_dump(void)
{
   uint16_t k;
   
   for (k=0; k < ulvCnt(pgm); k++) {
     dumplbl(ulvGet(trg,k));
     dmp_asmstep(ulvGet(pgm,k));
   } 
}

static uint32_t targ(uint8_t ty)
{
  switch(ty) {
    case 'A' : return 0x40 ; break ;
    case 'L' : return 0x80 ; break ;
    case 'R' : return 0xC0 ; break ;
  }
  return 0;
}

static void addop(uint8_t opcode, uint32_t arg)
{
  uint32_t step;
  
  step = (arg << 8) | opcode ;
  pgm = ulvAdd(pgm,step);
}

#define addop2(c,a,b) addop(c, ((a) << 8) | (b))

static void addtarget(uint8_t ty, uint32_t lbl)
{
  if (ty == 'L' && usvGet(trg_L,lbl) == 0) return;
  
  lbl = (lbl << 8) | JMP | targ(ty);
  trg = ulvSet(trg, ulvDepth(pgm), lbl);
}

static void addjmp(uint8_t opcode, uint8_t ty, uint32_t arg)
{
  if (opcode == JMP && arg == 0)
    opcode = RET;
    
  if (ty == 'L')
    trg_L = usvSet(trg_L,arg,ulvCnt(pgm));

  addop(opcode | targ(ty),arg);
}

static void addtags(tagset_t ts)
{
  uint16_t k;
  uint8_t op;
  uint8_t match = 0;
  uint8_t capnum;
  uint32_t arg;
  
  if (ts != NULL) {
    for (k=0; k < ulvCnt(ts); k++) {
      if (match == 0 || match == yrxTagExpr(ts[k])) {
        op = yrxTagType(ts[k]);
        arg = (yrxTagDelta(ts[k]) << 8) | yrxTagExpr(ts[k]);
        if (op == TAG_MRK) 
          op=MRK;
        else if (op == TAG_XPR) {
          op=NOP;
          arg = 0;
        }
        else if (op == TAG_FIN) {
          match = yrxTagExpr(ts[k]);
          op=MTC;
        }
        else if (op == TAG_INC) {
          op=INC;
        }
        else {
          capnum = op & 0x1F;
          if (op >= 'a')
            op = CPB;
          else
            op = CPE;
          op = (op & 0x03) | (capnum << 2);
        }
        if (op != NOP)
          addop(op,arg);
      }
    }
  }
}

static void optimizer(uint16_t optlvl)
{
  uint32_t k;
  uint32_t t,j;
  
  if (optlvl == 0) return;
  
  k = ulvCnt(pgm);
  
  if (k == 0) return;
  
  /* peephole optimization */
  while (k-- > 0) {
    _dbgmsg("STEP: %d\n",k);
    
    /* Eliminate NOP */
    if (pgm[k] == NOP) {
      if (ulvGet(trg,k-1) != 0 && ulvGet(trg,k+1) == 0) {
        trg = ulvSet(trg,k+1,trg[k]);
        trg[k] = 0;
      }
      if (ulvGet(trg,k) == 0) {
        ulvDel(pgm,k);
        ulvDel(trg,k);
      }
    }

    /* Eliminate dead code */
    if ( (k < ulvCnt(pgm)) &&
            ((pgm[k-1] & 0x3F) == JMP) &&
            (ulvGet(trg,k) == 0)) {
      ulvDel(pgm,k);
      ulvDel(trg,k);
    }

    if ((pgm[k] & 0x3F) == JMP) {
      /* Eliminate JMP to JMP */
      t = ulvGet(trg,k);
      if (( t & 0xC0) == 0x80) {
        j = usvGet(trg_L, (t >>8));
        pgm[j] = (pgm[j] & 0x3F) | (pgm[k] & 0xFFFFFFC0) ;
        ulvDel(pgm,k);
        ulvDel(trg,k);
        usvSet(trg_L, (t >>8),0);
        k--;
      }
    }

    /* Eliminate JMP to next step */
    if ((k > 0) && (k < ulvCnt(pgm)-1) &&
           (pgm[k] == ulvGet(trg,k+1))) {
      ulvDel(pgm,k);
      ulvDel(trg,k);
      k--;
    }
    
    /* Eliminate JGT x /JEQ y/JMP x sequence */
    if ( (pgm[k] & 0x3F) == JEQ) {
      if ((pgm[k-1] & 0x3F) == JGT) {
        t = (pgm[k-1] & 0xFFFFFFC0) | JMP;
        if ((pgm[k+1] == t) || usvGet(trg,k+1) == t) {
          ulvDel(pgm,k-1);
          ulvDel(trg,k-1);
          k--;
        }
      }
    }
    
    /* remove useless check for GET return */
    if (pgm[k]   == GET && (k < ulvCnt(pgm)-4) &&
        pgm[k+1] == RLT && (pgm[k+2] & 0xFF) == CMP &&
        (isret(pgm[k+3]) && (pgm[k+3] != REQ))) {
          ulvDel(pgm,k+1);
          ulvDel(trg,k+1);
    }    
  }
}

#if 0
static void linasm(state_t from, uint32_t first, ulv_t minmax)
{
  uint32_t k;
  uint8_t  pmin, pmax;
  uint32_t parc;
  uint8_t  jmpop;
  arc_t   *a;

    for (k = 0; k < ulvCnt(minmax); k++) {
      pmin = minmax[k] >> 24;
      pmax = (minmax[k] >> 16) & 0x00FF;
      parc = minmax[k] & 0xFFFF;
      a = yrxDFAGetArc(from, parc);
      if (pmin == pmax) {
        addop(CMP, pmin);
        jmpop = JEQ;
      }
      else {
        if ( pmin > 0) {
          addop(CMP, pmin);
          addop(RLT, 0);
        }
        jmpop = JMP;
        if ( pmax < 255) {
          addop(CMP, pmax);
          jmpop = JLE;
        }
      }      
      if (yrxTagsEmpty(a->tags))
         addjmp(jmpop, 'S', a->to);
      else
         addjmp(jmpop, 'A', first + parc);
    }    
}
#endif 

static void binasm(state_t from, uint32_t first, ulv_t minmax)
{
  int i,j,k;
  int t;
  uint8_t  pmin, pmax;
  uint32_t parc;
  ulv_t stck=NULL;  
  arc_t *a;
  uint8_t  jmpop;
  uint32_t cnt_T = 0;
  static uint32_t cnt_L = 1;

  if (ulvCnt(minmax) > 0) {
    stck = ulvPush(stck,0);
    stck = ulvPush(stck,ulvCnt(minmax) - 1);
    while (ulvDepth(stck) > 1) {
      j = ulvPop(stck);
      i = ulvPop(stck);
      k = (i+j)/2;

      pmin = minmax[k] >> 24;
      pmax = (minmax[k] >> 16) & 0x00FF;
      parc = minmax[k] & 0xFFFF;
      
      a = yrxDFAGetArc(from, parc);
      
      addtarget('L',cnt_L + k);
      jmpop = JLE;
      if (pmin == 0 && pmax == 255) {
        jmpop = JMP;
      }
      else if (((k+1) <= j) && (k-1) >= i) {  /* /\ */
        cnt_T++;
        addop(CMP,pmax);
        addjmp(JGT,'L',cnt_L + ((k+1)+j)/2 );
        if (pmin != pmax) {
          addop(CMP,pmin);
          jmpop = JGT;
        }
        else
          jmpop = JEQ;
      } 
      else if ((k-1) >= i) { /* /x */
        if (pmin == pmax) {
          addop(CMP,pmax);
          addop(RGT,0);          
          jmpop = JEQ;
        }
        else {
          if (pmax != 255) {
            addop(CMP,pmax);
            addop(RGT,0);
          }
          addop(CMP,pmin);
          jmpop = JGT;
        }
      }
      else if ((k+1) <= j) { /* x\ */
        if (pmin != 0) {
          addop(CMP,pmin);
          addop(RLT,0);          
        }
        if (pmin == pmax)
          jmpop = JEQ;
        else 
          addop(CMP,pmax);
      }
      else {  /* xx */
        if (pmin == pmax) {
          addop(CMP,pmin);
          addop(RNE,0);          
        }
        else {
          if (pmin != 0) {
            addop(CMP,pmin);
            addop(RLT,0);
          }
          if (pmax != 255) {
            addop(CMP,pmax);
            addop(RGT,0);
          }
        }
        jmpop = JMP;
      }
      if (yrxTagsEmpty(a->tags))
         addjmp(jmpop, 'S', a->to);
      else
         addjmp(jmpop, 'A', first + parc);
         
      if ((k+1) <= j) {
        stck = ulvPush(stck,k+1);
        stck = ulvPush(stck,j);
        t = ((k+1)+j)/2;
      
        if ((minmax[t] >> 24) == pmax + 1) {
          minmax[t] &= 0x00FFFFFF;
        } 
      }
      if ((k-1) >= i) {
        stck = ulvPush(stck,i);
        stck = ulvPush(stck,k - 1);
        t = (i+(k-1))/2;
      
        if (((minmax[t] >> 16) & 0x00FF) == pmin - 1) {
          minmax[t] |= 0x00FF0000;
        } 
      }
    }
  }
  cnt_L += ulvCnt(minmax);
  stck = ulvFree(stck);
}

static void asm_build(uint32_t optlvl)
{
  uint32_t from;
  uint8_t *pairs;
  tagset_t final_ts;
  uint32_t arcn = 1;
  uint32_t k;
  ulv_t minmax = NULL;
  arc_t *a;

  from = yrxDFAStartState();

  addop(NRX,yrxNRX);
  
  for (k=1; k <= yrxNRX; k++)
    addop2(NCP, ucvGet(yrxNCP,k), k);
  
  while (from != 0) {
    addtarget('S',from);
    final_ts = NULL;
    
    k = 0;
    a = yrxDFAGetArc(from, 0);
    if (a == NULL) err(978,"Unexpected state!");
    
    if (a->lbl == yrxLblLambda) {
      addtags(a->tags);
      a = yrxDFAGetArc(from, ++k);
    }
    if (a != NULL) {
      addop(GET,0);
      addop(RLT,0);
    }
    
    minmax = ulvReset(minmax);
    
    while (a != NULL) {
      pairs = yrxLblPairs(a->lbl);
      while (pairs[0] <= pairs[1]) {
        minmax = ulvAdd(minmax, (pairs[0] << 24) |
                                (pairs[1] << 16) | k);
        pairs += 2;
      }
      a = yrxDFAGetArc(from, ++k);
    }
    minmax = ulvSort(minmax);
    
    /*linasm(from, arcn, minmax);*/
    binasm(from, arcn, minmax);

    addop(RET,0); 
    
    k = 0;
    while ((a = yrxDFAGetArc(from, k++))) {
      if ((a->lbl != yrxLblLambda) && !yrxTagsEmpty(a->tags)) {
        addtarget('A',arcn+k-1);
        addtags(a->tags);    
        addjmp(JMP, 'S', a->to);
      }
    }

    arcn += k;
    from = yrxDFANextState(from);
  }
  minmax = ulvFree(minmax);
  optimizer(optlvl);
}

void yrxASM(uint32_t optlvl)
{
  asm_build(optlvl);
  asm_dump();
}

/**************************/

static uint8_t *dmp_cchr(uint8_t c)
{
  static char buf[8];
  
  if (c <= 32   || c > 126  || c == '\\' || c == '"' ||
      c == '\'') {
    sprintf(buf,"0x%02X",c);
  } 
  else {
    buf[0] = '\''; buf[1] = c;
    buf[2] = '\''; buf[3] = ' ';
    buf[4] = '\0';
  }
  return buf;
}

static ucv_t ncp = NULL;

static void dmp_cstep(uint32_t step,uint32_t lbl)
{
  uint8_t opcode;
  uint32_t arg;
  uint8_t capnum;
  static uint32_t ch=' ';
  
  opcode  = step & 0xFF;
  arg     = step >> 8;
  
  if (opcode == CMP) {
    ch = arg;
    if (lbl != 0) {
      dumplbl(lbl);
      fprintf(yrxFileOut,"\n");
    }
  }
  else if (opcode != NRX && opcode != NCP) {
    dumplbl(lbl);
    if (opcode == GET) {
      ch = '\0';
      fprintf(yrxFileOut,"ch = yrxGet(ys); ypos = yrxPos(ys);");          
    }
    else if (opcode == NOP) {
      fprintf(yrxFileOut,";");    
    }
    else if (opcode == INC) {
      fprintf(yrxFileOut,"delta++;");    
    }
    else if (opcode == DEC) {
      fprintf(yrxFileOut,"if (delta >0) delta--;");    
    }
    else if (opcode == ZRO) {
      fprintf(yrxFileOut,"delta = 0;");    
    }
    else if ((opcode & 0x03) == 0x00) {
      switch (opcode & 0x1F) {
        case RGT: fprintf(yrxFileOut,"if (ch >  %s) ",dmp_cchr(ch));  break;
        case RGE: fprintf(yrxFileOut,"if (ch >= %s) ",dmp_cchr(ch));  break;
        case RLT: fprintf(yrxFileOut,"if (ch <  %s) ",dmp_cchr(ch));  break;
        case RLE: fprintf(yrxFileOut,"if (ch <= %s) ",dmp_cchr(ch));  break;
        case REQ: fprintf(yrxFileOut,"if (ch == %s) ",dmp_cchr(ch));  break;
        case RNE: fprintf(yrxFileOut,"if (ch != %s) ",dmp_cchr(ch));  break;
      }
      if ((opcode & 0x20) != 0x00) {
        switch(opcode & 0xC0) {
          case 0x40: capnum = 'A'; break;
          case 0x80: capnum = 'L'; break;
          case 0xC0: capnum = 'R'; break;
          default  : capnum = 'S'; break;
        }
        fprintf(yrxFileOut,"goto %c%u;",capnum,arg);
      }
      else 
        fprintf(yrxFileOut,"goto S0;");    
    }
    else if (istag(opcode)) {
      capnum = (opcode & 0x7C) >> 2;
      
      if (opcode == MTC) {
        fprintf(yrxFileOut,"match = %u;\n",(arg & 0xFF));
        fprintf(yrxFileOut,"        ");
      }
      
      fprintf(yrxFileOut,"tags[%u] = ypos", 
                     ncp[(arg & 0xFF) - 1] + 2 * capnum + (opcode & 0x01));
      if ((arg >> 8) > 0)
        fprintf(yrxFileOut,"- %u", arg >>8);    
      fprintf(yrxFileOut,";");
    }
    fprintf(yrxFileOut,"\n");    
  }  
}

static void c_dump(char *fn)
{
  uint16_t k = 0;
  
  ncp = ucvReset(ncp);
  ncp = ucvSet(ncp,0,0);

  if (fn == NULL) fn = "yrxExec";
  
  fprintf(yrxFileOut,"yrxResult *%s(yrxStream ys) {\n",fn);

  fprintf(yrxFileOut,"        register int ch;\n");
  fprintf(yrxFileOut,"        unsigned long delta = 0;\n");
  fprintf(yrxFileOut,"        unsigned char nrx = %u;\n", yrxNRX);
  fprintf(yrxFileOut,"        unsigned char ncp[%u] = { 0", yrxNRX+1);
  
  for (k=1; k <= yrxNRX; k++) {
    ncp = ucvSet(ncp, k, ncp[k-1] + 2 * ucvGet(yrxNCP,k));
    fprintf(yrxFileOut,", %u",ncp[k]);
  }
  fprintf(yrxFileOut," };\n");
  
  fprintf(yrxFileOut,"        static yrxResult res;\n");
  fprintf(yrxFileOut,"        static yrxPosType tags[%u];\n", ncp[yrxNRX]);
  fprintf(yrxFileOut,"        yrxPosType start = yrxPos(ys);\n");
  fprintf(yrxFileOut,"        yrxPosType ypos;\n");
  
  fprintf(yrxFileOut,"        unsigned char match = 0;\n\n");
  
  fprintf(yrxFileOut,"        memset(tags,0,sizeof(tags));\n\n");
  
  for (k = 0; k < ulvCnt(pgm); k++) {
    dmp_cstep(ulvGet(pgm,k),ulvGet(trg,k));
  } 
  
  fprintf(yrxFileOut,"S0    :\n");
  fprintf(yrxFileOut,"        res.match = match;\n");
  fprintf(yrxFileOut,"        res.ncapt = 0;\n");
  fprintf(yrxFileOut,"        res.capt  = NULL;\n");
  fprintf(yrxFileOut,"        \n");
  fprintf(yrxFileOut,"        if (match != 0) {\n");
  fprintf(yrxFileOut,"          ch = ncp[match-1];\n");
  fprintf(yrxFileOut,"          if (tags[ch] != 0) tags[ch+1] = tags[ch];\n");
  fprintf(yrxFileOut,"          tags[ch] = start;\n");
  fprintf(yrxFileOut,"          res.ncapt = (ncp[match] - ncp[match -1])/2;\n");
  fprintf(yrxFileOut,"          res.capt  = tags + ncp[match -1];\n");
  fprintf(yrxFileOut,"        }\n");
  fprintf(yrxFileOut,"        \n");
  fprintf(yrxFileOut,"        return &res;\n");
  fprintf(yrxFileOut,"}\n");
  ucvFree(ncp);
}

void yrxC(uint32_t optlvl, char *fn)
{
  asm_build(optlvl);
  c_dump(fn);
}
