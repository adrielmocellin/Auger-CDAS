
#ifndef _CL_MSG_UNKNOWN_PACK_H_
#define _CL_MSG_UNKNOWN_PACK_H_

#include <cstdint>
#include <ctime>
#include <cstdio>

/* =========================================================
 * It may happen the UUB send data the CDAS done not understand (do
 *   not have a proper way to store a particular data). 
 * To avoid a loose these data and to have a way to make changes in
 *   UUB without to care too much with the changes in CDAS these
 *   data will be stored in 
 *   /Raid/tmp/cl_msg_unknown_pack/xxxx_YYYY_MM_DD.packs.nobackup
 * 
 * "xxxx" is set in "prefix" in the constructor of in the class
 *     cl_msg_unknown_pack
 *  
 * The data will be stored with the following format:
 * 
 * |preamble(8)|H|h1|data1|h2|data2|...|hn|datan|preamble(8)|H|h1|data1|...
 *
 * n = number of packs.
 *
 * the idea of the preamble is to be able to recover the data in case
 *   the part of the file is corrupted.
 * 
 * H  = struct cl_msg_unknown_pack_H 
 * h_i= struct cl_msg_unknown_pack_h
 *
 * 
 * The "preamble" are included in case the file get corrupted in some way.
 *   If the file get corrupted, it would have the possibility to search for
 *   the next preamble which would have others data.
 * 
 *
 * it should be used as:
 *   1) instanciate the object of class cl_msg_unknown_pack:
 *       cl_msg_unknown_pack  unknown_pack("teste","!!abcd!!")
 *             would generate files "teste_YYYY_MM_DD...." 
 *             with preambles "!!abcd!!" 
 *             buffer size is set as default of 1MB.
 *                     
 *   2) include the messages with method "add_msg":
 *       unknown_pack.add_msg(x,y,size,data)
 *       x-> type 
 *       y-> version
 *       size -> size of data in "data" (char[])
 *     including all headers and data should not go above buffer size.
 *   
 *   3) store the data with method "store":
 *       unknown-pack.store(Id,t_sec,t_nsec,t)
 *       the "Id" - is the a particualr identification. 
 *           "t_sec" and "t_nsec" is the time reference, if it is really needed.
 *           "t" - the timestamp related with the time the data is going to
 *                  be stored. This parameter is used to generate the
 *                  filename where the data will be stored.
 *   
 * The file is open and closed everytime the method "store" is called.
 * ==============================================
 */


struct __attribute__ ((__packed__)) cl_msg_unknown_pack_H
{
  int16_t LsId;
  int16_t npacks;
  int32_t timestamp_sec;
  int32_t timestamp_nsec;
  int32_t total_size; /* do not include this header */
};

//struct cl_msg_unknown_pack_h

struct __attribute__ ((__packed__)) cl_msg_unknown_pack_h
{
  int32_t type;
  int32_t version;
  int32_t size;     /* do not include this header */
};

class cl_msg_unknown_pack
{
 private:
  char fname_proto[150];
  char preamble[10]; /* it would consider to use 8 bytes, include 2
                        bytes to consider \n and/or \0 characters*/
  struct cl_msg_unknown_pack_H H;
  unsigned int maxbuffsize;
  char *buff;
  
  FILE *openfile(time_t t);
 public:
  cl_msg_unknown_pack(const char *prefix,const char *Preamble,
                   unsigned int buffsize=1048576);

  //cl_msg_unknown_pack(const char *prefix,const char *Preamble,
  //                 unsigned int buffsize);
  ~cl_msg_unknown_pack();
  int reset();
  int add_msg(int32_t mtype,int32_t mver,int32_t msize,
                               char *msg);
  int has_packs(); /*return if there are packs stored in the buffer */
  int store(int16_t LsId,
            int32_t t_sec,
            int32_t t_nsec,
            time_t t);
};

#endif
