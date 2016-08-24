#include <time.h>
#include <errno.h>
#ifndef __BASE_H__
#define __BASE_H__
#define MAX_FIELD 300
#define MAX_BUFFER 32768
#define SYSTEM_FOLDNAME        "APPL"
#define DB_MSG_TYPE           "DBMG"  //���ݿ�˽�������ͱ�ʶ
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
//�������ֶζ�����
#define FIELD_AMOUNT           4   	//���׽��
#define FIELD_AMOUNT_REAL      104 	//��ʵ���׽��
#define FIELD_PAY_FEE	       204 	//������
#define FIELD_MSGID            0   	//��Ϣͷ
#define FIELD_CARD_NO		   2   	//����
#define FIELD_POS_ENTRY_MD_CD  22  	//��������뷽ʽ��
#define FIELD_INSTI_CODE	   33  	//������
#define FIELD_TRACK2           35  	//���ŵ���Ϣ
#define FIELD_TRACK3           36  	//���ŵ���Ϣ
#define FIELD_TERM_ID1        41   	//�ն���Ϣ1
#define FIELD_PSAM_NO         20   	//PSAM_NO
#define FIELD_TERM_ID2        42   	//�ն���Ϣ2
#define FIELD_IC_DATA         55   	//����PBOC����Ǳ�׼��IC��������
#define FIELD_TPDU            91   	//TPDU 
#define FIELD_MENU_VERSION    92   	//�˵��汾
#define FIELD_RANDOM          93   	//RANDOM
#define FIELD_BILL            44   	//�˵���
#define FIELD_MSG_SEQ         94   	//MSG_SEQ
#define FIELD_PROG_VER        95   	//PROG_VER
#define FIELD_FUNC_CODE       96   	//FUNC_CODE 
#define POS_ENTRY_MD_CD       22   	//����㷽ʽ
#define FIELD_PIN             52   	//����������
#define FIELD_MAC             64   	//MAC��
#define FIELD_RECODE          39   	//Ӧ������
#define FIELD_DATE_TIME       7    	//����ʱ��
#define FIELD_DATE_TIME_Y     99   	//ϵͳʱ��
#define FIELD_ACQ_INSTI_CODE  32   	//����������
#define FIELD_IC_PARA         48   	//IC��������������
#define FIELD_KEY             48   	//��Կ����
#define SYS_TIME_OUT          60   	
#define FIELD_DATE            13   	//����
#define FIELD_TIME            12   	//ʱ��
#define FIELD_AUTH_ID         38   	//��Ȩ��
#define FIELD_TRA_NO          11   	//��ˮ��
#define FIELD_ACQ_INSTI_CODE  32   	//����������
#define FIELD_SYS_REF_NO      37   	//ϵͳ�ο���
#define FIELD_SETTLE_DATE     15   	//��������
#define FIELD_ORG_TRANS_INFO  90   	//ԭ������Ϣ
#define FIELD_BALANCE         54   	//���
#define FIELD_BALANCE_1       954  	//�������
#define FIELD_INFO            48   	//һ������
//������붨����
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

//��ʱ������ṹ

