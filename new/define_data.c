#include "general_util.h"


//char   *g_pcBcdaShmPtr;
//int exitFlag;
char gl_dbg_buf[8192];
int  gl_dbg_len,gl_dbg_cnt,gl_fn_cnt,errno;
GLOB_DEF gl_def_set;

struct unpack_func_def unpack_func_proc[]= {  //数据解包处理函数
    {"POSC",iso_unpack},
    {"TPOS",tpos_unpack},
    {"XMLP", xml_unpack},
    {"ZFYP",iso_unpack},
    {"", NULL}
};
struct pack_func_def pack_func_proc[]= { //数据打包处理函数
    {"POSC",iso_pack},
    {"TPOS",tpos_pack},
    {"ZFYP",iso_pack},
    {"XMLP", xml_pack},
    {"",NULL}
};

FLDSET_DEF gl_check_limit[]= {        //转接交易检测处理函数
    {"CARD_INFO_CHECK", card_info_check},
    {"TRANS_CONTROL", trans_control},
    {"ISO_CMAC_ALL_DES", iso_check_mac_all_des}, //ISO校验MAC，全报文DES模式
    {"ISO_CMAC_ALL_3DES", iso_check_mac_all_3des}, //ISO校验MAC，全报文3DES模式
    {"ISO_CMAC_CBC_3DES", iso_check_mac_cbc_3des}, //ISO校验MAC，CBC 3DES模式
    {"TPOS_CHECK_TERM",_tpos_check_terminal},
    {"TPOS_CHECK_MAC",_tpos_check_mac},
    {"zpos_check_terminal",_zpos_check_terminal},
    {"T_GET_L_ADDIDATA", tpos_get_last_addidata},
    {"GET_END_TRANS", get_end_trans},
    {"TRANS_CANCLE", trans_cancle},
    {"TPOS_TRANS_CANCLE", tpos_trans_cancle},
    {"TPOS_TRANS_REFUND", tpos_trans_refund},
    {"CALCULATE_FEE", calculate_fee},
    {"CHECK_EXPENSES_RESULT",check_expenses_result},//检查终端费用是否缴纳
    {"TPOS_CHECK_EXPENSES_RESULT",tpos_check_expenses_result},//检查终端费用是否缴纳
    {"TPOS_DISCOUNT_RESULT",tpos_discount_result},//检查是否参与折扣
    {"TPOS_FIELD_PRE_CONV",tpos_field_pre_conv},
    {"ZPOS_FIELD_PRE_CONV",zpos_field_pre_conv},
    {"VERIFY_PASSWD",verify_passwd},
    {"IS_VALID_CARDBIN",is_valid_cardbin},
    {"TC_SEND",tc_send},
    {"",NULL}
};
struct DEFAULT_PROC gl_direct_limit[]= { //直接应答交易检测处理函数
    {"check_mac",tpos_check_mac},
    {"tpos_check_terminal",tpos_check_terminal},
    {"zpos_check_terminal",zpos_check_terminal},
    {"check_prog_ver",tpos_check_prog_ver},
    {"check_menu_ver",tpos_check_menu_ver},
    {"",NULL}
};

struct DEFAULT_PROC gl_end_proc[]= {     // 交易在中心结束处理函数
    {"reversed_replay", reversed_replay},
    {"find_replay", find_replay},
    {"sign_m_p_32", sign_m_p_32},
    {"result_query_tl", result_query_tl},
    {"result_query_hnyl", result_query_hnyl},
    {"", NULL}
};

