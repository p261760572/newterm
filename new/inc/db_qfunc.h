#include "base.h"
#ifndef __DB_QFUNC_H__
#define __DB_QFUNC_H__
int get_insti_field_name(glob_msg_stru *pub_data_stru,char *field_name,int size);
//int folder_to_insti_code(char * fold_name,char *insti_code,int size);
int get_insti_info(glob_msg_stru * pub_data_stru);
int get_msg_key(glob_msg_stru * pub_data_stru);
int get_msg_intout_flag(glob_msg_stru * pub_data_stru);
#endif
