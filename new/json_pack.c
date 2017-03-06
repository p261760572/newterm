#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "base.h"
#include "tools.h"
#include "general_util.h"
#include <json-c/json.h>  

#define TAG(a)		#a
#define TAG2(a)		TAG(a)
typedef struct json_object* JSON;

int str2json(char *tag, char *str, enum json_type type, void **object_json) {
		
		if(*object_json == NULL) { 
				*object_json = json_object_new_object(); 
		}
		
		JSON obj_json = (JSON)*object_json;
		if(type == json_type_boolean || type == json_type_double || type == json_type_int) 
				json_object_object_add(obj_json, tag, json_object_new_int(atoi(str)));  
		else if(type == json_type_string) {
				json_object_object_add(obj_json, tag, json_object_new_string(str));
		}
		else if(type == json_type_array) {
				int i;
				char **p_str = (char **)str;
				JSON array_object = json_object_new_array();
				if(array_object == NULL) goto free;
				for(i = 0; p_str[i] && p_str[i][0]; i++) {
					json_object_array_add(array_object, json_object_new_string(p_str[i]));  
				}
		    json_object_object_add(obj_json, tag, array_object);  
		}
		return strlen(json_object_to_json_string(obj_json));
		
free:
		json_object_put(obj_json);//free  
		return -1;
}

char *json2str(void *object_json) {
		return (char *)json_object_to_json_string((JSON)object_json); 
}

void freeJson(void *object_json) {
		if(!object_json) return;
		JSON object = (JSON) object_json;
		json_object_put(object);
}

char *gettag(void *object_json, char *tag) {
		if(!tag || !object_json) return NULL;
		JSON object = (JSON) object_json;
		JSON ret_object;
		json_object_object_get_ex(object, tag, &ret_object);
		return (char *)json_object_to_json_string(ret_object);
}

void printfJson(void *object_json) {
		if(!object_json) return;
		JSON object = (JSON) object_json;
		json_object_object_foreach(object,key,val) {
				dcs_log(0, 0, "[%s]: %s\n", key,json_object_to_json_string(val));
		}
}

int json_pack(glob_msg_stru *pub_data_stru,char *buf,int size) {
		field_set *p_set;
		p_set=&pub_data_stru->route_set;
		char *p = NULL, tmp[512], tmp1[512], head[512];
		int i, len;
		int head_flag,headlen,msgid_flag,bitmap_flag,len_type;
		field_define    *p_fld_def;
		
		char *msg_type= pub_data_stru->route_msg_type;
    message_define *priv_def = pub_data_stru->route_priv_def;
		if(strcmp(msg_type,"JSON")!=0) {
        dcs_log(0,0,"<%s> not recognition msgtype[%s]!",__FUNCTION__,msg_type);
        return -1;
    }
    if(0>get_iso_para(msg_type,&head_flag,&headlen,&msgid_flag,
                      &bitmap_flag,&len_type)) {
        head_flag=0;
        headlen=0;
        msgid_flag=0;
        bitmap_flag=1;
        len_type=0;
    }
    for(i=0 ; i<p_set->num ; i++) {
    		memset(tmp, 0, sizeof(tmp));
    		memset(tmp1, 0, sizeof(tmp1));
        if(0<(len=get_field_data_safe(pub_data_stru, get_pub_field_id(msg_type,p_set->field.field_name[i]),
                                    msg_type, tmp,sizeof(tmp)))) {
            p_fld_def = get_priv_field_def_for_id(p_set->field.field_id[i], priv_def);
		        if(p_fld_def == NULL) {
		            dcs_log(0, 0, "<%s>取数据域定义出错p_set->field_id[%d]=[%d]",
		                    __FUNCTION__, i, p_set->field.field_id[i]);
		            return -1;
		        }
		        if(p_fld_def->is_compress) {
		            asc_to_bcd((unsigned char *)tmp1, (unsigned char *)tmp, len, p_fld_def->is_compress-1);
		        } else {
		            memcpy(tmp1, tmp, len);
		        }
		        if(head_flag && memcmp(p_set->field.field_name[i], "TPDU", 4) == 0) {
		        		dcs_log(0, 0, "[HEAD] :%s\n", tmp1);	
		        		memcpy(head, tmp1, headlen);
		        }
        		else 
        				str2json(p_set->field.field_name[i], tmp1, json_type_string, (void **)&p);                            	
        }
    }
    printfJson(p);
		p = json2str(p);
		
    len = strlen(p);
    if(head_flag) {
    		memcpy(buf, head, headlen);	
    }
    memcpy(buf+headlen, p, len);
    freeJson(p);
    return len+headlen;
}