#include <time.h>
#include <errno.h>
#ifndef __BASE_H__
#define __BASE_H__
#define MAX_FIELD 300
#define MAX_BUFFER 32768
#define SYSTEM_FOLDNAME        "APPL"
#define DB_MSG_TYPE           "DBMG"  //数据库私有域类型标识
/*#define ICS_DEBUG(a)  {    if( a == 1 ){ \
                             gl_dbg_len=sprintf(gl_dbg_buf,"%s",__FUNCTION__); \
                             gl_fn_cnt=1; \
                           }else if( a == 0 ){  \
                           	 if(gl_dbg_cnt>100){ gl_dbg_cnt=1;gl_dbg_len=0;} \
                           	 gl_fn_cnt++;  \
                           	 gl_dbg_len +=snprintf(gl_dbg_buf+gl_dbg_len,sizeof(gl_dbg_buf)-gl_dbg_len,"||%s",__FUNCTION__); \
                           }else if( a == 2 ){ \
                              if(gl_dbg_cnt>100){ gl_dbg_cnt=1;gl_dbg_len=0;} \
                              gl_fn_cnt++;\
                              gl_dbg_len +=snprintf(gl_dbg_buf+gl_dbg_len,sizeof(gl_dbg_buf)-gl_dbg_len,"||%s",__FUNCTION__); \
                              dcs_debug(0,0,"[%d]%s",gl_fn_cnt,gl_dbg_buf); \
                           } \
	                    }
*/
#define ICS_DEBUG(a)     
//数据域字段定义区
#define FIELD_AMOUNT           4   	//交易金额
#define FIELD_AMOUNT_REAL      104 	//真实交易金额
#define FIELD_PAY_FEE	       204 	//手续费
#define FIELD_MSGID            0   	//消息头
#define FIELD_CARD_NO		   2   	//卡号
#define FIELD_POS_ENTRY_MD_CD  22  	//服务点输入方式码
#define FIELD_INSTI_CODE	   33  	//机构号
#define FIELD_TRACK2           35  	//二磁道信息
#define FIELD_TRACK3           36  	//三磁道信息
#define FIELD_TERM_ID1        41   	//终端信息1
#define FIELD_PSAM_NO         20   	//PSAM_NO
#define FIELD_TERM_ID2        42   	//终端信息2
#define FIELD_IC_DATA         55   	//基于PBOC借贷记标准的IC卡数据域
#define FIELD_TPDU            91   	//TPDU 
#define FIELD_MENU_VERSION    92   	//菜单版本
#define FIELD_RANDOM          93   	//RANDOM
#define FIELD_BILL            44   	//账单号
#define FIELD_MSG_SEQ         94   	//MSG_SEQ
#define FIELD_PROG_VER        95   	//PROG_VER
#define FIELD_FUNC_CODE       96   	//FUNC_CODE 
#define POS_ENTRY_MD_CD       22   	//输入点方式
#define FIELD_PIN             52   	//个人密码域
#define FIELD_MAC             64   	//MAC域
#define FIELD_RECODE          39   	//应答码域
#define FIELD_DATE_TIME       7    	//日期时间
#define FIELD_DATE_TIME_Y     99   	//系统时间
#define FIELD_ACQ_INSTI_CODE  32   	//受理方机构号
#define FIELD_IC_PARA         48   	//IC参数下载上送域
#define FIELD_KEY             48   	//密钥数据
#define SYS_TIME_OUT          60   	
#define FIELD_DATE            13   	//日期
#define FIELD_TIME            12   	//时间
#define FIELD_AUTH_ID         38   	//授权码
#define FIELD_TRA_NO          11   	//流水号
#define FIELD_ACQ_INSTI_CODE  32   	//受理方机构号
#define FIELD_SYS_REF_NO      37   	//系统参考号
#define FIELD_SETTLE_DATE     15   	//清算日期
#define FIELD_ORG_TRANS_INFO  90   	//原交易信息
#define FIELD_BALANCE         54   	//余额
#define FIELD_BALANCE_1       954  	//可用余额
#define FIELD_INFO            48   	//一般数据
//错误代码定义区
#define CODE_CUPS_NOT_OPERATE  "91"
#define CODE_INVALID_CARD      "14"
#define CODE_SYSTEM_ERR        "96"
#define CODE_MAC_ERR		   "A0"
#define CODE_SAFE_ERR		   "A7"
#define CODE_TIME_OUT  		   "98"
#define CODE_INQUIRY		   "FA"
#define CODE_NOT_EXIST		   "25"
#define CODE_PIN_ERR           "99"
#define CODE_PACK_ERR          "30"
#define CODE_PROCESSING        "09"
#define CODE_NO_SUPPORT        "40"
#define CODE_AMOUNT_LIMIT      "61"
#define CODE_COUNT_LIMIT       "65"
#define CODE_CARD_NO_ALLOW     "57"
#define CODE_INVALID_TRANS     "12"
#define CODE_TRANS_ERR         "22"

//超时表基本结构

