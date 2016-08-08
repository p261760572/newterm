#include "base.h"
 #include "var.h"
 #include "tpos.h"
 #include "general_util.h"

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "ibdcs.h"
 #include "tools.h"
 #include "secu.h"

int zpos_login(glob_msg_stru * pub_data_stru)
{
	struct  tm *tm;   time_t  t;
	char tmp[128],psam[20];
	int i;
	ICS_DEBUG(0);
	dcs_debug(0,0,"entery zpos login proc");
	snprintf(pub_data_stru->route_msg_type,5,"%s",pub_data_stru->in_msg_type);
	snprintf(pub_data_stru->route_insti_code,9,"%s",pub_data_stru->insti_code);
	snprintf(pub_data_stru->route_fold_name,40,"%s",pub_data_stru->insti_fold_name);
	snprintf(pub_data_stru->route_trans_type,5,"%s",pub_data_stru->in_trans_type);
	pub_data_stru->tmp_order[0]=0x30;
	time(&t);
  tm = localtime(&t);
	snprintf(tmp,sizeof(tmp),"%4d%02d%02d%02d%02d%02d",
            tm->tm_year+1900,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec);

	add_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->route_msg_type,"DATE"),
									pub_data_stru->route_msg_type,8,tmp,1);
	add_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->route_msg_type,"TIME"),
									pub_data_stru->route_msg_type,6,tmp+8,1);
	
	if(0>(i=get_field_data_safe(pub_data_stru,20,pub_data_stru->in_msg_type,psam,17)))
	{
		dcs_log(0,0,"<%s> can not get psam!",__FUNCTION__);
		
		return -1;
	}
	psam[i]=0x00;
//	memset(tmp1,0,sizeof(tmp1));

	if( 0 >zpos_gen_work_key(pub_data_stru,psam))
		strcpy(pub_data_stru->center_result_code,"96");

	dcs_debug(0,0,"<%s> proc end",__FUNCTION__);
	return 1;
}

int zpos_check_terminal(glob_msg_stru * pub_data_stru)
{
	char psam[30];
	struct TPOS_TERM_INFO terminfo;
	int i;
	ICS_DEBUG(0);
	

	memset(&terminfo,0,sizeof(terminfo));
	if( 0>(i=get_field_data_safe(pub_data_stru,20,pub_data_stru->in_msg_type,psam,17)))
	{
		dcs_log(0,0,"<%s> get terminal no fail! field_num=%d",__FUNCTION__,pub_data_stru->msg_field_num);
		return -1;
	}
	psam[i]=0x00;
	if( pub_data_stru->insti_work_type )
	{
		 if( 0>tpos_get_work_key(psam,pub_data_stru->in_pin_index,pub_data_stru->in_mac_index,pub_data_stru->in_data_index,pub_data_stru->in_pin_key,pub_data_stru->in_mac_key,pub_data_stru->in_data_key))
		 {
		 	 dcs_log(0,0,"<%s> tpos_get_work_key fail psam=%s ",__FUNCTION__,psam);
		 	 strcpy(pub_data_stru->center_result_code,"96");
		 	 return -1;
		 }
		 dcs_debug(0,0,"pin_index=[%s] pin_key=[%s]\n mac_index=[%s] mac_key[%s]\ncd_index=[%s] cd_key=[%s]",
         pub_data_stru->in_pin_index,pub_data_stru->in_pin_key,pub_data_stru->in_mac_index,pub_data_stru->in_mac_key,pub_data_stru->in_data_index,pub_data_stru->in_data_key);
		 snprintf( pub_data_stru->route_mac_index,5,"%s",pub_data_stru->in_pin_index);
		 snprintf(pub_data_stru->route_mac_key,33,"%s",pub_data_stru->in_mac_key);
	}
	memset(&terminfo,0,sizeof(terminfo));
	i=get_tpos_info(psam,&terminfo);
	if( 0 > i)
	{
		strcpy(pub_data_stru->center_result_code,"Z1");
		return -1;
	}
	if( terminfo.status[0] ==0x30)
	{
		strcpy(pub_data_stru->center_result_code,"Z1");
		dcs_log(0,0,"<%s> status is %s",__FUNCTION__,terminfo.status);
		return -1;
	}
	dcs_debug(0,0,"<%s> check pass!",__FUNCTION__);
	return 1;
}



int _zpos_check_terminal(char *para, short fldid,glob_msg_stru * pub_data_stru)
{
	return zpos_check_terminal(pub_data_stru);
}

