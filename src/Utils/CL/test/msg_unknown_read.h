#ifndef _MSG_UNKNOWN_READ_H_
#define _MSG_UNKNOWN_READ_H_
#include "cl_msg_unknown_pack.h"
#include <vector>

/* see the data format at cl_msg_unknown_pack.h 
 *
 * use this library as:
 *  
 *   struct cl_msg_unknown_pack_h h; // struct for each packet
 *   struct cl_msg_unknown_pack_H H; // header for the event 
 * 
 * msg_unknown_read ff("filename","!!_T3_!!");
 * 
 * loop 
 *    flag=ff.search_preamble();
 *    if(flag==0){ ---has a new data ---
 *       ff.get_header(&H); //H would contain station Id, number of packets,...
 *       loop  ff.get_pack(&h,buff,max_buff_size) ==0
 *          //h    - contain few information of the packet
 *          //buff - content of the packet.
 *  
 */

class msg_unknown_read
{
private:
  char *buff;
  char preamble[8];
  int buffused,maxbuffsize;
  int next_pack;
  struct cl_msg_unknown_pack_H Header;
  std::vector <char *> pts;
  FILE *arq;
  
  int read_data();
public:
  msg_unknown_read(const char *filename,const char *Preamble,int Buffsize=1048576);
  int search_preamble();
  int get_header(struct cl_msg_unknown_pack_H *h);
  int get_pack(struct cl_msg_unknown_pack_h *h,char *pack,int max,int n=-1);
  int skip();
};
#endif
