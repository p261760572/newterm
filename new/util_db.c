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

int HexToDec(char *hex);

trans_log_handler_def trans_log_services[] = {
    {"SYS_DATE"        , NULL, 8 + 1   , 0},
    {"SYS_TIME"        , NULL, 6 + 1   , 0},
    {"QS_DATE"         , NULL, 8 + 1   , 0},
    {"ACQ_INSTI_CODE"  , NULL, 11 + 1   , 0},
    {"PAY_INSTI_CODE"  , NULL, 11 + 1   , 0},
    {"APP_INSTI_CODE"  , NULL, 11 + 1   , 0},
    {"ACQ_MSG_TYPE"    , NULL, 4 + 1   , 0},
    {"ACQ_TRANS_TYPE"  , NULL, 4 + 1   , 0},
    {"APP_TYPE"        , NULL, 4 + 1   , 0},
    {"PAY_MSG_TYPE"    , NULL, 4 + 1   , 0},
    {"PAY_TRANS_TYPE"  , NULL, 4 + 1   , 0},
    {"APP_MSG_TYPE"    , NULL, 4 + 1   , 0},
    {"APP_TRANS_TYPE"  , NULL, 4 + 1   , 0},
    {"RESP_CD_APP"     , NULL, 6 + 1   , 0},
    {"RESP_CD_PAY"     , NULL, 6 + 1   , 0},
    {"RESP_CD_RCV"     , NULL, 6 + 1   , 0},
    {"PAY_ACCT_NO"     , NULL, 30 + 1  , 0},
    {"CARD_ATTR"       , NULL, 2 + 1   , 0},
    {"ISS_INSTI_CODE"  , NULL, 8 + 1   , 0},
    {"AMOUNT_PAY"      , NULL, 12 + 1  , 0},
    {"AMOUNT_REAL"     , NULL, 12 + 1  , 0},
    {"FEE"             , NULL, 12 + 1  , 0},
    {"ACQ_TRA_NO"      , NULL, 6 + 1   , 0},
    {"PAY_TRA_NO"      , NULL, 6 + 1   , 0},
    {"APP_TRA_NO"      , NULL, 6 + 1   , 0},
    {"ACQ_DATE"        , NULL, 8 + 1   , 0},
    {"ACQ_TIME"        , NULL, 6 + 1   , 0},
    {"PAY_DATE"        , NULL, 8 + 1   , 0},
    {"PAY_TIME"        , NULL, 6 + 1   , 0},
    {"APP_DATE"        , NULL, 8 + 1   , 0},
    {"APP_TIME"        , NULL, 6 + 1   , 0},
    {"ACQ_TERM_ID1"    , NULL, 20 + 1  , 0},
    {"ACQ_TERM_ID2"    , NULL, 20 + 1  , 0},
    {"PAY_TERM_ID1"    , NULL, 20 + 1  , 0},
    {"PAY_TERM_ID2"    , NULL, 20 + 1  , 0},
    {"APP_TERM_ID1"    , NULL, 20 + 1  , 0},
    {"APP_TERM_ID2"    , NULL, 20 + 1  , 0},
    {"ACQ_ADDITION"    , NULL, 512 + 1 , 0},
    {"PAY_ADDITION"    , NULL, 512 + 1 , 0},
    {"APP_ADDITION"    , NULL, 512 + 1 , 0},
    {"SYS_REF_NO"      , NULL, 12 + 1  , 0},
    {"POS_ENTRY_MD_CD" , NULL, 3 + 1   , 0},
    {"POS_COND_CD"     , NULL, 2 + 1   , 0},
    {"RCV_ACCT_NO"     , NULL, 30 + 1  , 0},
    {"TRANS_CURR_CD"   , NULL, 3 + 1   , 0},
    {"RESP_CD_AUTH_ID" , NULL, 6 + 1   , 0},
    {"STEP"            , NULL, 1 + 1   , 0},
    {"VOID_FLAG"       , NULL, 1 + 1   , 0},
    {"PERMIT_VOID"     , NULL, 1 + 1   , 0},
    {"ACQ_CRY_TYPE"    , NULL, 1 + 1   , 0},
    {"ACQ_MAC"         , NULL, 16 + 1   , 0},
    {"MCC"         , NULL, 4 + 1   , 0},
    {"ACQ_PROC_CODE"  , NULL, 6 + 1   , 0},
    {"PAY_PROC_CODE"  , NULL, 6 + 1   , 0},
    {"PAY_MSG_ID"  , NULL, 6 + 1   , 0},

    {"MERCH_INFO"  , NULL, 80 + 1   , 0},
    {NULL              , NULL, 0       , 0}
};