int zpos_get_ic_para(glob_msg_stru *pub_data_stru,struct TPOS_TERM_INFO *terminfo)
{
	int n,cnt;
	char buf[512+1],node_set[200+1],tmp[200];
	ICS_DEBUG(0);
	//1、在ic参数表中查找大于last_ic_para_step的记录，不存在则类参数已下完。
	 dcs_debug(0,0,"<%s> begin",__FUNCTION__);
	 memset(node_set,0,sizeof(node_set));
	 strcpy(node_set,"3C");
	 cnt =0;
   
   n= get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type, "60"),
   													pub_data_stru->in_msg_type,tmp,sizeof(tmp));
   if( n > 0 ) tmp[n]=0x00;
   else
   {
   		strcpy(pub_data_stru->center_result_code,"30");
   		return -1;
   }
   if( tmp[0]==0x31)
   {
		 n= get_aid_data(buf+1,512,node_set+2,&terminfo->last_ic_para_step, &cnt);
		 dcs_log(buf, n + 1, "<%s> get_aid_data len=[%d]",__FUNCTION__, n);
		 if ( n <0 ) return n;
		 if( n !=0)	
		    db_save_set_ic(node_set,terminfo->last_ic_para_step, terminfo->psam,0);
	 
		 dcs_debug(buf+1,n,"<%s> getaid_data buf n=%d",__FUNCTION__,n);
		 if( n>0)
		 {
		 	  buf[0]=cnt;
		 	  add_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->route_msg_type, "62"), 
		 	  								pub_data_stru->route_msg_type, n+1, buf, 1);	 	  
			 n=count_aid(terminfo->last_ic_para_step);
			 if( n >0)	add_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->route_msg_type, "60"), 
			 														pub_data_stru->route_msg_type, 2, "11", 1);	 	  
			 else if ( n==0) add_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->route_msg_type, "60"), 
			 																pub_data_stru->route_msg_type, 2, "10", 1);
			 else return -1;
		 }	
		 
	 }
	 else 
	 {
	   cnt=0;
		//2、在ic pub key表中查找大于last_ic_key_step的记录，不存则此类参数已下完。
		 n=get_pubkey_data(buf,512,node_set+strlen(node_set),&terminfo->last_ic_key_step,&cnt);
		 dcs_log(buf, n, "<%s> len=[%d]", __FUNCTION__, n);
		 if ( n <0 ) return n;
		 if( n !=0)	
		    db_save_set_ic(node_set,terminfo->last_ic_key_step,terminfo->psam,1);
		 dcs_debug(buf,n,"<%s> get_pubkey_data buf n=%d",__FUNCTION__,n);
		 
		 
		 if( n >0)
		 {
		   add_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->route_msg_type,"62"), 
		   								pub_data_stru->route_msg_type, n+1, buf, 1);
		   buf[0]=cnt;
		   n=count_pubkey(terminfo->last_ic_key_step);
		   if( n >0)	add_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->route_msg_type, "60"), 
		   														pub_data_stru->route_msg_type, 2, "21", 1);	 	  
			 else if( n==0) add_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->route_msg_type, "60"), 
			 																pub_data_stru->route_msg_type, 2, "20", 1);
			 else return -1;
		 }
	 }
	 dcs_debug(buf,n+1,"<%s> end",__FUNCTION__);
	 return n;
}


