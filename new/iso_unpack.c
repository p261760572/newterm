#include "base.h"
#include "general_util.h"
#include <stdio.h>
#include <string.h>
#include "ibdcs.h"
#include "tools.h"
#include "var.h"
static int disp_len;
static char disp_buf[8192];

int iso_priv_to_pub(char * field_name ,const message_define *p_def,
                    const char *buf,int src_len,glob_msg_stru *pub_data_stru, int len_type);
int iso_unpack(char *src_buf,int src_len,glob_msg_stru *pub_data_stru) {
    int head_flag,headlen,msgid_flag,bitmap_flag,len_type;
    int len,j,n,bitnum,offset,i;
    unsigned char bitmap[64],bitmask;
    char *msg_type;
    message_define *priv_def;
    char tmp[40];
    priv_def = pub_data_stru->in_priv_def;
    msg_type = pub_data_stru->in_msg_type;
    if(priv_def == NULL) return -1;
    if(src_buf == NULL || pub_data_stru==NULL) return -1;
    if(src_len <10) return -1;
    if(0>get_iso_para(msg_type,&head_flag,&headlen,&msgid_flag,&bitmap_flag,
                      &len_type)) {
        head_flag=0;
        headlen=0;
        msgid_flag=0;
        bitmap_flag=1;
        len_type=0;
    }
    offset=0;
    disp_len=0;
    if(head_flag) {
        len=iso_priv_to_pub("TPDU", priv_def, src_buf+offset,src_len-offset,
                            pub_data_stru, len_type);
        if(len <=0) {
            dcs_log(0,0,"<%s>can not get TPDU",__FUNCTION__);
            return -1;
        }
        if(offset + len > src_len) {
            dcs_log(0, 0, "<%s>数据长度不够offset[%d] + [%s]_len[%d] > src_len[%d]",
                    __FUNCTION__, offset, "TPDU", len, src_len);
        }
        offset += len;
    }
    len = iso_priv_to_pub("MSG_ID", priv_def, src_buf+offset, src_len-offset,
                          pub_data_stru, len_type);
    if(len <=0) {
        dcs_log(0,0,"<%s>can not get MSG_ID",__FUNCTION__);
        return -1;
    }
    if(offset + len > src_len) {
        dcs_log(0, 0, "<%s>数据长度不够offset[%d] + [%s]_len[%d] > src_len[%d]",
                __FUNCTION__, offset, "MSG_ID", len, src_len);
        return -1;
    }
    offset += len;
    if(!bitmap_flag) {
        asc_to_bcd((unsigned char *)bitmap, (unsigned char *)src_buf + offset,
                   32, 0);
        len = 2;
    } else {
        memcpy(bitmap, src_buf + offset, 16);
        len = 1;
    }
    if(((unsigned char)src_buf[offset]) & 0x80)
        bitnum = 16 ;
    else
        bitnum = 8 ;
    len = bitnum * len;
    if(offset + len > src_len) {
        dcs_log(0, 0, "<%s>数据长度不够offset[%d] + [%s]_len[%d] > src_len[%d]",
                __FUNCTION__,offset, "BITMAP", len, src_len);
        return -1;
    }
    offset += len;
    for(i = 0 ; i < bitnum ; i ++) {
        bitmask = 0x80 ;
        for(j = 0 ; j < 8 ; j ++ , bitmask >>= 1) {
            if(i == 0 && j == 0)continue ;
            if((bitmap [ i ] & bitmask) == 0) continue ;
            n = (i << 3) + j ;
            sprintf(tmp, "%d", n+1);
            len=iso_priv_to_pub(tmp, priv_def,src_buf+offset,src_len-offset,
                                pub_data_stru,len_type);
            if(len <= 0) {
                dcs_log(0,0,"<%s>can not get field[%s]",__FUNCTION__,tmp);
                return -1;
            }
            if(offset + len > src_len) {
                dcs_log(0, 0, "<%s>数据长度不够offset[%d] + [%s]_len[%d] > src_len[%d]",
                        __FUNCTION__, offset, tmp, len, src_len);
                return -1;
            }
            offset += len;
        }
    }
    dcs_debug(0,0,"<%s>iso 解包\n%s",__FUNCTION__,disp_buf);
    return 1;
}

