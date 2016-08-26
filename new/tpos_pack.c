#include "base.h"
#include "general_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "var.h"
#include "tools.h"
#include "secu.h"
static int disp_len;
static char disp_buf[8192];


int tpos_pub_to_priv(short id ,const char *msg_type,const field_define *p_def,
                     char *buf,int size,glob_msg_stru *pub_data_stru,short from);

int tpos_pack(glob_msg_stru *pub_data_stru,char *buf,int size) {
    char *msg_type = NULL;
    message_define *priv_def =NULL;
//k 计算报文crc校验的起始位置
//mark B_LEN标签位置,需要通过后面的计算才知道B_LEN的值
//mark1 S_LEN标签位置,需要通过后面的计算才知道B_LEN的值
    int len1,len2,len3,n,s,i,mark,mark1,k,flag;
    unsigned char cnt;
    short id;
    char tmp_buf1[512],data_buf[1024],func_code[10],tmp[64];
    char *p,return_code[4],op_name[20];
    field_define *p_fld;
    field_set *p_set;
    char head_stru[]="TPDU,B_HEAD,B_LEN,RANDOM,MSG_SEQ,S_LEN," \
                     "P_HEAD,END_FLAG,PROG_VER,MENU_VER,PSAM_NO,DATE,TIME," \
                     "SEQ_NO,FUNC_CODE";
    msg_type = pub_data_stru->route_msg_type;
    priv_def = pub_data_stru->route_priv_def;
    if(strcmp(msg_type,"TPOS")!=0) return -1;

    len1 =0;
//设置报文头
    memset(data_buf, 0 , sizeof(data_buf));
    func_code[0]=0x00;
    _get_field_data_safe(pub_data_stru,get_pub_field_id(msg_type,"FUNC_CODE"),
                        msg_type,func_code,1,sizeof(func_code));
//tmp_order[1]，终端需要自动更新菜单标识
    if(pub_data_stru->tmp_order[1]==0x31 &&
       memcmp(pub_data_stru->center_result_code,"00",2) ==0) {

        if(is_permit_update_menu("TPOS",func_code)) { //为自动更新菜单加入
            del_route_set(pub_data_stru,msg_type,"26");//删除挂机指令
            add_route_set(pub_data_stru,msg_type,"0D", 1);
            add_pub_field(pub_data_stru,get_pub_field_id(msg_type,"0D"),
                          msg_type,0,tmp,1);
            add_route_set(pub_data_stru,msg_type,"24", 1);
            add_pub_field(pub_data_stru,get_pub_field_id(msg_type,"24"),msg_type,
                          0,NULL,1);
            add_route_set(pub_data_stru,msg_type,"25", 1);
            add_pub_field(pub_data_stru,get_pub_field_id(msg_type,"25"),msg_type,
                          0,NULL,1); //增加接收指令
            if(0>update_pub_field(pub_data_stru,get_pub_field_id(msg_type,"FUNC_CODE"),
                                  msg_type,3,"003",2)) {
                update_pub_field(pub_data_stru,get_pub_field_id(msg_type,"FUNC_CODE"),
                                 msg_type,3,"003",0);
            }
        }
    }
    disp_len=0,flag=0,k=0;
    for(p=my_split(head_stru,',',op_name,sizeof(op_name));
        p; p=my_split(p,',',op_name,sizeof(op_name))) {
        p_fld=get_priv_field_def(op_name, &id,priv_def);
        if(p_fld == NULL) {
            dcs_log(0,0,"<%s> can not get field[%s] property!",__FUNCTION__,op_name);
            return -1;
        }
        n=tpos_pub_to_priv(id ,msg_type,p_fld,buf+len1,
                           size-len1,pub_data_stru,1);
        if(0> n)          n=0;
        if(0 == flag) {
            flag = 1;
            if(n > 0) {
                memcpy(tmp,buf+1,4);
                memcpy(buf+1,tmp+2,2);
                memcpy(buf+3,tmp,2);
                k=5;
            }
        }
        if(strcmp(op_name,"B_LEN")==0) {
            mark=len1;
        } else if(strcmp(op_name,"S_LEN")==0) {
            mark1=len1;
        }
        len1 =len1+n;
    }

    if(0 >tpos_add_result_disp(pub_data_stru)) {
        dcs_log(0,0,"<%s> tpos_add_result_disp fail!",__FUNCTION__);
        return -1;
    }

    len3=2;
    cnt=0;
    if(strcmp(func_code,"051") !=0)
        len2=2;
    else
        len2=1;

    p_set=&pub_data_stru->route_set;
    dcs_debug(0,0,"<%s>  set num=%d",__FUNCTION__,p_set->num);

    for(i=0; i<p_set->num; i++) {
        p_fld=get_defstru_of_id(p_set->field.field_id[i],priv_def);
        if(p_fld==NULL) {
            dcs_log(0,0,"<%s> can not get field[%s] property!",
                    __FUNCTION__,p_set->field.field_name[i]);
        }
        n=tpos_pub_to_priv(p_set->field.field_id[i] ,msg_type,p_fld,
                           data_buf+len3,sizeof(data_buf)-len3,pub_data_stru,
                           p_set->field.from[i]);
        if(0>n) {
            continue;
        }

        len3=len3+n;
        n=tpos_conver_priv_field_code(p_fld,func_code,tmp_buf1+len2);
        if(n <0) return -1;
        len2=len2+n;
        cnt++;
    }
// memcpy(buf,head_buf,len1);
    if(strcmp(func_code,"051") !=0) {
        tmp_buf1[1]=0xA8;//插入校验MAC操作码
        dcs_debug(0,0,"插入校验MAC操作码");
        cnt++;
    } else if(strcmp(func_code,"003")==0 && cnt ==1) {
        tmp_buf1[len2]=0xA6;
        cnt++;
        len2++;
    }
    tmp_buf1[0]=cnt;
    if(len1+len2 >= size) {
        dcs_log(0,0,"<%s> out cache overflow! len2",__FUNCTION__);
        return -1;
    }
	dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,len2);
    memcpy(buf+len1,tmp_buf1,len2);
    dcs_debug(0,0,"<%s>len1=%d,tmp_buf1[0]=%02x,len3=%d",
              __FUNCTION__,len1,tmp_buf1[0],len3);
    len1=len1+len2;
    data_buf[0]=(len3-2+8)/256;
    data_buf[1]=(len3-2+8)%256;
    if(len1+len3 > size) {
        dcs_log(0,0,"<%s> out cache overflow! len3",__FUNCTION__);
        return -1;
    }
	dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,len3);
    memcpy(buf+len1,data_buf,len3);
    len1=len1+len3;
    buf[mark]=(len1-mark-2+8)/256;
    buf[mark+1]=(len1-mark-2+8)%256;
    buf[mark1]=(len1-mark1-2+8)/256;
    buf[mark1+1]=(len1-mark1-2+8)%256;
