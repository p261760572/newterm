#include "base.h"
#include "general_util.h"
#include <stdio.h>
#include <string.h>
#include "var.h"
#include "ibdcs.h"
#include "tools.h"
#include "db_qfunc.h"

int iso_pack(glob_msg_stru *pub_data_stru,char *buf,int size) {
    int len,offset,offset_bit,s,bitnum,i,l,len2, macLen = 0, macStart = 0;
    int head_flag,headlen,msgid_flag,bitmap_flag,len_type;
    unsigned char bitmap[16],bitmask;
    char tmp[1024],data[2048],tmp1[1024];
    char *msg_type = NULL;
    message_define *priv_def = NULL;
    int disp_len;
    char disp_buf[8192];
    field_set *p_set;
    field_define    *p_fld_def;

    msg_type= pub_data_stru->route_msg_type;
    priv_def = pub_data_stru->route_priv_def;
    if(priv_def == NULL) return -1;
    if(buf == NULL || pub_data_stru==NULL) return -1;
    disp_len=0;
    if(0>get_iso_para(msg_type,&head_flag,&headlen,&msgid_flag,
                      &bitmap_flag,&len_type)) {
        head_flag=0;
        headlen=0;
        msgid_flag=0;
        bitmap_flag=1;
        len_type=0;
    }
    add_pub_field(pub_data_stru, FIELD_RECODE,msg_type,
                  strlen(pub_data_stru->center_result_code),
                  pub_data_stru->center_result_code, 1);
    offset=0;
    p_set=&pub_data_stru->route_set;
    if(head_flag) {
        for(i = 0; i < p_set->num; i++)if(p_set->field.field_id[i] == FIELD_TPDU) break;
        if(i >= p_set->num) {
            dcs_log(0, 0, "<%s>ISO8583报文类型[%s]要求包头," \
                    "但TRANS_SET未设置报文头取值方式", __FUNCTION__, msg_type);
            return -1;
        }
        p_fld_def = get_priv_field_def_for_id(FIELD_TPDU, priv_def);
        if(p_fld_def == NULL) {
            dcs_log(0, 0, "<%s>取数据域定义出错p_set->field_id[%d]=[%d]",
                    __FUNCTION__, i, p_set->field.field_id[i]);
            return -1;
        }
        memset(tmp, 0, sizeof(tmp));
        if(0>=(len = _get_field_data_safe(pub_data_stru,FIELD_TPDU, msg_type,
                                          tmp, p_set->field.from[i],sizeof(tmp)-1))) {
            if(pub_data_stru->route_num==0)
                len=get_field_data_safe(pub_data_stru,FIELD_TPDU,msg_type,
                                        tmp,sizeof(tmp)-1);
            else
                len =-1;
        }

        if(0 > len) {
            dcs_log(0, 0, "<%s>ISO8583报文类型[%s]要求包头，但未设置报文头",
                    __FUNCTION__, msg_type);
            return -1;
        }
        tmp[len]=0x00;
        disp_len += snprintf(disp_buf + disp_len,sizeof(disp_buf)-disp_len,
                             "field[%s]=[%d][%s]\n","HEAD",p_fld_def->max_len,tmp);

        if(p_fld_def->is_compress) {
            asc_to_bcd((unsigned char *)tmp1, (unsigned char *)tmp ,len,0);
            if(offset + (p_fld_def->max_len+1)/2 > size)
                return -1;
            memcpy(buf+offset,tmp1,(p_fld_def->max_len+1)/2);
            offset = offset + (p_fld_def->max_len+1)/2;
            if(buf[0]==0x60) {
                memcpy(tmp,buf+1,2);
                memcpy(buf+1,buf+3,2);
                memcpy(buf+3,tmp,2);
            }
            dcs_debug(buf,offset,"<%s>add head msg,len=%d",__FUNCTION__,offset);
            macStart = offset;
        } else {
            if(offset + p_fld_def->max_len > size)
                return -1;
            memcpy(buf+offset, tmp ,p_fld_def->max_len);
            offset = offset + p_fld_def->max_len;
            if(buf[0]==0x60) {
                memcpy(tmp,buf+1,2);
                memcpy(buf+1,buf+3,2);
                memcpy(buf+3,tmp,2);
            }
            macStart = offset;
        }
    }
    for(i = 0; i < p_set->num; i++)
        if(p_set->field.field_id[i] == FIELD_MSGID)
            break;
    if(i >= p_set->num) {
        dcs_log(0, 0, "<%s>ISO8583报文类型[%s]要求MSGID,"
                "但TRANS_SET未设置MSG_ID取值方式", __FUNCTION__, msg_type);
        return -1;
    }

    len= _get_field_data_safe(pub_data_stru,FIELD_MSGID, msg_type, tmp,
                              p_set->field.from[i],sizeof(tmp));
    if(0> len) {
        dcs_log(0, 0, "<%s>ISO8583报文未设置MSG_ID,from=%d",
                __FUNCTION__,p_set->field.from[i]);
        return -1;
    }
    tmp[len]=0x00;
    p_fld_def = get_priv_field_def_for_id(FIELD_MSGID, priv_def);
    if(p_fld_def == NULL) {
        dcs_log(0, 0, "<%s>取数据域定义出错p_set->field_id[%d]=[%d]",
                __FUNCTION__, i, p_set->field.field_id[i]);
        return -1;
    }
    if(msgid_flag) {
        if(offset + (p_fld_def->max_len+1)/2 > size)
            return -1;
        disp_len += snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                             "field[%s]=[%d][%s]\n","MSG_ID",p_fld_def->max_len,tmp);

        asc_to_bcd((unsigned char *)tmp1, (unsigned char *)tmp,
                   p_fld_def->max_len, 0);
        memcpy(buf + offset, tmp1, (p_fld_def->max_len+1)/2);
        offset += (p_fld_def->max_len+1)/2;
    } else {
        if(offset + p_fld_def->max_len > size)
            return -1;
        memcpy(buf + offset, tmp, p_fld_def->max_len);
        offset += p_fld_def->max_len;
        disp_len += snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                             "field[%s]=[%d][%s]\n","MSG_ID",len,tmp);

    }
    offset_bit=offset;
    memset(bitmap, 0, sizeof(bitmap));
    bitnum =8;
    bitmask = 0x80;
    for(i=0, len2 = 0; i<p_set->num; i++) {

        if(p_set->field.field_id[i] == FIELD_MSGID ||
           p_set->field.field_id[i] == FIELD_TPDU)
            continue;

        if(pub_data_stru->route_num==0) {
            if(0>=(len = _get_field_data_safe(pub_data_stru,p_set->field.field_id[i],
                                              msg_type, tmp,
                                              p_set->field.from[i],sizeof(tmp))))
                len=get_field_data_safe(pub_data_stru,p_set->field.field_id[i],
                                        msg_type,tmp,sizeof(tmp));
        } else
            len = _get_field_data_safe(pub_data_stru,p_set->field.field_id[i],
                                       msg_type,tmp, p_set->field.from[i],sizeof(tmp));
        if(0 >= len && p_set->field.field_id[i] != FIELD_MAC) {
            dcs_debug(0,0,"<%s> field_id[%d] no data disp_len=[%d]",__FUNCTION__,
                      p_set->field.field_id[i],disp_len);
            dcs_log(0, 0, "<%s:%d>field_id=%d,msg_type=%s,from=%x",__func__,__LINE__,
                    p_set->field.field_id[i],msg_type,p_set->field.from[i]);
            continue;
        }
        tmp[len]=0x00;
        p_fld_def = get_priv_field_def_for_id(p_set->field.field_id[i], priv_def);
        if(p_fld_def == NULL) {
            dcs_log(0, 0, "<%s>取数据域定义出错p_set->field_id[%d]=[%d]",
                    __FUNCTION__, i, p_set->field.field_id[i]);
            return -1;
        }
        if(p_set->field.field_id[i] == FIELD_MAC) {
            if(p_fld_def->is_compress)  macLen = (p_fld_def->max_len+1)/2;
            else macLen = p_fld_def->max_len;
            tmp[0]=0x00;
        }
        s = atoi(p_fld_def->name);
        if(s<=0) {
            dcs_log(0, 0, "<%s>取数据域定义出错p_fld_def->name=[%s]", __FUNCTION__,
                    p_fld_def->name);
            return -1;
        }
        if(s>64) bitnum=16;
        l = (int)((s - 1) / 8);
        bitmap[l] = bitmap[l]| (bitmask>>((s - 1)%8));
        if(p_fld_def->len_type==0) { //固定长度
            len = p_fld_def->max_len;
        } else if(p_fld_def->len_type==1) {
            if(len > 99 || len > p_fld_def->max_len) {
                dcs_log(tmp, len, "<%s>ISO8583两位长数据域[%d]数据长度%d>%d[99]",
                        __FUNCTION__, s, len, p_fld_def->max_len);
                return -1;
            }
            if(len_type) {
                dcs_debug(0,0,"<%s> len=%d var1 %02x field name=%s",__FUNCTION__,len,
                          (unsigned char)((len/10) << 4) + len % 10,p_fld_def->name);
                data[len2++]= ((unsigned char)(len/10) << 4) + len % 10;
            } else {
                sprintf(data + len2, "%02d", len);
                len2 += 2;
            }
        } else {
            if(len > 999 || len > p_fld_def->max_len) {
                dcs_log(tmp, len, "<%s>ISO8583三位长数据域[%d]数据最大长度%d>%d[999]",
                        __FUNCTION__, s, len, p_fld_def->max_len);
                return -1;
            }
            if(len_type) {
                data[len2++]= (unsigned char)(len /100);
                data[len2++]= (unsigned char)((len % 100)/10 << 4) + len % 10;
            } else {
                sprintf(data + len2, "%03d", len);
                len2 += 3;
            }
        }
        if(p_fld_def->is_compress) {
            asc_to_bcd((unsigned char *)tmp1, (unsigned char *)tmp, len, p_fld_def->is_compress-1);
            memcpy(data+len2,tmp1,(len+1)/2);
            len2=len2+(len+1)/2;

            disp_len += snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                                 "field[%s]=[%d][%s]\n",p_fld_def->name,(len+1)/2,tmp);
        } else {
            memcpy(data+len2, tmp, len);
            len2 = len2 + len;
            disp_len += snprintf(disp_buf+disp_len,sizeof(disp_buf)-disp_len,
                                 "field[%s]=[%d][%s]\n",p_fld_def->name,len,tmp);
        }
    }
    if(bitnum == 16) bitmap[0] |= 0x80;
    if(bitmap_flag) {
        memcpy(buf + offset, bitmap, bitnum);
        offset += bitnum;
    } else {
        bcd_to_asc((unsigned char *)buf + offset, (unsigned char *)bitmap,
                   bitnum * 2, 0);
        offset += bitnum * 2;
    }

    memcpy(buf + offset, data, len2);
    offset += len2;
    if(macLen > 0) {
        if(0 > mac_calc_handle(1, pub_data_stru, tmp, buf + macStart,
                               offset - macLen - macStart)) {
            dcs_log(0, 0, "<%s>计算MAC出错",__FUNCTION__);
            return -1;
        }
        memcpy(buf + offset - macLen, tmp, macLen);
    }
    if(head_flag)
        head_proc((char *)msg_type,buf,macStart,offset);
    dcs_debug(0,0,"<%s>iso 打包\n%s",__FUNCTION__,disp_buf);
    return offset;
}

