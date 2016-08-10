#include "base.h"
#include "var.h"
#include "general_util.h"
#include "ibdcs.h"
#include "tpos.h"
//#include  "tmcibtms.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"
#include "db_tools.h"

#include "db.h"

int HexToDec(char *hex);


//add 2016/8/10 16:58:32

#define OFFSETOF(type, field) ((size_t)(&((type *)0)->field))

#define SIZEOF(type, field) sizeof(((type *)0)->field)

#define DEFINE_FIELD(type, field, ctype) {                              \
    #type, #field, OFFSETOF(type, field), SIZEOF(type, field), ctype    \
}

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))


#define CTYPE_STRING 1

trans_log_handler_def trans_log_services[] = {
    DEFINE_FIELD(tl_trans_log_def, sys_date , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, sys_time , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, qs_date , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, acq_insti_code , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, pay_insti_code , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, app_insti_code , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, acq_msg_type , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, acq_trans_type , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, app_type , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, pay_msg_type , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, pay_trans_type , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, app_msg_type , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, app_trans_type , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, resp_cd_app , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, resp_cd_pay , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, resp_cd_rcv , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, pay_acct_no , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, card_attr , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, iss_insti_code , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, amount_pay , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, amount_real , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, fee , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, acq_tra_no , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, pay_tra_no , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, app_tra_no , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, acq_date , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, acq_time , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, pay_date , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, pay_time , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, app_date , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, app_time , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, acq_term_id1 , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, acq_term_id2 , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, pay_term_id1 , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, pay_term_id2 , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, app_term_id1 , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, app_term_id2 , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, acq_addition , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, pay_addition , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, app_addition , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, sys_ref_no , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, pos_entry_md_cd , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, pos_cond_cd , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, rcv_acct_no , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, trans_curr_cd , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, resp_cd_auth_id , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, step , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, void_flag , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, permit_void , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, acq_cry_type , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, acq_mac , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, mcc , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, acq_proc_code , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, pay_proc_code , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, pay_msg_id , CTYPE_STRING),
    DEFINE_FIELD(tl_trans_log_def, merch_info , CTYPE_STRING)
};


size_t trans_log_services_size = ARRAY_SIZE(trans_log_services);

tl_trans_log_def *g_translog;


trans_log_handler_def *find_field_desc(const char *type, const char *field) {
    int i;
    for(i = 0; i < trans_log_services_size; i ++) {
        if(strcasecmp(trans_log_services[i].type, type) == 0 && strcmp(trans_log_services[i].field, field) == 0) {
            return &trans_log_services[i];
        }
    }
    return NULL;
}

char *get_field_string(const char *type, const char *field, void *data) {
    trans_log_handler_def *desc = find_field_desc(type, field);
    if(desc != NULL && desc->ctype == CTYPE_STRING) {
        return ((char *)data) + desc->offset;
    }
    return NULL;
}

int set_field_string(const char *type, const char *field, void *data, char *value) {
    trans_log_handler_def *desc = find_field_desc(type, field);
    if(desc != NULL && desc->ctype == CTYPE_STRING) {
        strcpy_s(((char *)data) + desc->offset, value, desc->size);
        return 0;
    }
    return -1;
}


//add end

void InitTransLogServices(tl_trans_log_def *pTransLog) {
    //memcpy(&g_translog, pTransLog, sizeof(g_translog));
    g_translog = pTransLog;

}

//20140731

//util_db.c
//修复if BUG