//做签名处理
    dcs_debug(0,0,"<%s> mark=%d,len1=%d buf[mark]=%02x buf[mark+1]=%02x,cnt=%d",
              __FUNCTION__,mark,len1,(len1-mark-2)/256,(len1-mark-2)%256,cnt);
// memset(data_buf,0,sizeof(data_buf));
	dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,len1-mark1-2);
    memcpy(data_buf,buf+mark1+2,len1-mark1-2);
    if(pub_data_stru->out_cry_flag) {
        if(((len1-mark1-2)%16)) {
            n=len1-mark1-2 +(16-((len1-mark1-2)%16));
        } else n=len1-mark1-2;
    } else {
        if(((len1-mark1-2)%8)) {
            n=len1-mark1-2 +(8-((len1-mark1-2)%8));
        } else n=len1-mark1-2;
    }
    if(strcmp(func_code,"051") !=0 || (strcmp(func_code,"051") ==0 &&
                                       memcmp(pub_data_stru->center_result_code,"00",2) ==0)) {
        memset(return_code,0,sizeof(return_code));
        dcs_debug(data_buf,n,"<tpos_pack> mac_index=[%s],mac_key=[%s]",
                  pub_data_stru->route_mac_index,pub_data_stru->route_mac_key);
        memset(tmp,0,16);

        if(pub_data_stru->out_cry_flag) {
            for(i=0; i<n;)
                for(s=0; i<n&&s<16; s++,i++)
                    tmp[s]=((unsigned char)tmp[s])^((unsigned char)data_buf[i]);
        } else {
            for(i=0; i<n;)
                for(s=0; i<n&&s<8; s++,i++)
                    tmp[s]=((unsigned char)tmp[s])^((unsigned char)data_buf[i]);
        }

        if(pub_data_stru->out_cry_flag)
            n=CALCMAC(return_code, pub_data_stru->route_mac_index,
                      pub_data_stru->route_mac_key, 0, 16, tmp, data_buf);
        else
            n=DESCALCMAC(return_code,pub_data_stru->route_mac_index,
                         pub_data_stru->route_mac_key,8,tmp,data_buf);
        if(n < 0) {
            dcs_log(0,0,"<%s> DESCALCMAC [%s] ",__FUNCTION__,
                    pub_data_stru->route_mac_index);
            return -1;
        }
        if(memcmp(return_code,"00",2)!=0) return -1;
        if(pub_data_stru->out_cry_flag) {
            asc_to_bcd((unsigned char *)tmp,(unsigned char *)data_buf,32,0);
            if(len1+16 >= size) {
                dcs_log(0,0,"<%s> out cache overflow! mac",__FUNCTION__);
                return -1;
            }
            memcpy(buf+len1,tmp,16);
            len1=len1+16;
        } else {
            asc_to_bcd((unsigned char *)tmp,(unsigned char *)data_buf,16,0);
            if(len1+8 >= size) {
                dcs_log(0,0,"<%s> out cache overflow! mac",__FUNCTION__);
                return -1;
            }
            memcpy(buf+len1,tmp,8);
            len1=len1+8;
        }
    }
    cnt=0x00;
    for(; k<len1; k++)
        cnt=cnt ^(unsigned char)buf[k];
    buf[len1]=cnt;
    len1++;
    dcs_debug(0,0,"at %s(%s:%d)\n%s",__func__, __FILE__, __LINE__,disp_buf);
    dcs_debug(buf,len1,"<%s> pack end len=%d,mark=%d",__FUNCTION__,len1,mark);
    return len1;
}



