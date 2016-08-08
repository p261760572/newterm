#ifndef __ZPOS_H__
#define __ZPOS_H__

int zpos_check_terminal(glob_msg_stru * pub_data_stru);
int zpos_login(glob_msg_stru * pub_data_stru);
int zpos_download_para(glob_msg_stru *pub_data_stru);
int zpos_field_pre_conv(char *para, short flag, glob_msg_stru *pub_data_stru);
int _zpos_get_work_key( glob_msg_stru *pub_data_stru);
int zpos_fill_shopname(char *para, short fldid, glob_msg_stru *pub_data_stru);
int _zpos_check_terminal(char *para, short fldid,glob_msg_stru * pub_data_stru);
#endif
