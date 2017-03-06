#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "base.h"
#include "tools.h"
#include "general_util.h"
#include <json-c/json.h>  

#define SOAP_TRANS_YPE  "__calltype"
#define SOAP_DATE				"__date"
#define SOAP_SEQNO			"__seqno"
typedef struct json_object* JSON;

int json_unpack(char *src_buf,int src_len,glob_msg_stru *pub_data_stru) {
		int len, pos;
		char *msg_type= pub_data_stru->in_msg_type;
		char trans_type[4], date[8], seqno[6];
		int head_flag,headlen,msgid_flag,bitmap_flag,len_type;
    
		if(strcmp(msg_type,"JSON")!=0) {
        dcs_log(0,0,"<%s> not recognition msgtype[%s]!",__FUNCTION__,msg_type);
        return -1;
    }
    if(0>get_iso_para(msg_type,&head_flag,&headlen,&msgid_flag,&bitmap_flag,
                      &len_type)) {
        head_flag=0;
        headlen=0;
        msgid_flag=0;
        bitmap_flag=1;
        len_type=0;
    }
    if(head_flag) {
		    memcpy(trans_type, src_buf, 4);
		    memcpy(date, src_buf+4, 8);
		    memcpy(seqno, src_buf+12, 6);
		    if(0>add_pub_field(pub_data_stru,get_pub_field_id(msg_type,SOAP_TRANS_YPE),
				                       msg_type,4,trans_type,0)) {
				    dcs_log(0,0,"<%s> add_pub_field error! mark=[%s] len=%d,data=[%s] ",__FUNCTION__,
				        							SOAP_TRANS_YPE,4,trans_type);
				        return -1;
				}
				if(0>add_pub_field(pub_data_stru,get_pub_field_id(msg_type,SOAP_DATE),
				                       msg_type,8,date,0)) {
				    dcs_log(0,0,"<%s> add_pub_field error! mark=[%s] len=%d,data=[%s] ",__FUNCTION__,
				        							SOAP_TRANS_YPE,8,trans_type);
				        return -1;
				}
				if(0>add_pub_field(pub_data_stru,get_pub_field_id(msg_type,SOAP_SEQNO),
				                       msg_type,6,seqno,0)) {
				    dcs_log(0,0,"<%s> add_pub_field error! mark=[%s] len=%d,data=[%s] ",__FUNCTION__,
				        							SOAP_TRANS_YPE,6,trans_type);
				        return -1;
				}
		}
		
		if(!isdigit(src_buf[headlen])) {
				add_pub_field(pub_data_stru,get_pub_field_id(msg_type,"retcode"),
		                       msg_type,2,"00",0);
		}
		else {
				if(0>add_pub_field(pub_data_stru,get_pub_field_id(msg_type,"retcode"),
		                       msg_type,3,src_buf+headlen,0)) {
		        dcs_log(0,0,"<%s> add_pub_field error! mark=[%s] len=%d,data=[%s] ",__FUNCTION__,
		        							"retcode",3,src_buf+headlen);
		        return -1;
		    }
		}
		
		if(src_buf[headlen] && src_buf[headlen] != '{') {
		    add_pub_field(pub_data_stru,get_pub_field_id(msg_type,"errmsg"),
		                       msg_type,src_len-headlen,src_buf+headlen,0);
		}
		else if(src_buf[headlen]) {
		    JSON object = json_tokener_parse(src_buf+headlen);
		    
		    if(!object) {
		    		dcs_log(src_buf+headlen, src_len-headlen, "unpack json error!");
		    		return -1;	
		    }
		    json_object_object_foreach(object,key,val) {
		    		// ×Ö·ûÈ¥ \"
		    		pos = 0;
		    		len = strlen(json_object_to_json_string(val));
		    		if(json_object_get_type(val) == json_type_string) {
		    				pos = 1;
		    		}
						if(0>add_pub_field(pub_data_stru,get_pub_field_id(msg_type,key),
				                       msg_type,len-(pos<<1),json_object_to_json_string(val)+pos,0)) {
				        dcs_log(0,0,"<%s> add_pub_field error! mark=[%s] len=%d,data=[%s] ",__FUNCTION__,
				        							key,len-(pos<<1),json_object_to_json_string(val)+pos);
				        return -1;
				    }
				}
		}
    return 0;
}