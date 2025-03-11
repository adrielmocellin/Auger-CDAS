#ifndef _MSG_PACK_UNPACK_H_
#define _MSG_PACK_UNPACK_H_

struct msg_pack_unpack_str
{
  int maxsize;
  int size;
  unsigned char *buff;
};


struct msg_pack_unpack_str *msg_pack_init(int maxsize)
  ;
int msg_pack_reset(struct msg_pack_unpack_str *m);
int msg_pack_set_header(struct msg_pack_unpack_str *m,
			int size,unsigned char *data);

int msg_pack_add(struct msg_pack_unpack_str *m,
		 unsigned int size,unsigned int type,unsigned int version,
		 unsigned char *data);





int msg_unpack_assign_data(struct msg_pack_unpack_str *m,
			   int maxsize,unsigned char *buff);

int msg_unpack_get_header(struct msg_pack_unpack_str *m,
			  unsigned char *buff,int h_size);

int msg_unpack_get_data(struct msg_pack_unpack_str *m,
		 unsigned int *type,unsigned int *version,
		 unsigned char *data,int maxsize);

#endif
