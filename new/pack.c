#include "ibdcs.h"
#include  "tmcibtms.h"
#include "base.h"
#include "var.h"
#include "general_util.h"
int pack_msg(glob_msg_stru *pub_data_stru,char *buf,int size)
{
	int i;
	
   for ( i =0 ; pack_func_proc[i].priv_pack != NULL ;i++) 
   {
   	   if( strcmp( pub_data_stru->route_msg_type ,pack_func_proc[i].msg_type )==0)
   	   	  return pack_func_proc[i].priv_pack(pub_data_stru,buf,size);
   }
  return iso_pack(pub_data_stru,buf,size);
}