#include "base.h"
#include "general_util.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "var.h"
#include "ibdcs.h"
#include "tools.h"
static int disp_len;
static char disp_buf[8192];

int get_oper_code(int num,char *src_buf,int src_len,char *code_buf,int max_buf);
int tpos_priv_to_pub(char * field_name ,const message_define *p_def,const char *buf,int size,
                     glob_msg_stru *pub_data_stru,const char *info);

int conver_op_code_name(char *op_code ,char * op_name,int size);


int tpos_unpack(char *src_buf,int src_len,glob_msg_stru *pub_data_stru) {
    char *msg_type,*p;
    message_define *priv_def;
    int offset,len,num,op_set,i,flag;
    char tmp_buf[1024],op_name[40],info[5];
    char head_stru[]="TPDU,TEL_NO,B_HEAD,B_LEN,RANDOM,MSG_SEQ,S_LEN," \
                     "P_HEAD,END_FLAG,PROG_VER,MENU_VER,DISP_FLG,PSAM_NO,SEQ_NO," \
                     "FUNC_CODE,F_NUM";
    assert(src_buf);
    msg_type = pub_data_stru->in_msg_type;
    priv_def = pub_data_stru->in_priv_def;
    offset=0;
    disp_len=0;
    if(memcmp(msg_type,"TPOS",4)!=0) {
        dcs_log(0,0,"<%s>error msgt_ype[%s]",__FUNCTION__,msg_type);
        return -1;
    }
    len=0,offset=0,flag=0;
    for(p=my_split(head_stru,',',op_name,sizeof(op_name));
        p; p=my_split(p,',',op_name,sizeof(op_name)), len=0) {
        if(0 == flag) {
            flag =1;
            if(src_buf[offset]==0x60)
                len=tpos_priv_to_pub(op_name,priv_def,src_buf+offset,src_len,
                                     pub_data_stru, NULL);
        } else if(1 == flag) {
            flag = 2;
            if(memcmp(src_buf +offset,"\x4C\x52\x49\x00\x1C", 5)==0)
                len=tpos_priv_to_pub(op_name,priv_def,src_buf+offset,src_len,
                                     pub_data_stru, NULL);
        } else if(2 == flag) {
            flag = 3;
            len=tpos_priv_to_pub(op_name,priv_def,src_buf+offset,src_len,
                                 pub_data_stru, NULL);
            if(memcmp(src_buf+offset,"\x087",1) != 0) {
                dcs_log(0,0,"<%s>error msg head [%02x]",__FUNCTION__,src_buf[offset]);
                return -1;
            }
        } else if(3 == flag) {
            flag = 4;
            len=tpos_priv_to_pub(op_name,priv_def,src_buf+offset,src_len,
                                 pub_data_stru, NULL);
            i=((unsigned char)src_buf[offset])*256+(unsigned char)src_buf[offset+1];
            if(i !=src_len-1-2) {
                dcs_log(src_buf+offset,len,
                        "<%s>len and data no match!len=%d,src_len=%d,field_name[%s] ",
                        __FUNCTION__,i,src_len-1-2, op_name);
                return -1;
            }
        } else if(4 == flag) {
            len=tpos_priv_to_pub(op_name,priv_def,src_buf+offset,src_len,
                                 pub_data_stru, NULL);
            if(strcmp(op_name,"MSG_SEQ")==0) {
                flag =3;
            } else if(strcmp(op_name,"F_NUM")==0) {
                num= (unsigned char)src_buf[offset];
            }
        }
        if(len <0) {
            dcs_log(0,0,"<%s> can not get <%s>",__FUNCTION__,op_name);
            return -1;
        }
		dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,len);
        memcpy(tmp_buf,src_buf+offset,len);
        offset = offset+len;
        src_len = src_len -len;
        if(src_len <=0) {
            dcs_log(tmp_buf,len,"<%s> [%s] after no data  ,src_len <=0",__FUNCTION__,op_name);
            return -1;
        }
    }

    len=get_oper_code(num,src_buf+offset,src_len,tmp_buf,sizeof(tmp_buf));
    if(len <=0) {
        dcs_log(0,0,"<%s>get oper code fail! num=%d",__FUNCTION__,num);
        return -1;
    }
    offset = offset+len+2;
    src_len = src_len -len-2;
    if(src_len <=0) {
        dcs_log(0,0,"<%s> oper code after no data !",__FUNCTION__);
        return -1;
    }
    //解析流程代码
    op_set=0;
    for(i=0; i<num; i++) {
        len=conver_op_code_name(tmp_buf+op_set,op_name,sizeof(op_name)); //转换操作代码的成为数据库中设置的名字
        if(len <=0) {
            dcs_log(0,0,"<%s>conver op_code name fail!",__FUNCTION__);
            return -1;
        }
        op_set = op_set+len;
        if(len ==2) {
            memset(info,0,sizeof(info));
            sprintf(info,"%02x",tmp_buf[op_set-1]);
            len=tpos_priv_to_pub(op_name,priv_def,src_buf+offset,src_len,pub_data_stru,info); //操作代码所对应的值解析
        } else
            len=tpos_priv_to_pub(op_name,priv_def,src_buf+offset,src_len,pub_data_stru,NULL); //操作代码所对应的值解析
        if(len <0) {
            dcs_log(0,0,"tpos_priv_to_pub fail op_name[%s]!",op_name);
            return -1;
        }
        offset = offset+len;
        src_len = src_len -len;
        if(src_len <0) {
            dcs_log(0,0,"<%s> op_name[%s] after no data ",__FUNCTION__,op_name);
            return -1;
        }
    }
    dcs_debug(0,0,"<%s>tpos解包\n%s",__FUNCTION__,disp_buf);
    return 1;
}