//根据根据数PRIV_DEF配置是否为压缩数据 20140910
int AnalyzeAddiData(glob_msg_stru * pub_data_stru, char *buff) {
    int dLen, lLen, fieldLen, fieldID;
    char tmpBuf[4], *p, tFieldID[3 + 1], fldVal[512 + 1], fldBin[512 + 1];
    field_define    *fld_def;
    message_define *priv_def;
    rtrim(buff);
    lLen = 3;
    dLen = strlen(buff);
    priv_def = match_priv_stru(DB_MSG_TYPE,&gl_def_set);
    for(p = buff; p - buff < dLen - lLen - 3;) {
        memcpy(tFieldID, p, 3);
        p += 3;
        tFieldID[3] = 0;
        fieldID = HexToDec(tFieldID);
        if(fieldID < 0)break;
        memcpy(tmpBuf, p, lLen);
        p += lLen;
        tmpBuf[lLen] = 0;
        fieldLen = atoi(tmpBuf);
        if(fieldLen > 512)break;
        if(p - buff > dLen - fieldLen) {
            memset(fldVal, ' ', fieldLen);
            memcpy(fldVal, p, dLen - (p - buff));
            fldVal[fieldLen] = 0;
        } else {
//          memset(fldVal, 0, sizeof(fldVal));
            memcpy(fldVal, p, fieldLen);
            fldVal[fieldLen] = 0;
        }
        fld_def = get_priv_field_def_for_id(fieldID, priv_def);
        if(fld_def == NULL) {
            dcs_log(0, 0, "<FILE:%s,LINE:%d>get_priv_field_def_for_id[%d]失败！", __FILE__, __LINE__, fieldID);
            return -1;
        }
        if(fld_def->is_compress) {
            asc_to_bcd((unsigned char *)fldBin, (unsigned char *)fldVal, fieldLen, 0);
            add_pub_field(pub_data_stru, fieldID, DB_MSG_TYPE, fieldLen/2, fldBin, 2);
            dcs_debug(0,0,"<%s> add_pub_field[%d][%s] is_compress",__FUNCTION__,fieldID,fldVal);
        } else {
            add_pub_field(pub_data_stru, fieldID, DB_MSG_TYPE, fieldLen, fldVal, 2);
            dcs_debug(0,0,"<%s> add_pub_field[%d][%s]",__FUNCTION__,fieldID,fldVal);
        }
        p += fieldLen;
    }
    return 1;
}


int HexToDec(char *hex) {
    int j,sum;
    sum=0;
    for(; *hex; hex++) {
        if(*hex <= 'f' && *hex >= 'a')
            j=(int)(*hex) - 87;
        else if(*hex <= 'F' && *hex >= 'A')
            j=(int)(*hex) - 55;
        else if(*hex <= '9' && *hex >= '0')
            j=(int)(*hex) - 48;
        else
            j = 0;
        sum = sum * 16 + j;
    }
    return sum;
}



int GetFieldData(char *buff, int iFieldID, char *fieldBuf, int size, int flag) {
    int dLen, lLen, fieldLen;
    char tmpBuf[4], *p, tFieldID[3 + 1], fieldID[3 + 1];
    size--;
    if(flag == 0) {
        strcpy_safe(fieldBuf, buff, size);
        rtrim(fieldBuf);
        return strlen(fieldBuf);
    }
    lLen = 3;
    sprintf(fieldID, "%03X", iFieldID);
    dLen = strlen(buff);
    for(p = buff; p - buff < dLen - lLen - 3 - 1;) {
        memcpy(tFieldID, p, 3);
        p += 3;
        tFieldID[3] = 0;
        memcpy(tmpBuf, p, lLen);
        p += lLen;
        tmpBuf[lLen] = 0;
        fieldLen = atoi(tmpBuf);
        if(p - buff > dLen - fieldLen) return -1;
        if(0 == memcmp(fieldID, tFieldID, 3)) {
            if(fieldLen > size) return -1;
            memcpy(fieldBuf, p, fieldLen);
            return fieldLen;
        }
        p += fieldLen;
    }
    return 0;
}


