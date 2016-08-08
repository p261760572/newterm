#ifndef _DBSTRUCT_H_
#define _DBSTRUCT_H_

#define DAS_NORMAL			0
#define DAS_NOT_FOUND_ERR	-1
#define DAS_DUPLICATE_ERR	-2
#define DAS_PARA_ERR		-3
#define DAS_NO_MORE_ERR		-4
#define DAS_CALL_SEQ_ERR	-5
#define DAS_OTHER_ERR		-6

#define  ROLLBACK	0
#define  COMMIT		1


struct TimeOut_REC
{
  int    iRcvFolderId;		 
  int    iOrgFolderId;		 
  int    iFlags;           
  int    iMaxTimeout;			
  long   iExpiredTime;    
  char   caKey[50];	     
  char   caRemark[80];
};

#endif

