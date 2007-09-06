
#define HUL_MAIN
#include "hul.h"
#include "vec.h"

typedef struct Arc {
  struct Arc *next;
  uint16_t from;
  uint16_t to;  
  uint16_t *tag;
  uint8_t *lbl;
} Arc;

uint32_t ArcHfn(Arc *a)
{
  uint32_t h;
  
  h = SuperFastHash((uint8_t *)&(a->from),sizeof(uint16_t),13);
  h = SuperFastHash((uint8_t *)&(a->from),sizeof(uint16_t),h);
  
  return h;  
}

int ArcCmp(Arc *a,Arc *b)
{
  if (a->to = b->to)
    if (a->tag == b->tag)
      if (a->lbl == b->lbl)
        return 0;
      else return -1;
    else return -1;
  else return (a->to - b->to);
}

static void ArcCpy(Arc *a, Arc *b)
{ *a=*b; }

static void ArcClr(Arc *a)
{ a->tag = NULL; a->lbl = NULL; }


int main(int argc, char * argv[])
{
  vec *v;
  ht  *h;
  
  uint32_t j,k,n=0;
  Arc a,*p;
  uint32_t t;
  vlb *l;
  
  v=vecNew(sizeof(Arc));
  
  printf("n;minsize;vsize;diff\n");
  for (j=1; j < 1000; j++) {
    for (k=j; k < 1000; k++) {
      if ((n & 0x3FF) == 0) {
      }
      a.from = j; a.to = k;
      vecSet(v,n++,&a); 
    }
  } 
 
  p = (Arc *)vecGet(v,n/2);
  
  /*printf("%u;%u;%u;%u\n",n,sizeof(Arc)*n,t, t-sizeof(Arc)*n);*/
  
  printf("%d -> %d\n",p->from,p->to);
  p = (Arc *)vecNext(v);
  printf("%d -> %d\n",p->from,p->to);
  p = (Arc *)vecPrev(v);
  printf("%d -> %d\n",p->from,p->to);
  p = (Arc *)vecNext(v);
  printf("%d -> %d\n",p->from,p->to);
  fflush(stdout);
  
  v=vecFree(v);
  
  v = vecNewL();
  t = vecSize(v);
  printf("%u\n",t);
  
  l = vecSetL(v,1,"Ciao",5); 
  printf("1 is %p\n",l);
  t = vecSize(v);
  printf("%u\n",t);
  l = vecSetL(v,2,"Pippo",6);
  printf("2 is %p\n",l);
  t = vecSize(v);
  printf("%u\n",t);
  
  l = vecGet(v,1);
  
  printf("ptr:%p len: %d buf:%s\n",l,l->len,l->buf);
  v = vecFree(v);
  
}


