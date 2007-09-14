
#define HUL_MAIN
#include "hul.h"
#include "vec.h"

typedef struct Arc {
  uint16_t from;
  uint16_t to;  
  uint32_t info;
} Arc;

int main(int argc, char * argv[])
{
  vec *v;
 
  uint32_t j,k,n=0;
  Arc a,*p;
  uint32_t t;
  uint8_t **p8;
  Arc *q;
  
  #if 1
  v=vecNew(sizeof(Arc));
  
  printf("n;minsize;vsize;diff\n");
  for (j=1; j < 100; j++) {
    for (k=j; k < 100; k++) {
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
  printf("CNT: %u\n",vecCnt(v));
  fflush(stdout);
 
  v=vecFree(v);
  #endif
  
  /**** SET ****/
  #if 0
  v = setNew(sizeof(Arc),offsetof(Arc,info));

  a.from = 100; a.to = 100; a.info = 2323;
    
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  /*
  p = setGet(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  p8 = (uint8_t **)p - 1;
  printf("%p == %p\n",p,*p8);
  
 fflush(stdout);
 */
  a.from++;
  /*
  p = setGet(v,&a);
  printf("%p\n",p);
 */
  
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  /*
  p8 = (uint8_t **)p - 1;
  printf("%p == %p\n",p,*p8);
*/
 a.from--;
 fflush(stdout);
  /*
  p = setGet(v,&a);
  printf("%p\n",p);
  / *
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);*/
  
  a.from = 10; a.to = 10;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  a.from = 210; a.to = 210;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  printf("\n");
  
  a.from = 40; a.to = 10;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  a.from = 100; a.to = 100;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  a.from = 140; a.to = 10;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  a.info = 92;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  a.from = 40; a.to = 10;
  p = setGet(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  setDel(v,&a);
  p = setGet(v,&a);
  printf("%p\n",p);
  a.from = 140; a.to = 10;
  p = setGet(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  setDel(v,&a);
  p = setGet(v,&a);
  printf("%p\n",p);
  
  fflush(stdout);
  setForeach(v,k,p) 
  printf("%d] %p %d -> %d\n",k,p,p->from,p->to);
  
  
  a.from = 40; a.to = 10;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  a.from = 140; a.to = 10;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  fflush(stdout);
  setForeach(v,k,p) 
  printf("%d] %p %d -> %d\n",k,p,p->from,p->to);
    
  /*
  for (k=0;k < vecCnt(v);k++) {
    p = vecGet(v,k);
    p = *(Arc **)p;
    printf("%d] %p %d -> %d\n",k,p,p->from,p->to);
    if (k>0) printf(" == %d ",memcmp(q,p,v->esz-sizeof(uint8_t *)));
    printf("\n");
    q = p;
  }
*/
  v = setFree(v);
 #endif
  exit(0); 
}


