#include "ibdcs.h"
#include  "tmcibtms.h"
#include "base.h"
#include "var.h"
#include "general_util.h"
int unpack_msg( char *src_buf,int src_len,glob_msg_stru *pub_data_stru)
{
	int i;
	char msg_type[5];
	if(0>=get_in_msg_type(pub_data_stru,msg_type,sizeof(msg_type)))
	{
			dcs_log(0,0,"<%s> get_in_msg_type fail!",__FUNCTION__);
			return -1;
	}
  for ( i =0 ; unpack_func_proc[i].msg_type != NULL &&  unpack_func_proc[i].priv_unpack != NULL ;i++) 
  {
   	   if( memcmp( msg_type ,unpack_func_proc[i].msg_type,4 )==0)
   	   	  return unpack_func_proc[i].priv_unpack(src_buf,src_len,pub_data_stru);
  }
 //默认处理函数
  dcs_debug(0,0,"default unpack proc (iso_unpack)");
  return iso_unpack(src_buf,src_len,pub_data_stru);
}