#include "base.h"
#include "var.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"
#include "db_tools.h"
#include "util_func.h"
#include "general_util.h"
#include "db_qfunc.h"
int timeout_proc(glob_msg_stru *pub_data_stru);

int app_timeout_proc(char *src_buf, int src_len ,char *desc_buf, int size,int *to_fid) {
    glob_msg_stru pub_data_stru;
    message_define *priv_def;
    int ret;
//  ICS_DEBUG(0);
    init_pub_data_stru(&pub_data_stru);
    pub_data_stru.src_buffer=src_buf;
    pub_data_stru.src_len=src_len;
    if(memcmp(src_buf, "0001", 4) == 0) { //超时处理
        memcpy(&pub_data_stru.timeout_table, src_buf + 4, sizeof(timeout_stru));
        ret = timeout_proc(&pub_data_stru);
        if(0 == ret)
            return -1;
        if(0 > ret) {
            dcs_log(0,0,"<timeout_proc> timeout_proc error! ");
            return -1;
        }
    }
    if(pub_data_stru.use_timeout) {    //判断是否插超时表
//暂时用1

        if(0>insert_timeout_table(&pub_data_stru,1)) {
            return -1;
        }
    }
    *to_fid=fold_locate_folder(pub_data_stru.route_fold_name);
    if(*to_fid <0) {
        dcs_log(0,0,"<timeout_proc> fold_locate_folder error! route_fold_name=[%s]",pub_data_stru.route_fold_name);
        return -1;
    }
    priv_def = match_priv_stru(pub_data_stru.route_msg_type,&gl_def_set); //根据进入的报文类型找到对应的解析数据字典
    if(priv_def == NULL) {
        dcs_log(0,0,"<timeout_proc> match_priv_stru error! route_msg_type=[%s]",pub_data_stru.route_msg_type);
        return -1;
    }
    if(0>get_route_trans_set(&pub_data_stru)) {
        dcs_log(0,0,"<timeout_proc> get_route_trans_set error! ");
        return -1;
    }
    pub_data_stru.route_priv_def=priv_def;
    dcs_debug(0, 0, "begin pack_msg");
    return pack_msg(&pub_data_stru,desc_buf,size);
}
int _update_db_voidflag(char *key,char flag) {
    char msgkey[256],*p,tmp[64];
    tl_trans_log_def TransLog;

    memset(&TransLog,0,sizeof(tl_trans_log_def));
    snprintf(msgkey,sizeof(msgkey),key);
    rtrim(msgkey);
    dcs_debug(0,0,"<%s> msgkey=[%s]",__FUNCTION__,msgkey);
    p=my_split(msgkey, ',',tmp,sizeof(tmp));
    if(p == NULL) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d>解析first_key[%s]失败！", __FILE__, __LINE__,msgkey);
        return -1;
    }
    //消息类型
    p=my_split(p, ',',tmp,sizeof(tmp));
    if(p == NULL) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d>解析first_key[%s]失败！", __FILE__, __LINE__,msgkey);
        return -1;
    }
    snprintf(TransLog.sys_date,sizeof(TransLog.sys_date),"%s", tmp);
    p=my_split(p, ',',tmp,sizeof(tmp));
    if(p == NULL) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d>解析first_key[%s]失败！", __FILE__, __LINE__, msgkey);
        return -1;
    }
    snprintf(TransLog.acq_insti_code,sizeof(TransLog.acq_insti_code),"%s", tmp);
    p=my_split(p, ',',tmp,sizeof(tmp));
    if(p == NULL) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d>解析first_key[%s]失败！", __FILE__, __LINE__, msgkey);
        return -1;
    }
    snprintf(TransLog.acq_tra_no,sizeof(TransLog.acq_tra_no),"%s", tmp);
    p=my_split(p, ',',tmp,sizeof(tmp));
    if(p == NULL) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d>解析first_key[%s]失败！", __FILE__, __LINE__, msgkey);
        return -1;
    }
    snprintf(TransLog.acq_date,sizeof(TransLog.acq_date),"%s", tmp);
    p=my_split(p, ',',tmp,sizeof(tmp));
    if(p == NULL) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d>解析first_key[%s]失败！", __FILE__, __LINE__, msgkey);
        return -1;
    }
    snprintf(TransLog.acq_term_id1,sizeof(TransLog.acq_term_id1),"%s", tmp);
    p=my_split(p, ',',tmp,sizeof(tmp));
    if(p == NULL) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d>解析first_key[%s]失败！", __FILE__, __LINE__, msgkey);
        return -1;
    }
    snprintf(TransLog.acq_term_id2,sizeof(TransLog.acq_term_id2),"%s", tmp);
    return update_db_voidflag(&TransLog,'1',NULL);

}
int timeout_proc(glob_msg_stru *pub_data_stru) {
    int saveflag = 0;
    char c=0x00;
    ICS_DEBUG(0);
//  dcs_log(0, 0, "<FILE:%s,LINE:%d>begin flag=%d", __FILE__, __LINE__,pub_data_stru->timeout_table.flag[0]);
    if(pub_data_stru->timeout_table.flag[0] == '0') return 0; //不处理
//  dcs_log(0, 0, "<FILE:%s,LINE:%d>", __FILE__, __LINE__);
    if(0>load_db_trans_info(pub_data_stru))//获取原始交易数据
        return -1;
    c=pub_data_stru->timeout_table.flag[0];
    
    switch(pub_data_stru->timeout_table.flag[0]) {
        case '1'://冲正并应答终端
            strcpy(pub_data_stru->center_result_code, CODE_TIME_OUT);
            pub_data_stru->timeout_table.flag[0]++;
            insert_timeout_table(pub_data_stru, 2); 
            _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"),
                                 DB_MSG_TYPE,pub_data_stru->route_insti_code, 2,9);
            _get_field_data_safe(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, "ACQ_TRANS_TYPE"),
                                 DB_MSG_TYPE, pub_data_stru->route_trans_type, 2,5);
            saveflag = 1;
            break;
        case '3'://查询查复，并应答给终端
            strcpy(pub_data_stru->center_result_code, CODE_INQUIRY);
            pub_data_stru->timeout_table.flag[0]++;
            insert_timeout_table(pub_data_stru, 2);
            _get_field_data_safe(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"),
                                 DB_MSG_TYPE,pub_data_stru->route_insti_code, 2,9);
            _get_field_data_safe(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, "ACQ_TRANS_TYPE"),
                                 DB_MSG_TYPE,pub_data_stru->route_trans_type, 2,5);
            saveflag = 1;
            break;
        case '2'://冲正处理
            pub_data_stru->use_timeout = 1;
            GetTimeoutTrans(pub_data_stru);
            strcpy(pub_data_stru->timeout_table.trans_type, pub_data_stru->route_trans_type);
            break;
        case '4'://查询查复
            pub_data_stru->use_timeout = 1;
            GetTimeoutTrans(pub_data_stru);
            strcpy(pub_data_stru->timeout_table.trans_type, pub_data_stru->route_trans_type);
            break;
        case '5'://直接应答给终端
            strcpy(pub_data_stru->center_result_code, CODE_TIME_OUT);
            _get_field_data_safe(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"),
                                 DB_MSG_TYPE,pub_data_stru->route_insti_code, 2,9);
            _get_field_data_safe(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, "ACQ_TRANS_TYPE"),
                                 DB_MSG_TYPE, pub_data_stru->route_trans_type, 2,5);
            saveflag = 1;
            break;
        case '6'://交易结果通知
            pub_data_stru->use_timeout = 1;

            memcpy(pub_data_stru->route_insti_code,pub_data_stru->timeout_table.remark,8);
            memcpy(pub_data_stru->route_trans_type,pub_data_stru->timeout_table.trans_type,4);
            dcs_debug(0,0,"<%s>交易结果通知inist=[%s],trans_type=[%s]",__FUNCTION__,pub_data_stru->route_insti_code,pub_data_stru->route_trans_type);
            break;
        default:
            dcs_log(0, 0,  "<FILE:%s,LINE:%d>timeout_table.flag[%s]定义出错", __FILE__, __LINE__, pub_data_stru->timeout_table.flag);
            return -1;
            break;

    }
    if(0 > timeout_handle(pub_data_stru)) return -1;
    if(0>get_route_insti_info(pub_data_stru)) { //获取路由机构信息
        return -1;
    }
    dcs_debug(0,0,"<%s> route insti=[%s]",__FUNCTION__,pub_data_stru->route_insti_code);
    if(0>conver_data(pub_data_stru, 2)) { //数据格式转换
        dcs_log(0, 0, "<FILE:%s,LINE:%d>域转换失败！", __FILE__, __LINE__, pub_data_stru->timeout_table.flag);
        return -1;
    }
    if(saveflag) {
        db_update(pub_data_stru, 0);
    }
    if(c =='1') _update_db_voidflag(pub_data_stru->timeout_table.first_key,'1');
    pub_data_stru->route_num=1;
    return 1;
}