typedef struct{
	char foldname[8 + 1];
	char sys_date[8 + 1];
	char sys_time[6 + 1]; 
	char key[80 + 1];    //报文关键字
	char flag[1 + 1]; //超时处理方式
	int num;          //超时处理次数
	time_t invalid_time; //报文应答超时时间
	char first_key[80 + 1];  //交易受理节点报文键值，数据库关键字
	char trans_type[4 + 1];
	char remark[512 + 1];  //备注信息存储
}timeout_stru;
//报文数据域定义结构
typedef struct{
   short id;     //不同接口报文字段域在系统全局定义的唯一标识符
   char  name[40+1]; //专有报文中对应的字段域名称
   char  is_compress; //上行 数据是否被压缩了 (0,不压缩，1压缩后补齐，2，压缩前补齐
   char  len_type;   // 上行 0固定长，1一字节长度，2二字节长度
   char  d_is_compress; //下行数据是否被压缩了 (0,不压缩，1压缩后补齐，2，压缩前补齐
   char  d_len_type;   //下行 0固定长，1一字节长度，2二字节长度
   short max_len;  //上行 数据最大长度(压缩前)，0为无最大长度
   short d_max_len;  //下行 数据最大长度(压缩前)，0为无最大长度
} field_define;
//报文数据域定义存储结构
typedef struct{
	char msg_type[5]; // 报文种类标识
	short use_num;   //  被定义的字段域个数
	field_define *fld_def; //字段域定义区集合
}message_define;

typedef struct{

	int num;
	message_define *priv_def;
}GLOB_DEF;
/*
typedef struct{

	int num;
	message_define priv_def[100];
}GLOB_DEF;
*/
// 字段域数据解析存储结构
typedef struct{
	short field_id; //字段域在本系统中定义的标识符
	char  msg_type[4]; //消息类型
	short len;      //数据存放的长度
	char *data_addr; //数据存放的起始地址	
	unsigned char Off; //数据有效标识 0无效，1有效
	unsigned char from; //数据来源 0，接收机构，1中心产生，2，数据库中
}field_data;

typedef struct{
	short field_id[MAX_FIELD];
	short from[MAX_FIELD];
	char field_name[MAX_FIELD][64];
}FIELD;
typedef struct{
  short num;   //报文数据域定义的个数
  FIELD field;  
}field_set;
typedef struct glob_msg_stru
{
	char *src_buffer;
	short  src_len;
	short req_flag; //1为请求交易，其他为应答交易
	short switch_src_flag; // 1为转接到原受理机构，其他为转接到其他目的机构
	char insti_code[12];    //发送报文的机构代码
	char insti_fold_name[40];//发送报文通信用的fold名字
	char insti_open_flag;  //发送机构打开标识,0关闭,1打开
	char insti_link_type;  //发送机构链路标识，0,长连接 1,短连接
	char insti_work_type;  //发送机构交易接入模式 0,机构 ,1 终端
	message_define *in_priv_def;    //进入报文种类数据域定义指针
	char in_msg_type[5]; //进入报文的报文类型
	char in_mastkey[33];
	char in_mac_index[6];
	char in_mac_key[33];
	char in_pin_index[6];
	char in_pin_key[33];
	char in_data_index[6];
	char in_data_key[33];
	char is_check_mac;//是否校验mac
	char msg_key[100]; //数据库记录关键字
	char app_type[5]; //业务类型
	char in_trans_type[5];//进入交易类型
	char open_flag;//业务打开标识 0关闭，1打开
	char is_route;//是否路由 0：转发初始接入点，1：业务下一路由节点，2：处理结束,3:原始交易路由节点
	int  route_num; //交易被路由的次数
	char step_type; // 1 支付渠道，2业务渠道
	char route_insti_code[12];// 路由机构代码
	char route_fold_name[40];//路由报文通信用的fold名字
	char route_is_make_mac;//是否生成mac
	char route_insti_open_flag;//路由机构打开标识 0关闭,1打开
	char route_insti_link_type;  //路由机构链路标识，0,长连接 1,短连接
	char route_insti_work_type;  //路由机构交易接入模式 0,机构 ,1 终端
	char route_is_check_mac;
	char route_msg_type[5];//路由转换的报文类型
	char route_trans_type[5]; //路由转换的交易类型
	message_define *route_priv_def;       //路由的报文种类数据域定义指针
	
	field_set route_set;//转发的报文数据域ID集合
	char route_mastkey[33];
	char route_mac_index[6];
	char route_mac_key[33];
	char route_pin_index[6];
	char route_pin_key[33];
	char route_data_index[6];
	char route_data_key[33];
	char center_result_code[5];//中心处理结果代码
	char is_void;             //是否需要冲正处理 ,0不冲正，1需要
	char permit_void;         // 应用是否许可受理方发起冲正
	char use_timeout ;         //使用超时表标志
	timeout_stru timeout_table;//超时表存放的数据
	short filed_key_num; //数据库关键域字段个数
	short filed_key_field_id[10];//数据库关键域字段组合
	short msg_field_num; //报文域总数
	short timeout;       // 超时时间
	field_data data_rec[MAX_FIELD]; //报文域数据解析
	char tmp_order[5];         //特殊报文使用的指令表示区
	unsigned char in_cry_flag; // 进入国密算法应用标识 默认0不应用，1应用
	unsigned char out_cry_flag; //出去国密算法应用标识 默认0不应用，1应用
	int off_set;         //buffer使用偏移量
	char buffer[MAX_BUFFER]; //数据存放区
}glob_msg_stru;