/**
将操作码还原单字节操作码，并返回当前操作码长度
**/
int conver_op_code_name(char *op_code ,char * op_name,int size) {
    int i,len;

    assert(op_code);
    assert(op_name);
    len =-1;
    if(op_code !=NULL)
        snprintf(op_name,size,"%02X",(unsigned char)(op_code[0]&0x03F));
    else return -1;
    i=(((unsigned char)op_code[0])>>6) & 0x03;
    if(i==0) len=2;             //双字节操作码
    else if(i == 2) len =1;    //单字节操作码
    else if(i== 3) len =3;    //三字节操作码
    return len;
}
/**
计算oper code 集合数据长度，并拷贝至缓冲区
**/
int get_oper_code(int num,char *src_buf,int src_len,char *code_buf,int max_buf) {
    int i,s;

    i=0;
    for(; num >0 && i<=src_len; num --) {
        s=(src_buf[i]>>6)&0x03;
        if(s==0) i=i+2;
        else if(s==2) i=i+1;
        else if(s == 3) i=i+3;
        else return -1;
    }
    if(i < max_buf && i<=src_len) {
		dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,i);
        memcpy(code_buf,src_buf,i);
    }
    else return -1;
    return i;
}

int tpos_priv_to_pub(char * field_name ,const message_define *p_def,const char *buf,int size,
                     glob_msg_stru *pub_data_stru,const char *info) {
    int i,len;
    char tmp_buf[1024];

    for(i=0; i<p_def->use_num; i++) {

        if(strcmp(field_name,p_def->fld_def[i].name)==0) {
            if(p_def->fld_def[i].len_type == 0) { //固定长度
                if(p_def->fld_def[i].is_compress) {
                    if(p_def->fld_def[i].max_len < sizeof(tmp_buf)/2)
                        bcd_to_asc((unsigned char *)tmp_buf,(unsigned char *)buf,
                                   p_def->fld_def[i].max_len,0);
                    else
                        return -1;
                    add_pub_field(pub_data_stru,p_def->fld_def[i].id,pub_data_stru->in_msg_type,
                                  p_def->fld_def[i].max_len,tmp_buf,0);
                    len = (p_def->fld_def[i].max_len+1)/2;
                    tmp_buf[p_def->fld_def[i].max_len]=0x00;
                    disp_len +=snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                                        "field[%s]=[%d][%s]\n",p_def->fld_def[i].name,len,tmp_buf);
                } else {

                    if(0>add_pub_field(pub_data_stru, p_def->fld_def[i].id, pub_data_stru->in_msg_type,
                                       p_def->fld_def[i].max_len, buf, 0)) return -1;
                    len =p_def->fld_def[i].max_len;
                    disp_len += snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                                         "field[%s]=[%d][%.*s]\n",p_def->fld_def[i].name,len,len,buf);
                }

                return len;
            } else if(p_def->fld_def[i].len_type == 1) {  //1字节可变长长度域
                len= (unsigned char)(buf[0]);
                if(len >p_def->fld_def[i].max_len) {
                    dcs_log(0,0,"<%s>name[%s]超出长度限制,len=%d,max_len=%d",__FUNCTION__,
                            p_def->fld_def[i].name,len,p_def->fld_def[i].max_len);
                    return -1;
                }

                if(p_def->fld_def[i].is_compress) {
                    bcd_to_asc((unsigned char *)tmp_buf,(unsigned char *)buf+1,len,
                               p_def->fld_def[i].is_compress-1);
                    add_pub_field(pub_data_stru,p_def->fld_def[i].id,pub_data_stru->in_msg_type,len*2,tmp_buf,0);
                    tmp_buf[len]=0x00;
                    len =len +1;
                    disp_len += snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                                         "field[%s]=[%d][%s]\n",p_def->fld_def[i].name,len,tmp_buf);
                } else {
                    add_pub_field(pub_data_stru,p_def->fld_def[i].id,pub_data_stru->in_msg_type,len,buf+1,0);
                    len =len+1;
                    disp_len += snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                                         "field[%s]=[%d][%.*s]\n",p_def->fld_def[i].name,len,
                                         len-1,buf+1);
                }
                return len;
            } else if(p_def->fld_def[i].len_type == 2) {  //2字节可变长长度域

                len =(unsigned char)(buf[0])*256+(unsigned char)(buf[1]);
                if(len >p_def->fld_def[i].max_len) {
                    dcs_log(0,0,"<%s>name[%s]超出长度限制,len=%d,max_len=%d",__FUNCTION__,
                            p_def->fld_def[i].name,len,p_def->fld_def[i].max_len);
                    return -1;
                }

                if(p_def->fld_def[i].is_compress) {
                    bcd_to_asc((unsigned char *)tmp_buf,(unsigned char *)(buf+2),len,
                               p_def->fld_def[i].is_compress-1);
                    add_pub_field(pub_data_stru,p_def->fld_def[i].id,pub_data_stru->in_msg_type,len*2,tmp_buf,0);
                    tmp_buf[len]=0x00;
                    len =len +2;
                    disp_len += snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                                         "field[%s]=[%d][%s]\n",p_def->fld_def[i].name,len,tmp_buf);

                } else {
                    add_pub_field(pub_data_stru,p_def->fld_def[i].id,pub_data_stru->in_msg_type,len,buf+2,0);
                    len =len+2;
                    disp_len += snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                                         "field[%s]=[%d][%.*s]\n",p_def->fld_def[i].name,len,len-2,buf+2);
                }
                return len;
            } else {
                dcs_log(0,0,"<%s> can not parase len_type=[%d]",
                        __FUNCTION__,p_def->fld_def[i].len_type);
            }
        }
    }
    dcs_log(0,0,"<%s>can not found field_name[%s]",__FUNCTION__,field_name);
    return -1;
}