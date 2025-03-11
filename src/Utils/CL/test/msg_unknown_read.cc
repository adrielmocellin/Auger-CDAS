
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstdint>

#include "msg_unknown_read.h"


msg_unknown_read::msg_unknown_read(const char *filename,const char *Preamble,int Buffsize)
{
  arq=fopen(filename,"r");
  if(arq==NULL){
    printf("Error to open file: %s\n",filename);
  }
  maxbuffsize=Buffsize;
  buff=new char[maxbuffsize];
  memset(preamble,0,8);
  strncpy(preamble,Preamble,8);
  buffused=0;
}

int msg_unknown_read::read_data()
{
  int n,flag;
  n = maxbuffsize - buffused;
  if(0<n){
    flag=fread(buff+buffused, 1, n, arq );
    if(0<flag){
      buffused += flag;
    }
    return(flag);
  }
  return(0);
}

int msg_unknown_read::search_preamble()
{
  /*return 0 -> preamble found
   *       -1 -> error or end of file
   *       the "buff" is modified in the way the main
   *       header to be at the begining of the buffer.
   */
  char *pt;
  int remind,skip;
  int not_found;
  not_found=1;
  while(not_found){
    if(buffused < 8){
      if(read_data()<1){
        return(-1);
      }
    } else {
      printf("Enter get preamble. buffsize=%d\n",buffused);       
      pt=(char *)memmem(buff, buffused, preamble, 8);
      if(pt!=NULL){
        pt+=8;
        skip=pt-buff;
        buffused -= skip;
        if(0 < buffused){
          memmove(buff, pt, buffused);
        } else {
          buffused=0;
        }
        return(0);
      } else {
        pt=buff+(buffused - 7);
        buffused = 7;
        memmove(buff,pt,buffused);
      }
    }
  }
  printf("preamble buffused=%d\n", buffused );
  return(0);
}

int msg_unknown_read::get_header(struct cl_msg_unknown_pack_H *H)
{
  int nbuffneed,got;
  int ibuff,n,i;
  struct cl_msg_unknown_pack_h h;
  
  nbuffneed = sizeof(struct cl_msg_unknown_pack_H);
  got=0;
  while(got==0){
    printf("buff used: %d/%d\n",nbuffneed,buffused);
    if(buffused<nbuffneed){
      if(read_data()<=0){
        printf("read error\n");
        return(-1);
      }
    } else {
      printf("buffused,nbuffneed: %d/%d\n",
             buffused,nbuffneed);
      memcpy(&Header, buff, sizeof(struct cl_msg_unknown_pack_H));
      ibuff=sizeof(struct cl_msg_unknown_pack_H);
      nbuffneed = ibuff + Header.total_size;
      printf("head size: %d; nbuffneed: %d\n",
             Header.total_size,nbuffneed);
      if(nbuffneed <= buffused){
        pts.clear();
        next_pack=0;
        for(i=0;i<Header.npacks;i++){
          n=sizeof(struct cl_msg_unknown_pack_h);
          if(ibuff + n > nbuffneed ){
            printf("Error. not enough data in buffer: %d ... %d\n",
                   ibuff+n,nbuffneed);
            exit(2);
          }
          memcpy(&h,buff + ibuff, sizeof(struct cl_msg_unknown_pack_h));
          n += h.size;
          if(nbuffneed < ibuff + n){
            printf("Error. not enough data in buffer: %d ... %d\n",
                   ibuff+n,nbuffneed);
            exit(3);
          }
          pts.push_back(buff+ibuff);
          ibuff+=n;
        }
        memcpy(H,&Header,sizeof(Header));
        return(0);
      } else {
        
        printf("requiring more data than buffer size: %d; buffersize=%d\n ",
               n,maxbuffsize);

      }
    }
  }
  return(4);
}
int msg_unknown_read::get_pack(struct cl_msg_unknown_pack_h *h,char *pack,int max,int n)
{
  int nn;
  if(n==-1){
    nn=next_pack;
  } else {
    nn=n;
  }
  next_pack=nn+1;
  if(nn < pts.size()){
    memcpy(h,pts[nn],sizeof(struct cl_msg_unknown_pack_h));
    if(max<h->size){
      return(-2);
    }
    memcpy(pack,pts[nn]+sizeof(struct cl_msg_unknown_pack_h),h->size);
    return(0);
  }
  return(-1);
}
int msg_unknown_read::skip()
{
  int to_skip,n;
  
  to_skip = sizeof(struct cl_msg_unknown_pack_H) + Header.total_size;
  buffused -= to_skip;
  if(0<buffused) {
    memmove(buff,buff+to_skip,buffused);
  } else {
    buffused=0;
  }
  return(0);
}