typedef struct{
	char foldname[8 + 1];
	char sys_date[8 + 1];
	char sys_time[6 + 1]; 
	char key[80 + 1];    //���Ĺؼ���
	char flag[1 + 1]; //��ʱ����ʽ
	int num;          //��ʱ�������
	time_t invalid_time; //����Ӧ��ʱʱ��
	char first_key[80 + 1];  //��������ڵ㱨�ļ�ֵ�����ݿ�ؼ���
	char trans_type[4 + 1];
	char remark[512 + 1];  //��ע��Ϣ�洢
}timeout_stru;
//������������ṹ
typedef struct{
   short id;     //��ͬ�ӿڱ����ֶ�����ϵͳȫ�ֶ����Ψһ��ʶ��
   char  name[40+1]; //ר�б����ж�Ӧ���ֶ�������
   char  is_compress; //���� �����Ƿ�ѹ���� (0,��ѹ����1ѹ�����룬2��ѹ��ǰ����
   char  len_type;   // ���� 0�̶�����1һ�ֽڳ��ȣ�2���ֽڳ���
   char  d_is_compress; //���������Ƿ�ѹ���� (0,��ѹ����1ѹ�����룬2��ѹ��ǰ����
   char  d_len_type;   //���� 0�̶�����1һ�ֽڳ��ȣ�2���ֽڳ���
   short max_len;  //���� ������󳤶�(ѹ��ǰ)��0Ϊ����󳤶�
   short d_max_len;  //���� ������󳤶�(ѹ��ǰ)��0Ϊ����󳤶�
} field_define;
//������������洢�ṹ
typedef struct{
	char msg_type[5]; // ���������ʶ
	short use_num;   //  ��������ֶ������
	field_define *fld_def; //�ֶ�����������
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
// �ֶ������ݽ����洢�ṹ
typedef struct{
	short field_id; //�ֶ����ڱ�ϵͳ�ж���ı�ʶ��
	char  msg_type[4]; //��Ϣ����
	short len;      //���ݴ�ŵĳ���
	char *data_addr; //���ݴ�ŵ���ʼ��ַ	
	unsigned char Off; //������Ч��ʶ 0��Ч��1��Ч
	unsigned char from; //������Դ 0�����ջ�����1���Ĳ�����2�����ݿ���
}field_data;

typedef struct{
	short field_id[MAX_FIELD];
	short from[MAX_FIELD];
	char field_name[MAX_FIELD][64];
}FIELD;
typedef struct{
  short num;   //������������ĸ���
  FIELD field;  
}field_set;
typedef struct glob_msg_stru
{
	char *src_buffer;
	short  src_len;
	short req_flag; //1Ϊ�����ף�����ΪӦ����
	short switch_src_flag; // 1Ϊת�ӵ�ԭ�������������Ϊת�ӵ�����Ŀ�Ļ���
	char insti_code[12];    //���ͱ��ĵĻ�������
	char insti_fold_name[40];//���ͱ���ͨ���õ�fold����
	char insti_open_flag;  //���ͻ����򿪱�ʶ,0�ر�,1��
	char insti_link_type;  //���ͻ�����·��ʶ��0,������ 1,������
	char insti_work_type;  //���ͻ������׽���ģʽ 0,���� ,1 �ն�
	message_define *in_priv_def;    //���뱨��������������ָ��
	char in_msg_type[5]; //���뱨�ĵı�������
	char in_mastkey[33];
	char in_mac_index[6];
	char in_mac_key[33];
	char in_pin_index[6];
	char in_pin_key[33];
	char in_data_index[6];
	char in_data_key[33];
	char is_check_mac;//�Ƿ�У��mac
	char msg_key[100]; //���ݿ��¼�ؼ���
	char app_type[5]; //ҵ������
	char in_trans_type[5];//���뽻������
	char open_flag;//ҵ��򿪱�ʶ 0�رգ�1��
	char is_route;//�Ƿ�·�� 0��ת����ʼ����㣬1��ҵ����һ·�ɽڵ㣬2���������,3:ԭʼ����·�ɽڵ�
	int  route_num; //���ױ�·�ɵĴ���
	char step_type; // 1 ֧��������2ҵ������
	char route_insti_code[12];// ·�ɻ�������
	char route_fold_name[40];//·�ɱ���ͨ���õ�fold����
	char route_is_make_mac;//�Ƿ�����mac
	char route_insti_open_flag;//·�ɻ����򿪱�ʶ 0�ر�,1��
	char route_insti_link_type;  //·�ɻ�����·��ʶ��0,������ 1,������
	char route_insti_work_type;  //·�ɻ������׽���ģʽ 0,���� ,1 �ն�
	char route_is_check_mac;
	char route_msg_type[5];//·��ת���ı�������
	char route_trans_type[5]; //·��ת���Ľ�������
	message_define *route_priv_def;       //·�ɵı���������������ָ��
	
	field_set route_set;//ת���ı���������ID����
	char route_mastkey[33];
	char route_mac_index[6];
	char route_mac_key[33];
	char route_pin_index[6];
	char route_pin_key[33];
	char route_data_index[6];
	char route_data_key[33];
	char center_result_code[5];//���Ĵ���������
	char is_void;             //�Ƿ���Ҫ�������� ,0��������1��Ҫ
	char permit_void;         // Ӧ���Ƿ���������������
	char use_timeout ;         //ʹ�ó�ʱ���־
	timeout_stru timeout_table;//��ʱ���ŵ�����
	short filed_key_num; //���ݿ�ؼ����ֶθ���
	short filed_key_field_id[10];//���ݿ�ؼ����ֶ����
	short msg_field_num; //����������
	short timeout;       // ��ʱʱ��
	field_data data_rec[MAX_FIELD]; //���������ݽ���
	char tmp_order[5];         //���ⱨ��ʹ�õ�ָ���ʾ��
	unsigned char in_cry_flag; // ��������㷨Ӧ�ñ�ʶ Ĭ��0��Ӧ�ã�1Ӧ��
	unsigned char out_cry_flag; //��ȥ�����㷨Ӧ�ñ�ʶ Ĭ��0��Ӧ�ã�1Ӧ��
	int off_set;         //bufferʹ��ƫ����
	char buffer[MAX_BUFFER]; //���ݴ����
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
/*������ˮ���ݿ�����ṹ*/
/*
1�������������Ϣ�洢��
2�����Ĵ�����Ϣ�洢��
3��֧��������Ϣ�洢��
4������������Ϣ�洢��
*/

//���ݿ�����ṹ
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
	char term_id[17]; //�ն˱��
	char type[2];     //���㷽��
	char begin_date[9]; // ��ʼ����
	char end_date[9];   // ��������
	char para[256+1];  //�������
	char check_list[512+1]; //�����ж��б�
}DISCNT_INFO;

#endif

