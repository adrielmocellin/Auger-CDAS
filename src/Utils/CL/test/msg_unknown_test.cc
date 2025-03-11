#include "cl_msg_unknown_pack.h"
#include <cstdlib>
int main()
{
  struct timespec tt;
  unsigned char bb[1024];
  int i,j,k,m,n;
  int type,ver;
  time_t t;
  cl_msg_unknown_pack unknown("teste","AbCdEfGh");
  srand(432234);
  for(k=0;k<100;k++){
    clock_gettime(CLOCK_REALTIME,&tt);
    printf("k=%d: store: %ld.%09ld %ld\n",
           k,
           tt.tv_sec,tt.tv_nsec,t);
    
    m=1+rand()%10;
    printf("m=%d\n",m);
    for(j=0; j<m; j++){
      n=rand()%1024;
      for(i=0; i<n; i++){
        bb[i]=(rand()) & 0xFF;
      }
      type=rand() & 0xFF;
      ver=rand() & 0xFF;
      unknown.add_msg(type,ver,n,(char *)bb);
      printf("type,ver,size=%d,%d,%d\n",type,ver,n);
    }
    t=time(NULL);
    unknown.store(136,tt.tv_sec,tt.tv_nsec,t);
  }
}