int SetFieldData(char *buff, int iFieldID, char *fieldBuf, int size, int flag) {
    int dLen, lLen, fieldLen;
    char tmpBuf[10], *p, outBuff[1024], *pOut, tFieldID[3 + 1], fieldID[3 + 1];
    size--;
    if(flag == 0) {
        strcpy_safe(buff, fieldBuf, size);
        return 1;
    }
    sprintf(fieldID, "%03X", iFieldID);
    lLen = 3;
    dLen = strlen(buff);
//  memset(outBuff, 0, sizeof(outBuff));
    for(p = buff, pOut = outBuff, flag = 1; p - buff < dLen - lLen - 3;) {
        memcpy(tFieldID, p, 3);
        p += 3;
        tFieldID[3] = 0;
        if(tFieldID[0] == ' ')break;
        memcpy(tmpBuf, p, lLen);
        p += lLen;
        tmpBuf[lLen] = 0;
        fieldLen = atoi(tmpBuf);
        if(p - buff > dLen - fieldLen) break;
        sprintf(tmpBuf, "%%0%dd", lLen);
        memcpy(pOut, tFieldID, 3);
        pOut += 3;
        if(0 == memcmp(fieldID, tFieldID, 3)) {
            if(strlen(fieldBuf) > size - (pOut + lLen - outBuff))
                return -1;
            sprintf(pOut, tmpBuf, strlen(fieldBuf));
            pOut += lLen;
            strcpy(pOut, fieldBuf);
            pOut += strlen(fieldBuf);
            flag = 0;
        } else {
            sprintf(pOut, tmpBuf, fieldLen);
            pOut += lLen;
            if(fieldLen > size - (pOut - outBuff))
                return -1;
            memcpy(pOut, p, fieldLen);
            pOut += fieldLen;
        }
        p += fieldLen;
    }
    *pOut=0x00;
    if(flag) {
        sprintf(tmpBuf, "%%0%dd", lLen);
        memcpy(pOut, fieldID, 3);
        pOut += 3;
        if(strlen(fieldBuf) > size - (pOut + lLen - outBuff))
            return -1;
        sprintf(pOut, tmpBuf, strlen(fieldBuf));
        pOut += lLen;
        strcpy(pOut, fieldBuf);
        pOut += strlen(fieldBuf);
    }
    strcpy_safe(buff, outBuff, pOut - outBuff);
//  memcpy(buff, outBuff, MIN(pOut - outBuff, size));
    return 1;
}

int SetTransLog(tl_trans_log_def *transLog, char *fieldName, char *fieldVal, short fieldID, glob_msg_stru * pub_data_stru) {
    int ret, i;
    short d_field_id;

    InitTransLogServices(transLog);

    if(set_field_string("tl_trans_log_def", fieldName, transLog, fieldVal) == 0) {
        if((d_field_id = get_pub_field_id(DB_MSG_TYPE, fieldName)) > 0) {
            add_pub_field(pub_data_stru, d_field_id, DB_MSG_TYPE, strlen(fieldVal), fieldVal, 2);
        }
        return 0;
    }

    ICS_DEBUG(0);
    dcs_log(0, 0, "<FILE:%s,LINE:%d>数据字段名[%s]不正确！", __FILE__, __LINE__, fieldName);
    return -1;
}

int GetTransLog(tl_trans_log_def *transLog, char *fieldName, char *fieldVal, short fieldID, int size) {
    int ret, i;
    char *value;
    InitTransLogServices(transLog);
    value = get_field_string("tl_trans_log_def", fieldName, transLog);
    if(value != NULL) {
        strcpy_s(fieldVal, value, size);
        return 0;
    }

    dcs_log(0, 0, "<FILE:%s,LINE:%d>数据字段名[%s]不正确！", __FILE__, __LINE__, fieldName);
    ICS_DEBUG(0);
    return -1;
}

int db_to_pub_daba(glob_msg_stru * pub_data_stru, tl_trans_log_def *pTransLog) {
    int i, j, k;
//  char fldVal[256 + 1];
    InitTransLogServices(pTransLog);
//  dcs_debug(0,0,"<%s> get_pub_field_id pay",__FUNCTION__);
    AnalyzeAddiData(pub_data_stru, pTransLog->pay_addition);
//  dcs_debug(0,0,"<%s> get_pub_field_id app",__FUNCTION__);
    AnalyzeAddiData(pub_data_stru, pTransLog->app_addition);
//  dcs_debug(0,0,"<%s> get_pub_field_id acq",__FUNCTION__);
    AnalyzeAddiData(pub_data_stru, pTransLog->acq_addition);

    message_define *db_def = match_priv_stru(DB_MSG_TYPE, &gl_def_set);

    for(j = 0; j < db_def->use_num; j++) {
        char *value = get_field_string("tl_trans_log_def", db_def->fld_def[j].name, pTransLog);
        if(value != NULL) {
            rtrim(value);
            add_pub_field(pub_data_stru, db_def->fld_def[j].id, DB_MSG_TYPE, strlen(value), value, 2);
        }
    }
    return 1;
}
