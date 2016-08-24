#include "base.h"
#include "general_util.h"
#include<stdio.h>
#include<string.h>
#include "ibdcs.h"
#include "tools.h"
#include "db_tools.h"
#include "db_qfunc.h"
#include "var.h"


int response_msg_proc(glob_msg_stru *pub_data_stru) {
    int i, checkFlag;
    char acq_insti_code[11 + 1];
    int flag;
    ICS_DEBUG(0);
    memset(acq_insti_code, 0, sizeof(acq_insti_code));
 //   sleep(20);
    if(0 > delete_timeout_table(pub_data_stru)) { //删除超时表
        return -1;
    }

    if(0>load_db_trans_info(pub_data_stru)) { //获取原始交易数据
        return -1;
    }

    i=confirm_trans_type(pub_data_stru);
    if(0>i) { //确定交易类型
        dcs_log(0,0,"<respose_msg_proc>can not confirm trans type!");
        return -1;
    } else if(i == 0) { //中心直接完成交易处理(含冲正应答类交易)
        dcs_debug(0,0,"<respose_msg_proc>  end_proc begin");
        return end_proc(pub_data_stru);
    }

    checkFlag = check_and_match(pub_data_stru);//检查应答信息，并与原交易匹配
    if(0 >= checkFlag) { //检查失败
        void_proc(pub_data_stru); //需要进行超时处理
        if(checkFlag < 0)
            strcpy(pub_data_stru->center_result_code, CODE_SYSTEM_ERR);
        dcs_log(0,0,"<%s> check_and_match fail!",__FUNCTION__);
        return reply_acq(pub_data_stru);
    }
    if(pub_data_stru->switch_src_flag) {
        memset(acq_insti_code, 0, sizeof(acq_insti_code));
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"),
                             DB_MSG_TYPE, acq_insti_code, 2,9);
        snprintf(pub_data_stru->route_insti_code,9,"%s", acq_insti_code);
        dcs_debug(0,0,"<%s> return src acq_insti_code=[%s]",__FUNCTION__,acq_insti_code);
    } else {
        if(0 > get_route_insti_code(pub_data_stru)) { //未配置路由机构
            dcs_log(0,0,"<%s> get_route_insti_code fail!",__FUNCTION__);
            strcpy(pub_data_stru->center_result_code, CODE_SYSTEM_ERR);
            return reply_acq(pub_data_stru);
        }
    }

    if(0 > get_route_insti_info(pub_data_stru)) { //获取路由机构信息
//      if(pub_data_stru->switch_src_flag)
        strcpy(pub_data_stru->center_result_code, CODE_SYSTEM_ERR);
        void_proc(pub_data_stru);
        dcs_log(0,0,"<%s> get_route_insti_info fail!",__FUNCTION__);
        return reply_acq(pub_data_stru);
    }

    if(!pub_data_stru->switch_src_flag) {
        if(0>check_route_insti(pub_data_stru)) { //检查路由机构信息 ,如机构是否被关闭，机构通信链路是否就绪
            strcpy(pub_data_stru->center_result_code, CODE_CUPS_NOT_OPERATE);
            void_proc(pub_data_stru);
            dcs_log(0,0,"<%s> check_route_insti fail!",__FUNCTION__);
            return reply_acq(pub_data_stru);
        }
        if(0>get_route_trans_info(pub_data_stru)) { //获取路由交易信息,包括报文类型等
            strcpy(pub_data_stru->center_result_code, CODE_SYSTEM_ERR);
            void_proc(pub_data_stru);
            dcs_log(0,0,"<%s> get_route_trans_info fail!",__FUNCTION__);
            return reply_acq(pub_data_stru);
        }
        flag = 1;
    } else {
        flag = 0;
    }
    if(0 > conver_data(pub_data_stru, 0)) {
        strcpy(pub_data_stru->center_result_code, CODE_SYSTEM_ERR);
        void_proc(pub_data_stru);
        dcs_log(0,0,"<%s> conver_data fail!",__FUNCTION__);
        return reply_acq(pub_data_stru);
    }
    if(0>db_update(pub_data_stru, flag)) { //转接更新流水表;
        strcpy(pub_data_stru->center_result_code, CODE_SYSTEM_ERR);
        void_proc(pub_data_stru);
        dcs_log(0,0,"<%s> db_update fail!",__FUNCTION__);
        return reply_acq(pub_data_stru);
    }
    if(pub_data_stru->switch_src_flag) { // 在交易返回来源处时系统想干的事情
        trans_end_notify(pub_data_stru);
    }
    pub_data_stru->use_timeout = !pub_data_stru->switch_src_flag;
    return 1;
}
