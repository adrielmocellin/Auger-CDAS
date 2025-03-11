/*
 * |H(n)|h1(4)|data1(n)|h2(4)|data2(n)|...|
 *
 * H: n bytes - it is the general header. It is used if needed. Depends on
 *      the way UUB/CDAS DAQ is implemented.
 *
 * h: 4 bytes (network byte order):
 *   size: 0-16: 0~131072 (128k) - it do not consider the header size
 *   type: 17-23: 0~128 (0~0x7F). type=0 is reserved.
 *   version: 24-31 (for the data format)
 *
 * to unpack data
 *     declare struct msg_str str;
 *     read the data  (in whatever way).
 *
 *   call msg_assign_data(&str,int maxsize,*buff):
 *       maxsize is the size (in bytes) of the buffer read.
 *       buff -> pointer (unsigned char) to the buffer
 *
 *   call msg_unpack_get_header(&str,*buff,h_size);
 *        if need (it really depends on the UUB/CDAS implementation).
 *
 *   call msg_get_data(&str,
 *                 &type,&version,data,maxsize)
 *        * - return size of the message and
 *                   corresponding type,version and data.
 *              -3 -> size of the data is bigger than maxsize
 *              -2 -> probably an internal error. Some buffer size and
 *                       data available in the buffer
 *              -1 -> not more data available
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "msg_pack_unpack.h"


int msg_unpack_assign_data(struct msg_pack_unpack_str *m,
                    int maxsize,unsigned char *buff)
{
  m->buff=buff;
  m->maxsize=maxsize;
  m->size=0;
  return(0);
}

int msg_unpack_get_header(struct msg_pack_unpack_str *m,
                          unsigned char *h,int h_size)
{
  if(h_size < m->maxsize ){
    memcpy(h,m->buff,h_size);
    m->size=h_size;
    return(0);
  }
  return(-1);
}


int msg_unpack_get_data(struct msg_pack_unpack_str *m,
                 unsigned int *type,unsigned int *version,
                 unsigned char *data,int maxsize)
{
  uint32_t h;
  int size;
  *type=0;
  *version=0;
  if(m->size+sizeof(h) <= m->maxsize){
    memcpy(&h,m->buff + m->size,sizeof(h));
    h=ntohl(h); /* convert the header from network to host byte order*/
    size = h & 0x1FFFF;
    *type = (h>>17) & 0x3F;
    *version = (h>>24) & 0xFF;
    if( size + m->size+sizeof(h) <= m->maxsize ){
      if( size <= maxsize ){
        memcpy(data,m->buff + m->size + sizeof(h),size);
        m->size +=sizeof(h) + size;
        return(size);
      } else {
        return(-3);
      }

    }
    return(-2);
  }
  return(-1);
}
