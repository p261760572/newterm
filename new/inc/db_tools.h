#include "base.h"
#ifndef __DB_TOOLS_H__
#define __DB_TOOLS_H__
int DasConectDB(void);
int DasEnd( int flag);
int SetFieldData(char *buff, int iFieldID, char *fieldBuf, int size, int flag);
int GetFieldData(char *buff, int iFieldID, char *fieldBuf, int size, int flag);
int db_update(glob_msg_stru *pub_data_stru,int flag);
int SetTransLog(tl_trans_log_def *transLog, char *fieldName, char *fieldVal, short fieldID, glob_msg_stru * pub_data_stru);
int update_translog(tl_trans_log_def *pTransLog);
int update_key(char *insti_code, char *mac_key, char *pin_key);
int db_genrate_insert(glob_msg_stru * pub_data_stru, int flag);
int db_insert(glob_msg_stru * pub_data_stru,int flag);
int insert_timeout_table(glob_msg_stru *pub_data_stru, int flag);
int insert_expenses (char *para, char *inst_code,glob_msg_stru *pub_data_stru);
int db_to_pub_daba(glob_msg_stru * pub_data_stru, tl_trans_log_def *pTransLog);
int delete_timeout_table(glob_msg_stru *pub_data_stru);
int insti_update_work_key(char * insti_code, char *pin_key, char *mac_key, char *cd_key);
#endif
