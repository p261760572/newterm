
#include Make.defines
LIBS=$(LIB) -L../lib  -ldcsutil -lfold
BIN =$(ICS_HOME)/bin
CFLAGS=-I ./inc 
ORACLE_LIB= -L$(ORACLE_HOME)/lib -lsqlplus -lclntsh

#EXE = TransRcv 
EXE =  termsvr timescanner sign 
#LIBOBJ=main.o request_proc.o respose_proc.o app_proc.o unpack.o util.o \
#      pack.o tpos_unpack.o load_config.o timeout_proc.o db_qfunc.o db_ifunc.o db_ufunc.o \
#      db_dfunc.o db_dssfunc.o iso_pack.o iso_unpack.o tpos_pack.o  xml_pack.o xml_unpack.o util_db.o \
#      tpos_util.o db_tpos_qfunc.o db_tpos_ufunc.o zpos_util.o db_zpos_qfunc.o secuLib.o
OBJ=main.o  load_config.o define_data.o db_dssfunc.o app_proc.o db_qfunc.o \
		timeout_proc.o general_util.o iso_pack.o iso_unpack.o tpos_pack.o  xml_pack.o xml_unpack.o \
		util_db.o tpos_util.o db_tpos_qfunc.o db_tpos_ufunc.o zpos_util.o db_zpos_qfunc.o \
		secuLib.o db_ifunc.o db_ufunc.o request_proc.o respose_proc.o  \
		unpack.o pack.o tpos_unpack.o db_user_func.o db_dfunc.o db.o
OBJ1= load_config.o define_data.o db_dssfunc.o app_proc.o db_qfunc.o \
		timeout_proc.o general_util.o iso_pack.o iso_unpack.o tpos_pack.o  xml_pack.o xml_unpack.o \
		util_db.o tpos_util.o db_tpos_qfunc.o db_tpos_ufunc.o zpos_util.o db_zpos_qfunc.o \
		secuLib.o db_ifunc.o db_ufunc.o request_proc.o respose_proc.o  \
		unpack.o pack.o tpos_unpack.o db_user_func.o db_dfunc.o db.o

.c.o:
	$(CC) -c -g -Wall $(CFLAGS) $*.c

exe:    $(EXE) cleanup
all:    $(OBJ)  $(EXE) cleanup


db_dssfunc.c:db_dssfunc.pc
	proc db_dssfunc.pc db_dssfunc.c unsafe_null=yes lines=yes  mode=oracle dbms=v8 include=./inc  
db_qfunc.c:db_qfunc.pc
	proc db_qfunc.pc db_qfunc.c unsafe_null=yes lines=yes  mode=oracle dbms=v8 include=./inc  
db_ifunc.c:db_ifunc.pc
	proc db_ifunc.pc db_ifunc.c unsafe_null=yes lines=yes  mode=oracle dbms=v8 include=./inc
db_ufunc.c:db_ufunc.pc
	proc db_ufunc.pc db_ufunc.c unsafe_null=yes lines=yes  mode=oracle dbms=v8 include=./inc
db_dfunc.c:db_dfunc.pc
	proc db_dfunc.pc db_dfunc.c unsafe_null=yes lines=yes  mode=oracle dbms=v8 include=./inc
db_tpos_qfunc.c:db_tpos_qfunc.pc
	proc db_tpos_qfunc.pc db_tpos_qfunc.c unsafe_null=yes lines=yes  mode=oracle dbms=v8 include=./inc
db_tpos_ufunc.c:db_tpos_ufunc.pc
	proc db_tpos_ufunc.pc db_tpos_ufunc.c unsafe_null=yes lines=yes  mode=oracle dbms=v8 include=./inc
db_zpos_qfunc.c:db_zpos_qfunc.pc
	proc db_zpos_qfunc.pc db_zpos_qfunc.c unsafe_null=yes lines=yes  mode=oracle dbms=v8 include=./inc
db_user_func.c:db_user_func.pc
	proc db_user_func.pc db_user_func.c unsafe_null=yes lines=yes  mode=oracle dbms=v8 include=./inc
timescanner.c:timescanner.pc
	proc timescanner.pc timescanner.c unsafe_null=yes lines=yes  mode=oracle dbms=v8 include=./inc
termsvr: $(OBJ)
	gcc -Wstrict-prototypes -Wimplicit-function-declaration -Wall -Werror -g -o termsvr $(CFLAGS) $(OBJ) $(LIBS) $(ORACLE_LIB)
	mv termsvr ../run/bin

timescanner:timescanner.c
	gcc -Wstrict-prototypes -o timescanner $(CFLAGS) timescanner.c db_dssfunc.c $(LIBS) $(ORACLE_LIB)
	mv timescanner ../run/bin
sign:sign.c 
	gcc -Wstrict-prototypes -o sign $(CFLAGS) sign.c $(OBJ1) $(LIBS) $(ORACLE_LIB)
	mv sign ../run/bin
test_util:test_main.c general_util.c secuLib.c db_tools.c
	gcc -Wstrict-prototypes -o test_util $(CFLAGS) test_main.c general_util.c secuLib.c db_tools.c $(LIBS) $(ORACLE_LIB)
cleanup:
	rm -f *.o dssdclfn.c db_*.c XmlMessage.c *.lis timescanner.c
#	rm -f *.o dssdclfn.c  XmlMessage.c *.lis timescanner.c




