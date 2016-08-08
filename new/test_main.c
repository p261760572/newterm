//#include "ibdcs.h"
#include "folder.h"
#include "util_func.h"
#include "db_tools.h"
#include "tools.h"
char   *g_pcBcdaShmPtr;             // 共享内存BCDA的首地址
static int gs_myFid    = -1;
static int OpenLog(char *IDENT);
GLOB_DEF gl_def_set;

char gs_buffer[8192],gs_fold_name[40];
int gs_len;

int main(int argc, char *argv[])
{
    int dont_daemon=0,o,num_childs=1,i;
    char buf[128];

    if(0 > OpenLog(argv[0])) /* 创建日志文件 */
    {
    	fprintf(stderr,"open log file [%s] fail!",argv[0]);
    	exit(1);
    }

    g_pcBcdaShmPtr = NULL;

    return 0;
}//main()



static int OpenLog(char *IDENT)
{
    char logfile[256],tmp[128];
    
 
    sprintf(tmp,"log/%s.log",IDENT);
    if(u_fabricatefile(tmp,logfile,sizeof(logfile)) < 0)
        return -1;

    return dcs_log_open(logfile, IDENT);
}




