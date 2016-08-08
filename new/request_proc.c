 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include "base.h"
 #include "var.h"
 #include "general_util.h"
 #include "ibdcs.h"
#include  "tmcibtms.h"
#include "db_qfunc.h"
#include "db_tools.h"

 int request_msg_proc(glob_msg_stru *pub_data_stru)
 {
 	 int i;
 	 ICS_DEBUG(0);
 	 i=confirm_app_type(pub_data_stru);
 	 if(0>i)//ȷ��ҵ������	
 	 {	
 	 	  dcs_log(0,0,"<%s>can not confirm app type!",__FUNCTION__);
 	 	  return -1;
 	 }   
	if(!pub_data_stru->open_flag)//ҵ�񱻹ر�
	{
 	 	  pub_data_stru->switch_src_flag=1;
 	 	  set_result_code(pub_data_stru, CODE_NO_SUPPORT);
 	 	  if(0>err_set_msg(pub_data_stru)) return -1;
 	 	  return 1;
	}

 if( i == 0 )//ֱ��Ӧ���ཻ�״���(�������ཻ��)
 {
 	 if(direct_respose(pub_data_stru)>0)
 	 { 
 		if(0>get_route_insti_info(pub_data_stru)) //��ȡ·�ɻ�����Ϣ 
 			return -1;
 	 //�����̶���Ϣ��������
     if(0>get_default_field(pub_data_stru))return -1;
     if(0>conver_data(pub_data_stru, 1))//���ݸ�ʽת��
	 	 {	
	 	    if(0>err_set_msg(pub_data_stru)) return -1;
	 	 	  return 1;
	 	 }
     return 1;
   } 
   else return -1;
 }
 	 dcs_debug(0,0," ��ת�ӽ��״���");
	
   i=check_app_limit(pub_data_stru);
 	 if(0>i)  //���ҵ������,Υ��ҵ����������ϵͳ�쳣���
 	 {		
 	 	  pub_data_stru->switch_src_flag=1;
 	 	  dcs_debug(0,0,"���ҵ������,Υ��ҵ����������ϵͳ�쳣��� i<0");
 	 	  if(0>err_set_msg(pub_data_stru)) return -1;
 	 	  return 1;
 	 }
 	 else if( i==0 ) //���ҵ��,������뷢�𷽽�����Ϣ�������Ҫ���ڶ��ն˶�����ܽ��������Ӧ��
 	 {
 	 	  pub_data_stru->switch_src_flag=1;
 	 	  if (pub_data_stru->in_cry_flag ) pub_data_stru->out_cry_flag =1;
 	 	  	dcs_debug(0,0,"���ҵ��,������뷢�𷽽�����Ϣ�������Ҫ���ڶ��ն˶�����ܽ��������Ӧ�� i=0");
 	 	  if(0 > conver_data(pub_data_stru, 2)) return -1; 
 	 	  i=db_insert(pub_data_stru,1);
 	 	  if( 0==i)
 	 	  {
 	 	  	strcpy(pub_data_stru->center_result_code,"94");
 	 	  	return 1; //���ݴ洢;
 	 	  }
 	 	  else if(0>i)
 	 	  {	 
 	 	  	strcpy(pub_data_stru->center_result_code,"96");
 	 	  	return 1; //���ݴ洢;
 	 	  }
 	 	  return 1;
 	 }  

 	 if(0>get_route_insti_code(pub_data_stru)) //��ȡ·�ɻ��� ,�ڴ˺����ڼ������Ƿ񱻹رգ�����ͨ����·�Ƿ��������Ҫ�ǿ��ǵ��������������߱��ݻ���
 	 {	
 	 	  pub_data_stru->switch_src_flag=1;
 	 	  if (pub_data_stru->in_cry_flag ) pub_data_stru->out_cry_flag =1;
 	 	  dcs_debug(0,0,"��ȡ·�ɻ��� ,�ڴ˺����ڼ������Ƿ񱻹رգ�����ͨ����·�Ƿ��������Ҫ�ǿ��ǵ��������������߱��ݻ���");
 	 	  if(0>err_set_msg(pub_data_stru)) return -1;
 	
 	 	  return 1;
 	 }	

 	 if(0>get_route_trans_info(pub_data_stru)) //��ȡ·�ɽ�����Ϣ,�����������͵� 
 	 {	
 	 	  pub_data_stru->switch_src_flag=1;
 	 	  if (pub_data_stru->in_cry_flag ) pub_data_stru->out_cry_flag =1;
 	 	  	dcs_debug(0,0,"��ȡ·�ɽ�����Ϣ,�����������͵� ");
 	 	  if(0>err_set_msg(pub_data_stru)) return -1; 	 	  
 	 	  return 1;
 	 }   	
 	 if(0>conver_data(pub_data_stru, 0))//���ݸ�ʽת��
 	 {	
 	    dcs_debug(0,0,"<%s>conver_data fail! ",__FUNCTION__);
 	    pub_data_stru->switch_src_flag=1;
 	    if(0>err_set_msg(pub_data_stru)) return -1;
 	 	  return 1;
 	 }
 	 pub_data_stru->use_timeout = 1;
   return 1;
 }

int direct_respose(glob_msg_stru *pub_data_stru)
{
	int i;
	char direct_name[80];
	ICS_DEBUG(0);
	
	if(0>check_direct_limit(pub_data_stru))  //���ֱ��Ӧ��ҵ��������
 	{	
 	 	  dcs_log(0,0,"<%s>check_direct_limit not pass",__FUNCTION__);
 	 	  if(0>err_set_msg(pub_data_stru)) return -1;
 	 	  return 1;
 	}   
	memset(direct_name,0,sizeof(direct_name));
	//ȡӦ�ô����������
	if( 0>get_direct_name(pub_data_stru->in_msg_type,pub_data_stru->app_type,
		                   direct_name,sizeof(direct_name)))
	    return -1;
	pub_data_stru->switch_src_flag=1;
	pub_data_stru->out_cry_flag = pub_data_stru->in_cry_flag;
	snprintf(pub_data_stru->route_insti_code,9,"%s",pub_data_stru->insti_code);
	snprintf(pub_data_stru->route_trans_type,5,"%s",pub_data_stru->in_trans_type);
	snprintf(pub_data_stru->route_msg_type,5,"%s",pub_data_stru->in_msg_type);
	snprintf(pub_data_stru->route_fold_name,40,"%s",pub_data_stru->insti_fold_name);
	snprintf(pub_data_stru->route_mac_index ,6 ,"%s",pub_data_stru->in_mac_index    );
	snprintf(pub_data_stru->route_mac_key   ,33 ,"%s",pub_data_stru->in_mac_key      );

	for( i=0; gl_direct_proc[i].func!=NULL;i++)
	{
		if( strcmp(gl_direct_proc[i].name,direct_name)==0)
			return gl_direct_proc[i].func(pub_data_stru);
	}
	dcs_log(0,0, "<%s>can not found direct proc msg_type=[%s],app_type=[%s],direct_name=[%s]",
	            __FUNCTION__,pub_data_stru->in_msg_type,pub_data_stru->app_type,
	            direct_name);
	return -1;
}
