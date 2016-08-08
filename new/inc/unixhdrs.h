#ifndef __UNIHDRS__H__
#define __UNIHDRS__H__

#include    <stdio.h>
#include    <stdlib.h>
#include    <sys/types.h>   /* basic system data types */
#include    <sys/socket.h>  /* basic socket definitions */
#include    <sys/time.h>    /* timeval{} for select() */
#include    <time.h>        /* timespec{} for pselect() */
#include    <netinet/in.h>
#include    <arpa/inet.h>   /* inet(3) functions */
#include    <errno.h>
#include    <fcntl.h>       /* for nonblocking */
#include    <netdb.h>
#include    <signal.h>
#include    <sys/uio.h>     /* for iovec{} and readv/writev */
#include    <unistd.h>
#include    <sys/un.h>      /* for Unix domain sockets */
#include    <sys/ipc.h>
#include    <sys/sem.h>
#include    <sys/msg.h>
#include    <sys/shm.h>
#include    <sys/stat.h>
#include    <sys/mman.h>
#include    <sys/wait.h>
#include    <stdarg.h>
#include    <string.h>

#ifndef TRUE
    #define TRUE (1==1)
    #define FALSE (1==0)
#endif

#ifndef max
    #define  max(a,b)   (((a) > (b)) ? (a) : (b))
    #define  min(a,b)   (((a) < (b)) ? (a) : (b))
#endif
    
typedef void sigfunc_t(int);

#endif //__UNIHDRS__H__


