#ifndef __IBDCS__H__

#define __IBDCS__H__



#include "folder.h"

//#include "iso8583.h"


#define IBDCS_SHM_NAME      "IbDC"

#define MONITOR_FOLD_NAME   "Monitor"

#define APPSRV_FOLD_NAME   "ISAN"

#define DCSCOM_FOLD_NAME    "DCSBANK"


#define  DELIMITER     " \t=;"

/*IBDCSÏµÍ³ÅäÖÃ²ÎÊı*/



#define DCS_KEY_SIZE  8

typedef struct tagIbdcsCfg

{

    /*¹²ÏíÄÚ´æÖĞ³¬Ê±±íµÄ¼ÇÂ¼Êı*/

    int iTbTRows;                  /* ³¬Ê±±íÖĞµÄ¼ÇÂ¼ÌõÊı*/



    /*³¬Ê±Ê±¼ä*/

    int iT1;  /*½¨Á´ÇëÇóµÄ³¬Ê±*/

    int iT2;  /*½áÊøÇëÇóµÄ³¬Ê±*/

    int iT3;  /*½»Ò×³¬Ê±µÄÇëÇó*/

    int    iRunMode; /*ÔËĞĞÄ£Ê½:0--µ÷ÊÔÄ£Ê½;1--Éú²úÄ£Ê½*/

} IBDCS_CFG;



#define TIMEOUT1        20

#define TIMEOUT2        21

#define TIMEOUT3        22

#define RUN_MODE        24



/*³¬Ê±±íµÄ¼ÇÂ¼¸ñÊ½*/

typedef struct eventqueue TBT_AREA;

typedef struct eventitem  TBT_BUF;



#define  TBT_STAT_USED          0x0001



#define  MAX_CHILDREN_NUM       100



/*layout of shared memory in IBDCS*/

typedef struct tagIbdcsShm

{

    int           is_semid;     /*º¬3¸ö·ÖÁ¿µÄĞÅºÅÁ¿;*/

                                /*0ºÅ·ÖÁ¿ÓÃÀ´»¥³â·ÃÎÊ¹²ÏíÄÚ´æÖĞµÄ*/

                                /*is_statÇø; 1,2ºÅ·ÖÁ¿ÓÃÀ´·ÃÎÊ³¬Ê±±í²¿·Ö*/

    int           is_MoniPid;   /*monitor½ø³ÌµÄpid*/

    int           is_nchld;     /*×Ó½ø³ÌµÄ¸öÊı*/

    int           is_children[MAX_CHILDREN_NUM]; /*¸÷×Ó½ø³ÌµÄpid*/

    IBDCS_CFG     is_config;    /*ÏµÍ³ÅäÖÃ²ÎÊıÇø*/

 //   TBT_AREA      is_tmTbl;     /*³¬Ê±±í*/

}IBDCSSHM;



#define MAX_IBDCS_MSG_SIZE 2048



struct IBDCSPacket

{

    short  pkt_cmd;     /*ÇëÇó°üÃüÁî,¶¨Òå¼ûÏÂ*/

    short  pkt_val;     /*ÇëÇóµÄÊı¾İ,º¬ÒåÒÀÀµÓÚ²»Í¬µÄÃüÁî*/

    long   pkt_bytes;   /*ÇëÇóÊı¾İÇøÖĞµÄ×Ö½ÚÊı*/

    char   pkt_buf[1];  /*ÇëÇóµÄÊı¾İÇø,º¬ÒåÒÀÀµÓÚ²»Í¬µÄÃüÁî*/

};

/*ÓÃÔÚ³¬Ê±¿ØÖÆÖĞ*/

struct savedtxn              /*±£´æÔ­Ê¼ÇëÇó±£ÎÄµÄÊı¾İ¸ñÊ½*/

 {

    int fromfid;   /* ¸Ã±¨ÎÄ´ÓÄÄ¸öÎÄ¼ş¼Ğ·¢ËÍÀ´µÄ*/

    int bytes ;    /*±¨ÎÄµÄ×Ö½Ú³¤¶È*/

    int databuf[1];   /* ±¨ÎÄÄÚÈİ*/

};



struct CFGTBL {

