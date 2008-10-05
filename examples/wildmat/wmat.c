/*
**
** This code is an adaptation of the original wildmat.c program written
** by Rich Salz and others and posted on comp.source.misc in 1991.
**
** Original code may be found here:
**  http://groups.google.co.uk/group/comp.sources.misc/msg/ebf19a3339debbcd
**
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define mTRUE          1
#define mFALSE         0

       
/*
** Match text and p, return mTRUE, mFALSE.
*/
#define mTEST(x) ((matched = x) != reverse) && ch && q 

#define mgetc()  (*text++)
#define mtell()  (text - (char *)0)

#define ic(c) (icase?tolower(c):c)

static char *r;
static char op;

static int isclass(char **cls, short ch)
{
  char *p = *cls;
  short reverse;
  short matched = mFALSE;
   
  reverse = (*++p == '^') ? (p++,mTRUE) : mFALSE;
  if (*p == ']' && ch == *p++) matched = mTRUE;
  if (!matched && *p == '-' && ch == *p++) matched = mTRUE;

  while (!matched && *p && *p != ']') {        
    matched = ((*p == '-' && (p[-1] <= ch && ch <= p[1])) || 
               (*p == ch) );
    ++p;
  }

  while (*p && *p != ']') p++;
  *cls = p;
  return matched != reverse;
}

static int isbrace(char *p,short ch)
{
  
}


#define NCAPT 10

typedef size_t bufmatch_t[NCAPT][2];

static bufmatch_t capt;

typedef size_t **bufMatch;

size_t bufMatchLen(bufmatch_t *mtc, unsigned char n)
{
  if (mtc && n<NCAPT) {
    if ((*mtc)[n][1] > (*mtc)[n][0])
      return ((*mtc)[n][1] - (*mtc)[n][0]);
  }
  return 0;
}

size_t bufMatchStart(bufmatch_t *mtc, unsigned char n)
{
  if (mtc && n<NCAPT) {
    return (*mtc)[n][0];
  }
  return 0;
}

