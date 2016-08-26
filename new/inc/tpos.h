#include "base.h"
#ifndef __TPOS_H__
#define __TPOS_H__

struct TPOS_TERM_INFO {
	char psam[16+1];
	char status[1+1];    //�ն�״̬ 0,�رգ�1��
	char menu_ver[8+1];  //�˵��汾��
	char tel_no[16+1];   //�����IP��ַ
	char comm_type[1+1];        //�ն�ͨ����������
	char is_bind[1+1];         //�Ƿ���󶨹�ϵ
	char menu_update_flag[1+1]; //1��Ҫ���� ,0����Ҫ����
	char download_flag[1+1] ;//ȫ�������ر�ʶ,0���ֲ˵����£�1ȫ�˵�����
	int  last_menu_para_step; //�ϴ����ص�ĩβ�˵����²��裻0Ϊ��ʼ״̬��δ�����ز˵�
	int  last_other_para_step;
	int  last_ic_para_step;
	int  last_ic_key_step;
	char name[60+1];
	char last_node_set[200+1]; //�ϴ����صĲ˵��ڵ㼯��
	char stack_detail[64+1]; //�˵����ص�ջ������
	char settle_time[14+1];
};
struct TPOS_MENU_NODE {
	char menu_ver[8+1];//�˵��汾��
	int node_id;//�˵���id
	int pre_node_id;//ǰ���
	int right_node_id;
	int left_node_id;
	char is_leaf[1+1];//Ҷ����־ 0��ʾ��Ҷ��� 1��ʾ��Ҷ���
	char pre_flag[1+1];//ǰ����־ 0Ϊǰ�������� 1Ϊǰ�����ҽ��
};
struct MENU_NODE_INFO {
	int node_id;
	char func_code[3+1];
	char void_flag[1+1];
	int func_disp_indx;
	char title [32+1];
	char op_code[128+1];
	char use[1+1];
	int step;
};
struct TPOS_PARA_INFO {
	char para_type[2+1];
	int id;
	char ver[4];
	int step;
	char detail[300+1];
//	int last_op;//0�޸�1����2ɾ��
  char control_info[20+1];
  char use[1+1];
	int last_step;
};

struct STACK {
	int use;
	int nmax;
	struct TPOS_MENU_NODE nodes[30];
};
typedef struct
{
	char para_type[3];
	int id;
}para_set_stru;

int _tpos_get_key32(char *sek_indx,char *tek_indx,char *tm_key,char *key1,char *key2);
int get_psam_no(glob_msg_stru * pub_data_stru ,char *psam,int size);
int is_trust_tel( char * tel);
int get_tpos_sn( char *psam , char * sn);
int update_tpos_sn( char *psam ,char *sn);
int tpos_gen_work_key(glob_msg_stru *pub_data_stru,const char *psam);
int menu_preorder_traversal(char *menu_ver ,struct TPOS_MENU_NODE * menu_node);
int get_menu_root_node( char * menu_ver ,struct TPOS_MENU_NODE * menu_node);
int get_node_info(char *menu_ver,int id,struct MENU_NODE_INFO *node_info);
int	get_last_part_para(glob_msg_stru *pub_data_stru,const char *psam,char *menu_ver,char *node_set);
int get_next_menu_node(char *menu_ver,int step, struct MENU_NODE_INFO *node_info);
int save_download_para_info(int flag,int step,char *set,char *psam);
int get_para_ver(char *menu_ver, char *para_ver);
int get_next_part_para( char *ver,int step,struct TPOS_PARA_INFO * para);
int get_ic_para(glob_msg_stru *pub_data_stru,struct TPOS_TERM_INFO *terminfo);
int update_auto_download_flag( char *psam);
int db_init_stack(char *node_set,int *stack, size_t stack_size);
int db_push(int *stack,int id,int *use);
int db_pop(int *stack , int *id,int *use);
int get_menu_node( char * menu_ver , int node_id , struct TPOS_MENU_NODE * menu_node);
int db_save_stack(char *psam,int *stack,int use);
int db_save_set( char *node_set,int step,char *psam);
int get_max_step( char *menu_ver);
int  tpos_init_stack(struct STACK *stack);
int tpos_push(struct STACK *stack,struct TPOS_MENU_NODE *node);
int tpos_pop(struct STACK *stack,struct TPOS_MENU_NODE *node);
int stackempt(struct STACK *stack);
int get_menu_cnt(char *menu_ver);
int get_other_para_info(char *type, int id, char *para_ver,struct TPOS_PARA_INFO *para_info);
int get_pubkey_data_sm(char *buf, int bSize, char *node_set, int *last_ic_para_step, int *cnt);
int _tpos_get_smkey(char *sek_indx,char *tek_indx,char *tm_key,char *key1,char *key2);
/*���ع������*/
int tpos_count_trans(glob_msg_stru *pub_data_stru, char *psam,char *settle_time);
/***/
int select_old_tpos_log(tl_trans_log_def *pTransLog);

#endif