  char caSubSystem[8];		/* ×ÓÏµÍ³´úºÅ
*/
				            /* DCS:Í¨Ñ¶×ÓÏµÍ³
*/
				            /* TPA:½»Ò××ÓÏµÍ³
*/
				            /* ISA:½»»»×ÓÏµÍ³
*/
  int  iMaxForkNo;		    /* ÔÊĞíFORKµÄ×î´ó½ø³ÌÊı
*/
  int  iMinForkNo;		    /* µÚÒ»´ÎForkµÄ½ø³ÌÊı
*/
  int  iUnForkNo;		    /* Î´±»ForkµÄ½ø³ÌÊı
*/
  int  iUnFkIdx;		    /* ÏÂÒ»¸ö±»ForkµÄ½ø³ÌÔÚ½ø³Ì±íÖĞµÄË÷ÒıºÅ
*/
  char caPrgName[40];  /* ³ÌĞòÃû³Æ
*/
  char caPara[80];	    /* ³ÌĞòËùĞ:èÒªµÄ²ÎÊı
*/
  char caRemark[100];       /*Æô¶¯½ø³ÌÌáÊ¾ĞÅÏ¢
*/
};



#define PKT_CMD_TCPCONN         1000   /*tcpÁ´Â·×´Ì¬·¢Éú¸Ä±ä*/

#define PKT_CMD_DATAFROMSWITCH  1002   /*À´×Ô½»»»ÖĞĞÄµÄ½»Ò×Êı¾İ*/

#define PKT_CMD_CYHREQ          1003   /*·¢ËÍ³ÉÔ±ĞĞ¿ªÊ¼ÇëÇó*/

#define PKT_CMD_BUSIDOWN        1004   /*ÒµÎñÏµÍ³½áÊø½»Ò×*/

#define PKT_CMD_DATATOSWITCH    1005   /*·¢Íù½»»»ÖĞĞÄµÄ½»Ò×Êı¾İ*/

#define PKT_CMD_TIMER           1006

#define PKT_CMD_TMOUT           1007

extern IBDCSSHM     *g_pIbdcsShm;

extern IBDCS_CFG    *g_pIbdcsCfg;

extern TBT_AREA     *g_pTimeoutTbl;

extern unsigned long g_seqNo;



/*dcs_log.c*/

int  dcs_log_open(const char * logfile, char *ident);

int  dcs_set_logfd(int fd);

void dcs_log_close(void);

void dcs_log(void *pbytes, int nbytes,const char * message, ...);

void dcs_debug(void *pbytes, int nbytes,const char * message, ...);

void dcs_dump(void *pbytes, int nbytes,const char * message,...);



/*int dcs_chkmsg.c*/



/*dcs_shminit.c*/

int dcs_delete_shm(void);

int dcs_create_shm(int tblrows);

int dcs_connect_shm(void);



/*dcs_sysconf.c*/

int dcs_load_config(IBDCS_CFG *pIbdcsCfg);

int dcs_set_sysconf(int which, int val);

int dcs_dump_sysconf(FILE *fpOut);



/*dcs_sysstat.c*/

int dcs_get_sysstat(int which, int* val);

int dcs_set_sysstat(int which, int val);

int dcs_dump_sysstat(FILE *fpOut);



/*dcs_seqno.c*/

int dcs_save_seqNo(void);

unsigned long dcs_make_seqNo(void);



/*from dcs_sock.c*/

extern int tcp_open_server(const char *listen_addr, int listen_port) ;

extern int tcp_accept_client(int listensock,int *cliaddr, int *cliport);

extern int tcp_connet_server(char *servaddr, int servport, int cliport);

extern int tcp_close_socket(int sockfd);

extern int tcp_check_readable(int conn_sockfd,int ntimeout);

extern int tcp_check_writable(int conn_sockfd,int ntimeout);

extern int tcp_read_nbytes(int conn_sock, void *buffer, int nbytes);

extern int tcp_write_nbytes(int conn_sock, const void *buffer, int nbytes);

extern int read_msg_from_net(int connfd,void *msgbuf,int nbufsize,int ntimeout);

extern int write_msg_to_net(int connfd,void *msgbuf, int nbytes,int ntimeout);

extern int tcp_pton(char *strAddr);



#endif /*__IBDCS__H__*/

