#include "base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ibdcs.h"
#include "var.h"
extern int get_msgtype_total(void);
extern int load_priv_def(void);

int init_priv_def() {
    gl_def_set.num=0;
    gl_def_set.priv_def=NULL;
    return 1;
}
int load_config(void) {
    int i;

    init_priv_def();
    i=get_msgtype_total();
    if(i <=0 || i >100) { //报文种类不能超过100种
        dcs_log(0,0,"<%s>get msg type total fail total=%d",__FUNCTION__,i);
        return -1;
    }
    gl_def_set.num=i;

    gl_def_set.priv_def=malloc(sizeof(message_define)*i);
    if(gl_def_set.priv_def == NULL) {
        dcs_log(0,0,"<load_config> priv_def malloc fail!");
        return -1;
    }

    dcs_debug(0,0,"<load_config> malloc priv_def succ! i=[%d] sizeof=%d",i,sizeof(message_define)*i);
    memset(&gl_def_set.priv_def[0],0,sizeof(message_define)*i);
    if(0>load_priv_def()) { //获取各类报文的定义结构
        for(i=0; i<gl_def_set.num; i++)
            if(gl_def_set.priv_def[i].fld_def != NULL) {
                free(gl_def_set.priv_def[i].fld_def);
            } else break;
        free(gl_def_set.priv_def);
        dcs_log(0,0,"load config fail!");
        return -1;
    }
    dcs_debug(0,0,"load config succ!");
// load 应用检查限制处理模块
    return 0;
}
void free_priv_def() {
    int i;
    if(gl_def_set.priv_def != NULL) {
        for(i=0; i<gl_def_set.num; i++)
            if(gl_def_set.priv_def[i].fld_def != NULL) {
                free(gl_def_set.priv_def[i].fld_def);
            }
        free(gl_def_set.priv_def);
    }
}