static size_t DoMatch(char *text, char *p)
{
  short optional = mFALSE;
  short reverse;
  short icase = mFALSE;
  short failed = mFALSE;
  short ch;
  size_t start = mtell();
  
  unsigned char capt_opn = 0;
  unsigned char capt_stk_cnt = 0;
  unsigned char capt_stk[NCAPT];
    
  for (capt_opn = 0; capt_opn < NCAPT; capt_opn++) {
    capt[capt_opn][0] = (capt[capt_opn][0] = 0);
  }
  capt_opn = 0;
  
  capt[0][0] = start;
  
  for (ch = mgetc(); *p && ch; p++) {
    /*printf("+++ %c %c\n",ch, *p);*/
    failed = mFALSE;
    reverse = mFALSE;
    switch (*p) {
      
      case '(' : if (capt_opn < NCAPT-1) {
                   capt_stk[capt_stk_cnt++] = ++capt_opn;
                   capt[capt_opn][0] = mtell();
                 }
                 continue;
                 
      case ')' : if (capt_stk_cnt>0) {
                   capt_stk_cnt--;
                   capt[capt_stk[capt_stk_cnt]][1] = mtell();
                 }
                 continue;
      
      case '.' : break;  /* anything matches */
                 
      case '*' : while (p[1] == '*') p++; /*multiple * are meaningless */
                 /* a * at the end will match the rest the input text */
                 if (p[1] == '\0') 
                   while ((ch = mgetc())) ;
                 r = p;
                 continue;
                 
      case '?' : switch (p[1]) {
                   case '*':
                   case '@':
                   case '?':
                   case '.': break;
                   default: optional = mTRUE;
                 }
                 continue;
                 
      case '%' : op = *++p;
                 if (isupper(op)) {
                   reverse = mTRUE;
                   op = tolower(op);
                 }
                 switch (op) {
                   case 'q' : failed = !isalnum(ch) ; break;
                   case 'a' : failed = !isalpha(ch) ; break;
                   case 'i' : failed = !isascii(ch) ; break;
                   case 'k' : failed = !isblank(ch) ; break;
                   case 'c' : failed = !iscntrl(ch) ; break;
                   case 'd' : failed = !isdigit(ch) ; break;
                   case 'g' : failed = !isgraph(ch) ; break;
                   case 'l' : failed = !islower(ch) ; break;
                   case 'r' : failed = !isprint(ch) ; break;
                   case 'p' : failed = !ispunct(ch) ; break;
                   case 's' : failed = !isspace(ch) ; break;
                   case 'u' : failed = !isupper(ch) ; break;
                   case 'x' : failed = !isxdigit(ch); break;
                   case 'i' : icase  = !reverse     ; continue
                   default  : failed = (ic(ch) != ic(*p)); break;
                 }
                 failed = failed != reverse;
                 break;
                               
      case '@' : op = *++p;
                 if (isupper(op)) {
                   reverse = mTRUE;
                   op = tolower(op);
                 }
                 switch (op) {
                   #define  mWHILE(x) while (ch && (!x == reverse))
                   case 'q' : mWHILE(isalnum(ch))  ch = mgetc() ; break;
                   case 'a' : mWHILE(isalpha(ch))  ch = mgetc() ; break;
                   case 'i' : mWHILE(isascii(ch))  ch = mgetc() ; break;
                   case 'k' : mWHILE(isblank(ch))  ch = mgetc() ; break;
                   case 'c' : mWHILE(iscntrl(ch))  ch = mgetc() ; break;
                   case 'd' : mWHILE(isdigit(ch))  ch = mgetc() ; break;
                   case 'g' : mWHILE(isgraph(ch))  ch = mgetc() ; break;
                   case 'l' : mWHILE(islower(ch))  ch = mgetc() ; break;
                   case 'r' : mWHILE(isprint(ch))  ch = mgetc() ; break;
                   case 'p' : mWHILE(ispunct(ch))  ch = mgetc() ; break;
                   case 's' : mWHILE(isspace(ch))  ch = mgetc() ; break;
                   case 'u' : mWHILE(isupper(ch))  ch = mgetc() ; break;
                   case 'x' : mWHILE(isxdigit(ch)) ch = mgetc() ; break;
                   case '[' : r = p;
                              mWHILE(isclass((p=r,&p),ch)) ch = mgetc();
                              break;
                   default  : return 0;
                 }
                 r=NULL; /* any @ ends a * */
                 /* needed a `continue` rather than a `break` because
                 ** the while already read the next character!
                 */
                 continue;
       
      case '[' : failed = !isclass(&p,ch);
                 break;

      default  : failed = (ic(ch) != ic(*p));
                 break;
    }
    /*printf("xxx %d %p\n",failed, r);*/
    
    if (!failed || !optional) {
      if (!failed) r = NULL;
      
      if (r) p = r;
      else if (failed) break;
      
      ch = mgetc();
    }
    optional = mFALSE;
  } 
  /*printf("+xx %c %d\n",*p,failed);*/
  if (failed) return 0;
  capt[0][1] = mtell()-1;
  /* remove the optional elements at the end of the pattern */
  while (*p) {
    switch (*p) {
      
      case ')': if (capt_stk_cnt>0) {
                  capt_stk_cnt--;
                  capt[capt_stk[capt_stk_cnt]][1] = capt[0][1];
                }
      case '(': /* FALLTHROUGH */
      case '*': p++;
                break;

      case '#': p+=2;
            
      case '?': p++;
                if (*p != '%') break;
                /* FALLTHROUGH */          
      case '@': p++; 
                if (*p++ == '[') {
                  p++;
                  while (*p && *p != ']') p++;
                  if (*p) p++;
                }
                break;
                
      default : return 0;
    }
  }
  return 1;
}


/*
** User-level routine. Returns mTRUE or mFALSE.
*/
bufmatch_t *match(char *text, char *p)
{
  size_t ret;
  ret = DoMatch(text, p);
  if (ret > 0) {
    return &capt;
  }
  return NULL;
}


#ifdef TEST

/* Yes, we use gets not fgets. Sue me. */
extern char  *gets();

int main()
{
  char    p[80];
  char    text[80];
  bufmatch_t  *ret;
  int k;

  printf("Wildmat tester. Enter pattern, then strings to test.\n");
  printf("A blank line gets prompts for a new pattern; a blank pattern\n");
  printf("exits the program.\n");

  for ( ; ; ) {
    printf("\nEnter pattern: ");
    (void)fflush(stdout);
    if (gets(p) == NULL || p[0] == '\0')
      break;
    for ( ; ; ) {
      printf("Enter text: ");
      (void)fflush(stdout);
      if (gets(text) == NULL)
        exit(0);
      if (text[0] == '\0')
        /* Blank line; go back and get a new pattern. */
        break;
      ret = match(text, p);
      if (ret) {
        for (k=0; k < NCAPT; k++) {
          if (bufMatchLen(ret,k) > 0) {
            printf("%2d: \"",k);
            fwrite((char*)0+bufMatchStart(ret,k),bufMatchLen(ret,k),1,stdout);
            printf("\"\n");
          }
        }
      printf("   %s %d\n",  ret? "YES" : "NO", bufMatchLen(ret,0));
      }
      else {
        printf("   NO\n");
      }
      
    }
  }

  exit(0);
  /* NOTREACHED */
}
#endif /* TEST */