int zpos_download_para(glob_msg_stru *pub_data_stru)
{
	char psam[20],tmp[30];
	int n;
	struct TPOS_TERM_INFO terminfo;
	snprintf(pub_data_stru->route_msg_type,5,"%s",pub_data_stru->in_msg_type);
	snprintf(pub_data_stru->route_insti_code,9,"%s",pub_data_stru->insti_code);
	snprintf(pub_data_stru->route_fold_name,40,"%s",pub_data_stru->insti_fold_name);
	snprintf(pub_data_stru->route_trans_type,5,"%s",pub_data_stru->in_trans_type);
	pub_data_stru->tmp_order[0]=0x30;
	n=get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type, "20"),
													pub_data_stru->in_msg_type,psam,17);
	
	if( n >0 ) psam[n]=0x00;
	else{
  	 
  	 dcs_log(0,0,"<%s> get_field_data 20 fail! ",__FUNCTION__);
  	 strcpy(pub_data_stru->center_result_code,"30");
  	 return -1;
  }
	n=get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type, "60"),
													pub_data_stru->in_msg_type,tmp,sizeof(tmp));
  if( n > 0 ) tmp[n]=0x00;
  else{
  	 
  	 dcs_log(0,0,"<%s> get_field_data 60 fail! ",__FUNCTION__);
  	 strcpy(pub_data_stru->center_result_code,"30");
  	 return -1;
  }
	memset(&terminfo,0,sizeof(terminfo));
	if( 0>get_tpos_info(psam,&terminfo))
	{
			dcs_log(0,0,"<%s> get_tpos_info fail!",__FUNCTION__);
			return -1;
	}
	if( tmp[1]==0x30)
	{
		n=get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type, "39"),
														pub_data_stru->in_msg_type,tmp,sizeof(tmp));
		
		if( n >0 ) 
		{
			 tmp[n]=0x00;
			 if( strcmp( tmp,"00")==0) //中继下载，上次数据终端处理成功
			 {
			 	  empty_tmp_para(psam);
					terminfo.last_node_set[0]=0x00;
			 }
			 else //中继下载，上次数据终端处理未成功，需重传数据
			 {
				 char data[512+1];
				 int f;
				 f=0;
				 if( (n=get_ic_data(data+1, sizeof(data)-1,terminfo.last_node_set+2,&f))<0 ) return -1;//修改get_ic_data返回BUG 20140909
				 tmp[0]=f;
				 add_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->route_msg_type,"62"), 
				 								pub_data_stru->route_msg_type, n+1, tmp, 1);
				 return  1;
			 }
		}
		else
		{
			 dcs_log(0,0,"<%s> get_field_data 39 fail!",__FUNCTION__);
			 return -1;
		}
	}else
	{
		  if( 0>tpos_reset_download(psam,0))
		  {
		  	dcs_log(0,0,"<%s> tpos_reset_download fail!",__FUNCTION__);
		  	strcpy(pub_data_stru->center_result_code,"96");	
		  	return 1;
		  }
		  terminfo.last_node_set[0]=0x00;
	}
	n=zpos_get_ic_para(pub_data_stru,&terminfo);
	if( n >=0 ) strcpy( pub_data_stru->center_result_code,"00");	
	else strcpy( pub_data_stru->center_result_code,"96");	
	return 1;
}

//消费类交易成功时需返回商户名称
int zpos_fill_shopname(char *para, short fldid, glob_msg_stru *pub_data_stru)
{
	char psam[20];
	struct TPOS_TERM_INFO terminfo;
	int n;
	if(!pub_data_stru->switch_src_flag ) return 1;
	if(memcmp(pub_data_stru->center_result_code, "00000", strlen(pub_data_stru->center_result_code)))
	{
		return 1;
	}
	dcs_debug(0,0,"<%s> begin",__FUNCTION__);
	n=get_field_data_safe(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID1"),
													pub_data_stru->route_msg_type,psam,17);
	if( n >0 ) psam[n]=0x00;
	else{
  	 
  	 dcs_log(0,0,"<%s> get_field_data 20 fail! ",__FUNCTION__);
  	 strcpy(pub_data_stru->center_result_code,"30");
  	 return -1;
  }
	memset(&terminfo,0,sizeof(terminfo));
	if( 0>get_tpos_info(psam,&terminfo))
	{
			dcs_log(0,0,"<%s> get_tpos_info fail!",__FUNCTION__);
			return -1;
	}
	dcs_debug(0,0,"<%s> end name=[%s]",__FUNCTION__,terminfo.name);
	return add_pub_field(pub_data_stru,fldid, pub_data_stru->in_msg_type, 
												strlen(terminfo.name), terminfo.name, 1);
}