int tpos_pub_to_priv(short id ,const char *msg_type,const field_define *p_def,
                     char *buf,int size,glob_msg_stru *pub_data_stru,short from) {
    int n;
    char tmp_buf[1024];

    if(id <0 || p_def == NULL) {
        dcs_log(0,0,"<%s> can not prase id[%d]",__FUNCTION__,id);
        return -1;
    }
    /*
    if( pub_data_stru->route_num==0)
      n=get_field_data_safe(pub_data_stru,id,msg_type,tmp_buf,sizeof(tmp_buf));
    else*/
    n=_get_field_data_safe(pub_data_stru,id,msg_type,tmp_buf,from,sizeof(tmp_buf));
    if(n <0) return -1;

//	dcs_debug(tmp_buf,n,"at %s(%s:%d) msg_type[%s] id=[%d] from=[%d]",__FUNCTION__,__FILE__,__LINE__,
//		msg_type, id, from);

    if(p_def->d_len_type == 0) { //固定长度
        if(n >p_def->d_max_len) {
            dcs_log(0,0,"<%s> data limited n=%d, max_len=%d",
                    __FUNCTION__,n,p_def->d_max_len);
            return -1;
        }
        if(n >0) {
            if(p_def->d_is_compress) {
                if((n+1)/2 >size) {
                    dcs_log(0,0,"<%s> data limited n=%d, size=%d",
                            __FUNCTION__,(n+1)/2,size);
                    return -1;
                }
                asc_to_bcd((unsigned char *)buf,(unsigned char *)tmp_buf,n,p_def->d_is_compress-1);
                n = (p_def->d_max_len+1)/2;
                tmp_buf[p_def->d_max_len]=0x00;
            } else {
                if(n >size) {
                    dcs_log(0,0,"<%s> data limited n=%d, size=%d",
                            __FUNCTION__,n,size);
                    return -1;
                }
				dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,n);
                memcpy(buf,tmp_buf,n);
                n =p_def->d_max_len;
            }
        }
        disp_len += snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                             "field[%s]=[%d][%s]\n",p_def->name,n,tmp_buf);
        return n;
    } else if(p_def->d_len_type == 1) {  //1字节可变长长度域
        if(n==0) return 0;
        if(n >p_def->d_max_len) {
            dcs_log(0,0,"<%s> name[%s]超出长度限制,len=%d,max_len=%d",
                    __FUNCTION__,p_def->name,n,p_def->d_max_len);
            return -1;
        }
        buf[0]=n;
        if(p_def->d_is_compress) {
            if((n+1)/2 >size) {
                dcs_log(0,0,"<%s> data limited n=%d, size=%d",
                        __FUNCTION__,(n+1)/2,size);
                return -1;
            }
            asc_to_bcd((unsigned char *)buf+1,(unsigned char *)tmp_buf,
                       n,p_def->d_is_compress-1);
            n =(n+1)/2 +1;

        } else {
            if(n >size) {
                dcs_log(0,0,"<%s> data limited n=%d, size=%d",
                        __FUNCTION__,n,size);
                return -1;
            }
			dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,n);
            memcpy(buf+1,tmp_buf,n);
            n =n+1;
        }
        disp_len += snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                             "field[%s]=[%d][%s]\n",p_def->name,n,tmp_buf);
        return n;
    } else if(p_def->d_len_type == 2) {  //2字节可变长长度域
        if(n==0) return 0;
        if(n > p_def->d_max_len) {
            dcs_log(0,0,"<%s>name[%s]var 2超出长度限制,len=%d,max_len=%d",
                    __FUNCTION__,p_def->name,n,p_def->d_max_len);
            return -1;
        }
        buf[0]=n/256;
        buf[1]=n%256;
        if(p_def->d_is_compress) {
            if((n+1)/2 >(size-2)) {
                dcs_log(0,0,"<%s> data limited n=%d, size=%d",
                        __FUNCTION__,(n+1)/2,size-2);
                return -1;
            }
            asc_to_bcd((unsigned char *)buf+2,(unsigned char *)tmp_buf,
                       n,p_def->d_is_compress-1);
            n =(n+1)/2 +2;

        } else {
            if(n >size-2) {
                dcs_log(0,0,"<%s> data limited n=%d, size=%d",
                        __FUNCTION__,n,size-2);
                return -1;
            }
			dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,n);
            memcpy(buf+2,tmp_buf,n);
            n =n+2;
        }

        disp_len += snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                             "field[%s]=[%d][%s]\n",p_def->name,n,tmp_buf);
        return n;
    } else {
        dcs_log(0,0,"<%s> can not parase len_type=[%d]",__FUNCTION__,
                p_def->d_len_type);
    }
    return -1;
}
