#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "base.h"
#include "tools.h"
#include "general_util.h"
typedef struct {
    int top;
    char cache[10][64];
    char flag[10][3];
} XML_STACK;

int xml_init_stack(XML_STACK *stack) {
    if(stack == NULL) return 0;
    stack->top=0;
    return 1;
}

int  xml_stack_is_empty(XML_STACK *stack) {
    if(stack == NULL) return 1;
    if(stack->top == 0) return 1;
    return 0;
}
char * xml_pop_stack(XML_STACK *stack,char *flag) {
    if(stack == NULL) return NULL;
    if(stack->top <= 0) return NULL;
    stack->top--;
    memcpy(flag,stack->flag[stack->top],3);
    return stack->cache[stack->top];
}

int xml_push_stack(XML_STACK *stack,char *str,char *flag) {
    if(stack == NULL || str == NULL) return 0;
    if(stack->top >=10) return 0;
    if(strlen(str) >=64) return 0;
    strcpy(stack->cache[stack->top],str);
    memcpy(stack->flag[stack->top],flag,3);
    stack->top++;
    return 1;
}

int xml_unpack(const char *msg_type, message_define *priv_def,char *src_buf,int src_len,glob_msg_stru *pub_data_stru) {
    XML_STACK stack;
    char flag[4],*p,tmp[512],curr_mark[64+1];
    int n,len;
    //初始化栈;
    strcpy(flag,"000");
    xml_init_stack(&stack);
    curr_mark[0]=0x00;
    len=0;
    tmp[0]=0x00;
    for(p=src_buf,n=src_len; *p&& n>0; p++,n--) {
//      fprintf(stderr,"flag=[%s],tmp=[%s]\n",flag,tmp);
        if(flag[0] == '0') { // 初始状态
            if(flag[1] =='0') { //无序状态
                if(*p == '<') {
                    flag[1]='1';
                    tmp[len++]=*p;
                    tmp[len]=0x00;
                }
            } else if(flag[1] =='1') {
                if(*p == '!') { // 进入注释标记匹配
                    flag[1]='2';
                    tmp[len++]=*p;
                    tmp[len]=0x00;
                } else if(*p == '?') { // 进入版本控制标记匹配
                    flag[1]='3';
                    tmp[len++]=*p;
                    tmp[len]=0x00;
                } else
                    tmp[len++]=*p;
                flag[0]='1';
            } else { // 程序存在异常
                dcs_log(0,0,"<%s>program error flag=[%s]!\n",__FUNCTION__,flag);
                return -1;
            }
        } else if(flag[0] == '1') { // 标记头处理状态

            if(flag[1] =='1') { // 正常标记匹配
                tmp[len++]=*p;
                tmp[len]=0x00;
                if(*p=='>') {
                    tmp[len-1]=0x00;
                    if(len >64) {
                        dcs_log(0,0,"<%s> mark too long! len=[%d][%s]",__FUNCTION__,len,tmp);
                        return -1;
                    }
                    memcpy(curr_mark,tmp+1,len-2);
                    curr_mark[len-2]=0x00;
                    dcs_debug(0,0,"new mark [%s]\n",curr_mark);
                    flag[0]='2';
                    len=0;
                }
            } else if(flag[1] =='2') { // 注释标记匹配
                tmp[len++]=*p;
                tmp[len]=0x00;
                if(len>3 && memcmp(p, "-->",3)==0) {
                    flag[0]='0';
                    flag[1]='0';
                    p++;
                    n--;
                    p++;
                    n--;
                    dcs_debug(0,0,"remark=[%s]\n",tmp);
                    len=0;
                }
            } else if(flag[1] =='3') { // 版本控制标记匹配
                tmp[len++]=*p;
                tmp[len]=0x00;
                if(len>2 && memcmp(p, "?>",2)==0) {
                    flag[0]='0';
                    flag[1]='0';
//                  p++;n--;
                    dcs_debug(0,0,"control=[%s] flag=[%s]\n",tmp,flag);
                    len=0;
                }
                //可对版本控制内容做处理
            } else { //程序错误
                dcs_log(0,0,"<%s> programe error ! flag=[%s]\n",__FUNCTION__,flag);
                return -1;
            }
        } else if(flag[0] == '2') { // 内容处理状态
            if(*p == 0x20) continue;
            tmp[len++]=*p;
            tmp[len]=0x00;
            if(memcmp(p,"</",2)==0) {
                flag[2]='1';
                tmp[len-1]=0x00;
                flag[0]='3';
                p++;
                n--;
                //  在这可将TMP数据进行处理, 读栈顶
                dcs_debug(0,0,"flag=[%s] <%s>,tmp=[%s]\n",flag,curr_mark,tmp);
                if(0>add_pub_field(pub_data_stru,get_pub_field_id(msg_type,curr_mark),
                                   msg_type,len-1,tmp,0)) {
                    dcs_log(0,0,"<%s> add_pub_field error! mark=[%s] len=%d,data=[%s] ",__FUNCTION__,curr_mark,len-1,tmp);
                    return -1;
                }
                len=0;
                continue;
            }
            if(*p == '<') { // 进入新的标记处理
                len=0;
                tmp[len++]=*p;
                tmp[len]=0x00;
                if(!xml_push_stack(&stack,curr_mark,flag)) { //普通标记压栈
                    return -1;
                }
                flag[0]='0';
                flag[1]='1';
            }
            if(len > 510) return -1;
        } else if(flag[0] == '3') { //标记尾处理状态
            tmp[len++]=*p;
            tmp[len]=0x00;
//          fprintf(stderr,"*p=[%c] ,tmp=[%s]\n",*p,tmp);
            if(*p =='>') {
                //弹栈，与弹出数据进行匹配
                char *s=NULL;
                tmp[len-1]=0x00;
                if(strcmp(tmp,curr_mark) !=0) {
                    dcs_log(0,0,"<%s>no match tmp=[%s] mark=[%s]\n",__FUNCTION__,tmp,curr_mark);
                    return -1;
                }
                dcs_debug(0,0,"mark=[%s] match succ!\n",tmp);
                s=xml_pop_stack(&stack,flag);
                if(s == NULL) {
                    //解析出错
//                  fprintf(stderr,"parase error stack is NULL !");
                    strcpy(flag,"000");
                    curr_mark[0]=0x00;
                    continue;
//                  return -1;
                }
                strcpy(curr_mark,s);
                dcs_debug(0,0,"stack pop mark=[%s] !\n",curr_mark);
            }
        }
    }
    if(!xml_stack_is_empty(&stack)) {
        dcs_log(0,0,"parse end ,stack is not null!\n");
        return -1;
    }
    return 0;
}

