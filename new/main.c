//#include "ibdcs.h"
#include "folder.h"
#include "util_func.h"
#include "db_tools.h"
#include "tools.h"
char   *g_pcBcdaShmPtr;             // �����ڴ�BCDA���׵�ַ
static int gs_myFid    = -1;
static void TermSvrExit(int signo); /* �źŴ����� */
static int OpenLog(char *IDENT);
static int CreateMyFolder(void);
static int DoLoop(void);
static int gs_work_flag = 0;
static volatile sig_atomic_t srv_shutdown = 0;
static int child = 0,exitFlag=0;
static void show_version(void) {
    char *b = " -  application server\n" \
              "Build-Date: " __DATE__ " " __TIME__ "\n";
    write(STDOUT_FILENO, b, strlen(b));
}
static void show_help(void) {
    write(STDOUT_FILENO, "�����������!", strlen("�����������!"));
}
char gs_buffer[8192],gs_fold_name[40];
int gs_len;

int main(int argc, char *argv[]) {
    int dont_daemon=0,o,num_childs=1,i;
    char buf[128];

    while(-1 != (o = getopt(argc, argv, "hvDn:"))) {
        switch(o) {
            case 'D':
                dont_daemon=1 ;
                break;
            case 'v':
                show_version();
                return 0;
            case 'h':
                show_help();
                return 0;
            case 'n':
                if(atoi(optarg)>0)
                    num_childs=atoi(optarg);
                break;
            default:
                show_help();
                return -1;
        }
    }
    if(dont_daemon)
        u_daemonize(NULL); /* ת��Ϊ��̨���� */
    if(0 > OpenLog(argv[0])) { /* ������־�ļ� */
        fprintf(stderr,"open log file [%s] fail!",argv[0]);
        exit(1);
    }
    catch_all_signals(TermSvrExit); /* ע���źŴ����� */
    exitFlag = 0;
    //prepare the logging stuff


    dcs_log(0, 0, "<%s>Terminal Servers is starting up...",__FUNCTION__);
    //attach to SHM of IBDCS



    while(!child && !srv_shutdown) {
        if(num_childs > 0) {

            switch(fork()) {
                case -1:
                    return -1;
                case 0:
                    child = 1;
                    usleep(1000);
//                      dcs_debug(0,0,"i am is child");
                    break;
                default:
                    num_childs--;
                    break;
            }
        } else {
            int status;

            dcs_debug(0,0,"master process wait child die!");
            if(-1 !=wait(&status)) {
                /**
                 * one of our workers went away
                 */
                num_childs++;
            } else {
                switch(errno) {
                    case EINTR:
                        if(!srv_shutdown) {
                            srv_shutdown = 1;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }

    if(!child) {
        if(srv_shutdown) {
            dcs_log(0,0,"<%s> master process prepare kill childs!",__FUNCTION__);
            kill(0, SIGTERM);
        }
        dcs_log(0,0,"<%s> master process exit!",__FUNCTION__);
        TermSvrExit(0);
    }

    if(dcs_connect_shm() < 0) { /* ���ӹ����ڴ�IBDCS_SHM_NAME "IbDC" ? */
        dcs_log(0, 0, "<%s>dcs_connect_shm() failed:%s\n",__FUNCTION__, strerror(errno));
        exit(1);
    }
    g_pcBcdaShmPtr = (char *)shm_connect("BCDA",NULL); /*  ���ӹ����ڴ�BCDA?*/
    if(g_pcBcdaShmPtr == NULL) {
        dcs_log(0, 0, "<%s>cannot connect shared memory 'BCDA'!",__FUNCTION__);
        TermSvrExit(0);
    }

    //attach to folder system and create folder of myself
    if(CreateMyFolder() < 0)
        TermSvrExit(0);
    sprintf(buf,"%s:",argv[0]);
    strcpy(argv[0],buf);
    for(i=1; i< argc; i++)
        strcpy(argv[i]," ");

    if(0 > DasConectDB()) { /* ����Oracle���ݿ� */
        dcs_log(0, 0, "<%s>Can not open oracle DB !",__FUNCTION__);
        TermSvrExit(0);
    }
    //do my main logic in a infinite loop
    dcs_log(0, 0, "*************************************************\n"
            "!!        term servers startup completed.        !!\n"
            "*************************************************\n");
    DoLoop();

    TermSvrExit(0);
    return 0;
}//main()

static void TermSvrExit(int signo) {
//    int i;
//    if(signo > 0)
    dcs_log(0,0,"<%s>catch a signal %d\n",__FUNCTION__,signo);
    if(signo >31)
        return;
    if(signo == 15) { //�����˳�
        srv_shutdown = 1;  //���ؽ������˳���ʶ
        if(!child) {
            return;
        } else {
            exitFlag = 1;  //�����������˳���ʶ
            if(gs_work_flag) return;   //�������������ִ�����񣬵�ִ�����������˳�
        }
    }
    if(signo==SIGCHLD) return;
    if(signo == SIGPIPE ||
       signo == SIGALRM ||
       signo == SIGURG  ||
       signo == SIGIO   ||
       signo == SIGVTALRM ||
       signo ==SIGPROF) return;

    if(g_pcBcdaShmPtr)
        shm_detach((char *)g_pcBcdaShmPtr);
    if(child) {
        DasEnd(0);
        dcs_log(0,0,"<%s> child AppServer terminated.\n",__FUNCTION__);
        free_priv_def();
    } else
        dcs_log(0,0,"<%s> master process terminated.",__FUNCTION__);
    exit(signo);
}

static int OpenLog(char *IDENT) {
    char logfile[256],tmp[128];

    //assuming the log file is "$FEP_RUNPATH/log/appsvr1.log"
    sprintf(tmp,"log/%s.log",IDENT);
    if(u_fabricatefile(tmp,logfile,sizeof(logfile)) < 0)
        return -1;

    return dcs_log_open(logfile, IDENT);
}

static int CreateMyFolder(void) {
    if(fold_initsys() < 0) {
        dcs_log(0,0, "<%s>cannot attach to folder system!",__FUNCTION__);
        return -1;
    }

    gs_myFid = fold_create_folder("APPL");
    if(gs_myFid < 0)
        gs_myFid = fold_locate_folder("APPL");
    if(gs_myFid < 0) {
        dcs_log(0,0,"<%s>cannot create folder '%s':%s\n",
                __FUNCTION__,"APPL", strerror(errno));
        return -1;
    }

    if(fold_get_maxmsg(gs_myFid) <2)
        fold_set_maxmsg(gs_myFid, 20) ;

    return 0;
}

static int DoLoop(void) {
    char   *caBuffer,msg_type[5];
    int    iRead, fromFid,toFid,len;
    time_t t1,t2;
    if(0> load_config()) {
        dcs_log(0,0,"<%s>load_config fail!",__FUNCTION__);
        return -1;
    }
    for(;;) {
        //read from my folder in blocking mode
        if(exitFlag) break;
        gs_work_flag =0;
        memset(gs_buffer, 0, sizeof(gs_buffer));
        iRead = fold_read(gs_myFid, &fromFid, gs_buffer, sizeof(gs_buffer), 1);
        if(iRead < 0) {
            dcs_log(0,0,"fold_read() failed:%s\n",strerror(errno));
            break;
        }

        if(iRead >= 10)
            dcs_debug(gs_buffer, iRead,"<%s>recvice from my folder myid=%d, fromid=%d, len =[%d]", __FUNCTION__,gs_myFid, fromFid, iRead);
        else {
            dcs_log(gs_buffer,iRead,"��Ч�����ݰ�");
            continue;
        }
        gs_work_flag =1;
        caBuffer=gs_buffer;
        gs_len=iRead;
        gs_fold_name[0]=0x00;
        fold_get_name(fromFid, gs_fold_name,sizeof(gs_fold_name));
//              ICS_DEBUG(1);
        if(memcmp(caBuffer,"TIME",4)==0) { //���׳�ʱ����
            len= app_timeout_proc(caBuffer+4,iRead-4,gs_buffer+sizeof(time_t),
                                  sizeof(gs_buffer)-sizeof(time_t),&toFid);
        } else {
            memcpy(msg_type,caBuffer,4);
            msg_type[4]=0x00;
            memcpy(&t1,(caBuffer+4),sizeof(time_t));
            t2=time(NULL);
            if(t2-t1 > 5) {
                dcs_log(gs_buffer,iRead,"<%s> Expired message ,discard!" \
                        "t1=[%ld],t2=[%ld]",__FUNCTION__,t1,t2);
                continue;
            }
            caBuffer = caBuffer+4+sizeof(time_t);
            len=app_proc(msg_type,caBuffer,iRead-4-sizeof(time_t),
                         gs_buffer+sizeof(time_t),
                         sizeof(gs_buffer)-sizeof(time_t),&toFid);
        }
        if(len >0) {  //���ⷢ��
            t1=time(NULL);
            memcpy(gs_buffer,&t1,sizeof(time_t));
            dcs_debug(gs_buffer+sizeof(time_t),len,"send data fid=%d",toFid);
            iRead= fold_write(toFid,gs_myFid,gs_buffer,len+sizeof(time_t));
            if(iRead <=0) {
                dcs_log(caBuffer,len+sizeof(time_t),"<%s> send to fid[%d] message fail!",__FUNCTION__,toFid);
            }

        } else if(len == 0) { // ���ش�����������
            continue;
        } else { //����ʧ��
            dcs_log(caBuffer,iRead,"<%s>can not process message!",__FUNCTION__);
        }
    }//for(;;)

    return -1;
}