struct DEFAULT_PROC gl_direct_proc[]= {  // 直接应答交易应用处理函数
    {"tpos_login",tpos_login}, //tpos签到
//  {"download_menu",tpos_download_menu},
    {"download_para",tpos_download_para}, //tpos参数下载
    {"link_test",tpos_link_test},//tpos链路测试
    {"tpos_reversed", tpos_reversed}, // tpos冲正
    {"get_end_trans", end_trans},
    {"end_trans_print", end_trans_print},   // 末笔交易补打
    {"app_reversed", app_reversed}, //冲正
    {"sign_request", sign_request},
    {"key_reset", key_reset},//密钥重置
    {"bill_fee_find",bill_fee_find},
    {"zpos_login",zpos_login}, //zpos 签到
    {"zpos_download_para",zpos_download_para}, //ic参数下载
    {"query_expenses",query_expenses}, //查询终端费用
    {"tpos_query_expenses",tpos_query_expenses}, //查询终端费用
    {"save_tc_value",save_tc_value},//对TC值上送的应答
    {"insti_login",insti_login},//对TC值上送的应答
    {"tpos_settle",tpos_settle},//终端结算,
    {"",NULL}
};
struct DEFAULT_PROC gl_pre_field_conver[]= { //数据解包后字段域预处理函数
//  {"TPOS",tpos_field_pre_conv},
//  {"ZPOS",zpos_field_pre_conv},
    {"",NULL}
};
struct INSTI_CODE_PROC gl_insti_code[]= { //机构转换函数
    {"TPOS",folder_to_insti_code},
    {"",NULL},
};
struct DEFAULT_PROC gl_route_proc[]= { // 特殊交易路由判断处理函数
    {"BILL_PAY", bill_pay},
    {"",NULL}
};
struct DB_PROC gl_db_iproc[]= {  //数据库插入处理函数
//  {"TPOS",db_tpos_insert},
    {"", NULL}
};

FLDSET_DEF gl_genrate_field_conver[]= { //通用域转换处理函数
    {"PIN_CHANGE", pin_change},
    {"TPOS_GEN_CONVER", tpos_gen_field_conver},
    {NULL, NULL}
};


FLDSET_DEF gl_priv_field_conver[]= {  //特殊域转换处理函数
    {"GET_DB_DATA", get_db_data},
//  {"GET_TPOS_DATA", get_tpos_data},
    {"GET_SEQ", get_seq},
    {"GET_REF", get_ref},
    {"GET_SYSTIME", get_systime},
    {"GET_DEFAULT", get_default},
    {"PRINT_FORMAT", print_format},
    {"ORG_TRANS_INFO", get_org_trans_info},
    {"GET_MSG_DATA", get_msg_data},
    {"GET_MSG_DATA_FILL", get_msg_data_fill},
    {"GET_MSG_DATA_TLVFILL", get_msg_data_tlvfill},
    {"GET_DB_DATA_FILL", get_db_data_fill},
    {"GET_DATA_STANDARD", get_data_standard},
    {"IC55_RET", ic55_ret},
    {"SAVE_TIMEOUT", save_timeout},
    {"READ_TIMEOUT", read_timeout},
    {"SET_SERVICE_ENTRY", set_field_service_entry},
    {"MAKE_DATA", make_data},
//  {"IC_PARA_DATA", ic_para_data},
    {"SHOW_FORMAT", show_format},
    {"SET_CTRL_INFO", set_ctrl_info},
    {"E_PAYMENT_48", electricity_payment_48},
    {"FILL_SHOP_NAME", zpos_fill_shopname},
    {"SET_SERVICE_CODE", set_service_code},
    {"SET_FIELD_60",set_field_60},
    {"FORMAT_ECHO_INPUT", format_echo_input},
    {"TPOS_DISCOUNT_RESULT",tpos_discount_result},//检查是否参与折扣
    {"CHANGE_DB_DATA",change_db_data},//
    {"CON_DEL_FLD48",con_del_fld48},//

    {NULL, NULL}
};

FLDSET_DEF gl_timeout_handle[]= {  //超时应用特殊处理函数
    {"", NULL}
};

