#ifndef __TOOLS__
#define __TOOLS__

void free_priv_def(void);
extern int fold_initsys(void);
extern int  fold_create_folder(const char *folder_name);
extern int  fold_locate_folder(const char *folder_name);
extern int fold_get_maxmsg(int folder_id);
extern int fold_set_maxmsg(int folder_id, int nmaxmsg);
extern int fold_read(int folder_Id,int* org_folderId,void *user_buffer, int nsize,int fBlocked);
extern int  fold_get_name(int folder_id, char *folder_name,int nsize);
extern int fold_write(int dest_folderId,int org_folderId,void *user_data,int nbytes);
extern void bcd_to_asc (unsigned char* ascii_buf ,unsigned char* bcd_buf , int conv_len ,unsigned char type );
extern void asc_to_bcd ( unsigned char* bcd_buf , unsigned char* ascii_buf , int conv_len ,unsigned char type );
extern int load_config(void );
extern int app_timeout_proc(char *src_buf, int src_len ,char *desc_buf,int size, int *to_fid);
extern int app_proc(const char *msg_type,char *src_buf,int src_len,
             char * desc_buf,int desc_size,int *to_fid);
extern void dcs_log(void *ptrbytes, int nbytes,const char * message,...);
extern void dcs_debug(void *ptrbytes, int nbytes,const char * message,...);
extern int  dcs_log_open(const char * logfile, char *ident);
extern int dcs_delete_shm(void);
extern int dcs_create_shm(int tblrows);
extern int dcs_connect_shm(void);
#endif
