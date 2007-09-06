
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
  uint32_t j,k,n=0;
  Arc a;
  uint32_t t;
  
  v=vecNew(sizeof(Arc));

  for (j=1; j < 1000; j++) {
    for (k=j; k < 1000; k++) {
      a.from = j; a.to = k;
      vecSet(v,n++,&a); 
    }
  } 
  t = vecSize(v);
  printf("Size: %u (%u) %u %u",t,v->npg,sizeof(Arc)*n, t-sizeof(Arc)*n);
 
  v=vecFree(v);
}