struct unpack_func_def{
	char *msg_type;
	int (* priv_unpack)(char *src_buf,int src_len,glob_msg_stru *pub_data_stru);
};
struct pack_func_def {
	char *msg_type;
	int (* priv_pack)(glob_msg_stru *pub_data_stru,char *buf,int size);
};
struct DEFAULT_PROC{
	char *name;
	int (* func)(glob_msg_stru *pub_data_stru);
};
struct DB_PROC{
	char *name;
	int (* func)(glob_msg_stru *pub_data_stru,int flag);
};
struct INSTI_CODE_PROC {
	char *msg_type;
	int (*func)(char *insti_code);
};
typedef struct{
	char *handle;
	int (* func)(char *para, short fldid, glob_msg_stru *pub_data_stru);
	int (* func_ret)(char *para, short fldid, glob_msg_stru *pub_data_stru);
}UPDATE_DB_DEF;


typedef struct{
	char *handle;
	int (* func)(char *para, short fldid, glob_msg_stru *pub_data_stru);
}FLDSET_DEF;
/*交易流水数据库基本结构*/
/*
1、交易受理端信息存储区
2、中心处理信息存储区
3、支付渠道信息存储区
4、其他渠道信息存储区
*/

//数据库基本结构
typedef struct
{
   char sys_date             [8 + 1];
   char sys_time             [6 + 1];
   char qs_date              [8 + 1];
   char acq_insti_code       [11 + 1];
   char pay_insti_code       [11 + 1];
   char app_insti_code       [11 + 1];
   char acq_msg_type         [4 + 1];
   char acq_trans_type       [4 + 1];
   char app_type             [4 + 1];
   char pay_msg_type         [4 + 1];
   char pay_trans_type       [4 + 1];
   char app_msg_type         [4 + 1];
   char app_trans_type       [4 + 1];
   char resp_cd_app          [6 + 1];
   char resp_cd_pay          [6 + 1];
   char resp_cd_rcv          [6 + 1];
   char pay_acct_no          [30 + 1];
   char card_attr            [2 + 1];
   char iss_insti_code       [11 + 1];
   char amount_pay           [12 + 1];
   char amount_real          [12 + 1];
   char fee                  [12 + 1];
   char acq_tra_no           [6 + 1];
   char pay_tra_no           [6 + 1];
   char app_tra_no           [6 + 1];
   char acq_date             [8 + 1];
   char acq_time             [6 + 1];
   char pay_date             [8 + 1];
   char pay_time             [6 + 1];
   char app_date             [8 + 1];
   char app_time             [6 + 1];
   char acq_term_id1         [20 + 1];
   char acq_term_id2         [20 + 1];
   char pay_term_id1         [20 + 1];
   char pay_term_id2         [20 + 1];
   char app_term_id1         [20 + 1];
   char app_term_id2         [20 + 1];
   char acq_addition         [512 + 1];
   char pay_addition         [512 + 1];
   char app_addition         [512 + 1];
   char sys_ref_no           [12 + 1];
   char pos_entry_md_cd      [3 + 1];
   char pos_cond_cd          [2 + 1];
   char rcv_acct_no          [30 + 1];
   char trans_curr_cd        [3 + 1];
   char resp_cd_auth_id      [6 + 1];
   char step                 [1 + 1];
   char void_flag            [1 + 1];
   char permit_void          [1 + 1];
   char acq_cry_type         [1 + 1];
   char acq_mac              [16 + 1];
   char mcc                  [4+1];
   char acq_proc_code        [6+1];
   char pay_proc_code        [6+1];
   char pay_msg_id           [4+1];
   char merch_info           [80+1];
}tl_trans_log_def;

typedef struct
{
	const char *type;
	const char *field;
	size_t offset;
	size_t size;
	int ctype;
}trans_log_handler_def;
typedef struct{
	unsigned char fieldLen;
	unsigned char field[2 + 1];
	unsigned char datalen;
	unsigned char data[128 + 1];
}TAG_def;
typedef struct{
	char *handle;
	int (* func)(char *para, char *buf, int bufLen, char *mac, glob_msg_stru *pub_data_stru, int flag);
}MAC_CALC_DEF;
struct HEAD_PROC {
	
	 char *func_name;
	 int (*func)(char * buf ,int start,int len);
};
typedef struct{
	char *handle;
	int (* func)(char *para,char* inst_code ,glob_msg_stru *pub_data_stru);
}NOTIFY_DEF;
typedef struct {
	char term_id[17]; //终端编号
	char type[2];     //计算方法
	char begin_date[9]; // 开始日期
	char end_date[9];   // 结束日期
	char para[256+1];  //计算参数
	char check_list[512+1]; //条件判断列表
}DISCNT_INFO;

#endif