int zpos_field_pre_conv(char *para, short flag, glob_msg_stru *pub_data_stru)
{
	//磁道信息解密
	char tmp[512],tmp1[1024],return_code[4];
//	unsigned char *p;
	int n,outlen, i;
	ICS_DEBUG(0);
	if( (n=get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"36"),
															pub_data_stru->in_msg_type,tmp,sizeof(tmp)))>0)
	{
		tmp[n]=0x00;
		
			i=DecTrackPrg(return_code, pub_data_stru->in_data_index, pub_data_stru->in_data_key, n, tmp, &outlen, tmp);
			if( i>0 && i < 512)
			{
				if( memcmp(return_code,"00",2)!=0) return -1;
				bcd_to_asc((unsigned char *)tmp1,(unsigned char *)tmp,outlen*2,0);
				
				tmp1[outlen*2]=0x00;
				n=outlen*2;
				for ( ;n>0;n--)
				if( tmp1[n-1]=='D' || tmp1[n-1]=='d' ) tmp1[n-1]='=';
				dcs_debug(0,0,"track=[%s]",tmp1);
				memcpy(tmp,tmp1,2);
				tmp[2]=0x00;
				n=atoi(tmp);
				if( n > 37 )
				{
					dcs_log(0,0,"<%s>track2 error![%s]",__FUNCTION__,tmp1);
					strcpy(pub_data_stru->center_result_code,"30");					
					return -1;
				}
				if( n>0 )
				 add_pub_field(pub_data_stru, FIELD_TRACK2,pub_data_stru->route_msg_type,n,tmp1+2,1);
				
				for(i = 2; tmp1[i] && tmp1[i] != '='; i++);
				if(tmp1[i]  == '=' && i + 5 < strlen(tmp1)) 
				{
					if(tmp1[i + 5] == '2' || tmp1[i + 5] == '6')
					{
						add_pub_field(pub_data_stru, 60, pub_data_stru->route_msg_type, 15, "000005200900000", 1);
					}
					else
					{
						add_pub_field(pub_data_stru, 60, pub_data_stru->route_msg_type, 15, "000005000900000", 1);
					}
				}
				else
					add_pub_field(pub_data_stru, 60, pub_data_stru->route_msg_type, 15, "000005000900000", 1);
		    tmp1[i] = 0x00;
		    if( strlen(tmp1+2)<=19 && strlen(tmp1+2)>0)
					add_pub_field(pub_data_stru, FIELD_CARD_NO, pub_data_stru->route_msg_type, 
													strlen(tmp1+2), tmp1+2, 1);
				dcs_debug(0,0,"<%s>card_no=[%s]",__FUNCTION__,tmp1+2);
				memcpy(tmp,tmp1+2+n,3);
				tmp[3]=0x00;
				i=atoi(tmp);
				if( i >0 )
				{
						if( i >104)
						{
							dcs_log(0,0,"<%s>track3 error![%s]",__FUNCTION__,tmp+2+3+n);
							strcpy(pub_data_stru->center_result_code,"30");					
							return -1;
						}
						add_pub_field(pub_data_stru,FIELD_TRACK3,pub_data_stru->route_msg_type,i,tmp1+2+3+n,1);
						dcs_debug(0,0,"<%s>track3=[%d][%s]",__FUNCTION__,i,tmp1+2+3+n );
				}
				del_pub_field(pub_data_stru,FIELD_TRACK3,pub_data_stru->route_msg_type,0 );
			}
			else
			{
				dcs_log(0,0,"<%s> convert track date failed! ",__FUNCTION__);
				strcpy(pub_data_stru->center_result_code,"91");
				return -1;
			}
		
	}
	else
		add_pub_field(pub_data_stru, 60, pub_data_stru->route_msg_type, 15, "000000000300000", 1);
	return 1;
}

int _zpos_get_work_key(glob_msg_stru * pub_data_stru)
{
	char psam[20];
	int i;
	ICS_DEBUG(0);
	i=_get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->route_msg_type, "20"),
													pub_data_stru->route_msg_type,psam, pub_data_stru->route_num == 0 ? 0 : 1,17);
	if( i<=0)
	{
		dcs_log(0,0,"<%s> can not got psam_no!",__FUNCTION__);
		return -1;
	}
	psam[i]=0x00;
	return tpos_get_work_key(psam,pub_data_stru->route_pin_index,pub_data_stru->route_mac_index,pub_data_stru->route_data_index,pub_data_stru->route_pin_key,pub_data_stru->route_mac_key,pub_data_stru->route_data_key);
}

int insti_login(glob_msg_stru * pub_data_stru)
{
	struct  tm *tm;   time_t  t;
	char tmp[128];

	ICS_DEBUG(0);
	dcs_debug(0,0,"entery insti_login login proc");
	snprintf(pub_data_stru->route_msg_type,5,"%s",pub_data_stru->in_msg_type);
	snprintf(pub_data_stru->route_insti_code,9,"%s",pub_data_stru->insti_code);
	snprintf(pub_data_stru->route_fold_name,40,"%s",pub_data_stru->insti_fold_name);
	snprintf(pub_data_stru->route_trans_type,5,"%s",pub_data_stru->in_trans_type);
	pub_data_stru->tmp_order[0]=0x30;
	time(&t);
  tm = localtime(&t);
	snprintf(tmp,sizeof(tmp),"%4d%02d%02d%02d%02d%02d",
            tm->tm_year+1900,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec);

	add_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->route_msg_type,"DATE"),
									pub_data_stru->route_msg_type,8,tmp,1);
	add_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->route_msg_type,"TIME"),
									pub_data_stru->route_msg_type,6,tmp+8,1);
	if( 0 >insti_gen_work_key(pub_data_stru))
		strcpy(pub_data_stru->center_result_code,"96");
	else 
		strcpy(pub_data_stru->center_result_code,"00");
	dcs_debug(0,0,"<%s> proc end",__FUNCTION__);
	return 1;
}
