#include "ibdcs.h"
//#include "iso8583.h"
#include "tmcibtms.h"
#include "dbstruct.h"
#include "base.h"
#include "var.h"
#include "general_util.h"
#include <string.h>
#include <stdlib.h>

char gs_fold_name[40];
static int OpenLog(char *IDENT) {
    char logfile[256];

    //assuming the log file is "$FEP_RUNPATH/log/appsvr1.log"
    if(u_fabricatefile("log/sign.log",logfile,sizeof(logfile)) < 0)
        return -1;
    return dcs_log_open(logfile, IDENT);
}

int main(int argc, char *argv[]) {
    int iDescFid;
    char buff[255];
    message_define *priv_def;
    char *g_pcBcdaShmPtr;
    char g_Snd_Buffer[4096];
    int g_SndBufferSize;
    struct tm *time_tm;
    glob_msg_stru pub_data_stru;
    time_t time_cl ;
    if(argc != 2) {
        printf("入口参出错，要求带机构号！");
        return -1;
    }
    if(strlen(argv[1]) > 11) {
        printf("入口参出错，机构号不能大于11位！");
        return -1;
    }
    u_daemonize(NULL);
    if(0 > OpenLog(argv[0])) {
        exit(1);
    }
    if(dcs_connect_shm() < 0) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d>dcs_connect_shm() failed:%s\n",strerror(errno),__FILE__,__LINE__);
        exit(1);
    }
    if(0 > DasConectDB()) { /* 连接Oracle数据库 */
        dcs_log(0, 0, "<Entry>Can not open oracle DB !");
        return -1;
    }
    g_pcBcdaShmPtr = (char *)shm_connect("BCDA",NULL);
    if(g_pcBcdaShmPtr == NULL) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d><appSrv> cannot connect shared memory 'BCDA'!",__FILE__,__LINE__);
        return(-1);
    }
    if(fold_initsys() < 0) {
        dcs_log(0,0, "<FILE:%s,LINE:%d>cannot attach to folder system!",__FILE__,__LINE__);
        return -1;
    }
    if(0> load_config()) return -1;
    strcpy(pub_data_stru.route_insti_code, argv[1]);
    if(0>get_route_insti_info(&pub_data_stru)) { //获取路由机构信息
        dcs_log(0,0, "<FILE:%s,LINE:%d>get_route_insti_info error!", __FILE__, __LINE__);
        return -1;
    }
    strcpy(pub_data_stru.route_trans_type, "9999");
    strcpy(pub_data_stru.timeout_table.flag, "5");
    memcpy(pub_data_stru.in_msg_type, "TLZZ",4);
    pub_data_stru.req_flag=1;
    pub_data_stru.timeout = 60;
    pub_data_stru.step_type = 1;
    if(0>conver_data(&pub_data_stru, 0)) { //数据格式转换
        dcs_log(0,0, "<FILE:%s,LINE:%d>conver_data error!", __FILE__, __LINE__);
        return -1;
    }
    if(0>insert_timeout_table(&pub_data_stru, 1)) {
        dcs_log(0,0,"<app_proc> insert_timeout_table error!");
        return -1;
    }
    priv_def = match_priv_stru(pub_data_stru.route_msg_type,&gl_def_set); //根据进入的报文类型找到对应的解析数据字典
    if(priv_def == NULL) {
        dcs_log(0,0,"<app_proc> match_priv_stru error! route_msg_type=[%s]",pub_data_stru.route_msg_type);
        return -1;
    }
    if(0>get_route_trans_set(&pub_data_stru)) {
        dcs_log(0,0,"<app_proc> get_route_trans_set error! ");
        return -1;
    }
    pub_data_stru.route_priv_def=priv_def;
    dcs_debug(0,0,"begin pack_msg");
    g_SndBufferSize = pack_msg(&pub_data_stru, g_Snd_Buffer, sizeof(g_Snd_Buffer));
    iDescFid= fold_locate_folder(pub_data_stru.route_fold_name);
    if(iDescFid  <0) {
        dcs_log(0,0, "接收机构fold ID[%s]出错！", pub_data_stru.route_fold_name);
        return -1;
    }
    if(0 > fold_write(iDescFid, -1, g_Snd_Buffer, g_SndBufferSize)) {
        dcs_log(g_Snd_Buffer, g_SndBufferSize, "<FILE:%s,LINE:%d>Failure to write msg to folder[%d]!",__FILE__,__LINE__, "PBSP");
        return -1;
    } else
        dcs_debug(0, 0, "<FILE:%s,LINE:%d>write msg[%d] to folder[%d]!",__FILE__,__LINE__, g_SndBufferSize, iDescFid);
    return 1;
}

