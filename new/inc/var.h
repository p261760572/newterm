
#ifndef __VAR__
#define __VAR__

extern FLDSET_DEF gl_check_limit[];
extern struct DEFAULT_PROC gl_direct_limit[];
extern GLOB_DEF gl_def_set;

extern struct unpack_func_def unpack_func_proc[];
extern struct INSTI_CODE_PROC gl_insti_code[];
extern struct DEFAULT_PROC gl_route_proc[];
extern struct DB_PROC gl_db_iproc[];
extern struct DEFAULT_PROC gl_direct_proc[];
extern struct pack_func_def pack_func_proc[];
extern struct DEFAULT_PROC gl_db_qproc[];
extern struct DEFAULT_PROC gl_pre_field_conver[];
extern char gs_buffer[8192],gs_fold_name[40];
extern FLDSET_DEF gl_priv_field_conver[];
extern FLDSET_DEF gl_genrate_field_conver[];
extern int gs_len,exitFlag;
extern char   *g_pcBcdaShmPtr;
extern TAG_def TAG[];
extern FLDSET_DEF gl_special_business_handle[];
extern MAC_CALC_DEF gl_mac_calc[];
extern UPDATE_DB_DEF gl_update_db[];
extern FLDSET_DEF gl_check_and_match[];
extern FLDSET_DEF gl_timeout_handle[];
extern struct DEFAULT_PROC gl_terminfo_qproc[];
extern struct DEFAULT_PROC gl_end_proc[];
extern int  gl_dbg_len,gl_dbg_cnt,gl_fn_cnt;
extern char gl_dbg_buf[8192];
extern struct HEAD_PROC gl_head_proc[];
extern NOTIFY_DEF gl_end_nofify[];
extern FLDSET_DEF gl_advert_proc[];

#endif