void SetTransLogServices(char *val, int index) {
    if(trans_log_services[index].Str == NULL) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d>SetTransLogServices出错，下标越界！", __FILE__, __LINE__);
        return;
    }
    trans_log_services[index].Val = val;
}

void InitTransLogServices(tl_trans_log_def *pTransLog) {
    int i=0;
    ICS_DEBUG(0);
    SetTransLogServices(pTransLog->sys_date         , i++);
    SetTransLogServices(pTransLog->sys_time         , i++);
    SetTransLogServices(pTransLog->qs_date          , i++);
    SetTransLogServices(pTransLog->acq_insti_code   , i++);
    SetTransLogServices(pTransLog->pay_insti_code   , i++);
    SetTransLogServices(pTransLog->app_insti_code   , i++);
    SetTransLogServices(pTransLog->acq_msg_type     , i++);
    SetTransLogServices(pTransLog->acq_trans_type   , i++);
    SetTransLogServices(pTransLog->app_type         , i++);
    SetTransLogServices(pTransLog->pay_msg_type     , i++);
    SetTransLogServices(pTransLog->pay_trans_type   , i++);
    SetTransLogServices(pTransLog->app_msg_type     , i++);
    SetTransLogServices(pTransLog->app_trans_type   , i++);
    SetTransLogServices(pTransLog->resp_cd_app      , i++);
    SetTransLogServices(pTransLog->resp_cd_pay      , i++);
    SetTransLogServices(pTransLog->resp_cd_rcv      , i++);
    SetTransLogServices(pTransLog->pay_acct_no      , i++);
    SetTransLogServices(pTransLog->card_attr        , i++);
    SetTransLogServices(pTransLog->iss_insti_code   , i++);
    SetTransLogServices(pTransLog->amount_pay       , i++);
    SetTransLogServices(pTransLog->amount_real      , i++);
    SetTransLogServices(pTransLog->fee              , i++);
    SetTransLogServices(pTransLog->acq_tra_no       , i++);
    SetTransLogServices(pTransLog->pay_tra_no       , i++);
    SetTransLogServices(pTransLog->app_tra_no       , i++);
    SetTransLogServices(pTransLog->acq_date         , i++);
    SetTransLogServices(pTransLog->acq_time         , i++);
    SetTransLogServices(pTransLog->pay_date         , i++);
    SetTransLogServices(pTransLog->pay_time         , i++);
    SetTransLogServices(pTransLog->app_date         , i++);
    SetTransLogServices(pTransLog->app_time         , i++);
    SetTransLogServices(pTransLog->acq_term_id1     , i++);
    SetTransLogServices(pTransLog->acq_term_id2     , i++);
    SetTransLogServices(pTransLog->pay_term_id1     , i++);
    SetTransLogServices(pTransLog->pay_term_id2     , i++);
    SetTransLogServices(pTransLog->app_term_id1     , i++);
    SetTransLogServices(pTransLog->app_term_id2     , i++);
    SetTransLogServices(pTransLog->acq_addition     , i++);
    SetTransLogServices(pTransLog->pay_addition     , i++);
    SetTransLogServices(pTransLog->app_addition     , i++);
    SetTransLogServices(pTransLog->sys_ref_no       , i++);
    SetTransLogServices(pTransLog->pos_entry_md_cd  , i++);
    SetTransLogServices(pTransLog->pos_cond_cd      , i++);
    SetTransLogServices(pTransLog->rcv_acct_no      , i++);
    SetTransLogServices(pTransLog->trans_curr_cd    , i++);
    SetTransLogServices(pTransLog->resp_cd_auth_id  , i++);
    SetTransLogServices(pTransLog->step             , i++);
    SetTransLogServices(pTransLog->void_flag        , i++);
    SetTransLogServices(pTransLog->permit_void      , i++);
    SetTransLogServices(pTransLog->acq_cry_type     , i++);
    SetTransLogServices(pTransLog->acq_mac          , i++);
    SetTransLogServices(pTransLog->mcc          , i++);
    SetTransLogServices(pTransLog->acq_proc_code    , i++);
    SetTransLogServices(pTransLog->pay_proc_code    , i++);
    SetTransLogServices(pTransLog->pay_msg_id    , i++);
    SetTransLogServices(pTransLog->merch_info       , i++);
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
    for(i = 0; trans_log_services[i].Str != NULL; i++) {
        if(0 == strcasecmp(fieldName, trans_log_services[i].Str)) {
            ret = SetFieldData(trans_log_services[i].Val, fieldID, fieldVal, trans_log_services[i].Size, trans_log_services[i].Flag);
            if(0 > ret) {
                dcs_log(0, 0, "<FILE:%s,LINE:%d>取%s出错[%s][%s][%s]！", __FILE__, __LINE__, fieldName, fieldID, fieldVal, trans_log_services[i].Val);
                ICS_DEBUG(0);
                return -1;
            } else {
                if(0 == trans_log_services[i].Flag) {
                    if(0 < (d_field_id = get_pub_field_id(DB_MSG_TYPE, fieldName))) {
                        add_pub_field(pub_data_stru, d_field_id, pub_data_stru->route_msg_type,
                                      strlen(fieldVal), fieldVal, 2);
                    }
                }
                return ret;
            }
        }
    }
    ICS_DEBUG(0);
    dcs_log(0, 0, "<FILE:%s,LINE:%d>数据字段名[%s]不正确！", __FILE__, __LINE__, fieldName);
    return -1;
}

