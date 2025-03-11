#include "cl_msg_unknown_pack.h"
#include <cstring>
#include <cstdlib>

cl_msg_unknown_pack::cl_msg_unknown_pack(const char *prefix,const char *Preamble,unsigned int buffsize)
{
  int n;
  if(strlen(prefix)<20){
    sprintf(fname_proto,"/Raid/tmp/cl_msg_unknown_pack/%s_%%Y_%%m_%%d.packs.nobackup",prefix);
  }

  maxbuffsize=buffsize;
  buff=new char[maxbuffsize];
  memset(preamble,0,8);
  strncpy(preamble,Preamble,8);
  reset();
}
cl_msg_unknown_pack::~cl_msg_unknown_pack()
{
  if(buff!=NULL){
    delete(buff);
  }
}

int cl_msg_unknown_pack::reset()
{
  memset(&(H),0,sizeof(struct cl_msg_unknown_pack_H));
  return(0);
}

FILE *cl_msg_unknown_pack::openfile(time_t t)
{
  char fname[200];
  struct tm t_res;
  gmtime_r(&t, &t_res);
  strftime(fname,200,fname_proto,&t_res);
  return(fopen(fname,"a"));
}

int cl_msg_unknown_pack::add_msg(int32_t mtype,int32_t mver,int32_t msize,
                             char *msg)
{
  int size,offset;
  struct cl_msg_unknown_pack_h h;

  offset = 8+sizeof(struct cl_msg_unknown_pack_H);
  
  if(H.npacks==0){
    memcpy(buff,preamble,8);
    /* skip header to be filled later (just before store) */
    H.total_size = 0;
  }
  h.type=mtype;
  h.version=mver;
  h.size = msize;
  size = sizeof(struct cl_msg_unknown_pack_h) + msize;
  if( offset + H.total_size + size < maxbuffsize){
    memcpy(buff + offset + H.total_size, &h, sizeof(h));
    H.total_size += sizeof(h);
    memcpy(buff + offset + H.total_size, msg,msize);
    H.total_size += msize;

    H.npacks++;
    return(0);
  }
  return(1);
}
int cl_msg_unknown_pack::has_packs()
{
  return(0<H.npacks);
}


int cl_msg_unknown_pack::store(int16_t LsId,
                            int32_t t_sec,
                            int32_t t_nsec,
                            time_t t)
{
  FILE *arq;
  int size,n;
  int n_preamble_header;
  
  n_preamble_header = 8+sizeof(struct cl_msg_unknown_pack_H);  
  if(0 < H.total_size && 0 < H.npacks ){
    H.LsId=LsId;
    H.timestamp_sec  = t_sec;
    H.timestamp_nsec = t_nsec;
    /*need to include the general Header in the corresponding buffer 
     * location: just after the preamble
     */
    memcpy(buff + 8, &(H),sizeof(struct cl_msg_unknown_pack_H));
    arq = openfile(t);
    if(arq!=NULL){
      fwrite(buff, 1 , n_preamble_header + H.total_size, arq);
      fclose(arq);
      memset(&(H), 0, sizeof(struct cl_msg_unknown_pack_H));
      return(0);
    } else {
      printf("Error to open file\n");
    }
  } else {
    printf("Too bit or too short message to be stored. Size=%d\n",
           H.total_size);
  }
  return(-1);
}