int iso_priv_to_pub(char * field_name ,const message_define *p_def,
                    const char *buf,int src_len,glob_msg_stru *pub_data_stru, int len_type) {
    int i,len, len_len;
    char tmp_buf[1024],tmp[30];
    ICS_DEBUG(0);
    for(i=0; i<p_def->use_num; i++) {
        if(strcmp(field_name,p_def->fld_def[i].name)==0) {
            len_len = p_def->fld_def[i].len_type;
            if(p_def->fld_def[i].len_type == 0) { //固定长度
                len = p_def->fld_def[i].max_len;
            } else if(p_def->fld_def[i].len_type == 1) {  //2字节可变长长度域
                if(len_type)
                    len=((unsigned char)buf[0]>>4)*10 +((unsigned char)buf[0]&0x0f);
                else {
                    memset(tmp,0,sizeof(tmp));
                    memcpy(tmp,buf,2);
                    len=atoi(tmp);
                    len_len++;
                }
                if(len >p_def->fld_def[i].max_len) {
                    dcs_log(0,0,"<%s> The length of the field[%s]  exceeds the limit ! len[%d] max[%d]",
                            __FUNCTION__,p_def->fld_def[i].name,len,p_def->fld_def[i].max_len);
                    return -1;
                }

            } else if(p_def->fld_def[i].len_type == 2) { //3字节可变长长度域
                if(len_type)
                    len=(unsigned char)buf[0]*100+((unsigned char)buf[1]>>4)*10 +((unsigned char)buf[1]&0x0f);
                else {
//                  memset(tmp,0,sizeof(tmp));
                    memcpy(tmp,buf,3);
                    tmp[3]=0x00;
                    len=atoi(tmp);
                    len_len++;
                }

                if(len >p_def->fld_def[i].max_len) {
                    dcs_log(0,0,"<%s> The length of the field[%s]  exceeds the limit ! len[%d] max[%d]",
                            __FUNCTION__,p_def->fld_def[i].name,len,p_def->fld_def[i].max_len);
                    return -1;
                }
            } else {
                dcs_log(0,0,"<%s> can not parase len_type=[%d]",__FUNCTION__,
                        p_def->fld_def[i].len_type);
                return -1;
            }

            if(p_def->fld_def[i].is_compress) {
                if((len+1)/2+len_len >src_len) {
                    dcs_log(0,0,"<%s> src_len < data len [%s]",__FUNCTION__,
                            p_def->fld_def[i].name);
                    return -1;
                }
                bcd_to_asc((unsigned char *)tmp_buf , (unsigned char *)buf+ len_len,
                           len, p_def->fld_def[i].is_compress-1);
                if(0 > add_pub_field(pub_data_stru, p_def->fld_def[i].id,
                                     pub_data_stru->in_msg_type,len, tmp_buf, 0))
                    return -1;
                tmp_buf[len]=0x00;
                len = (int)((len + 1) / 2);
                disp_len += snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                                     "field[%s]=[%d][%s]\n",p_def->fld_def[i].name,len,tmp_buf);
            } else {
                if(len+len_len >src_len) {
                    dcs_log(0,0,"<%s> src_len < data len [%s]",__FUNCTION__,
                            p_def->fld_def[i].name);
                    return -1;
                }
                if(0>add_pub_field(pub_data_stru,p_def->fld_def[i].id,
                                   pub_data_stru->in_msg_type,len, buf + len_len, 0))
                    return -1;
                disp_len += snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                                     "field[%s]=[%d][%.*s]\n",p_def->fld_def[i].name,
                                     len,len,buf+len_len);
            }
            return len + len_len;
        }
    }
    dcs_log(0, 0, "<%s>can not found field_name[%s]",__FUNCTION__, field_name);
    return -1;
}