int GetTransLog(tl_trans_log_def *transLog, char *fieldName, char *fieldVal, short fieldID, int size) {
    int ret, i;
    InitTransLogServices(transLog);
    for(i = 0; trans_log_services[i].Str != NULL; i++) {
        if(0 == strcasecmp(fieldName, trans_log_services[i].Str)) {
            ret = GetFieldData(trans_log_services[i].Val, fieldID, fieldVal, size, trans_log_services[i].Flag);
            if(0 > ret) {
                dcs_log(0, 0, "<FILE:%s,LINE:%d>取置%s出错[%s][%d][%s]！", __FILE__, __LINE__, fieldName, fieldID, fieldVal, trans_log_services[i].Val);
                ICS_DEBUG(0);
                return -1;
            } else
                return ret;
        }
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
    for(i = 0; i < gl_def_set.num; i++) {
        if(strcmp(gl_def_set.priv_def[i].msg_type, DB_MSG_TYPE) == 0) {
            for(j = 0; j < gl_def_set.priv_def[i].use_num; j++) {
                for(k = 0; trans_log_services[k].Str != NULL; k++) {
                    if(0 == strcasecmp(gl_def_set.priv_def[i].fld_def[j].name, trans_log_services[k].Str)) {
                        if(trans_log_services[k].Flag == 0) {
                            rtrim(trans_log_services[k].Val);
                            if(0>=add_pub_field(pub_data_stru, gl_def_set.priv_def[i].fld_def[j].id, DB_MSG_TYPE,
                                                strlen(trans_log_services[k].Val), trans_log_services[k].Val, 2))
                                ;//dcs_debug(0,0,"<%s> add_pub_field name=[%s] fail! ",__FUNCTION__,trans_log_services[k].Str);
//                          else dcs_debug(0,0,"<%s> name=[%s],value=[%s]",__FUNCTION__,gl_def_set.priv_def[i].fld_def[j].name,trans_log_services[k].Val);
                            /*//                                if ( strcmp("ACQ_INSTI_CODE",trans_log_services[k].Str)==0)
                                                            {
                                                                    char tmp[128];
                                                                    memset(tmp,0,sizeof(tmp));
                                                                    _get_field_data(get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"), pub_data_stru, tmp, 2);
                                                                dcs_debug(0,0,"<%s> add_pub_field acq_insti_code=[%s]",__FUNCTION__,tmp);
                                                            }

                            */
//                        else dcs_debug(0,0,"<%s> add_pub_filed[%d][%s]",__FUNCTION__,gl_def_set.priv_def[i].fld_def[j].id,trans_log_services[k].Val);
                        }

                    }
                }
            }
        }
    }
    /*
        {
            char tmp[128];
            memset(tmp,0,sizeof(tmp));
            _get_field_data(get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"), pub_data_stru, tmp, 2);
        dcs_debug(0,0,"<%s> acq_insti_code=[%s]",__FUNCTION__,tmp);
      }
    */
    return 1;
}
