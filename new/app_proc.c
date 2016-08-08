
#include "base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "var.h"
#include "tools.h"
#include "db_tools.h"
#include "general_util.h"
#include "db_qfunc.h"
int app_proc(const char *msg_type,char *src_buf,int src_len,
             char * desc_buf,int desc_size,int *to_fid)
{
// ����ͨ�û�������


/*1������ msg_type �������ֵ��в��Һ��ʵĽ��ģ�顣
  2�������Ľ�����ͨ�û�����
  3����ȡ������Ϣ�Ļ�������
  4��������Ϸ��Լ��
  5���ҳ����Ĺؼ���
  6���жϱ����������ģ�����Ӧ����
  
  7.1�������Ĵ��� 
    A���ҳ�Ŀ��·�ɻ���
    B��������ת������
    C�����ݿ�洢
  
    
  7.2��Ӧ���Ĵ���
    A��ɾ����ʱ��
    B���жϽ����Ƿ�ɹ�
    C���ɹ���ȡ��һ������·�ɽڵ㣬ʧ��������һ�ڵ��Ƿ�Ҫ������������ԭ��������ڵ����սṹ��
    D���������ݿ�����
    E����ȡ��һ������·�ɽڵ㱨�����ͣ�ʼ�սڵ��򷵻س�ʼ�ڵ������
    F��������ת������
  8�� �����
  9�����ݴ��
 */
  glob_msg_stru pub_data_stru;
  message_define *priv_def;
  int n=0;
  char tmp[128];
  

  assert(msg_type != NULL );
  assert(src_buf != NULL );
  assert(desc_buf != NULL );
 
  init_pub_data_stru(&pub_data_stru);
  
  if( is_gm_type(msg_type)) 
  	set_cry_flag(&pub_data_stru,1);
  priv_def = match_priv_stru(msg_type,&gl_def_set); //���ݽ���ı��������ҵ���Ӧ�Ľ��������ֵ�
  if ( priv_def == NULL ) 
  {  	
  	dcs_log(0,0,"<%s>match priv define fail msg_type=%4.4s",__FUNCTION__,msg_type);
  	return -1;
  }
  pub_data_stru.in_priv_def=priv_def;
  pub_data_stru.src_buffer=src_buf;
  pub_data_stru.src_len=src_len;
  memcpy(pub_data_stru.in_msg_type,msg_type,4);
  pub_data_stru.in_msg_type[4]=0x00;
  snprintf(pub_data_stru.insti_fold_name,sizeof(pub_data_stru.insti_fold_name),
           "%s",gs_fold_name);
  n = unpack_msg(src_buf,src_len,&pub_data_stru);//����������ݵ���ͨ�û�����
  if( 0 > n)
  {
		dcs_log(src_buf,src_len,"<%s>unpack msg fail! msg_type=%4.4s",
		        __FUNCTION__,msg_type);
 		return -1;
  }

  if (get_insti_field_name(&pub_data_stru,tmp,sizeof(tmp)) < 0)
  {	
  	dcs_log(0,0,"<%s> get_insti_field_name fail!",__FUNCTION__);
  	return -1;
  }
  if ( get_insti_code(&pub_data_stru,tmp)<0)//��ȡ��������������
  {
  	dcs_log(0,0,"<%s> get insti code fail!",__FUNCTION__);
  	return -1;
  }
  if ( get_insti_info(&pub_data_stru) <0 )  // ��ȡ������������������Ϣ
  {
  	  dcs_log(0,0,"<%s> get_insti_info fail!",__FUNCTION__);
  	  return -1;
  }
  if( !pub_data_stru.insti_open_flag)
  {
  	  dcs_log(0,0,"<%s> insti not open![%s]",__FUNCTION__,
  	              pub_data_stru.insti_code);
  	  return -1;
  }
  if ( get_msg_key(&pub_data_stru) <0 )  // ��ȡ���Ĺؼ���
  {
  	dcs_log(0,0,"<%s> get_msg_key fail!",__FUNCTION__);
  	return -1;
  }
  n=get_msg_intout_flag(&pub_data_stru);                //�жϳ����ĵĴ����࣬��Ӧ���Ļ��������� 0����1Ӧ��
  
  if( n< 0 ) //�ж�ʧ��
  {
  	dcs_log(0,0,"<%s> get_msg_intout_flag is error ",__FUNCTION__);
  	return -1;
  }
  else if ( n == 0 )    //�����Ĵ���
  {
   	if( 0>request_msg_proc(&pub_data_stru))
  	{
  			dcs_log(0,0,"<%s> request_msg_proc error!",__FUNCTION__);
  			return -1;
  	}
  }
  else                //Ӧ���Ĵ���
  {
	  n = response_msg_proc(&pub_data_stru);
  	if( 0> n)
  	{
  		  dcs_log(0,0,"<%s> respose_msg_proc error!",__FUNCTION__);
  			return -1;
  	}
  	if( n == 0) return 0;
  	
  }
  
  *to_fid=fold_locate_folder(pub_data_stru.route_fold_name );
  if( *to_fid <0 )
  {
  	dcs_log(0,0,"<%s> fold_locate_folder error! route_fold_name=[%s]",
  	        __FUNCTION__,pub_data_stru.route_fold_name);
  	return -1;
  }
  priv_def = match_priv_stru(pub_data_stru.route_msg_type,&gl_def_set); //���ݽ���ı��������ҵ���Ӧ�Ľ��������ֵ�
  if ( priv_def == NULL ) 
  {  	
  	dcs_log(0,0,"<%s> match_priv_stru error! route_msg_type=[%s]",
  	        __FUNCTION__,pub_data_stru.route_msg_type);
  	return -1;
  }
  pub_data_stru.route_priv_def=priv_def;
  if( 0>get_route_trans_set(&pub_data_stru))
  {	
  	 dcs_log(0,0,"<%s> get_route_trans_set error! ",__FUNCTION__);
  	 return -1;
  }
  
	if( pub_data_stru.req_flag ) 
	{
		int i;
		 dcs_debug(0,0,"<%s> db_insert begin",__FUNCTION__);   
 	   i=db_insert(&pub_data_stru,0);
	 	 if( 0 >=i) //���ݴ洢
	 	 {
	  	 	dcs_debug(0,0,"<%s> db_insert fail!",__FUNCTION__);
	  	 	if( i == 0 ) strcpy(pub_data_stru.center_result_code,"94");
	  	 	else strcpy(pub_data_stru.center_result_code,"96");
	  	 	pub_data_stru.switch_src_flag=1;
	  	 	if (pub_data_stru.in_cry_flag ) pub_data_stru.out_cry_flag =1;
	  	 	strcpy(pub_data_stru.route_insti_code,pub_data_stru.insti_code);
				strcpy(pub_data_stru.route_trans_type,pub_data_stru.in_trans_type);
				strcpy(pub_data_stru.route_msg_type,pub_data_stru.in_msg_type);
				strcpy(pub_data_stru.route_fold_name,pub_data_stru.insti_fold_name);
				strcpy(pub_data_stru.route_trans_type,pub_data_stru.in_trans_type);
	  	 	if( 0>get_route_trans_set(&pub_data_stru))
			  {	
			  	 dcs_log(0,0,"<%s> get_route_trans_set error! ",__FUNCTION__);
			  	 return -1;
			  }
	  	 	if(0>err_set_msg(&pub_data_stru)) return -1;
	  	 	*to_fid=fold_locate_folder(pub_data_stru.route_fold_name );
			  if( *to_fid <0 )
			  {
			  	dcs_log(0,0,"<%s> fold_locate_folder error! route_fold_name=[%s]",
			  	        __FUNCTION__,pub_data_stru.route_fold_name);
			  	return -1;
			  }
			  pub_data_stru.route_priv_def=pub_data_stru.in_priv_def;
	  	 	
	 	 }
	 	 else if( !(pub_data_stru.switch_src_flag) && 
	 	 	        pub_data_stru.use_timeout)
	 	 {
		 	 	if( 0>insert_timeout_table(&pub_data_stru,1))
		  	{
		  		dcs_log(0,0,"<%s> insert_timeout_table error!",__FUNCTION__);
		  		return -1;
		  	}
  	 }
	}

  
  dcs_debug(0,0,"<%s>begin pack_msg",__FUNCTION__);
  return pack_msg(&pub_data_stru,desc_buf,desc_size);
}