TAG_def TAG[]= {
    {2, "\x9F\x1E", 0, ""},
    {2, "\x9F\x36", 0, ""},
    {2, "\x9F\x26", 0, ""},
    {2, "\x9F\x27", 0, ""},
    {2, "\x9F\x10", 0, ""},
    {2, "\x9F\x37", 0, ""},
    {1, "\x95", 0, ""},
    {1, "\x9A", 0, ""},
    {1, "\x9C", 0, ""},
    {2, "\x9F\x02", 0, ""},
    {2, "\x5F\x2A", 0, ""},
    {1, "\x82", 0, ""},
    {2, "\x9F\x1A", 0, ""},
    {2, "\x9F\x03", 0, ""},
    {2, "\x9F\x33", 0, ""},
    {2, "\x9F\x34", 0, ""},
    {2, "\x9F\x35", 0, ""},
    {1, "\x84", 0, ""},
    {2, "\x9F\x09", 0, ""},
    {2, "\x9F\x41", 0, ""},
    {1, "\x91", 0, ""},
    {1, "\x71", 0, ""},
    {1, "\x72", 0, ""},
    {2, "\xDF\x31", 0, ""},
    {2, "\x9F\x74", 0, ""},
    {2, "\x9F\x63", 0, ""},
    {0, "", 0, ""}
};
FLDSET_DEF gl_special_business_handle[]= {
    {"SPECIFIC_BUSINESS", specific_business_handle}, //通过业务确定交易商户信息
    {"GENERAL_BUSINESS", general_buisiness_handle},  //通过上送终端，业务确认交易商户信息
    {"CANCLE_SPECIFIC", cancle_trans_specific},  //撤销类交易,针对湖南银联一个通道存在两种不同类型业务参数需特殊处理，转换为不同交易类型
    {NULL, NULL}
};

MAC_CALC_DEF gl_mac_calc[]= {
    {"MAC_CALC_ALL_DES", mac_calc_all_des},
    {"MAC_CALC_ALL_3DES", mac_calc_all_3des},
    {"MAC_CALC_CBC_3DES", mac_calc_cbc_iso_3des},
    {NULL,NULL}
};
FLDSET_DEF gl_check_and_match[]= {
    {"ISO_CMAC_ALL_DES", iso_check_mac_all_des}, //ISO校验MAC，全报文DES模式
    {"ISO_CMAC_ALL_3DES", iso_check_mac_all_3des}, //ISO校验MAC，全报文3DES模式
    {"ISO_CMAC_CBC_3DES", iso_check_mac_cbc_3des}, //ISO校验MAC，CBC 3DES模式
    {"CHECK_REPLAY_CD", check_replay_cd},
    {NULL, NULL}
};
struct DEFAULT_PROC gl_db_qproc[]= {
//  {"TPOS", db_tpos_query},
    {"", NULL}
};
struct DEFAULT_PROC gl_terminfo_qproc[]= {
    {"TPOS", _tpos_get_work_key},
    {"ZPOS", _zpos_get_work_key},
    {NULL,NULL}
};

UPDATE_DB_DEF gl_update_db[]= {
//  {"UPDATE_DB_TPOS"   , update_db_tpos   , update_db_tpos   },
    {"UPDATE_DB_PAY_RET", update_db_pay_ret, update_db_pay_ret},
    {"UPDATE_DB_PAY_APP", update_db_pay_app, update_db_pay_ret},
    {"UPDATE_DB_APP_RET", update_db_app_ret, update_db_app_ret},
    {"UPDATE_DB_APP_PAY", update_db_app_pay, update_db_app_ret},
    {NULL, NULL}
};
struct HEAD_PROC gl_head_proc[]= { // iso打包结束时对包头的特殊处理
    {"TL_HEAD_PROC", iso_tl_head_proc},
    {NULL,NULL}
};
NOTIFY_DEF gl_end_nofify[]= { //交易完成通知处理函数
    {"FDC_NOTIFY",fdc_notify},//房地产交易完成通知处理函数
    {"INSERT_EXPENSES",insert_expenses},// 押金缴费成功写入数据库
    {NULL,NULL}
};

FLDSET_DEF gl_advert_proc[]= {
    {"CHECK_AMOUNT",check_amount},
    {"CHECK_CARDBIN",check_cardbin},
    {"CHECK_CNT_MER",check_cnt_mer},
    {"CHECK_VALID_DATE",check_valid_date},
    {"CHECK_CNTUSE_CURDATE",check_cntuser_curdate},
    {NULL,NULL}
};

