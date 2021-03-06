#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <json-c/json.h>
#include "base.h"
#include "tools.h"
#include "db_qfunc.h"
#include "general_util.h"
#include "var.h"
#include "tmcibtms.h"
#include "db_tools.h"
#include "secu.h"
#include "ibdcs.h"

#define MAX(a, b) ((a)>(b)?(a):(b))
#define MIN(a, b) ((a)<(b)?(a):(b))
#define _ATOI(a, b, c) {char _tmp[10]; bzero(_tmp, sizeof(_tmp)); memcpy(_tmp, a, b); c=atoi(_tmp);}

int my_substring(char *buf, int buf_len, char type, int start, int end, 
									char *data_out, int size, void (*func)(char *)) { 		
		int n;
		if(!buf || !buf[0] || start >= buf_len) {
				data_out[0] = '\0';
				return 0;
		}
		if(type == 0x00) 
			end = MIN(buf_len, start+end);
		if(type == 0x01)
			end = MIN(buf_len, end);
		if(end <= start) end = buf_len;
		
		n = MIN(size-1, end-start);
		memcpy(data_out, buf+start, n);
		data_out[n] = '\0';
		if(n>0 && func) 
			func(data_out);
		return strlen(data_out);
}

int my_separate(char *buf, int buf_len, const char div, int start, int end, 
								char *data_out, int size, void (*func)(char *)) {
		char *p, *s;
		int count = 0, len = 0, n;
		if(!buf || !buf[0]) {
				data_out[0] = '\0';
				return 0;
		}
		if(end < start) end = start;
		for(p = buf, s = p; count <= end; p++) {
	      if(*p == div || *p == '\0') {
        		if(count >= start) {
				 	      n = MIN(size-1-len, p-s); 				
				 	      if(n <= 0) break;
		            memcpy(data_out+len, s, n);
		            len += n;
          	}
          	if(*p == '\0')  break;
          	s = p + 1;
          	count++;
        }
    }
    
    data_out[len] = '\0';
    if(len>0 && func) 
			func(data_out);
		return strlen(data_out);
}

int my_date_time(char *src_time, int src_len, int type, char *format_time, int size) {
		struct tm *time_tm;
		time_t time_cl;
		int len = 0;
		
		if(src_len < 0) {
				time(&time_cl) ;
        time_tm = localtime(&time_cl);
        if(strftime(src_time, 20, "%Y%m%d%H%M%S",time_tm) == 0) return -1;
    }
    if(type == 1) {
		    memcpy(format_time + len, src_time, 4);
		    len += 4;
		    format_time[len++] = '-';
		    memcpy(format_time + len, src_time + 4, 2);
		    len += 2;
		    format_time[len++] = '-';
		    memcpy(format_time + len, src_time + 6, 2);
		    len += 2;
		    format_time[len++] = ' ';
		    memcpy(format_time + len, src_time + 8, 2);
		    len += 2;
		    format_time[len++] = ':';
		    memcpy(format_time + len, src_time + 10, 2);
		    len += 2;
		    format_time[len++] = ':';
		    memcpy(format_time + len, src_time + 12, 2);
		    len += 2;	
		}
    if(size < len) return -1;
    return len;
}

int format_msg_data(char *data, int data_len, char *format, int len_start, 
										int len_end, char *format_data, int size) {
		if(!format || !format[0] ) return -1;
		int format_len = 0;
		char format_type, div;
		
		format_type = format[0];
				
		if(format_type == '1') {											// 字符位截取
				format_len = my_substring(data, data_len, 1, len_start, len_end, format_data, size, rtrim);
		}
		else if(format_type == '2') {									// 字符分割截取
				div = format[1];
				format_len = my_separate(data, data_len, div, len_start, len_end, format_data, size, rtrim);
		}
		else if(format_type == '3') {											// 字符位截取
				format_len = my_substring(data, data_len, 1, len_start, len_end, format_data, size, NULL);
		}
		else if(format_type == '4') {									// 字符分割截取
				div = format[1];
				format_len = my_separate(data, data_len, div, len_start, len_end, format_data, size, NULL);
		}
		else if(format_type == '5') {									// 格式化卡号
				strncpy(format_data, data, size-1);
				format_data[size-1] = 0x0;
				if(data_len > 10) memset(format_data + 6, '*', data_len - 10);
				format_len = data_len;
		}
		else if(format_type == '6') {									// 格式化日期时间
				format_len = my_date_time(data, data_len, 1, format_data, size);
		}
		else if(format_type == '7') {									// 格式化金额
				if(data_len > 0) {
            if(data[0] >= '0' && data[0] <= '9')
                sprintf(format_data, "%.2f", atof(data)/100);
            else if(data[0]=='C')	                
                sprintf(format_data, "%.2f", atof(data+1)/100);
            else if(data[0]=='D')
                sprintf(format_data, "-%.2f", atof(data+1)/100);
           	format_len = strlen(format_data);
				}
		}
		else if(format_type == '8') {									// 格式化持卡人信息
				if(data_len > 0) {
            if(memcmp(data+2+1+1+7+1,"NM",2)==0) {
            		strncpy(format_data, data, size-1);
            		format_data[size-1] = 0x0;
			          format_data[2+1+1+7+1+30]=0x00;
			          rtrim(format_data+2+1+1+7+1);
			          format_len = strlen(format_data);
			      }
				}
		}
		else if(format_type == '9') {									// 格式化余额显示
				if(data_len > 0) {
            sprintf(format_data, "%.2f", (data[27] == 'D' ? -1 : 1) * atof(data + 28)/100);
            format_len = strlen(format_data);
				}
		}
		return format_len ;
}

void init_pub_data_stru(glob_msg_stru * pub_data_stru) {
    assert(pub_data_stru);  // 2016/7/20 星期三 下午 4:55:57
    memset(pub_data_stru, 0, sizeof(*pub_data_stru));
    pub_data_stru->src_buffer=NULL;
    pub_data_stru->src_len=0;
    pub_data_stru->req_flag=0; //1为请求交易，其他为应答交易
    pub_data_stru->switch_src_flag = 0; // 1为转接到原受理机构，其他为转接到其他目的机构
    pub_data_stru->insti_code[0]=0x00;    //发送报文的机构代码
    pub_data_stru->insti_fold_name[0] = 0x00;//发送报文通信用的fold名字
    pub_data_stru->insti_open_flag = 0x00;  //发送机构打开标识,0关闭,1打开
    pub_data_stru->insti_link_type =0x00;  //发送机构链路标识，0,长连接 1,短连接
    pub_data_stru->insti_work_type =0x00;  //发送机构交易接入模式 0,机构 ,1 终端
    pub_data_stru->in_priv_def = NULL;    //进入报文种类数据域定义指针
    pub_data_stru->in_msg_type[0] = 0x00; //进入报文的报文类型
    pub_data_stru->in_mastkey[0] = 0x00;
    pub_data_stru->in_mac_index[0] = 0x00;
    pub_data_stru->in_mac_key[0] = 0x00;
    pub_data_stru->in_pin_index[0] = 0x00;
    pub_data_stru->in_pin_key[0] = 0x00 ;
    pub_data_stru->in_data_index[0] = 0x00;
    pub_data_stru->in_data_key[0] = 0x00 ;
    pub_data_stru->is_check_mac = 0x00 ;//是否校验mac
    pub_data_stru->msg_key[0] = 0x00 ; //报文关键字
    pub_data_stru->app_type[0] = 0x00 ; //业务类型
    pub_data_stru->in_trans_type[0] = 0x00;//进入交易类型
    pub_data_stru->open_flag =0x00;//业务打开标识 0关闭，1打开
    pub_data_stru->is_route = 0x00;//是否路由 0：转发初始接入点，1：业务下一路由节点，2：处理结束,3:原始交易路由节点
    pub_data_stru->route_num = 0; //交易被路由的次数
    pub_data_stru->step_type = 1; // 1 支付渠道，2业务渠道
    pub_data_stru->route_insti_code[0] = 0x00;// 路由机构代码
    pub_data_stru->route_fold_name[0] = 0x00;//路由报文通信用的fold名字
    pub_data_stru->route_is_make_mac=0x00;//是否产生mac
    pub_data_stru->route_insti_open_flag=0x00;//路由机构打开标识 0关闭,1打开
    pub_data_stru->route_insti_link_type=0x00;  //路由机构链路标识，0,长连接 1,短连接
    pub_data_stru->route_insti_work_type = 0x00;  //路由机构交易接入模式 0,机构 ,1 终端
    pub_data_stru->route_is_check_mac = 0x00;
    pub_data_stru->route_msg_type[0] = 0x00;//路由转换的报文类型
    pub_data_stru->route_trans_type[0] = 0x00; //路由转换的交易类型
    pub_data_stru->route_priv_def = NULL;       //路由的报文种类数据域定义指针
    pub_data_stru->route_set.num=0;//转发的报文数据域ID集合
    pub_data_stru->route_mastkey[0] = 0x00;
    pub_data_stru->route_mac_index[0] =0x00;
    pub_data_stru->route_mac_key[0]= 0x00;
    pub_data_stru->route_pin_index[0] = 0x00;
    pub_data_stru->route_pin_key[0] =0x00 ;
    pub_data_stru->route_data_index[0] = 0x00;
    pub_data_stru->route_data_key[0] = 0x00;
    pub_data_stru->center_result_code[0] = 0x00;//中心处理结果代码
    pub_data_stru->is_void = 0x00;             //是否需要冲正处理 ,0不冲正，1需要
    pub_data_stru->permit_void = 0x00;         // 应用是否许可受理方发起冲正
    pub_data_stru->use_timeout = 0x00 ;         //使用超时表标志
    pub_data_stru->timeout_table.foldname[0]=0x00;//超时表存放的数据
    pub_data_stru->timeout_table.sys_date[0]=0x00;
    pub_data_stru->timeout_table.sys_time[0]=0x00;
    pub_data_stru->timeout_table.key[0]=0x00;
    pub_data_stru->timeout_table.flag[0]=0x00;
    pub_data_stru->timeout_table.num=0;
    pub_data_stru->timeout_table.invalid_time=0;
    pub_data_stru->timeout_table.first_key[0]=0x00;
    pub_data_stru->timeout_table.trans_type[0]=0x00;
    pub_data_stru->timeout_table.remark[0] = 0x00;
    pub_data_stru->filed_key_num=0; //关键域字段个数
//  in_filed_key_field_id[5];//报文关键域字段组合
    pub_data_stru->msg_field_num = 0; //报文域总数
    pub_data_stru->timeout = 0;
//  memset(&pub_data_stru->data_rec[0],0x00,sizeof(field_data)*MAX_FIELD) ; //报文域数据解析
    memset(pub_data_stru->tmp_order,0x00,5);         //特殊报文使用的指令表示区
    pub_data_stru->in_cry_flag=0x00; // 进入国密算法应用标识 默认0不应用，1应用
    pub_data_stru->out_cry_flag=0x00; //出去国密算法应用标识 默认0不应用，1应用
    pub_data_stru->off_set=0;         //buffer使用偏移量
    pub_data_stru->buffer[0]=0x00; //数据存放区

}
/**
功能：匹配报文类型所对应的域定义规则集合，并返回集合指针
**/
message_define *match_priv_stru(const char *msg_type,GLOB_DEF * gl_def) {
    int i;

    assert(msg_type) ;      //2016/7/20 星期三 下午 4:56:02
    assert(gl_def) ;    // 2016/7/20 星期三 下午 4:56:07
    for(i=0; i<gl_def->num; i++) {
        if(memcmp(msg_type,gl_def->priv_def[i].msg_type,4)==0)
            return &gl_def->priv_def[i];
    }
    return NULL;
}


/*
 from： 0,报文，1中心，2数据库
 对于往同一个数据域增加多次数据时，采取每次增加往后追加的方式，
 每次增加的数据之间以0x00作为分格。 当增加数据的长度为0时，
 即代表删除原来增加的数据
*/
int add_pub_field(glob_msg_stru * pub_data_stru,short id,const char *msg_type,
                  int data_len,const char *data,char from) {
    int i,s,k;
    unsigned char caBuf[2048];

    assert(pub_data_stru);   // 2016/7/20 星期三 下午 4:56:11
    assert(msg_type);    // 2016/7/20 星期三 下午 4:56:15
    s=-1;
    k=0;
    if((data_len <0) || ((MAX_BUFFER - pub_data_stru->off_set) < data_len)) {
        dcs_log(0,0,"<add_pub_field> data_len[%d] <0 or data limited",data_len);
        return -1;
    }
    if(pub_data_stru->msg_field_num >=MAX_FIELD || id <0) {
        dcs_log(0,0,"<add_pub_field> begin error id=%d",id);
        return -1;
    }
    for(i=0 ; i<pub_data_stru->msg_field_num ; i++) {
        if(id == pub_data_stru->data_rec[i].field_id &&
           from == pub_data_stru->data_rec[i].from &&
           memcmp(msg_type,pub_data_stru->data_rec[i].msg_type,4)==0 &&
           pub_data_stru->data_rec[i].Off ==1) {
            if(!data_len) {
                pub_data_stru->data_rec[i].len=0;
                dcs_debug(0,0,"<%s>data_len=0 ,update old data len =0",__FUNCTION__);
                return 0;
            }
            if(from == 0) {
                if(pub_data_stru->data_rec[i].data_addr+pub_data_stru->data_rec[i].len != pub_data_stru->buffer+pub_data_stru->off_set) {
//                      dcs_debug(0,0,"不是连续的增加了同一个数据域 id=%d,from=%d",id,from);//需要重新分配缓冲区地址
                    if((MAX_BUFFER - pub_data_stru->off_set) <= (pub_data_stru->data_rec[i].len+1+data_len)) {
                        dcs_log(0,0,"<%s> data_len[%d] <0 or data limited",
                                __FUNCTION__,data_len);
                        return -1;
                    }
                    if(pub_data_stru->data_rec[i].len <2048) {
						//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,pub_data_stru->data_rec[i].len);
                        memcpy(caBuf,pub_data_stru->data_rec[i].data_addr,pub_data_stru->data_rec[i].len);
                    } else {
                        dcs_log(0,0,"<%s> data_len[%d] limited",
                                __FUNCTION__,pub_data_stru->data_rec[i].len);
                        return -1;
                    }
                    pub_data_stru->data_rec[i].data_addr= pub_data_stru->buffer+pub_data_stru->off_set;
                    pub_data_stru->off_set= pub_data_stru->off_set+pub_data_stru->data_rec[i].len;
					//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,pub_data_stru->data_rec[i].len);
                    memcpy(pub_data_stru->data_rec[i].data_addr,caBuf,pub_data_stru->data_rec[i].len);
                }
                pub_data_stru->buffer[pub_data_stru->off_set]=0x00;
                pub_data_stru->off_set++;
                pub_data_stru->data_rec[i].len++;

            } else {
//              dcs_debug(0,0,"<%s> 系统增加相同数据域，采用更新方法 update_pubfield",__FUNCTION__);
                return update_pub_field(pub_data_stru,id,msg_type,data_len,
                                        data,from);
            }

            k=1;//不再分配新的缓冲区地址
            pub_data_stru->msg_field_num--;
            s=i;
//              dcs_debug(0,0,"<add_pub_field> delete field id=[%d]",id);
            break;
        }
    }
    if(s <0)  i=pub_data_stru->msg_field_num;

    pub_data_stru->data_rec[i].field_id = id;


    if(!k)
        pub_data_stru->data_rec[i].data_addr=pub_data_stru->buffer+pub_data_stru->off_set;
    pub_data_stru->data_rec[i].Off=1;
    pub_data_stru->data_rec[i].from=from;
	//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,data_len);
    memcpy(pub_data_stru->data_rec[i].data_addr+pub_data_stru->data_rec[i].len,data,data_len);
    memcpy(pub_data_stru->data_rec[i].msg_type, msg_type, 4); //2016/7/20 星期三 下午 4:54:48 data_rec.msg_type
    pub_data_stru->data_rec[i].len=pub_data_stru->data_rec[i].len+data_len;
    pub_data_stru->off_set=pub_data_stru->off_set+data_len;
    pub_data_stru->msg_field_num++;
    return pub_data_stru->data_rec[i].len;
}

/*更新数据域 */
int update_pub_field(glob_msg_stru * pub_data_stru,short id,
                     const char * msg_type,int data_len,const char *data,
                     char from) {
    int i,s,k;
//  unsigned char caBuf[2048];
    s=-1;
    k=0;
    if((data_len <0) || ((MAX_BUFFER - pub_data_stru->off_set) < data_len)) {
        dcs_log(0,0,"<%s> data_len[%d] <0 or data limited",__FUNCTION__,data_len);
        return -1;
    }
    if(id <0) {
        dcs_log(0,0,"<%s> begin error id=%d",__FUNCTION__,id);
        return -1;
    }
    for(i=0 ; i<pub_data_stru->msg_field_num ; i++) {
        if(id == pub_data_stru->data_rec[i].field_id &&
           from == pub_data_stru->data_rec[i].from &&
           memcmp(msg_type,pub_data_stru->data_rec[i].msg_type,4)==0 &&
           pub_data_stru->data_rec[i].Off ==1) {
            if(!data_len) {
                pub_data_stru->data_rec[i].len=0;
                return 0;
            }
            if(data_len > pub_data_stru->data_rec[i].len) {
                if(pub_data_stru->data_rec[i].data_addr+pub_data_stru->data_rec[i].len != pub_data_stru->buffer+pub_data_stru->off_set) {
//                      dcs_debug(0,0,"<%s>更新数据所需空间大于原有数据空间 id=%d",__FUNCTION__,id);//需要重新分配缓冲区地址
                    k=1;
                } else k=2;
            }
            s=i;
//              dcs_debug(0,0,"<add_pub_field> delete field id=[%d]",id);
            break;
        }
    }
    if(s <0) {
        dcs_log(0,0,"<%s> not found update msg_type=[%s],id=%d,from=[%c]",__FUNCTION__,msg_type, id,from);
        return -1;
    }

    if(k ==1)
        pub_data_stru->data_rec[i].data_addr=pub_data_stru->buffer+pub_data_stru->off_set;

	//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,data_len);
    memcpy(pub_data_stru->data_rec[i].data_addr,data,data_len);
    pub_data_stru->data_rec[i].len=data_len;
    if(k == 1)
        pub_data_stru->off_set=pub_data_stru->off_set+data_len;
    else if(k==2)
        pub_data_stru->off_set= pub_data_stru->data_rec[i].data_addr - pub_data_stru->buffer+data_len;
//  dcs_debug(0,0,"<%s end k=%d>",__FUNCTION__,k);
    return data_len;
}

int del_pub_field(glob_msg_stru * pub_data_stru,short id,const char *msg_type,
                  char from) {
    int i;

    for(i=0 ; i<pub_data_stru->msg_field_num ; i++) {
        if(id == pub_data_stru->data_rec[i].field_id &&
           from == pub_data_stru->data_rec[i].from &&
           memcmp(msg_type,pub_data_stru->data_rec[i].msg_type,4)==0 &&
           pub_data_stru->data_rec[i].Off ==1) {
//          pub_data_stru->data_rec[i].Off=0;                       // 2016/7/21 星期四 下午 2:37:58 modified
            if(i != pub_data_stru->msg_field_num-1) {            // 2016/7/21 星期四 下午 2:37:51 modified
                memcpy(&pub_data_stru->data_rec[i],
                       &pub_data_stru->data_rec[pub_data_stru->msg_field_num-1],
                       sizeof(field_data));
            }
            pub_data_stru->data_rec[pub_data_stru->msg_field_num-1].len=0;
            pub_data_stru->msg_field_num--;
            return 1;
        }
    }
    return -1;
}

int get_field_data_safe(glob_msg_stru *pub_data_stru,int field_id,
                        const char *msg_type,char *data,int size) {
    int i;
    if(data == NULL) return -1;
    for(i=0; i<pub_data_stru->msg_field_num; i++) {
        if(field_id == pub_data_stru->data_rec[i].field_id &&
           memcmp(msg_type, pub_data_stru->data_rec[i].msg_type,4)==0 &&
           pub_data_stru->data_rec[i].Off) {
            //dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,MIN(pub_data_stru->data_rec[i].len,size-1));
            memcpy(data,pub_data_stru->data_rec[i].data_addr,
                   MIN(pub_data_stru->data_rec[i].len,size-1));
            data[MIN(pub_data_stru->data_rec[i].len,size-1)]=0x00;
            return MIN(pub_data_stru->data_rec[i].len,size-1);
        }
    }
    return -1;
}

int _get_field_data_safe(glob_msg_stru *pub_data_stru,short field_id,
                         const char *msg_type,char *data, char flag,int size) {
    int i;
    if(size <1 || data == NULL) return -1;
    for(i=0; i<pub_data_stru->msg_field_num; i++) {
        if(field_id == pub_data_stru->data_rec[i].field_id &&
           memcmp(msg_type,pub_data_stru->data_rec[i].msg_type,4)==0 &&
           pub_data_stru->data_rec[i].Off &&
           pub_data_stru->data_rec[i].from== flag) {
            //dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,MIN(pub_data_stru->data_rec[i].len,size-1));
            memcpy(data,pub_data_stru->data_rec[i].data_addr,
                   MIN(pub_data_stru->data_rec[i].len,size-1));
            data[MIN(pub_data_stru->data_rec[i].len,size-1)]=0x00;
            return MIN(pub_data_stru->data_rec[i].len,size-1);
        }
    }
    return -1;
}

/**
   获取进入报文类型
**/
int get_in_msg_type(glob_msg_stru *pub_data_stru,char *msg_type,int size) {
    assert(pub_data_stru);  // 2016/7/20 星期三 下午 4:56:19
    assert(msg_type);    // 2016/7/20 星期三 下午 4:56:24
    if(size <5) return -1;
    return snprintf(msg_type,size,"%s",pub_data_stru->in_msg_type);
}

field_define* get_defstru_of_name(const char *name,const message_define *priv_def) {
    int i;
    if(priv_def == NULL || name== NULL) return NULL;

    for(i=0 ; i<priv_def->use_num; i++) {
        if(strcmp(name,priv_def->fld_def[i].name)==0)
            return (field_define*)&priv_def->fld_def[i];
    }
    return NULL;
}

field_define* get_defstru_of_id(short id,const message_define *priv_def) {
    int i;
    if(priv_def == NULL) return NULL;

    for(i=0 ; i<priv_def->use_num; i++)
        if(id==priv_def->fld_def[i].id)
            return (field_define*)&priv_def->fld_def[i];
    return NULL;
}

int set_result_code(glob_msg_stru *pub_data_stru, char *ret_code) {
    assert(pub_data_stru);  // 2016/7/20 星期三 下午 4:56:27
    assert(ret_code);    // 2016/7/20 星期三 下午 4:56:32
    return snprintf(pub_data_stru->center_result_code,5,"%s",ret_code);
}

/**
判断报文输出机构是否关闭
**/
int is_open_of_out_insti(glob_msg_stru *pub_data_stru) {
    return !pub_data_stru->route_insti_open_flag;
}
int get_outlink_type(glob_msg_stru *pub_data_stru) {
    return pub_data_stru->route_insti_link_type;
}
int route_insti_is_open(glob_msg_stru *pub_data_stru) {
    return pub_data_stru->insti_open_flag;
}
int check_route_insti(glob_msg_stru *pub_data_stru) {
    int iDescFid;

    if(is_open_of_out_insti(pub_data_stru)) {
        dcs_log(0, 0, "<%s>接收机构[%s]已关闭！",   __FUNCTION__, pub_data_stru->route_insti_code);
        set_result_code(pub_data_stru, CODE_CUPS_NOT_OPERATE);
        return -1;
    }
    iDescFid = fold_locate_folder(pub_data_stru->route_fold_name);
    if(iDescFid  <0) {
        dcs_log(0, 0, "<%s>取接收机构fold name[%s]出错！",  __FUNCTION__, pub_data_stru->route_fold_name);
        set_result_code(pub_data_stru, CODE_CUPS_NOT_OPERATE);
        return -1;
    }

    if(get_outlink_type(pub_data_stru) == 1) {
        if(IsReady((const char *)pub_data_stru->route_fold_name) < 0) {
            set_result_code(pub_data_stru, CODE_CUPS_NOT_OPERATE);
            return -1;
        }
    }

    return 1;
}

int IsReady(const char *pcaName) {
    int iBnkIdx;
    struct BCDAst *pBCDAst;
//    ICS_DEBUG(0);
    pBCDAst = (struct BCDAst *)g_pcBcdaShmPtr;
    if(!pBCDAst) {
        dcs_log(0,0,"<%s>通信机构共享内存区尚未连接就绪!",__FUNCTION__);
        return -1;
    }
    iBnkIdx = GetFolderIdByName(pcaName);
    if(iBnkIdx < 0) {   //invalid bank code
        dcs_debug(0,0,"<%s>LocateBankByName invalid pcaName =%s",
                  __FUNCTION__,pcaName);
        return -1;
    }
    if(pBCDAst->stBcda[iBnkIdx].cNetFlag <= 0) {
        //无网络连接或没有发开始请求报文
        dcs_debug(0, 0,"<%s>cNetFlag is 0 net not connect [%s]",__FUNCTION__,
                  pcaName);
        return -1;
    }
    return 1;
}

char *my_split(char *s1, const char s2,char *s3, int size_s3) {
    char *p;

    if(s1 == NULL)   return NULL;
    if(!s1[0]) return NULL;
    p = s1;
    while(*p) {
        if(*p == s2) break;
        p++;
    }

	//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,size_s3-1);
    memcpy(s3,s1,MIN(p-s1, size_s3-1));
    s3[MIN(p-s1, size_s3-1)]=0x00;
    if(*p)
        return (p+1);
    else return p;
}

int pack_key(char *keyBuf, int keySize, char *keySet,
             glob_msg_stru *pub_data_stru, char *msg_type, int flag, char d) {
    int n = 0, offset = 0;
    char tmp[256], *p,identifier[64+1];

    assert(keySet);     // 2016/7/20 星期三 下午 4:56:37
    assert(keyBuf);     // 2016/7/20 星期三 下午 4:56:40

    rtrim(keySet);
    for(p=my_split(keySet, ',',identifier, sizeof(identifier)); p ;
        p=my_split(p, ',',identifier, sizeof(identifier))) {
        n = _get_field_data_safe(pub_data_stru,get_pub_field_id(msg_type, identifier),
                                 msg_type,tmp, flag,sizeof(tmp));
        if(n < 0 && d == 0) {
            if(flag !=2) {
                n = _get_field_data_safe(pub_data_stru,get_pub_field_id(msg_type, identifier),
                                         msg_type, tmp, 2,sizeof(tmp));
                if(n < 0 && d == 0) {
                    continue;
                }
            } else {
                continue;
            }
        }

        if(offset + n > keySize) {
            dcs_log(0, 0, "<%s>缓冲区溢出！offset[%d], n[%d], size[%d]",
                    __FUNCTION__,offset, n, keySize);
            return -1;
        }
        if(0 < n) {
            rtrim(tmp);
            offset +=snprintf(keyBuf + offset,keySize-offset,"%s", tmp);
        }
        if(d) { // d为真时,代表数据域分隔符
            keyBuf[offset++] = d;
        }
    }
    if(d) {
        offset--;
        keyBuf[offset] = 0;
    }
    dcs_debug(0,0,"<%s> keyBuf=[%s]",__FUNCTION__,keyBuf);
    return 1;
}

int GetFolderIdByName(const char *pcaBankName) {
    int    iBnkNum, i;
    struct BCDA *pBCDA;
//  ICS_DEBUG(0);
    iBnkNum = ((struct BCDAst *)g_pcBcdaShmPtr)->iBankNum;
    pBCDA = ((struct BCDAst *)g_pcBcdaShmPtr)->stBcda;
    for(i=0; i < iBnkNum; i++) {
        if(memcmp(pcaBankName, pBCDA[i].caBankName,strlen(pcaBankName)) == 0)
            return(i);
    }
    return(-1);
}

int get_pub_field_id(const char *msg_type,const char *name) {
    int i;
    message_define *    p=match_priv_stru(msg_type,&gl_def_set);
    if(p == NULL) {
        dcs_log(0,0,"<%s> can not match priv stru msg_type=[%s]",__FUNCTION__,msg_type);
        return -1;
    }
    for(i=0 ; i<p->use_num; i++)
        if(strcmp(name,p->fld_def[i].name)==0) return p->fld_def[i].id;
    return -1;
}

/**
设置进入报文是否国密加密方式
**/
void set_in_cry_flag(glob_msg_stru *pub_data_stru, int flag) {
    assert(pub_data_stru);  // 2016/7/20 星期三 下午 4:56:43
    if(flag)
        pub_data_stru->in_cry_flag =1;
    else
        pub_data_stru->in_cry_flag =0;
    return;
}

/**
设置输出报文是否国密加密方式
**/
void set_out_cry_flag(glob_msg_stru *pub_data_stru, int flag) {
    assert(pub_data_stru);  // 2016/7/20 星期三 下午 4:56:46
    if(flag)
        pub_data_stru->out_cry_flag =1;
    else
        pub_data_stru->out_cry_flag =0;
    return;
}

/**
 增加输出报文域
**/
int add_route_set(glob_msg_stru *pub_data_stru, const char *msg_type,
                  const char *field_name, int from) {
    int i,id;
    field_set *set;

    set = &pub_data_stru->route_set;
    if(set->num >= MAX_FIELD) return -1;
    id = get_pub_field_id(msg_type,field_name);
    if(id < 0) return -1;
    for(i=0 ; i< set->num; i++) {
        if(set->field.field_id[i] == id)
            return -1;
    }
    set->field.field_id[i] = id;
    set->field.from[i] = from ;
    snprintf(set->field.field_name[i],64,"%s",field_name);
    set->num++;

    return 1;
}

/**
 删除输出报文域
**/
int del_route_set(glob_msg_stru *pub_data_stru,const char *msg_type,
                  const char *field_name) {
    int i,id;
    field_set *set;

    set = &pub_data_stru->route_set;
    if(set->num == 0) return -1;
    id = get_pub_field_id(msg_type,field_name);
    if(id < 0) return -1;
    for(i=0 ; i< set->num; i++) {
        if(set->field.field_id[i] == id)
            break;
    }
    if(i< set->num) {
        set->field.field_id[i] = set->field.field_id[set->num-1];
        set->field.from[i] = set->field.from[set->num-1] ;
        snprintf(set->field.field_name[i],64,"%s",set->field.field_name[set->num-1]);
        set->num--;
        return 1;
    }
    return -1;
}

void rtrim(char * str) {
    int i=0;
    if(str == NULL) return ;
    i=strlen(str);
    for(; i>0; i--)
        if(str[i-1]!=0x20) break;
    if(i >=0)   str[i]=0x00;
    return ;
}

void rtrim_c(char * str, char c) {
    int i=0;
    if(str == NULL) return ;
    i=strlen(str);
    for(i--; i>=0; i--)
        if(str[i]!=c) break;
    if(i >=0)   str[i+1]=0x00;
    return ;
}

void void_proc(glob_msg_stru *pub_data_stru) {
//  ICS_DEBUG(0);
    switch(pub_data_stru->timeout_table.flag[0]) {
        case '1':
            pub_data_stru->timeout_table.flag[0]++;
        case '2':
            break;
        default:
            return;
    }
    write_voidtrans_to_fold(&pub_data_stru->timeout_table);
    return;
}


int GenMacSM(char *key_index, char *key_data, char *macData, int length, char *mac_out) {
    char resp_cd[3];
    char caMac[32+1];
    int nLen,nFillBytes;
    char kIndex[5 + 1];
//ICS_DEBUG(0);
    nLen = length;
    memset(caMac, 0, sizeof(caMac));
    memset(resp_cd, 0, sizeof(resp_cd));
    if(nLen >= 1500) {
        dcs_log(0,0,"<%s>Calculate MACLen Error Len=%d",__FUNCTION__,nLen);
        return(-1);
    }
    if(nLen % 16) {
        nFillBytes = 16 - (nLen % 16);
        memset(macData + nLen, 0, nFillBytes);
    } else
        nFillBytes = 0;

    nLen += nFillBytes;
    dcs_debug(0,0,"<%s>start to call _DESCALCMAC().",__FUNCTION__);
    memcpy(kIndex , key_index, 5);
    kIndex[5] = 0;
    resp_cd[0]=0x00;
    CALCMAC(resp_cd, key_index, key_data, 0, nLen, macData, mac_out);
    dcs_debug(0,0,"<%s>end call CALCMAC.",__FUNCTION__);
    if(memcmp(resp_cd, "00", 2) != 0) {
        dcs_log(0,0,"<%s> failed,rtnCode=%s",__FUNCTION__,resp_cd);
        return(-1);
    }
    dcs_debug(mac_out, 32, "<%s>[%s]",__FUNCTION__,mac_out);
    return 1;
}
int GenMac(char *key_index, char *key_data, char *macData, int length, char *mac_out) {
    char resp_cd[5];
    char caMac[32+1];
    int nLen,nFillBytes;
//ICS_DEBUG(0);
    nLen = length;
    memset(caMac, 0, sizeof(caMac));
    memset(resp_cd, 0, sizeof(resp_cd));
    if(nLen >= 1500) {
        dcs_log(0,0,"<%s>Calculate MACLen Error Len=%d",__FUNCTION__,nLen);
        return(-1);
    }
    if(nLen % 8) {
        nFillBytes = 8 - (nLen % 8);
        memset(macData + nLen, 0, nFillBytes);
    } else
        nFillBytes = 0;

    nLen += nFillBytes;

    dcs_debug(0,0,"<%s>start to call _DESCALCMAC().",__FUNCTION__);

    _DESCALCMAC(resp_cd, key_index, key_data, nLen, macData, mac_out);
    dcs_debug(0,0,"<%s>end call DESMACOP.",__FUNCTION__);

    if(memcmp(resp_cd, "00", 2) != 0) {
        dcs_log(0,0,"<%s>_DESCALCMAC() failed,rtnCode=%s",__FUNCTION__,resp_cd);
        return(-1);
    }
    dcs_debug(mac_out, 8, "<%s>_DESCALCMAC()", __FUNCTION__);
    return 0;
}

//生成系统时间
//para: 0: 生成mmddhhmmss, 1:生成mmdd, 2:生成hhmmss
int get_systime(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char buf[20 + 1];
    int i;
    struct tm *time_tm;
    time_t time_cl ;
    time(&time_cl) ;
    time_tm = localtime(&time_cl);
    if(strftime(buf, 20, "%Y%m%d%H%M%S",time_tm) == 0) return 0;
//  add_pub_field(pub_data_stru,FIELD_DATE_TIME_Y, pub_data_stru->route_msg_type,
//                 14, buf, 1);
    i = atoi(para);
    switch(i) {
        case 1:
            add_pub_field(pub_data_stru,fldid, pub_data_stru->route_msg_type,
                          4, buf + 4, 1);
            break;
        case 2:
            add_pub_field(pub_data_stru, fldid,pub_data_stru->route_msg_type,
                          6, buf + 8, 1);
            break;
        case 3:
            add_pub_field(pub_data_stru,fldid, pub_data_stru->route_msg_type,
                          8, buf, 1);
            break;
        default:
            add_pub_field(pub_data_stru, fldid, pub_data_stru->route_msg_type,
                          10, buf + 4, 1);
            break;
    }
    return 1;
}

int set_field_service_entry(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[3 + 1], tmpBuf[3];
    memset(fieldVal, 0, sizeof(fieldVal));
    if(!para || !para[0]) strcpy(para, "04");
    if(0 <= get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,para),
                                pub_data_stru->in_msg_type, tmpBuf,2)) {
        dcs_debug(tmpBuf,2,"<%s> FIELD_IC_DATA 2 ",__FUNCTION__);
        if(tmpBuf[0]==0x35)
            memcpy(fieldVal, "05", 2);
        else if(tmpBuf[0]==0x31)
            memcpy(fieldVal, "02", 2);
        else memcpy(fieldVal, "07", 2);
    } else if(0 <= get_field_data_safe(pub_data_stru,FIELD_TRACK2,
                                       pub_data_stru->in_msg_type, tmpBuf,1)) {
        memcpy(fieldVal, "02", 2);
    } else if(0 <= get_field_data_safe(pub_data_stru, FIELD_CARD_NO,
                                       pub_data_stru->in_msg_type,tmpBuf,1)) {
        memcpy(fieldVal, "01", 2);
    } else {
        memcpy(fieldVal, "00", 2);
    }
    if(0 <= get_field_data_safe(pub_data_stru,FIELD_PIN,
                                pub_data_stru->route_msg_type, tmpBuf,1)) {
        fieldVal[2] = '1';
    } else
        fieldVal[2] = '2';
    dcs_debug(0,0,"<%s> end  fieldVal=[%s]",__FUNCTION__,fieldVal);
    return add_pub_field(pub_data_stru, fldid,pub_data_stru->route_msg_type, 3,
                         fieldVal, 1);
}

/*************************************************************
没有#表示直接输出
#后面表示格式化数据
第1个字节表示数据来源
   0：表示从接收报文中取
   1：表示从中心数据中取
   2：表示从数据库中取
第2-7个字节表示数据格式
   1：表示取固定长度数据，后面再加5个字节，两个字节起始位值，两个字节结束位置（00表示取到结尾)，一个>结束符
   2：表示取分隔符数据，后面再加5个字节，一个字节分隔，三个字节分隔符数(表示从第几个分隔符起取数据。一个>结束符
   3：表示取固定长度数据, 不去后空格，后面再加5个字节，两个字节起始位值，两个字节结束位置（00表示取到结尾)，一个>结束符
   4：表示取分隔符数据, 不去后空格，后面再加5个字节，一个字节分隔，三个字节分隔符数(表示从第几个分隔符起取数据。一个>结束符
第8-9个字节表示数据域名长度
第10-n个字表示数据域名
第n+1个字节^结束符
**************************************************************/
int make_data(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[256 + 1], *p, tmpBuf[1024 + 1];
    char *p1, buf[10 + 1], fieldName[40 +1], *msgtype;
    int fieldLen, start, end, i, data_from, n;
    rtrim(para);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    for(p1 = tmpBuf, p = para; *p;) {
        if(*p == '#') {
            p++;
            switch(*p++) {
                case '1':
                    data_from = 1;
                    msgtype = pub_data_stru->route_msg_type;
                    break;
                case '2':
                    data_from = 2;
                    msgtype = DB_MSG_TYPE;
                    break;
                default:
                    msgtype = pub_data_stru->in_msg_type;
                    data_from = 0;
                    break;
            }
            if(strlen(p) < 8) {
                dcs_log(0, 0, "<%s>参数[%s]设置出错", __FUNCTION__, p);
                return -1;
            }
            if(*p =='1' || *p == '3') {
                _ATOI(p+1, 2, start);						// 获取开始位置
                _ATOI(p+3, 2, end);							// 获取结束位置
            } else if(*p =='2' || *p == '4') {
            		_ATOI(p+2, 3, start);							// 获取开始位置
            		end = start;
            } 
            /*else {
                dcs_log(0, 0, "<%s>参数[%s]设置出错", __FUNCTION__, p);
                return -1;
            }*/
            _ATOI(p+6, 2, i);								// 获取字段长度
            //dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,i);
            memcpy(fieldName, p+8, i);			// 获取字段名称
            fieldName[i] = 0;
            fieldLen = _get_field_data_safe(pub_data_stru,get_pub_field_id(msgtype, fieldName),
                                            msgtype, fieldVal, data_from,sizeof(fieldVal));
            n = format_msg_data(fieldVal, fieldLen, p, start, end, p1, sizeof(tmpBuf)-strlen(tmpBuf)-1);
	       		p1 += n;
	       		p += 8+i;
	       		p++;														// 结束符
        } else if(*p == '\\') {
            memcpy(buf, p + 1, 2);
            p += 3;
            asc_to_bcd((unsigned char *)p1, (unsigned char *)buf, 2, 0);
            p1++;
        } else {
            *p1++ = *p++;
        }
    }
    *p1 = 0;
    return add_pub_field(pub_data_stru, fldid,pub_data_stru->route_msg_type,
                         strlen(tmpBuf), tmpBuf, 1);
}

/*************************************************************
“,”前面表示原数据域名;如果没有“,”表示不进行格式化直接输出。如果有,表示格式化输出
格式化定义:
\后面两个字节表示ASCII数据 如‘\0A’表示换行;
#后面表示格式化数据,
第1个字节为1表示取固定长度数据，后面再加5个字节，两个字节起始位值，两个字节结束位置（00表示取到结尾)，一个>结束符
第1字节为2表示取分隔符数据，后面再加4个字节，一个字节分隔，两个字节分隔符数(表示从第几个分隔符起取数据。一个>结束符
如：原数据域48为ABCD|EFGHI|JKLMN|OPQ
配置：48,第二:#2|01>\0A第一四:#2|00>#2|03
结果：
第二:EFGHI
第一四:ABCDOPQ
配置：48,第三：#11017>\0A第四#11700
结果：
第三：|EFGHI|
第四：OPQ
 ************************************************************/
int get_msg_data(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[512 + 1], *p, *ps, flag, tmpBuf[1024 + 1], *p1, buf[10 + 1];
    int fieldLen, start, end, offset, n;
    rtrim(para);
    for(p = para; *p && *p != ','; p++);
    flag = *p;
    *p = 0;
    memset(fieldVal, 0, sizeof(fieldVal));
    fieldLen = get_field_data_safe(pub_data_stru,
                                   get_pub_field_id(pub_data_stru->in_msg_type,para),
                                   pub_data_stru->in_msg_type,fieldVal,sizeof(fieldVal));
    if(fieldLen <= 0)return 0;
    if(flag && memcmp(pub_data_stru->center_result_code, "00000", strlen(pub_data_stru->center_result_code)) == 0)
        p++;
    else
        return add_pub_field(pub_data_stru, fldid,pub_data_stru->route_msg_type,
                             fieldLen, fieldVal, 1);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    ps = p;
    for(p1 = tmpBuf; *p && p-ps<strlen(ps);) {
        if(*p == '#') {
            p++;
            if (*p == '1') {						// 截取字符
                _ATOI(p+1, 2, start);
                _ATOI(p+3, 2, end);
                offset = 2*2+1;					//2*2为起止， 1结束符
            } else if(*p == '2') {					// 分割字符					
                _ATOI(p+2, 2, start);
                end = start;
                offset = 1+2+1;					// 1分隔符， 2起止符， 1结束符
            }	
            /*else {
                dcs_log(0, 0, "<%s>参数[%s]设置出错", __FUNCTION__, p);
                return -1;
            }*/
	       		n = format_msg_data(fieldVal, fieldLen, p, start, end, p1, sizeof(tmpBuf)-strlen(tmpBuf)-1);
	       		p1 += n;
	       		p += offset+1;
        } else if(*p == '\\') {
            memcpy(buf, p + 1, 2);
            p += 3;
            asc_to_bcd((unsigned char *)p1, (unsigned char *)buf, 2, 0);
            p1++;
        } else {
            *p1++ = *p++;
        }
    }
    *p1 = 0;
    return add_pub_field(pub_data_stru, fldid, pub_data_stru->route_msg_type,
                         strlen(tmpBuf), tmpBuf, 1);
}
//把pub_data中数据库区的数据转换成系统区数据(ID转换)
//para: PRIV_DEF 中对应 DBMSG 的 name; 后面加,1表示转换成Bin再发送, 后面再加,1表示未取到数据不报错
//参数之间用,分隔，第一个参数为name，第二个参数为1表示转成bin后再发送(可省略有第三个参数时第二个参数不能省)，第三个参数 1表示未取到数据不报错(可省略)
int get_db_data(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[512 + 1], *p, fieldBin[512 + 1],tmp[64];
    int fieldLen,id=-1;
    ICS_DEBUG(0);

    if(pub_data_stru->route_num == 0 && pub_data_stru->switch_src_flag == 1)
        return 1;
    p =my_split(para, ',',tmp, sizeof(tmp));
    if(p != NULL)
        id=get_pub_field_id(DB_MSG_TYPE, tmp);
    if(id < 0) {
        dcs_log(0,0,"<%s> get_pub_field_id fail [%s]",__FUNCTION__,p);
        return -1;
    }
    if(0 > (fieldLen = _get_field_data_safe(pub_data_stru,id, DB_MSG_TYPE,
                                            fieldVal, 2,sizeof(fieldVal)))) {

        dcs_log(0, 0, "<%s>从公共数据区中未取到数据 类型[%s]字段[%s]", __FUNCTION__, DB_MSG_TYPE, p);
        return 0;
    }
    p = my_split(p, ',',tmp, sizeof(tmp));
    if(p != NULL) {
        if(tmp[0] == '1') {
            asc_to_bcd((unsigned char *)fieldBin, (unsigned char *)fieldVal, fieldLen, 0);
            dcs_debug(fieldVal,fieldLen/2,"<%s> id=[%d] flid=[%d]  ",__FUNCTION__,id,fldid);
            return add_pub_field(pub_data_stru, fldid, pub_data_stru->route_msg_type,
                                 fieldLen/2, fieldBin, 1);
        }
    }
    return add_pub_field(pub_data_stru, fldid, pub_data_stru->route_msg_type,
                         fieldLen, fieldVal, 1);
}

/*************************************************************
各数据域直接以逗号分割，若数据域以#号开始，则该数据域后面作为固定
数据内容填充，否则的话，则通过从进入报文中获取相应数据域填充到对应位置。
如："#123af,09" 标识 数据域的组成由“123af”+get_field_data(09)返回的数据组成新的数据域
 ************************************************************/
int get_db_data_fill(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[512 + 1], *p, tmpBuf[1024 + 1],tmp[64];
    int fieldLen, i=0;
    rtrim(para);
    fieldLen=0;
    tmpBuf[0]=0x00;
//  dcs_debug(0,0,"<%s> begin",__FUNCTION__);
    for(p =my_split(para,',',tmp,sizeof(tmp)); p ; p=my_split(p,',',tmp,sizeof(tmp))) {
        if(p && tmp[0] == '#' && strlen(tmp)>1) {
//          p++;
			//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,strlen(tmp+1));
            memcpy(tmpBuf+fieldLen,tmp+1,strlen(tmp+1));
            fieldLen +=strlen(tmp+1);
            tmpBuf[fieldLen]=0x00;
        } else if(p) {
            i=get_field_data_safe(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, tmp),
                                  DB_MSG_TYPE,fieldVal,sizeof(fieldVal));
            if(i > 0 && (i+fieldLen) <1024) {
				//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,i);
                memcpy(tmpBuf+fieldLen,fieldVal,i);
                fieldLen +=i;
                tmpBuf[fieldLen]=0x00;
            }
        }
//      dcs_debug(0,0,"<%s> tmpbuf=[%s]",__FUNCTION__,tmpBuf);
    }
    if(fieldLen <= 0)return 0;
//  dcs_debug(0,0,"<%s> end",__FUNCTION__);
    return add_pub_field(pub_data_stru, fldid, pub_data_stru->route_msg_type,
                         fieldLen, tmpBuf, 1);
}
//取原交易信息
//参数说明：
//参数为消息名，#开头表示固定值
int get_org_trans_info(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[512 + 1], tmpBuf[1024 + 1],tmp[64];
    int fieldLen,id;
    char *p;
    ICS_DEBUG(0);
    //消息类型
    memset(tmpBuf, 0, sizeof(tmpBuf));
    p=my_split(para, ',',tmp,sizeof(tmp));
    while(p != NULL) {
        if(tmp[0] == '#')
            strcat(tmpBuf, tmp + 1);
        else {
            memset(fieldVal, 0, sizeof(fieldVal));
            id=get_pub_field_id(DB_MSG_TYPE, tmp);
            fieldLen = _get_field_data_safe(pub_data_stru,id,  DB_MSG_TYPE,
                                            fieldVal, 2,sizeof(fieldVal));
            if(0 >= fieldLen) {
                dcs_log(0, 0, "<%s>取数据库域[%s][%s]id=[%d]出错！", __FUNCTION__, DB_MSG_TYPE, tmp,id);
                return -1;
            }
            fieldVal[fieldLen]=0x00;
            rtrim(fieldVal);
            strcat(tmpBuf, fieldVal);
        }
        p=my_split(p, ',',tmp,sizeof(tmp));
    }
    return add_pub_field(pub_data_stru, fldid,pub_data_stru->route_msg_type,
                         strlen(tmpBuf), tmpBuf, 1);
}

//全报文MAC, 单des
//para：参数用,分隔
//第一个参数: 0：全报文MAC 1：全报异合后MAC
//第二个参数：0：BIN模式 1：ASC模式
int mac_calc_all_3des(char *para, char *buf, int bufLen, char *mac, glob_msg_stru *pub_data_stru, int flag) {
    int xorFlag = 0, binFlag = 1, i, j;
    char *p, macAsc[16 + 1], macBuf[17],tmp[64];
    ICS_DEBUG(0);
    p=my_split(para, ',',tmp,sizeof(tmp));
    if(p != NULL) {
        if(tmp[0] == '1') xorFlag = 1;
        p = my_split(p, ',',tmp,sizeof(tmp));
    }
    if(p != NULL) {
        if(tmp[0] == '0') binFlag = 0;
        p = my_split(p, ',',tmp,sizeof(tmp));
    }
    if((flag && pub_data_stru->out_cry_flag) ||
       (!flag && pub_data_stru->in_cry_flag)) {
        if(xorFlag) {
            memset(macBuf, 0, sizeof(macBuf));
            for(i = 0; i < bufLen;) {
                for(j = 0; j < 16 && i< bufLen;) {
                    macBuf[j++] ^= buf[i++];
                }
            }
            memcpy(buf, macBuf, 16);
            bufLen = 16;
        }
    } else {
        if(xorFlag) {
            memset(macBuf, 0, sizeof(macBuf));
            for(i = 0; i < bufLen;) {
                for(j = 0; j < 8 && i< bufLen;) {
                    macBuf[j++] ^= buf[i++];
                }
            }
            memcpy(buf, macBuf, 8);
            bufLen = 8;
        }
    }
    if(flag) {
        if(pub_data_stru->out_cry_flag)
            i = GenMacSM(pub_data_stru->route_mac_index, pub_data_stru->route_mac_key, buf, bufLen, macAsc);
        else
            i = GenMac3Des(pub_data_stru->route_mac_index, pub_data_stru->route_mac_key, buf, bufLen, macAsc);
    } else {
        if(pub_data_stru->in_cry_flag)
            i= GenMacSM(pub_data_stru->in_mac_index, pub_data_stru->in_mac_key, buf, bufLen, macAsc);
        else
            i = GenMac3Des(pub_data_stru->in_mac_index, pub_data_stru->in_mac_key, buf, bufLen, macAsc);
    }
    if(0 > i) {
        dcs_log(0, 0, "< %s>计算MAC失败! ", __FUNCTION__);
        return -1;
    }
    if(pub_data_stru->out_cry_flag) {
        if(binFlag)
            asc_to_bcd((unsigned char *)mac, (unsigned char *)macAsc, 32, 0);
        else
            memcpy(mac, macAsc, 32);
    } else {
        if(binFlag)
            asc_to_bcd((unsigned char *)mac, (unsigned char *)macAsc, 16, 0);
        else
            memcpy(mac, macAsc, 16);
    }
    return 1;
}
//全报文MAC, 单des
//para：参数用,分隔
//第一个参数: 0：全报文MAC 1：全报异合后MAC
//第二个参数：0：BIN模式 1：ASC模式
int mac_calc_all_des(char *para, char *buf, int bufLen, char *mac, glob_msg_stru *pub_data_stru, int flag) {
    int xorFlag = 0, binFlag = 1, i, j;
    char *p, macAsc[16 + 1], macBuf[17],tmp[64];
    ICS_DEBUG(0);
    p=my_split(para, ',',tmp,sizeof(tmp));
    if(p != NULL) {
        if(tmp[0] == '1') xorFlag = 1;
        p = my_split(p, ',',tmp,sizeof(tmp));
    }
    if(p != NULL) {
        if(tmp[0] == '0') binFlag = 0;
        p = my_split(p, ',',tmp,sizeof(tmp));
    }
    if(xorFlag) {
        memset(macBuf, 0, sizeof(macBuf));
        for(i = 0; i < bufLen;) {
            for(j = 0; j < 8 && i< bufLen;) {
                macBuf[j++] ^= buf[i++];
            }
        }
        memcpy(buf, macBuf, 8);
        bufLen = 8;
    }
    if(flag)
        i = GenMac(pub_data_stru->route_mac_index, pub_data_stru->route_mac_key, buf, bufLen, macAsc);
    else
        i = GenMac(pub_data_stru->in_mac_index, pub_data_stru->in_mac_key, buf, bufLen, macAsc);
    if(0 > i) {
        dcs_log(0, 0, "<%s>计算MAC失败! ", __FUNCTION__);
        return -1;
    }
    if(binFlag)
        asc_to_bcd((unsigned char *)mac, (unsigned char *)macAsc, 16, 0);
    else
        memcpy(mac, macAsc, 16);
    return 1;
}

//密钥转换
//para: 只能省略后面的参数。
//第一个参数，输出PIN格式, 0:BIN格式(默认)，1:ASC格式
//第二个参数，输出PIN是否带PAN, 0:不带 1：带(默认)
//第三个参数，输入PIN格式，0:BIN格式(默认)，1:ASC格式
//第四个参数，输入PIN是否带PAN, 0:不带 1: 带(默认)
//第五个参数，输入PIN数据所在fldid, 默认为输出PIN同字段
int pin_change(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char pinBin[16 + 1], reCode[2 + 1], pinNewBuf[32 + 1], inPan[16 + 1], outPan[16 + 1], tmpBuf[256 + 1],tmp[64];
    unsigned char inMode, outMode, inPanFlag, outPanFlag;
    short inFldid;
    int l;
    char *p;
//  ICS_DEBUG(0);
    inFldid = fldid;
    inMode = 0;
    outMode = 0;
    inPanFlag = 1;
    outPanFlag = 1;
    memset(tmpBuf, 0, sizeof(tmpBuf));
    dcs_debug(0,0,"<%s> POS_ENTRY_MD_CD=%d, req_flag=%d,fldid=%d",__FUNCTION__,POS_ENTRY_MD_CD,pub_data_stru->req_flag,fldid);
    if(pub_data_stru->req_flag) {
        if(0>=get_field_data_safe(pub_data_stru,POS_ENTRY_MD_CD,  pub_data_stru->in_msg_type,
                                  tmpBuf,sizeof(tmpBuf))) {
            dcs_debug(0,0,"<%s> get_field_data POS_ENTRY_MD_CD fail !",__FUNCTION__);
            return 0;
        }

        if(tmpBuf[2] != '1') {
            dcs_debug(0,0,"<%s> not conver pin ! [%s]",__FUNCTION__,tmpBuf);
            return 1;
        }
        p=my_split(para, ',',tmp,sizeof(tmp));
        if(p != NULL) {
            if(tmp[0] == '1') outMode = 1;
            p = my_split(p, ',',tmp,sizeof(tmp));
        }
        if(p != NULL) {
            if(tmp[0] == '0') outPanFlag = 0;
            p=my_split(p, ',',tmp,sizeof(tmp));
        }
        if(p != NULL) {
            if(tmp[0] == '1') inMode = 1;
            p=my_split(p, ',',tmp,sizeof(tmp));
        }
        if(p != NULL) {
            if(tmp[0] == '0') inPanFlag = 0;
            p=my_split(p, ',',tmp,sizeof(tmp));
        }
        if(p != NULL) {
            inFldid = atoi(tmp);
            if(inFldid == 0) inFldid = fldid;
        }

        l = get_field_data_safe(pub_data_stru,inFldid, pub_data_stru->in_msg_type,
                                tmpBuf,sizeof(tmpBuf));
        if(l == 16) {
            memcpy(pinBin, tmpBuf, 16);
        } else if(l == 8) {
            bcd_to_asc((unsigned char *)pinBin, (unsigned char *)tmpBuf, 16, 0);
        } else {
            dcs_log(0, 0, "< %s>取[%d]域密码信息失败! len[%d], inMode[%d]",
                    __FUNCTION__, inFldid, l, inMode);
            strcpy(pub_data_stru->center_result_code, CODE_PACK_ERR);
            return -1;
        }
        memset(outPan, '0', 16);
        outPan[16] = 0;
        memset(inPan, '0', 16);
        inPan[16] = 0;
        if(outPanFlag || inPanFlag) {
            l = get_field_data_safe(pub_data_stru,FIELD_CARD_NO,  pub_data_stru->in_msg_type,
                                    tmpBuf,sizeof(tmpBuf));
            if(l < 13) {
                dcs_log(0, 0, "<%s>取[%d]域卡号信息失败! len[%d], tmpBuf[%s]",
                        __FUNCTION__,FIELD_CARD_NO, l, tmpBuf);
                strcpy(pub_data_stru->center_result_code, CODE_PACK_ERR);
                return -1;
            }
            if(outPanFlag) memcpy(outPan + 4, tmpBuf + l - 13, 12);
            if(inPanFlag) memcpy(inPan + 4, tmpBuf + l - 13, 12);
        }
        memset(reCode, 0 ,sizeof(reCode));
        if(0>DESTRANSPIN2(reCode,  pub_data_stru->in_pin_index, pub_data_stru->route_pin_index, pub_data_stru->in_pin_key, pub_data_stru->route_pin_key, pinBin, inPan, pinNewBuf, outPan)) {
            dcs_log(0, 0, "<%s>Pin转换错误!",__FUNCTION__);
            strcpy(pub_data_stru->center_result_code, CODE_PIN_ERR);
            return -1;
        }
        if(memcmp(reCode,"00",2)!=0) {
            dcs_log(0,0,"<%s>Pin 转换错误!,错误码＝%s",__FUNCTION__, reCode);
            strcpy(pub_data_stru->center_result_code, CODE_PIN_ERR);
            return -1;
        }
        dcs_debug(0,0,"<%s>conver pin=[%s] ",__FUNCTION__,pinNewBuf);
        if(outMode) {
            add_pub_field(pub_data_stru, fldid, pub_data_stru->route_msg_type,16,
                          pinNewBuf, 1);
        } else {
            asc_to_bcd((unsigned char *)tmpBuf, (unsigned char *)pinNewBuf, 16, 0);
            add_pub_field(pub_data_stru, fldid, pub_data_stru->route_msg_type,
                          8, tmpBuf, 1);
            dcs_debug(tmpBuf,8,"<%s> fldid=%d",__FUNCTION__,fldid);
        }
        dcs_debug(0,0,"<%s>end",__FUNCTION__);
    }
    return 1;
}
int write_voidtrans_to_fold(timeout_stru *table) {
    char buf[1024];
    int myFid, sLen;
    ICS_DEBUG(0);
    myFid = fold_locate_folder("APPL");
    memcpy(buf,"TIME0001", 8);
    sLen = sizeof(timeout_stru);
   // sLen -= sizeof(table->remark);
    rtrim(table->remark);
  //  sLen += strlen(table->remark) + 1;
	//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,sLen);
    memcpy(buf + 8, (char*)table, sLen);
    dcs_debug(0,0,"<%s> flag=%s",__FUNCTION__,table->flag);
    if(fold_write(myFid, myFid, buf, sLen + 8)<0) {
        dcs_log(0, 0, "<%s>写超时表到FOLD[%d]失败!", __FUNCTION__, myFid);
        return -1;
    }
    return 1;
}
int iso_cbc_str(char *para, char *macBuf, int *mac_len, char * field_name, const message_define *p_def,
                const char *buf, glob_msg_stru *pub_data_stru, int len_type) {
    int i, len, len_len, l, j,f;
    char tmp[4], *p, t_buf[512 + 1],str[256];
    ICS_DEBUG(0);
    if(para == NULL || strlen(para) >512) return -1;
    snprintf(t_buf,sizeof(t_buf),"%s", para);
    for(i=0; i<p_def->use_num; i++) {
        if(strcmp(field_name, p_def->fld_def[i].name)==0) {
            if(p_def->fld_def[i].id == FIELD_MAC) {
                *mac_len = 0;
                return 0;
            }
            len_len = p_def->fld_def[i].len_type;
            if(p_def->fld_def[i].len_type == 0) { //固定长度
                len = p_def->fld_def[i].max_len;
            } else if(p_def->fld_def[i].len_type == 1) {  //2字节可变长长度域
                if(len_type)
                    len=((unsigned char)buf[0]>>4)*10 +((unsigned char)buf[0]&0x0f);
                else {
                    memset(tmp, 0, sizeof(tmp));
                    memcpy(tmp, buf, 2);
                    len=atoi(tmp);
                    len_len++;
                }
            } else if(p_def->fld_def[i].len_type == 2) { //3字节可变长长度域
                if(len_type)
                    len=(unsigned char)buf[0]*100+((unsigned char)buf[1]>>4)*10 +((unsigned char)buf[1]&0x0f);
                else {
                    memset(tmp,0,sizeof(tmp));
                    memcpy(tmp,buf,3);
                    len=atoi(tmp);
                    len_len++;
                }
            } else {
                dcs_log(0,0,"<%s> can not parase len_type=[%d]",__FUNCTION__,
                        p_def->fld_def[i].len_type);
                return -1;
            }
            if(p_def->fld_def[i].is_compress)
                len = (int)((len + 1) / 2);
            if(atoi(field_name)==90) f=20;
            else
                f= len;
            p = my_split(t_buf, ',',str,sizeof(str));
            while(p != NULL) {
                if(strcmp(field_name, str) == 0) {
                    for(j = 0, l = 0, i = 0; j < f + len_len; j++) {
                        if(buf[j] == ' ') {
                            if(l)
                                macBuf[i++] = buf[j];
                            else
                                l = 0;
                        } else {
                            l = 1;

                            if(buf[j] >= 'a' && buf[j] <= 'z')
                                macBuf[i++] = buf[j] - 0x20;
                            else if(buf[j] >= 'A' && buf[j] <= 'Z')
                                macBuf[i++] = buf[j];
                            else if(buf[j] >= '0' && buf[j] <= '9')
                                macBuf[i++] = buf[j];
                            else if(buf[j] == ' ' && buf[j] == ',' && buf[j] == '.')
                                macBuf[i++] = buf[j];
                        }
                    }
                    if(l) macBuf[i++] = ' ';
                    *mac_len = i;
                    return len + len_len;
                }
                p = my_split(p, ',',str,sizeof(str));
            }
            *mac_len = 0;
            return len + len_len;
        }
    }
    dcs_log(0,0,"<%s> can not found filed[%s]",__FUNCTION__,field_name);
    return -1;
}


int iso_tl_head_proc(char *buf ,int start, int len) {
    char tmp[10];

    sprintf(tmp,"%04d",len-start);
    memcpy(buf+2,tmp,4);
    return 1;
}


int CalcFee(char *caFeeType, char *caFee, int amount, char *caKee) {
    char *p, *p1;
    char tmpBuf[255];
    double rate, cakee;
    double  maxkee;
    if(caKee  == NULL) {
        dcs_log(0, 0, "<%s>入口参数为空!", __FUNCTION__);
        return -1;
    }
    dcs_debug(0,0,"<%s>type[%s],fee[%s],amount[%d]\n",__FUNCTION__,caFeeType,caFee,amount);
    rtrim(caFee);
    switch(caFeeType[0]) {
        case '1':
            cakee = atol(caFee);
            break;
        case '3':
            memset(tmpBuf, 0, sizeof(tmpBuf));
            for(p = caFee, p1 = tmpBuf; *p && *p != ',' && ((*p >= '0' && *p <= '9') || *p == '.'); *p1++ = *p++);
            if(*p == ',') {
                rate = atof(tmpBuf) * 0.01;
                p++;
            } else {
                dcs_log(0, 0, "<%s>caFeeType为[3]，caFee[%s]设置出错!", __FUNCTION__
                        , caFee);
                return -1;
            }
            p1= p;
            for(; *p && *p >= '0' && *p <= '9'; p++);
            if(*p) {
                dcs_log(0, 0, "<%s>caFeeType为[3]，caFee[%s]设置出错!", __FUNCTION__,
                        caFee);
                return -1;
            } else {
                maxkee = atol(p1);
            }
            cakee = amount * rate / (1 - rate) + 0.5;
            if(cakee > maxkee)cakee = maxkee;
            break;
        case '2':
            for(p = caFee; *p && ((*p >= '0' && *p <= '9') || *p == '.'); p++);
            if(*p) {
                dcs_log(0, 0, "<%s>caFeeType为[2]，caFee[%s]设置出错!", __FUNCTION__, caFee);
                return -1;
            } else {
                rate = atof(caFee) * 0.01;
            }
            cakee = amount * rate / (1 - rate) + 0.5;
            break;
        default:
            dcs_log(0, 0, "< %s>caFeeType[%c]设置出错！", __FUNCTION__, caFeeType[0]);
            return -1;
    }
    sprintf(caKee, "%08d", (int)cakee);
    return 1;
}

void print_field_data(glob_msg_stru *pub_data_stru, char *fileName, int fileline,
                      int d, int fieldid, char *fieldName, char *msg_type) {
    char tmpbuf[1024];
    int len;
    if(fieldName != NULL && msg_type != NULL &&
       fieldName[0] && msg_type[0]) { // 2016/7/22 星期五 下午 3:16:25 modified
        fieldid = get_pub_field_id(msg_type, fieldName);
    }
    len = _get_field_data_safe(pub_data_stru,fieldid,pub_data_stru->route_msg_type,
                               tmpbuf, d,sizeof(tmpbuf));
    if(len > 0) {
        tmpbuf[len]=0x00;
        dcs_log(tmpbuf, len, "<%s>print_field_data：fieldid[%d]:",__FUNCTION__, fieldid);
    } else
        dcs_log(0, 0, "<%s>print_field_data：ieldid[%d]=NULL",__FUNCTION__, fieldid);
    return;
}

/*************************************************************
各数据域直接以逗号分割，若数据域以#号开始，则该数据域后面作为固定
数据内容填充，否则的话，则通过从进入报文中获取相应数据域填充到对应位置。
如："#123af,09" 标识 数据域的组成由“123af”+get_field_data(09)返回的数据组成新的数据域
 ************************************************************/
int get_msg_data_fill(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[512 + 1], *p, tmpBuf[1024 + 1],tmp[64];
    int fieldLen, i=0;
    rtrim(para);
    fieldLen=0;
    tmpBuf[0]=0x00;
//  dcs_debug(0,0,"<%s> begin",__FUNCTION__);
    for(p =my_split(para,',',tmp,sizeof(tmp)); p ; p=my_split(p,',',tmp,sizeof(tmp))) {
        if(p && tmp[0] == '#' && strlen(tmp)>1) {
//          p++;
			//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,strlen(tmp+1));
            memcpy(tmpBuf+fieldLen,tmp+1,strlen(tmp+1));
            fieldLen +=strlen(tmp+1);
            tmpBuf[fieldLen]=0x00;
        } else if(p) {
            i=get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type, tmp),
                                  pub_data_stru->in_msg_type,
                                  fieldVal,sizeof(fieldVal));
            if(i > 0 && (i+fieldLen) <1024) {
				//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,i);
                memcpy(tmpBuf+fieldLen,fieldVal,i);
                fieldLen +=i;
                tmpBuf[fieldLen]=0x00;
            }
        }

    }
    if(fieldLen <= 0)return 0;

    return add_pub_field(pub_data_stru, fldid,pub_data_stru->route_msg_type,
                         fieldLen, tmpBuf, 1);
}

int get_msg_data_tlvfill(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[512 + 1], *p, tmpBuf[1024 + 1],tmp[64];
    int fieldLen, i=0;
    rtrim(para);
    fieldLen=0;
    tmpBuf[0]=0x00;

    if(strcmp(pub_data_stru->in_msg_type,"TPOS")==0 &&
       (i=get_field_data_safe(pub_data_stru, get_pub_field_id("TPOS", "3E"),
                              pub_data_stru->in_msg_type,fieldVal,sizeof(fieldVal))) >0) {
        fieldVal[i]=0x00;
        if(strcmp(fieldVal,"88880300")==0) {
            i=get_field_data_safe(pub_data_stru, get_pub_field_id("TPOS", "08"),
                                  "TPOS",fieldVal,sizeof(fieldVal));
            fieldVal[i]=0x00;
            fieldLen=snprintf(tmpBuf,sizeof(tmpBuf),"PW%s",fieldVal);
            return add_pub_field(pub_data_stru, fldid, pub_data_stru->route_msg_type,
                                 fieldLen, tmpBuf, 1);
        }
    }

    for(p =my_split(para,',',tmp,sizeof(tmp)); p ; p=my_split(p,',',tmp,sizeof(tmp))) {
        if(p && tmp[0] == '#' && strlen(tmp)>1) {
			//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,strlen(tmp+1));
            memcpy(tmpBuf+fieldLen,tmp+1,strlen(tmp+1));
            fieldLen +=strlen(tmp+1);
            tmpBuf[fieldLen]=0x00;
        } else if(p && tmp[0] == '?' && strlen(tmp)>1) {
            i=get_field_data_safe(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, tmp+1),
                                  DB_MSG_TYPE,fieldVal,sizeof(fieldVal));
            if(i > 0 && (i+fieldLen) <1024) {
                sprintf(tmpBuf+fieldLen,"%03d",i);
                fieldLen +=3;
				//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,i);
                memcpy(tmpBuf+fieldLen,fieldVal,i);
                fieldLen +=i;
                tmpBuf[fieldLen]=0x00;
            }
        } else if(p) {
            i=get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type, tmp),
                                  pub_data_stru->in_msg_type,
                                  fieldVal,sizeof(fieldVal));
            if(i > 0 && (i+fieldLen) <1024) {
                sprintf(tmpBuf+fieldLen,"%03d",i);
                fieldLen +=3;
				//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,i);
                memcpy(tmpBuf+fieldLen,fieldVal,i);
                fieldLen +=i;
                tmpBuf[fieldLen]=0x00;
            }
        }
    }
    if(fieldLen <= 0)return 0;

    return add_pub_field(pub_data_stru, fldid,pub_data_stru->route_msg_type,
                         fieldLen, tmpBuf, 1);
}

int get_data_standard(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[512 + 1], *p, tmpBuf[1024 + 1],tmp[64];
    int fieldLen, i=0;
    rtrim(para);
    fieldLen=0;
    tmpBuf[0]=0x00;
    dcs_debug(0,0,"<%s> begin",__FUNCTION__);
    for(p =my_split(para,',',tmp,sizeof(tmp)); p ; p=my_split(p,',',tmp,sizeof(tmp))) {
        if(p && tmp[0] == '#' && strlen(tmp)>1) {
			//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,strlen(tmp+1));
            memcpy(tmpBuf+fieldLen,tmp+1,strlen(tmp+1));
            fieldLen +=strlen(tmp+1);
            tmpBuf[fieldLen]=0x00;
        } else if(p) {
            i=get_field_data_safe(pub_data_stru,atoi(tmp), pub_data_stru->in_msg_type,
                                  fieldVal,sizeof(fieldVal));
            if(i > 0 && (i+fieldLen) <1024) {
				//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,i);
                memcpy(tmpBuf+fieldLen,fieldVal,i);
                fieldLen +=i;
                tmpBuf[fieldLen]=0x00;
            }
        }
    }
    if(fieldLen <= 0)return 0;
    dcs_debug(0,0,"<%s> end",__FUNCTION__);
    return add_pub_field(pub_data_stru, fldid,pub_data_stru->route_msg_type,
                         fieldLen, tmpBuf, 1);
}

int set_service_code(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[64 + 1],tmp[64];
    int  i=0;

    i=get_field_data_safe(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, "PAY_PROC_CODE"),
                          DB_MSG_TYPE, tmp,sizeof(tmp));
    if(i >0)   tmp[i]=0x00;
    else tmp[0]=0x00;
    i=get_field_data_safe(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, para),
                          DB_MSG_TYPE,fieldVal,sizeof(fieldVal));
    if(i >0) {
        if(memcmp(fieldVal,"36",2)==0)               // 2016/7/21 星期四 下午 4:15:22 modified
            return add_pub_field(pub_data_stru, fldid,pub_data_stru->route_msg_type,
                                 2, "36", 1);
        else if(memcmp(fieldVal,"00",2)==0 && strcmp(tmp,"400000")==0)   // 2016/7/21 星期四 下午 4:15:34 modified
            return add_pub_field(pub_data_stru, fldid,pub_data_stru->route_msg_type,
                                 2, "00", 1);
        else return add_pub_field(pub_data_stru, fldid, pub_data_stru->route_msg_type,
                                      2, "80", 1);
    } else return -1;
}
int set_field_60(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char tmp[10];
    int len;
    len=get_field_data_safe(pub_data_stru,POS_ENTRY_MD_CD, pub_data_stru->route_msg_type,
                            tmp,sizeof(tmp));

    if(len <=0) return -1;
    rtrim(para);
    if(tmp[1]=='5' || tmp[1]=='7')
        add_pub_field(pub_data_stru,fldid,pub_data_stru->route_msg_type,
                      strlen("0000060000000000000000000000000001"),"0000060000000000000000000000000001", 1);
    else  // 由于银行已关闭ic卡交易降级处理,所以改为一样
        add_pub_field(pub_data_stru,fldid,pub_data_stru->route_msg_type,
                      strlen("0000060000000000000000000000000001"),"0000060000000000000000000000000001", 1);
    return 1;
}

int change_db_data(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[512 + 1], *p,tmp[128];
    int fieldLen;
    ICS_DEBUG(0);

    if(pub_data_stru->route_num == 0 && pub_data_stru->switch_src_flag == 1) return 1;
    rtrim(para);
    p = my_split(para, ',',tmp,sizeof(tmp));
    if(0 > (fieldLen = _get_field_data_safe(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, tmp),
                                            DB_MSG_TYPE, fieldVal,
                                            2,sizeof(fieldVal)))) {
        dcs_debug(0, 0, "<%s>从公共数据区中未取到数据库[%s]数据[%s]", __FUNCTION__,
                  DB_MSG_TYPE, p);
        return 0;
    }
    fieldVal[fieldLen]=0x00;
    dcs_debug(0,0,"<%s> p=[%s]",__FUNCTION__,p);
    if(strcmp(p,"POS_ENTRY_MD_CD")==0) fieldVal[2]='0';
    else if(strcmp(p,"POS_COND_CD")==0) {
        if(strcmp(fieldVal,"36") ==0)  //扫码支付服务点代码
            ;
        else strcpy(fieldVal,"80");
    }
    return add_pub_field(pub_data_stru, fldid,pub_data_stru->route_msg_type,
                         fieldLen, fieldVal, 1);
}

int con_del_fld48(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char tmp[64];
    int len;
    len=get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type, "3D"),
                            pub_data_stru->in_msg_type,tmp,sizeof(tmp));
    if(len <=0) return -1;
    if(atol(tmp+1) == 0)
        del_pub_field(pub_data_stru,fldid,pub_data_stru->in_msg_type,1);
    return 1;
}

int check_amount(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char *p;
    char amount[20];
    int i;
    if(para == NULL) return -1;

    p=para;
    while(*p) {
        if(*p <'0' || *p >'9') return -1;
        p++;
    }
    i=get_field_data_safe(pub_data_stru, 917, pub_data_stru->in_msg_type,
                          amount,sizeof(amount));
    if(i <=0) return -1;
    amount[i]=0x00;
    if(atol(amount) < atol(para)) return -1;
    return 1;
}

int cancle_trans_specific(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char *p,tmp[64];
    p=my_split(para,'|',tmp,sizeof(tmp));
    if(p == NULL) return -1;
    if(memcmp(pub_data_stru->route_insti_code,tmp,8)==0) {
        p=my_split(p,'|',tmp,sizeof(tmp));
        if(p == NULL) return -1;
        memcpy(pub_data_stru->route_trans_type,tmp,4);
    }
    return 1;
}

void asc_to_bcd(unsigned char* bcd_buf , unsigned char* ascii_buf , int conv_len ,unsigned char type) {
    int         cnt ;
    char        ch , ch1 ;

    if(conv_len & 0x01 && type)
        ch1 = 0 ;
    else
        ch1 = 0x55 ;

    for(cnt = 0 ; cnt < conv_len ; ascii_buf ++ , cnt ++) {
        if(*ascii_buf >= 'a')
            ch = *ascii_buf - 'a' + 10 ;
        else if(*ascii_buf >= 'A')
            ch = *ascii_buf- 'A' + 10 ;
        else if(*ascii_buf >= '0')
            ch = *ascii_buf-'0' ;
        else
            ch = 0;
        if(ch1 == 0x55)
            ch1 = ch ;
        else {
            *bcd_buf ++ = ch1 << 4 | ch ;
            ch1 = 0x55 ;
        }
    }
    if(ch1 != 0x55)
        *bcd_buf = ch1 << 4 ;

    return ;
}


void bcd_to_asc(unsigned char* ascii_buf ,unsigned char* bcd_buf , int conv_len ,unsigned char type) {
    int  cnt ;
    if(conv_len & 0x01 && type) {
        cnt = 1 ;
        conv_len ++ ;
    } else
        cnt = 0 ;
    for(; cnt < conv_len ; cnt ++ , ascii_buf ++) {
        *ascii_buf = ((cnt & 0x01) ? (*bcd_buf ++ & 0x0f) : (*bcd_buf >> 4)) ;
        *ascii_buf += ((*ascii_buf > 9) ? ('A' - 10) : '0') ;
    }
    return ;
}
int GenMac3Des(char *key_index, char *key_data, char *macData, int length, char *mac_out) {
    char resp_cd[5];
    char caMac[32+1];
    int nLen,nFillBytes;
    char kIndex[5 + 1];
    nLen = length;
    memset(caMac, 0, sizeof(caMac));
    memset(resp_cd, 0, sizeof(resp_cd));
    if(nLen >= 1500) {
        dcs_log(0,0,"<%s>Calculate MACLen Error Len=%d",__FUNCTION__,nLen);
        return(-1);
    }
    if(nLen % 8) {
        nFillBytes = 8 - (nLen % 8);
        memset(macData + nLen, 0, nFillBytes);
    } else
        nFillBytes = 0;

    nLen += nFillBytes;
    dcs_debug(0,0,"<%s>start to call _DESCALCMAC().",__FUNCTION__);

    memcpy(kIndex , key_index, 5);
    kIndex[5] = 0;
    _DESCALCMAC(resp_cd, key_index, key_data, nLen, macData, mac_out);
    dcs_debug(0,0,"<%s>end call DESMACOP.",__FUNCTION__);
    if(memcmp(resp_cd, "00", 2) != 0) {
        dcs_log(0,0,"<%s>_DESCALCMAC() failed,rtnCode=%s",__FUNCTION__,resp_cd);
        return(-1);
    }
    dcs_debug(mac_out, 16, "<%s>_DESCALCMAC()", __FUNCTION__);
    return 1;
}

//IC卡数据处理
int tag(unsigned char *icBuf, int len) {
    unsigned char *p;
    int i, j, ret = -1;
    if(icBuf == NULL) return ret;
    for(p = icBuf, i = 0; i < len;) {
        for(j = 0, ret = -1; TAG[j].fieldLen > 0; j++) {
            if(memcmp(p, TAG[j].field, TAG[j].fieldLen) == 0) {
                p +=  TAG[j].fieldLen;
                i += TAG[j].fieldLen;
                TAG[j].datalen = *p++;
                i++;
				//dcs_debug(0,0,"at %s(%s:%d) memcpy[%d]",__FUNCTION__,__FILE__,__LINE__,TAG[j].datalen);
                memcpy(TAG[j].data, p, TAG[j].datalen);
                p +=  TAG[j].datalen;
                i += TAG[j].datalen;
                ret = 1;
                break;
            }
        }
        if(0 > ret) break;
    }
    return ret;
}

//卡信息处理
//para, flag保留
int card_info_check(char *para, short flag, glob_msg_stru *pub_data_stru) {
    char tmpBuf[512 + 1], cardType[2 + 1], bankCode[12 + 1];
    char *p;
    int i, l;
    ICS_DEBUG(0);
//  memset(tmpBuf, 0, sizeof(tmpBuf));
    i=get_field_data_safe(pub_data_stru, POS_ENTRY_MD_CD, pub_data_stru->in_msg_type, tmpBuf,sizeof(tmpBuf));
    if(0 > 0) tmpBuf[i]=0x00;
    else {
        dcs_log(0, 0, "<%s> get_field_data POS_ENTRY_MD_CD fail!",__FUNCTION__);
        strcpy(pub_data_stru->center_result_code, CODE_PACK_ERR);
        return -1;
    }
    if(memcmp(tmpBuf, "02", 2) == 0) { //刷卡交易
//      memset(tmpBuf, 0, sizeof(tmpBuf));
        l = get_field_data_safe(pub_data_stru, FIELD_TRACK2, pub_data_stru->in_msg_type, tmpBuf,sizeof(tmpBuf));
        if(l >0)   tmpBuf[l]=0x00;
        for(p = tmpBuf, i = 0; *p && *p != '=' && i < l; i++, p++);
        if(i > 20 || *p != '=') {
            dcs_log(0, 0, "<FILE:%s,LINE:%d %s>处理035域磁道信息[%s]错误! i = [%d] *p = [%c] ", __FILE__, __LINE__, __FUNCTION__,tmpBuf, i, *p);
            strcpy(pub_data_stru->center_result_code, CODE_PACK_ERR);
            return -2;
        }
        add_pub_field(pub_data_stru, FIELD_CARD_NO, pub_data_stru->route_msg_type, i, tmpBuf, 1);
    } else if(memcmp(tmpBuf, "05", 2) == 0) { //IC交易
//      memset(tmpBuf, 0, sizeof(tmpBuf));
        l = get_field_data_safe(pub_data_stru, FIELD_IC_DATA, pub_data_stru->in_msg_type, tmpBuf,sizeof(tmpBuf));
        if(l >0)   tmpBuf[l]=0x00;
        if(0 > tag((unsigned char *)tmpBuf, l)) {
            dcs_log(0, 0, "<FILE:%s,LINE:%d %s>解析055域IC卡信息失败! ", __FILE__, __LINE__,__FUNCTION__);
            strcpy(pub_data_stru->center_result_code, CODE_PACK_ERR);
            return -3;
        }
        //保存冲正用IC卡TAG域
//      memset(tmpBuf, 0, sizeof(tmpBuf));
        for(i = 0, p = tmpBuf; i < 2; i++) {
            if(TAG[i].datalen > 0) {
                bcd_to_asc((unsigned char *)p, (unsigned char *)TAG[i].field, TAG[i].fieldLen * 2, 0);
                p +=  TAG[i].fieldLen * 2;
                sprintf(p, "%02X", TAG[i].datalen);
                p += 2;
                bcd_to_asc((unsigned char *)p,(unsigned char *)TAG[i].data, TAG[i].datalen * 2, 0);
                p += TAG[i].datalen * 2;
            }
        }
        *p=0x00;
        add_pub_field(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "55"),
                      pub_data_stru->route_msg_type, strlen(tmpBuf), tmpBuf, 1);
        if(0 < (l = get_field_data_safe(pub_data_stru, FIELD_CARD_NO, pub_data_stru->in_msg_type, tmpBuf,sizeof(tmpBuf)))) {
            add_pub_field(pub_data_stru, FIELD_CARD_NO, pub_data_stru->route_msg_type, l, tmpBuf, 1);
        } else {
            dcs_log(0, 0, "<FILE:%s,LINE:%d %s>取卡号域失败! ", __FILE__, __LINE__,__FUNCTION__);
            strcpy(pub_data_stru->center_result_code, CODE_PACK_ERR);
            return -1;
        }
    } else {
//      memset(tmpBuf, 0, sizeof(tmpBuf));
        if(0 < (l = get_field_data_safe(pub_data_stru, FIELD_CARD_NO, pub_data_stru->in_msg_type, tmpBuf,sizeof(tmpBuf)))) {
            tmpBuf[l]=0x00;
            add_pub_field(pub_data_stru, FIELD_CARD_NO, pub_data_stru->route_msg_type, l, tmpBuf, 1);
        }
    }
    if(0 <(l= get_field_data_safe(pub_data_stru, FIELD_CARD_NO, pub_data_stru->in_msg_type, tmpBuf,sizeof(tmpBuf)))) {
        tmpBuf[l]=0x00;
        if(0 > GetBank(tmpBuf, bankCode, sizeof(bankCode), cardType, sizeof(cardType)))
            return -1;
        add_pub_field(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "CARD_ATTR"),
                      DB_MSG_TYPE, strlen(cardType), cardType, 2);
        add_pub_field(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ISS_INSTI_CODE"),
                      DB_MSG_TYPE, strlen(bankCode), bankCode, 2);
    }
    return 1;
}

//卡交易限制
//无参数表示为终端制模式，为1表示限制信用卡交易
int trans_control(char *para, short flag, glob_msg_stru *pub_data_stru) {
    char tmpBuf[255], card_attr[2 + 1], cardno[30 + 1];
    char credit_flag       [1 + 1] ;
    int credit_card_limit          ;
    int credit_d_a_limit           ;
    int credit_d_c_limit           ;
    int debit_card_limit           ;
    int debit_d_a_limit           ;
    int debit_d_c_limit           ;
    int ret;
    int amount, addup_amount, addup_count;
    if(0 >(ret= get_field_data_safe(pub_data_stru, FIELD_CARD_NO, pub_data_stru->in_msg_type, cardno,sizeof(cardno)))) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d><%s>取卡号域[%d]出错！", __FILE__, __LINE__,__FUNCTION__, FIELD_CARD_NO);
        return -1;
    }
    cardno[ret]=0x00;
    if(0 > (ret=_get_field_data_safe(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, "CARD_ATTR"),
                                     pub_data_stru->route_msg_type, card_attr, 2,sizeof(card_attr)))) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d><%s>取卡类型[%d]出错！", __FILE__, __LINE__,__FUNCTION__,  get_pub_field_id(DB_MSG_TYPE, "CARD_ATTR"));
        return -1;
    }
    card_attr[ret]=0x00;
    if(0 >(ret= get_field_data_safe(pub_data_stru, FIELD_AMOUNT, pub_data_stru->in_msg_type, tmpBuf,sizeof(tmpBuf)))) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d><%s>取金额域[%d]出错！", __FILE__, __LINE__, __FUNCTION__, FIELD_AMOUNT);
        return -1;
    }
    tmpBuf[ret]=0x00;
    amount = atol(tmpBuf);
    if(*para == '1') {
        if(card_attr[0] == '2' || card_attr[0] == '3') {
            strcpy(pub_data_stru->center_result_code, CODE_CARD_NO_ALLOW);
            return -1;
        }
    }
    ret = get_term_trans_control(credit_flag, &credit_card_limit, &credit_d_a_limit, &credit_d_c_limit, &debit_card_limit, &debit_d_a_limit, &debit_d_c_limit, pub_data_stru);
    if(ret == 0) return 1;
    if(ret < 0) return -1;
    if(credit_flag[0] == '0' && (card_attr[0] == '2' || card_attr[0] == '3')) {
        strcpy(pub_data_stru->center_result_code, CODE_CARD_NO_ALLOW);
        return -1;
    }
    if(card_attr[0] == '2' || card_attr[0] == '3') { //贷记卡
        if(credit_card_limit < amount && credit_card_limit > 0) {
            strcpy(pub_data_stru->center_result_code, CODE_AMOUNT_LIMIT);
            dcs_log(0, 0, "<FILE:%s,LINE:%d><%s>信用卡金额单笔超限！", __FILE__, __LINE__, __FUNCTION__);
            return -1;
        }
        if(credit_d_a_limit > 0 || credit_d_c_limit > 0) {
            if(0 > AddUpTrans(cardno, &addup_amount, &addup_count, pub_data_stru->app_type, 0))
                return -1;
            if(credit_d_a_limit > 0 && credit_d_a_limit < (addup_amount + amount)) {
                strcpy(pub_data_stru->center_result_code, CODE_AMOUNT_LIMIT);
                dcs_log(0, 0, "<FILE:%s,LINE:%d><%s>信用卡单卡当日金额超限！", __FILE__, __LINE__, __FUNCTION__);
                return -1;
            }
            if(credit_d_c_limit > 0 && addup_count + 1 > credit_d_c_limit) {
                strcpy(pub_data_stru->center_result_code, CODE_COUNT_LIMIT);
                dcs_log(0, 0, "<FILE:%s,LINE:%d><%s>信用卡单卡当日笔数超限！", __FILE__, __LINE__, __FUNCTION__);
                return -1;
            }
        }
    } else {
        if(debit_card_limit < amount && debit_card_limit > 0) {
            strcpy(pub_data_stru->center_result_code, CODE_AMOUNT_LIMIT);
            dcs_log(0, 0, "<FILE:%s,LINE:%d><%s>借记卡单卡单笔金额超限！", __FILE__, __LINE__, __FUNCTION__);
            return 0;
        }
        if(debit_d_a_limit > 0 || debit_d_c_limit > 0) {
            if(0 > AddUpTrans(cardno, &addup_amount, &addup_count, pub_data_stru->app_type, 0))
                return -1;
            if(debit_d_a_limit > 0 && debit_d_a_limit < (addup_amount + amount)) {
                strcpy(pub_data_stru->center_result_code, CODE_AMOUNT_LIMIT);
                dcs_log(0, 0, "<FILE:%s,LINE:%d><%s>借记卡单卡当日金额超限！", __FILE__, __LINE__, __FUNCTION__);
                return -1;
            }
            if(debit_d_c_limit > 0 && addup_count + 1 > debit_d_c_limit) {
                strcpy(pub_data_stru->center_result_code, CODE_COUNT_LIMIT);
                dcs_log(0, 0, "<FILE:%s,LINE:%d><%s>信用卡单卡当日笔数超限！", __FILE__, __LINE__, __FUNCTION__);
                return -1;
            }
        }
    }
    return 1;
}

//ISO8583校验MAC全报文DES方式
//flag无效。
//para：参数用,分隔
//第一个参数: 0：全报文MAC 1：全报异合后MAC
//第二个参数：1：BIN模式 0：ASC模式
int iso_check_mac_all_des(char *para, short flag, glob_msg_stru *pub_data_stru) {
    int xorFlag = 0, binFlag = 1, i, j;
    char *p, macAsc[16 + 1], macBuf[17], *buf, inMacBuf[16 + 1], cMacBuf[16 + 1], tmpBuf[256 + 1],tmp[64];
    int bufLen, macLen, headLen = 0;
    field_define *def;
    ICS_DEBUG(0);
    if(!pub_data_stru->is_check_mac) return 1;
    memset(inMacBuf, 0, sizeof(inMacBuf));
    macLen = _get_field_data_safe(pub_data_stru, FIELD_MAC, pub_data_stru->in_msg_type, inMacBuf, 0,sizeof(inMacBuf));
    if(macLen <= 0) {
        if(0 == strcmp(pub_data_stru->center_result_code, "00") || pub_data_stru->center_result_code[0] == 0) {
            dcs_log(0, 0, "<FILE:%s,LINE:%d>无MAC数据域! ", __FILE__, __LINE__);
            strcpy(pub_data_stru->center_result_code, CODE_MAC_ERR);
            return -1;
        } else
            return 1;
    }
    def=get_priv_field_def_for_id(FIELD_MAC,match_priv_stru(pub_data_stru->in_msg_type,&gl_def_set));
    if(def ==  NULL) {
        dcs_log(0,0,"<%s> get_priv_field_def_for_id fail , id=[%d]",__FUNCTION__,FIELD_MAC);
        return -1;
    }
    if(def->is_compress) macLen /=2;
    headLen = _get_field_data_safe(pub_data_stru, FIELD_TPDU, pub_data_stru->in_msg_type, tmpBuf, 0,sizeof(tmpBuf));
    def=get_priv_field_def_for_id(FIELD_TPDU,match_priv_stru(pub_data_stru->in_msg_type,&gl_def_set));
    if(def ==  NULL) {
        dcs_log(0,0,"<%s> get_priv_field_def_for_id fail , id=[%d]",__FUNCTION__,FIELD_TPDU);
//       return -1;
        bufLen = pub_data_stru->src_len;
        buf = pub_data_stru->src_buffer;
    } else {
        if(def->is_compress) headLen /=2;
        bufLen = pub_data_stru->src_len;
        buf = pub_data_stru->src_buffer;
        if(headLen > 0) {
            buf += headLen;
            bufLen -= headLen;
        }
    }
    bufLen -= macLen;
    dcs_debug(0,0,"<%s> para=[%s]",__FUNCTION__,para);
    p=my_split(para, ',',tmp,sizeof(tmp));
    if(p != NULL) {
        if(tmp[0] == '1') xorFlag = 1;
        p=my_split(p, ',',tmp,sizeof(tmp));
    }
    if(p != NULL) {
        if(tmp[0] == '0') binFlag = 0;
        p=my_split(p, ',',tmp,sizeof(tmp));
    }
    dcs_debug(0,0,"<%s> xorFlag=%d,binFlag=%d",__FUNCTION__,xorFlag,binFlag);
    if(xorFlag) {
        memset(macBuf, 0, sizeof(macBuf));
        for(i = 0; i < bufLen;) {
            for(j = 0; j < 8 && i< bufLen;) {
                macBuf[j++] ^= buf[i++];
            }
        }
        memcpy(buf, macBuf, 8);
        bufLen = 8;
    }
    if(0 > GenMac(pub_data_stru->in_mac_index, pub_data_stru->in_mac_key, buf, bufLen, macAsc)) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d %s>计算MAC失败! ", __FILE__, __LINE__,__FUNCTION__);
        strcpy(pub_data_stru->center_result_code, CODE_SAFE_ERR);
        return -1;
    }
    dcs_debug(0,0,"<%s> binFlag=%d",__FUNCTION__,binFlag);
    if(binFlag)
        asc_to_bcd((unsigned char *)cMacBuf, (unsigned char *)macAsc, 16, 0);
    else
        memcpy(cMacBuf, macAsc, 16);
    def=get_priv_field_def_for_id(FIELD_MAC,match_priv_stru(pub_data_stru->in_msg_type,&gl_def_set));
    if(def ==  NULL) return -1;
    if(def->is_compress) macLen *=2;
    if(memcmp(cMacBuf, inMacBuf, macLen)) {
        dcs_log(cMacBuf, macLen, "<%s>计算的MAC",__FUNCTION__);
        dcs_log(inMacBuf, macLen, "<%s>收到的MAC",__FUNCTION__);
        strcpy(pub_data_stru->center_result_code, CODE_MAC_ERR);
        return -1;
    }
    return 1;
}

//ISO8583校验MAC全报文3DES方式
//flag无效。
//para：参数用,分隔
//第一个参数: 0：全报文MAC 1：全报异合后MAC
//第二个参数：0：BIN模式 1：ASC模式
int iso_check_mac_all_3des(char *para, short flag, glob_msg_stru *pub_data_stru) {
    int xorFlag = 0, binFlag = 1, i, j;
    char *p, macAsc[16 + 1], macBuf[17], *buf, inMacBuf[16 + 1], cMacBuf[16 + 1], tmpBuf[256 + 1],tmp[64];
    int bufLen, macLen, headLen= 0;
    field_define *def=NULL;
    ICS_DEBUG(0);
    if(!pub_data_stru->is_check_mac) return 1;
    memset(inMacBuf, 0, sizeof(inMacBuf));
    macLen = _get_field_data_safe(pub_data_stru, FIELD_MAC, pub_data_stru->in_msg_type, inMacBuf, 0,sizeof(inMacBuf));
    if(macLen <= 0) {
        if(0 == strcmp(pub_data_stru->center_result_code, "00") || pub_data_stru->center_result_code[0] == 0) {
            dcs_log(0, 0, "<FILE:%s,LINE:%d %s>无MAC数据域! ", __FILE__, __LINE__,__FUNCTION__);
            strcpy(pub_data_stru->center_result_code, CODE_MAC_ERR);
            return -1;
        } else
            return 1;
    }
    def=get_priv_field_def_for_id(FIELD_MAC,match_priv_stru(pub_data_stru->in_msg_type,&gl_def_set));
    if(def ==  NULL) return -1;
    if(def->is_compress) macLen /=2;
    if(0<(headLen = _get_field_data_safe(pub_data_stru, FIELD_TPDU, pub_data_stru->in_msg_type, tmpBuf, 0,sizeof(tmpBuf)))) {
        def=get_priv_field_def_for_id(FIELD_TPDU,match_priv_stru(pub_data_stru->in_msg_type,&gl_def_set));
        if(def ==  NULL) return -1;
        if(def->is_compress) headLen /=2;
    } else headLen =0;
    bufLen = pub_data_stru->src_len - headLen;
    buf = pub_data_stru->src_buffer+headLen;
    bufLen -= macLen;
    dcs_debug(0,0,"<%s> buflen=%d,maclen=%d",__FUNCTION__,bufLen,macLen);
    p=my_split(para, ',',tmp,sizeof(tmp));
    if(p != NULL) {
        if(tmp[0] == '1') xorFlag = 1;
        p=my_split(p, ',',tmp,sizeof(tmp));
    }
    if(p != NULL) {
        if(tmp[0] == '0') binFlag = 0;
        p=my_split(p, ',',tmp,sizeof(tmp));
    }
    if(pub_data_stru->in_cry_flag) {
        if(xorFlag) {
            memset(macBuf, 0, sizeof(macBuf));
            for(i = 0; i < bufLen;) {
                for(j = 0; j <16 && i< bufLen;) {
                    macBuf[j++] ^= buf[i++];
                }
            }
            memcpy(buf, macBuf, 16);
            bufLen = 16;
        }
    } else {
        if(xorFlag) {
            memset(macBuf, 0, sizeof(macBuf));
            for(i = 0; i < bufLen;) {
                for(j = 0; j < 8 && i< bufLen;) {
                    macBuf[j++] ^= buf[i++];
                }
            }
            memcpy(buf, macBuf, 8);
            bufLen = 8;
        }
    }
    if(pub_data_stru->in_cry_flag) {
        if(0 > GenMacSM(pub_data_stru->in_mac_index, pub_data_stru->in_mac_key, buf, bufLen, macAsc)) {
            dcs_log(0, 0, "<FILE:%s,LINE:%d %s>计算MAC失败! ", __FILE__, __LINE__,__FUNCTION__);
            strcpy(pub_data_stru->center_result_code, CODE_SAFE_ERR);
            return -1;
        }
        if(binFlag)
            asc_to_bcd((unsigned char *)cMacBuf, (unsigned char *)macAsc, 32, 0);
        else
            memcpy(cMacBuf, macAsc, 16);
    } else {
        if(0 > GenMac3Des(pub_data_stru->in_mac_index, pub_data_stru->in_mac_key, buf, bufLen, macAsc)) {
            dcs_log(0, 0, "<FILE:%s,LINE:%d %s>计算MAC失败! ", __FILE__, __LINE__,__FUNCTION__);
            strcpy(pub_data_stru->center_result_code, CODE_SAFE_ERR);
            return -1;
        }
        if(binFlag)
            asc_to_bcd((unsigned char *)cMacBuf, (unsigned char *)macAsc, 16, 0);
        else
            memcpy(cMacBuf, macAsc, 16);
    }
    if(memcmp(cMacBuf, inMacBuf, macLen)) {
        dcs_log(cMacBuf, macLen, "<%s>计算的MAC",__FUNCTION__);
        dcs_log(inMacBuf, macLen, "<%s>收到的MAC",__FUNCTION__);
        strcpy(pub_data_stru->center_result_code, CODE_MAC_ERR);
        return -1;
    }
    return 1;
}

int iso_check_mac_cbc_3des(char *para, short flag, glob_msg_stru *pub_data_stru) {

    char  *buf=NULL, inMacBuf[16 + 1], cMacBuf[16 + 1], tmpBuf[256 + 1];
    int bufLen, macLen, headLen= 0;
    ICS_DEBUG(0);
    memset(inMacBuf, 0, sizeof(inMacBuf));
    dcs_debug(0,0,"<%s> begin ",__FUNCTION__);
    if(!pub_data_stru->is_check_mac) return 1;
    macLen = _get_field_data_safe(pub_data_stru, FIELD_MAC, pub_data_stru->in_msg_type, inMacBuf, 0,sizeof(inMacBuf));
    if(macLen <= 0) {
        if(0 == strcmp(pub_data_stru->center_result_code, "00") || pub_data_stru->center_result_code[0] == 0) {
            dcs_log(0, 0, "<FILE:%s,LINE:%d>无MAC数据域! ", __FILE__, __LINE__);
            strcpy(pub_data_stru->center_result_code, CODE_MAC_ERR);
            return -1;
        } else
            return 1;
    }
    headLen = _get_field_data_safe(pub_data_stru, FIELD_TPDU, pub_data_stru->in_msg_type, tmpBuf, 0,sizeof(tmpBuf));
    bufLen = pub_data_stru->src_len;
    if(headLen > 0) {
//      headLen= headLen/2;
        buf = pub_data_stru->src_buffer + headLen;
        bufLen -= headLen;
    }
    bufLen -= macLen;
    if(0 > mac_calc_cbc_iso_3des(para, buf, bufLen, cMacBuf, pub_data_stru, 0)) {
        dcs_log(0,0,"<%s> mac_calc_cbc_iso_3des error",__FUNCTION__);
        return -1;
    }
    if(memcmp(cMacBuf, inMacBuf, macLen)) {
        dcs_log(cMacBuf, macLen, "计算的MAC");
        dcs_log(inMacBuf, macLen, "收到的MAC");
        strcpy(pub_data_stru->center_result_code, CODE_MAC_ERR);
        return -1;
    }
    dcs_debug(0,0,"<%s> end",__FUNCTION__);
    return 1;
}

int _tpos_check_terminal(char *para, short flag, glob_msg_stru *pub_data_stru) {
    ICS_DEBUG(0);
    return tpos_check_terminal(pub_data_stru);
}

int _tpos_check_mac(char *para, short flag, glob_msg_stru *pub_data_stru) {
    ICS_DEBUG(0);
    return tpos_check_mac(pub_data_stru);
}

int trans_cancle(char *para, short flag, glob_msg_stru *pub_data_stru) {
    char fieldVal[256 + 1];
    struct  tm *time_tm;
    time_t time_cl;
    char tmp[64];
    int ret, len;
    tl_trans_log_def TransLog;
//  tl_tpos_log_def tpos_log;
    ICS_DEBUG(0);
    memset(&TransLog, 0, sizeof(TransLog));
    time(&time_cl) ;
    time_tm = localtime(&time_cl);
    strftime(tmp, 64, "%Y%m%d%H%M%S", time_tm);
//  memset(fieldVal, 0, sizeof(fieldVal));
    len = get_field_data_safe(pub_data_stru, FIELD_ORG_TRANS_INFO, pub_data_stru->in_msg_type, fieldVal,sizeof(fieldVal));
    if(len < 0) {
        dcs_log(0, 0, "<%s>读原交易信息数据不正确[%d]-[%d]！", __FUNCTION__, FIELD_ORG_TRANS_INFO, len);
        return -1;
    }
    fieldVal[len]=0x00;
    memcpy(TransLog.sys_date, tmp, 8);
    memcpy(TransLog.acq_date, fieldVal, 4);
    memcpy(TransLog.acq_tra_no, fieldVal + 4, 6);
    memcpy(TransLog.acq_term_id1, fieldVal + 10, 20);
    memcpy(TransLog.acq_term_id2, fieldVal + 30, 20);
    get_field_data_safe(pub_data_stru, FIELD_INSTI_CODE, pub_data_stru->in_msg_type, TransLog.acq_insti_code,9);
    ret = select_translog(&TransLog);
    if(ret < 0) return -1;
    rtrim(TransLog.resp_cd_rcv);
    if(ret == 1 && memcmp("00", TransLog.resp_cd_rcv, 2)==0 && TransLog.void_flag[0] == '0') {
        len=get_field_data_safe(pub_data_stru, FIELD_AMOUNT, pub_data_stru->in_msg_type, fieldVal,sizeof(fieldVal));
        if(len >0) fieldVal[len]=0x00;
        else fieldVal[0]=0x00;
        if(atol(fieldVal) != atol(TransLog.amount_pay)) {
            strcpy(pub_data_stru->center_result_code, CODE_INVALID_TRANS);
            return 1;
        }
        if(0>(len=get_field_data_safe(pub_data_stru, FIELD_CARD_NO, pub_data_stru->in_msg_type, fieldVal,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            rtrim(TransLog.pay_acct_no);
            if(strcmp(fieldVal,TransLog.pay_acct_no)!=0) {
                strcpy(pub_data_stru->center_result_code, CODE_INVALID_TRANS);
                return 1;
            }
        }
        if(0 > db_to_pub_daba(pub_data_stru, &TransLog)) return -1;
        return 1;
    } else {
        strcpy(pub_data_stru->center_result_code, ret == 1 ? CODE_PROCESSING : CODE_INVALID_TRANS);
        return -1;
    }
}

//冲正应答处理
int reversed_replay(glob_msg_stru * pub_data_stru) {
//  tl_tpos_log_def tpos_log;
    tl_trans_log_def log;
//  char msgkey[100 + 1], *p;
    ICS_DEBUG(0);
    if(memcmp(pub_data_stru->center_result_code, "00000", strlen(pub_data_stru->center_result_code)) != 0 &&
       strcmp(pub_data_stru->center_result_code, CODE_NOT_EXIST) != 0) {

        if(0 < pub_data_stru->timeout_table.num--) {
            pub_data_stru->timeout_table.invalid_time = time(NULL)+(30-pub_data_stru->timeout_table.num)*SYS_TIME_OUT;
            insert_timeout_table(pub_data_stru, 2);
        }
        dcs_debug(0,0,"<%s> timeout num=[%d]",__FUNCTION__,pub_data_stru->timeout_table.num);
    } else {
        /*
                strcpy(msgkey, pub_data_stru->timeout_table.first_key);
                rtrim(msgkey);
                p=my_strtok(msgkey, ',');
                if(p == NULL)
                {
                    dcs_log(0, 0, "<FILE:%s,LINE:%d>解析first_key[%s]失败！", __FILE__, __LINE__, pub_data_stru->timeout_table.first_key);
                    return -1;
                }
        */
        memset(&log,0,sizeof(log));
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "SYS_DATE"),
                             DB_MSG_TYPE, log.sys_date      , 2,9);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"),
                             DB_MSG_TYPE, log.acq_insti_code, 2,9);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TRA_NO"),
                             DB_MSG_TYPE, log.acq_tra_no    , 2,7);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_DATE"),
                             DB_MSG_TYPE, log.acq_date      , 2,8);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID1"),
                             DB_MSG_TYPE, log.acq_term_id1  , 2,17);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID2"),
                             DB_MSG_TYPE, log.acq_term_id2  , 2,17);
        update_db_voidflag(&log, '2', NULL);

    }
    return 0;
}

int find_replay(glob_msg_stru * pub_data_stru) {
//  tl_tpos_log_def tpos_log;
    tl_trans_log_def transLog;
    char msgkey[100 + 1], *p,tmp[64];
    ICS_DEBUG(0);
    snprintf(msgkey,sizeof(msgkey),"%s", pub_data_stru->timeout_table.first_key);
    rtrim(msgkey);
    p=my_split(msgkey, ',',tmp,sizeof(tmp));
    if(p == NULL) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d>解析first_key[%s]失败！", __FILE__, __LINE__, pub_data_stru->timeout_table.first_key);
        return -1;
    }
    if(!(memcmp(pub_data_stru->center_result_code, "00000", strlen(pub_data_stru->center_result_code)) == 0 ||
         strcmp(pub_data_stru->center_result_code, CODE_NOT_EXIST) == 0)) {

        memset(&transLog,0,sizeof(transLog));
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "SYS_DATE"),
                             DB_MSG_TYPE, transLog.sys_date, 2,9);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"),
                             DB_MSG_TYPE, transLog.acq_insti_code, 2,9);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TRA_NO"),
                             DB_MSG_TYPE, transLog.acq_tra_no, 2,7);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_DATE"),
                             DB_MSG_TYPE, transLog.acq_date, 2,9);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID1"),
                             DB_MSG_TYPE, transLog.acq_term_id1, 2,17);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID2"),
                             DB_MSG_TYPE, transLog.acq_term_id2, 2,17);
        if(0 >= select_translog(&transLog)) return -1;
        if(transLog.permit_void[0] == '1') {
            pub_data_stru->timeout_table.flag[0]  = '2';
            write_voidtrans_to_fold(&pub_data_stru->timeout_table);
        }

    }

    update_db_app_ret(NULL, 0, pub_data_stru);

    return 0;
}

int sign_m_p_32(glob_msg_stru * pub_data_stru) {
    char tmpbuf[256 + 1];
    ICS_DEBUG(0);
    memset(tmpbuf, 0, sizeof(tmpbuf));
    if(64 > get_field_data_safe(pub_data_stru, FIELD_KEY, pub_data_stru->in_msg_type, tmpbuf,sizeof(tmpbuf))) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d>取密钥信息域出错[%s]", __FILE__, __LINE__, tmpbuf);
        return -1;
    }
    memcpy(pub_data_stru->in_mac_key, tmpbuf, 32);
    memcpy(pub_data_stru->in_pin_key, tmpbuf, 32);
    return update_key(pub_data_stru->insti_code, pub_data_stru->in_mac_key, pub_data_stru->in_pin_key);
}

int result_query_tl(glob_msg_stru * pub_data_stru) {

    tl_trans_log_def log;
    char msgkey[100 + 1], *p,tmp[256],flag,ret_code[5];
    int  i;
    ICS_DEBUG(0);
    flag=0x00;
    memset(ret_code,0,sizeof(ret_code));
    i=get_field_data_safe(pub_data_stru, get_pub_field_id(pub_data_stru->in_msg_type, "121"),
                          pub_data_stru->in_msg_type, tmp,sizeof(tmp));
    if(i > 0) {
        tmp[i]=0x00;
        flag= tmp[31];
        if(flag == '0') strcpy(ret_code,"00");
        else if(flag !='0' && memcmp(tmp,"IS",2)==0 && memcmp(tmp+2,"00",2)!=0) memcpy(ret_code,tmp+2,2);
        else strcpy(ret_code,"96") ;
    }
    dcs_debug(0,0,"<%s> flag=[%c],center_result_code=[%s]",__FUNCTION__,flag,pub_data_stru->center_result_code);
    if(memcmp(pub_data_stru->center_result_code, "00000", strlen(pub_data_stru->center_result_code)) != 0 ||
       flag == '5') {

        if(0 < pub_data_stru->timeout_table.num--) {
            pub_data_stru->timeout_table.invalid_time = time(NULL)+(30-pub_data_stru->timeout_table.num)*SYS_TIME_OUT;
            insert_timeout_table(pub_data_stru, 2);
            dcs_debug(0,0,"<%s> reset timeout table",__FUNCTION__);
        }

    } else {
        snprintf(msgkey,sizeof(msgkey),"%s", pub_data_stru->timeout_table.first_key);
        rtrim(msgkey);
        p=my_split(msgkey, ',',tmp,sizeof(tmp));
        if(p == NULL) {
            dcs_log(0, 0, "<FILE:%s,LINE:%d %s>解析first_key[%s]失败！", __FILE__, __LINE__, __FUNCTION__,pub_data_stru->timeout_table.first_key);
            return -1;
        }
        memset(&log,0,sizeof(log));
        i=_get_field_data_safe(pub_data_stru, get_pub_field_id(pub_data_stru->in_msg_type, "15"),
                               pub_data_stru->in_msg_type, tmp,0 ,sizeof(tmp));
        if(i > 0) memcpy(log.qs_date,tmp,4);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "SYS_DATE"),
                             DB_MSG_TYPE, log.sys_date      , 2,9);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"),
                             DB_MSG_TYPE, log.acq_insti_code, 2,9);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TRA_NO"),
                             DB_MSG_TYPE, log.acq_tra_no    , 2,7);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_DATE"),
                             DB_MSG_TYPE, log.acq_date      , 2,9);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID1"),
                             DB_MSG_TYPE, log.acq_term_id1  , 2,17);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID2"),
                             DB_MSG_TYPE, log.acq_term_id2  , 2,17);
        if(0>update_db_result_pay(&log,  ret_code)) {
            if(0 < pub_data_stru->timeout_table.num--) {
                pub_data_stru->timeout_table.invalid_time = time(NULL)+(30-pub_data_stru->timeout_table.num)*SYS_TIME_OUT;
                insert_timeout_table(pub_data_stru, 2);
                dcs_debug(0,0,"<%s> reset timeout table",__FUNCTION__);
            }
        }

    }
    return 0;
}

int result_query_hnyl(glob_msg_stru * pub_data_stru) {

    tl_trans_log_def log;
    char tmp[256],flag,ret_code[5];
    int  i;
    ICS_DEBUG(0);
    flag=0x00;
    memset(ret_code,0,sizeof(ret_code));
    strcpy(ret_code,pub_data_stru->center_result_code);
    if(memcmp(pub_data_stru->center_result_code, "FC", strlen(pub_data_stru->center_result_code)) ==0) {

        if(0 < pub_data_stru->timeout_table.num--) {
            pub_data_stru->timeout_table.invalid_time = time(NULL)+(30-pub_data_stru->timeout_table.num)*SYS_TIME_OUT;
            insert_timeout_table(pub_data_stru, 2);
            dcs_debug(0,0,"<%s> reset timeout table",__FUNCTION__);
        }

    } else {
        /*
        strcpy(msgkey, pub_data_stru->timeout_table.first_key);
        rtrim(msgkey);
        p=my_strtok(msgkey, ',');
        if(p == NULL)
        {
            dcs_log(0, 0, "<FILE:%s,LINE:%d>解析first_key[%s]失败！", __FILE__, __LINE__, pub_data_stru->timeout_table.first_key);
            return -1;
        }
        */
        memset(&log,0,sizeof(log));
        i=_get_field_data_safe(pub_data_stru, get_pub_field_id(pub_data_stru->in_msg_type, "15"),
                               pub_data_stru->in_msg_type, tmp,0 ,sizeof(tmp));
        if(i > 0) memcpy(log.qs_date,tmp,4);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "SYS_DATE"),
                             DB_MSG_TYPE, log.sys_date      , 2,9);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"),
                             DB_MSG_TYPE, log.acq_insti_code, 2,9);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TRA_NO"),
                             DB_MSG_TYPE, log.acq_tra_no    , 2,7);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_DATE"),
                             DB_MSG_TYPE, log.acq_date      , 2,9);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID1"),
                             DB_MSG_TYPE, log.acq_term_id1  , 2,17);
        _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID2"),
                             DB_MSG_TYPE, log.acq_term_id2  , 2,17);
        if(0>update_db_result_pay(&log,  ret_code)) {
            if(0 < pub_data_stru->timeout_table.num--) {
                pub_data_stru->timeout_table.invalid_time = time(NULL)+(30-pub_data_stru->timeout_table.num)*SYS_TIME_OUT;
                insert_timeout_table(pub_data_stru, 2);
                dcs_debug(0,0,"<%s> reset timeout table",__FUNCTION__);
            }
        }

    }
    return 0;
}

// 接收冲正请求处理
int app_reversed(glob_msg_stru * pub_data_stru) {
    char fieldVal[256 + 1],old_date[5],old_time[7];
    struct  tm *time_tm;
    time_t time_cl;
    char tmp[64];
    int ret, len;
    tl_trans_log_def TransLog;
    ICS_DEBUG(0);
    dcs_debug(0,0,"<%s> begin",__FUNCTION__);
    if(strcmp(pub_data_stru->in_msg_type,"ZPOS")==0) {
        dcs_debug(0,0,"<%s> zpos recvered proc sign",__FUNCTION__);
        snprintf(pub_data_stru->route_msg_type,5,"%s",pub_data_stru->in_msg_type);
        snprintf(pub_data_stru->route_insti_code,9,"%s",pub_data_stru->insti_code);
        snprintf(pub_data_stru->route_fold_name,40,"%s",pub_data_stru->insti_fold_name);
        snprintf(pub_data_stru->route_trans_type,5,"%s",pub_data_stru->in_trans_type);
        pub_data_stru->tmp_order[0]=0x30;
    }

    memset(&TransLog, 0, sizeof(TransLog));
    time(&time_cl) ;
    time_tm = localtime(&time_cl);
    strftime(tmp, 64, "%Y%m%d%H%M%S", time_tm);
//  memset(fieldVal, 0, sizeof(fieldVal));
    len = get_field_data_safe(pub_data_stru, FIELD_ORG_TRANS_INFO, pub_data_stru->in_msg_type, fieldVal,sizeof(fieldVal));
    if(len <= 0) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d %s>读原交易信息数据不正确[%d]-[%d]！", __FILE__, __LINE__,__FUNCTION__, FIELD_ORG_TRANS_INFO, len);
        return -1;
    }
    fieldVal[len]=0x00;
    memcpy(TransLog.sys_date, tmp, 8);
//  memcpy(TransLog.acq_date, fieldVal, 4);
//  memcpy(TransLog.acq_time, fieldVal + 4, 6);
    memcpy(old_date, fieldVal, 4);
    old_date[4]=0x00;
    memcpy(old_time, fieldVal + 4, 6);
    old_time[6]=0x00;
    memcpy(TransLog.acq_tra_no, fieldVal + 4+6, 6);
    if(strcmp(pub_data_stru->in_msg_type,"ZPOS")==0) {
        get_field_data_safe(pub_data_stru, 20, pub_data_stru->in_msg_type, TransLog.acq_term_id1,17);

    } else if(strcmp(pub_data_stru->in_msg_type,"LINS")==0) {
        get_field_data_safe(pub_data_stru, 41, pub_data_stru->in_msg_type, TransLog.acq_term_id1,17);
        get_field_data_safe(pub_data_stru, 42, pub_data_stru->in_msg_type, TransLog.acq_term_id2,17);
        dcs_debug(0,0,"<%s> term_id1=[%s],id2=[%s]",__FUNCTION__,TransLog.acq_term_id1,TransLog.acq_term_id2);
    } else {
        memcpy(TransLog.acq_term_id1, fieldVal + 10, 20);
        memcpy(TransLog.acq_term_id2, fieldVal + 30, 20);

    }
    get_field_data_safe(pub_data_stru, FIELD_INSTI_CODE, pub_data_stru->in_msg_type, TransLog.acq_insti_code,9);
    ret = select_translog(&TransLog);
    if(ret < 0) return -1;
    rtrim(TransLog.resp_cd_rcv);
    dcs_debug(0,0,"<%s> ret=[%d],resp_cd_rcv=[%s],void_flag=[%s],permit_void=[%s]",__FUNCTION__,ret,
              TransLog.resp_cd_rcv,TransLog.void_flag,TransLog.permit_void);
    if(ret == 1 && memcmp("00", TransLog.resp_cd_rcv, 2)==0 && TransLog.void_flag[0] == '0' && TransLog.permit_void[0] == '1') {
        len=get_field_data_safe(pub_data_stru, FIELD_AMOUNT, pub_data_stru->in_msg_type, fieldVal,sizeof(fieldVal));
        if(len >0) fieldVal[len]=0x00;
        else fieldVal[0]=0x00;
        if(atol(fieldVal) != atol(TransLog.amount_pay)) {
            strcpy(pub_data_stru->center_result_code, CODE_INVALID_TRANS);
            dcs_log(0,0,"<%s> 金额不一致! 原始交易金额[%s],冲正上送金额[%s]",__FUNCTION__,TransLog.amount_pay,fieldVal);
            return 1;
        }
        rtrim(TransLog.acq_date);
        rtrim(TransLog.acq_time);
        if(strcmp(old_date,TransLog.acq_date)!=0 ||
           strcmp(old_time,TransLog.acq_time)!=0) {
            strcpy(pub_data_stru->center_result_code, CODE_INVALID_TRANS);
            dcs_log(0,0,"<%s> 日期时间不一致! 原始交易值[%s][%s],冲正上送值[%s][%s]",
                    __FUNCTION__,TransLog.acq_date,TransLog.acq_time,old_date,old_time);
            return 1;
        }
        memset(pub_data_stru->timeout_table.first_key, 0, sizeof(pub_data_stru->timeout_table.first_key));
        rtrim(TransLog.acq_insti_code);
        rtrim(TransLog.acq_term_id1);
        rtrim(TransLog.acq_term_id2);
        rtrim(TransLog.acq_tra_no);
        rtrim(TransLog.acq_date);
        snprintf(pub_data_stru->timeout_table.first_key,sizeof(pub_data_stru->timeout_table.first_key), "%s,%s,%s,%s,%s,%s,%s",
                 pub_data_stru->in_msg_type, TransLog.sys_date,TransLog.acq_insti_code,TransLog.acq_tra_no,TransLog.acq_date,TransLog.acq_term_id1,TransLog.acq_term_id2);
//      sprintf(pub_data_stru->timeout_table.first_key, "%s,%s,%s,%s,%s,%s", TransLog.
//          TransLog.sys_date,TransLog.acq_insti_code,TransLog.acq_tra_no,TransLog.acq_term_id1,TransLog.acq_term_id2);
        memcpy(pub_data_stru->timeout_table.sys_date, tmp, 8);
        memcpy(pub_data_stru->timeout_table.sys_time, tmp + 8, 6);
        strcpy(pub_data_stru->timeout_table.foldname, SYSTEM_FOLDNAME);
        snprintf(pub_data_stru->timeout_table.key,sizeof(pub_data_stru->timeout_table.key) ,"%s",pub_data_stru->timeout_table.first_key);
        pub_data_stru->timeout_table.num = 30;
        strcpy(pub_data_stru->timeout_table.flag, "2");
        //更新原始交易的冲正标识，表示已发起冲正
        if(update_db_voidflag(&TransLog, '1', NULL)<0) return -1;
        write_voidtrans_to_fold(&pub_data_stru->timeout_table);
    }
    if(ret > 0) {
        if(TransLog.resp_cd_rcv[0] || TransLog.permit_void[0] == '0')
            strcpy(pub_data_stru->center_result_code, "00");
        else
            strcpy(pub_data_stru->center_result_code, CODE_PROCESSING);
        dcs_debug(0,0,"<%s> 3",__FUNCTION__);
    } else
        strcpy(pub_data_stru->center_result_code, CODE_NOT_EXIST);
    dcs_debug(0,0,"<%s> end",__FUNCTION__);
    return 1;
}

int sign_request(glob_msg_stru * pub_data_stru) {
    char return_code[6 + 1], key_data1[64 + 1], key_data2[64 + 1], check_value[32 + 1], tmpbuf[128 + 1],
         mac_tm_key[64 + 1], pin_tm_key[64 + 1], mac_tek_indx[5 + 1], pin_tek_indx[5 + 1];
    ICS_DEBUG(0);
    memset(return_code, 0, sizeof(return_code));
    memset(key_data1, 0, sizeof(key_data1));
    memset(key_data2, 0, sizeof(key_data2));
    memset(check_value, 0, sizeof(check_value));
    if(0 > get_key_info(pub_data_stru->insti_code, mac_tek_indx, sizeof(mac_tek_indx), mac_tm_key, sizeof(mac_tm_key),
                        pin_tek_indx, sizeof(pin_tek_indx), pin_tm_key, sizeof(pin_tm_key)))
        return -1;
    if(0 > DESTMKGETPIKMAK2(return_code, pub_data_stru->in_mac_index, mac_tek_indx, mac_tm_key, key_data1, key_data2, check_value, "Y", "Y")) {
        strcpy(pub_data_stru->center_result_code, CODE_SAFE_ERR);
        dcs_log(0,0,"<FILE:%s,LINE:%d> DESGETTMK2() failed:%s\n",__FILE__,__LINE__,strerror(errno));
        return -1;
    }
    if(memcmp(return_code, "00", 2)) {
        dcs_log(0,0,"<FILE:%s,LINE:%d> DESGETTMK2() return_code[%s]\n",__FILE__,__LINE__, return_code);
        strcpy(pub_data_stru->center_result_code, CODE_SAFE_ERR);
        return -1;
    }
    memcpy(pub_data_stru->in_mac_key, key_data1, 32);
    memcpy(tmpbuf, key_data2, 32);
    memset(key_data1, 0, sizeof(key_data1));
    memset(key_data2, 0, sizeof(key_data2));
    if(0 > DESTMKGETPIKMAK2(return_code, pub_data_stru->in_pin_index, pin_tek_indx, pin_tm_key, key_data1, key_data2, check_value, "Y", "Y")) {
        dcs_log(0,0,"<FILE:%s,LINE:%d> DESGETTMK2() failed:%s\n",__FILE__,__LINE__,strerror(errno));
        strcpy(pub_data_stru->center_result_code, CODE_SAFE_ERR);
        return -1;
    }
    if(memcmp(return_code, "00", 2)) {
        dcs_log(0,0,"<FILE:%s,LINE:%d> DESGETTMK2() return_code[%s]\n",__FILE__,__LINE__, return_code);
        strcpy(pub_data_stru->center_result_code, CODE_SAFE_ERR);
        return -1;
    }
    memcpy(pub_data_stru->in_pin_key, key_data1, 32);
    memcpy(tmpbuf + 32, key_data2, 32);
    add_pub_field(pub_data_stru, FIELD_KEY, pub_data_stru->in_msg_type, 64, tmpbuf, 1);
    return update_key(pub_data_stru->insti_code, pub_data_stru->in_mac_key, pub_data_stru->in_pin_key);
}

int save_timeout(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char *p, flag, buff[512 + 1], t_buf[1024 + 1],tmp[64];
    int d_fldid, f_len, d_flag = 0;
//  ICS_DEBUG(0);
    assert(para != NULL);
    p=my_split(para, ',',tmp,sizeof(tmp));
    if(p == NULL) {
        dcs_log(0, 0, "<%s>参数配置出错[%s]出错！", __FUNCTION__, para);
        return -1;
    }
    d_fldid = atoi(tmp);
    p = my_split(p, ',',tmp,sizeof(tmp));
    if(p == NULL) {
        flag = '0';
    } else {
        flag = tmp[0];
        p = my_split(p, ',',tmp,sizeof(tmp));
        if(p != NULL) d_flag = atoi(tmp);
    }
    memset(buff, 0, sizeof(buff));
    f_len = _get_field_data_safe(pub_data_stru, d_fldid, pub_data_stru->in_msg_type, buff, d_flag,sizeof(buff));
//  memset(t_buf, 0, sizeof(t_buf));
    t_buf[0]=0x00;
    if(f_len > 0) {
        buff[f_len]=0x00;
        if(flag == '1') {
            bcd_to_asc((unsigned char *)t_buf, (unsigned char *)buff, f_len * 2, 0);
            t_buf[f_len*2]=0x00;
        } else
            strcpy(t_buf, buff);
    }
    return SetFieldData(pub_data_stru->timeout_table.remark, fldid, t_buf, sizeof(pub_data_stru->timeout_table.remark), 1);
}

int read_timeout(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char *p, flag, buff[512 + 1], t_buf[1024 + 1],tmp[64];
    int d_fldid, f_len;
//  ICS_DEBUG(0);
    p=my_split(para, ',',tmp,sizeof(tmp));
    if(p == NULL) {
        dcs_log(0, 0, "<%s>参数配置出错[%s]出错！", __FUNCTION__, para);
        return -1;
    }
    d_fldid = atoi(tmp);
    p = my_split(p, ',',tmp,sizeof(tmp));
    if(p == NULL) {
        flag = '0';
    } else
        flag = tmp[0];
//  memset(buff, 0, sizeof(buff));
    buff[0]=0x00;
    f_len = GetFieldData(pub_data_stru->timeout_table.remark, d_fldid, buff, sizeof(buff), 1);
    if(flag == '1' && f_len >0) {
        asc_to_bcd((unsigned char *)t_buf, (unsigned char *)buff, f_len, 0);
        f_len /= 2;
    } else
        strcpy(t_buf, buff);
    return add_pub_field(pub_data_stru, fldid, pub_data_stru->in_msg_type, f_len, t_buf, 1);
}

//通过交易机构、交易类型取得交易终端信息
//para: 输入附加数据数据域集合，用,分隔
//fldid无效。
int specific_business_handle(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char in_add_data[20 + 1], out_add_data[20 + 1];

    return GetSpecificBusinessPara(in_add_data, out_add_data, pub_data_stru);
}

//通过交易机构、交易类型、上送交易终端信息和步骤取交易终端信息
// fldid无效
int general_buisiness_handle(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    return GetGeneralBusinessPara(pub_data_stru,para);
}

int mac_calc_cbc_iso_3des(char *para, char *src_buf, int src_len, char *mac, glob_msg_stru *pub_data_stru, int flag) {
    char macBuf[2048];
    char *msg_type, tmp[10];
    int ret, len, i, j, n;
    int head_flag, headlen, msgid_flag, bitmap_flag, len_type, offset=0, mac_offset = 0, mac_len, bitnum;
    unsigned char bitmap[64],bitmask;
    message_define *priv_def;
    ICS_DEBUG(0);
    msg_type = flag ? pub_data_stru->route_msg_type : pub_data_stru->in_msg_type;
    priv_def = match_priv_stru(msg_type,&gl_def_set);
    if(0>get_iso_para(msg_type, &head_flag, &headlen, &msgid_flag, &bitmap_flag, &len_type)) {
        head_flag=0;
        headlen=0;
        msgid_flag=0;
        bitmap_flag=1;
        len_type=0;
    }
//  offset= headlen;
    len = iso_cbc_str(para, macBuf + mac_offset, &mac_len, "MSG_ID", priv_def, src_buf+offset, pub_data_stru, len_type);
    if(len <=0) {
        dcs_log(0,0,"<%s>can not get MSG_ID",__FUNCTION__);
        return -1;
    }
    if(offset + len > src_len) {
        dcs_log(0, 0, "<%s>数据长度不够offset[%d] + [%s]_len[%d] > src_len[%d]",__FUNCTION__, offset, "MSG_ID", len, src_len);
        return -1;
    }
    offset += len;
    mac_offset += mac_len;
    if(!bitmap_flag) {
        asc_to_bcd((unsigned char *)bitmap, (unsigned char *)(src_buf + offset), 32, 0);
        len = 2;
    } else {
        memcpy(bitmap, src_buf + offset, 16);
        len = 1;
    }
    if(src_buf[offset] & 0x80)
        bitnum = 16 ;
    else
        bitnum = 8 ;
    len = bitnum * len;
    dcs_log(bitmap, len, "<%s>[bitmap]", __FUNCTION__);
    if(offset + len > src_len) {
        dcs_log(0, 0, "<%s>数据长度不够offset[%d] + [%s]_len[%d] > src_len[%d]", __FUNCTION__,offset, "BITMAP", len, src_len);
        return -1;
    }
    offset += len;
    for(i = 0 ; i < bitnum ; i ++) {
        bitmask = 0x80 ;
        for(j = 0 ; j < 8 ; j ++ , bitmask >>= 1) {
            if(i == 0 && j == 0)continue ;
            if((bitmap [ i ] & bitmask) == 0) continue ;
            n = (i << 3) + j ;
            sprintf(tmp, "%d", n+1);
            len = iso_cbc_str(para, macBuf + mac_offset, &mac_len, tmp, priv_def,src_buf+offset,pub_data_stru,len_type);
            if(len < 0) {
                dcs_log(0,0,"<%s>can not get field[%s]",__FUNCTION__,tmp);
                return -1;
            }
            if(offset + len > src_len) {
                dcs_log(0, 0, "<%s>数据长度不够offset[%d] + [%s]_len[%d] > src_len[%d]",__FUNCTION__, offset, tmp, len, src_len);
                return -1;
            }
            offset += len;
            mac_offset += mac_len;
        }
    }
    mac_offset--;
    macBuf[mac_offset]=0x00;
    if(flag)
        ret = GenMac3Des(pub_data_stru->route_mac_index, pub_data_stru->route_mac_key, macBuf, mac_offset, mac);
    else
        ret = GenMac3Des(pub_data_stru->in_mac_index, pub_data_stru->in_mac_key, macBuf, mac_offset, mac);
    if(0 > ret) {
        dcs_log(0, 0, "<s>计算MAC失败! ",__FUNCTION__, __FILE__, __LINE__);
        return -1;
    }
//  asc_to_bcd(mac,mac,16,0);
    return 1;
}

int check_replay_cd(char *para, short flag, glob_msg_stru *pub_data_stru) {
    ICS_DEBUG(0);
    if(memcmp(pub_data_stru->center_result_code, "00000", strlen(pub_data_stru->center_result_code))) {
        if(*para == '1') //冲正处理
            pub_data_stru->timeout_table.flag[0] = '2';
        else
            pub_data_stru->timeout_table.flag[0] = '0';
        return 0;
    }
    return 1;
}

int check_nothing(char *para, short flag, glob_msg_stru *pub_data_stru) {
    ICS_DEBUG(0);
    
    return 1;
}

int update_db_pay_ret(char *para, short flag, glob_msg_stru *pub_data_stru) {
    tl_trans_log_def TransLog;
    char fieldVal[1024 + 1];
    int len;
    ICS_DEBUG(0);
    save_addidata(pub_data_stru, 1);
    memset(&TransLog, 0, sizeof(TransLog));
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "SYS_DATE"),
                         DB_MSG_TYPE, TransLog.sys_date, 2,9);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"),
                         DB_MSG_TYPE, TransLog.acq_insti_code, 2,9);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TRA_NO"),
                         DB_MSG_TYPE, TransLog.acq_tra_no, 2,7);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_DATE"),
                         DB_MSG_TYPE, TransLog.acq_date, 2,9);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID1"),
                         DB_MSG_TYPE, TransLog.acq_term_id1, 2,17);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID2"),
                         DB_MSG_TYPE, TransLog.acq_term_id2, 2,17);
    if(0 >= select_translog(&TransLog)) return -1;
    strcpy(TransLog.resp_cd_rcv, pub_data_stru->center_result_code);
//  memset(fieldVal, 0, sizeof(fieldVal));
    if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_RECODE, pub_data_stru->in_msg_type,
                                     fieldVal, 0,sizeof(fieldVal)))) {
        fieldVal[len]=0x00;
        SetTransLog(&TransLog, "RESP_CD_PAY", fieldVal, 0, pub_data_stru);
    }
//  memset(fieldVal, 0, sizeof(fieldVal));
    if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_AUTH_ID, pub_data_stru->in_msg_type,
                                     fieldVal, 0,sizeof(fieldVal)))) {
        fieldVal[len]=0x00;
        SetTransLog(&TransLog, "RESP_CD_AUTH_ID", fieldVal, 0, pub_data_stru);
    }
//  memset(fieldVal, 0, sizeof(fieldVal));
    if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_SYS_REF_NO, pub_data_stru->in_msg_type,
                                     fieldVal, 0,sizeof(fieldVal)))) {
        fieldVal[len]=0x00;
        SetTransLog(&TransLog, "SYS_REF_NO", fieldVal, 0, pub_data_stru);
    }
//  memset(fieldVal, 0, sizeof(fieldVal));
    if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_SETTLE_DATE, pub_data_stru->in_msg_type,
                                     fieldVal, 0,sizeof(fieldVal)))) {
        fieldVal[len]=0x00;
        SetTransLog(&TransLog, "QS_DATE", fieldVal, 0, pub_data_stru);
    }
    sprintf(TransLog.step, "%d", pub_data_stru->route_num);
    if(pub_data_stru->timeout_table.flag[0] == '1' ||  pub_data_stru->timeout_table.flag[0] == '2')
        strcpy(TransLog.permit_void, "1");
    else
        strcpy(TransLog.permit_void, "0");
//  if(0 < get_field_data(get_pub_field_id(DB_MSG_TYPE, "ACQ_ADDITION"), pub_data_stru, fieldVal))
//      SetTransLog(&TransLog, "ACQ_ADDITION", fieldVal, 0, pub_data_stru);
//  memset(fieldVal, 0, sizeof(fieldVal));
    if(0 < (len=get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "PAY_ADDITION"),
                                    DB_MSG_TYPE, fieldVal,sizeof(fieldVal)))) {
        fieldVal[len]=0x00;
        SetTransLog(&TransLog, "PAY_ADDITION", fieldVal, 0, pub_data_stru);
    }
    if(0 > update_translog(&TransLog)) return -1;
    return 1;
}

//更新数据库, PAY返回去APP。flag参见db_update说明。
//para: 保留
int update_db_pay_app(char *para, short flag, glob_msg_stru *pub_data_stru) {
    tl_trans_log_def TransLog;
    char fieldVal[1024 + 1];
    int len;
    ICS_DEBUG(0);
    save_addidata(pub_data_stru, 1);
    memset(&TransLog, 0, sizeof(TransLog));
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "SYS_DATE"),
                         DB_MSG_TYPE, TransLog.sys_date, 2,9);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"),
                         DB_MSG_TYPE, TransLog.acq_insti_code, 2,9);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TRA_NO"),
                         DB_MSG_TYPE, TransLog.acq_tra_no, 2,7);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_DATE"),
                         DB_MSG_TYPE, TransLog.acq_date, 2,7);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID1"),
                         DB_MSG_TYPE, TransLog.acq_term_id1, 2,17);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID2"),
                         DB_MSG_TYPE, TransLog.acq_term_id2, 2,26);
    if(0 >= select_translog(&TransLog)) return -1;
    if(flag == 1) {
        SetTransLog(&TransLog, "APP_MSG_TYPE", pub_data_stru->route_msg_type, 0, pub_data_stru);
        SetTransLog(&TransLog, "APP_TRANS_TYPE", pub_data_stru->route_trans_type, 0, pub_data_stru);
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 <(len=_get_field_data_safe(pub_data_stru, FIELD_CARD_NO, pub_data_stru->in_msg_type,
                                        fieldVal, 1,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "PAY_ACCT_NO", fieldVal, 0, pub_data_stru);
        }
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 <(len= _get_field_data_safe(pub_data_stru, FIELD_INSTI_CODE, pub_data_stru->in_msg_type,
                                         fieldVal, 1,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "APP_INSTI_CODE", fieldVal, 0, pub_data_stru);
        }
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 < (len= _get_field_data_safe(pub_data_stru, FIELD_TERM_ID1, pub_data_stru->in_msg_type,
                                          fieldVal, 1,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "APP_TERM_ID1", fieldVal, 0, pub_data_stru);
        }
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_TERM_ID2, pub_data_stru->in_msg_type,
                                         fieldVal, 1,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "APP_TERM_ID2", fieldVal, 0, pub_data_stru);
        }
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_DATE, pub_data_stru->in_msg_type,
                                         fieldVal, 1,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "APP_DATE", fieldVal, 0, pub_data_stru);
        }
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_TIME, pub_data_stru->in_msg_type,
                                         fieldVal, 1,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "APP_TIME", fieldVal, 0, pub_data_stru);
        }
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 <(len= _get_field_data_safe(pub_data_stru, FIELD_TRA_NO, pub_data_stru->in_msg_type,
                                         fieldVal, 1,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "APP_TRA_NO", fieldVal, 0, pub_data_stru);
        }
        if(0 < (len=get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "APP_ADDITION"),
                                        DB_MSG_TYPE, fieldVal,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "APP_ADDITION", fieldVal, 0, pub_data_stru);
        }
        if(0 < (len=get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "PAY_ADDITION"),
                                        DB_MSG_TYPE, fieldVal,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "PAY_ADDITION", fieldVal, 0, pub_data_stru);
        }
    }
//  memset(fieldVal, 0, sizeof(fieldVal));
    if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_RECODE, pub_data_stru->in_msg_type,
                                     fieldVal, 0,sizeof(fieldVal)))) {
        fieldVal[len]=0x00;
        SetTransLog(&TransLog, "RESP_CD_PAY", fieldVal, 0, pub_data_stru);
    }
//  memset(fieldVal, 0, sizeof(fieldVal));
    if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_AUTH_ID, pub_data_stru->in_msg_type,
                                     fieldVal, 0,sizeof(fieldVal)))) {
        fieldVal[len]=0x00;
        SetTransLog(&TransLog, "RESP_CD_AUTH_ID", fieldVal, 0, pub_data_stru);
    }
//  memset(fieldVal, 0, sizeof(fieldVal));
    if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_SYS_REF_NO, pub_data_stru->in_msg_type,
                                     fieldVal, 0,sizeof(fieldVal)))) {
        fieldVal[len]=0x00;
        SetTransLog(&TransLog, "SYS_REF_NO", fieldVal, 0, pub_data_stru);
    }
//  memset(fieldVal, 0, sizeof(fieldVal));
    if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_SETTLE_DATE, pub_data_stru->in_msg_type,
                                     fieldVal, 0,sizeof(fieldVal)))) {
        fieldVal[len]=0x00;
        SetTransLog(&TransLog, "QS_DATE", fieldVal, 0, pub_data_stru);
    }
    sprintf(TransLog.step, "%d", pub_data_stru->route_num);
    if(pub_data_stru->timeout_table.flag[0] == '1' ||  pub_data_stru->timeout_table.flag[0] == '2')
        strcpy(TransLog.permit_void, "1");
    else
        strcpy(TransLog.permit_void, "0");
    if(0 > update_translog(&TransLog)) return -1;
    return 1;
}

int update_db_app_ret(char *para, short flag, glob_msg_stru *pub_data_stru) {
    tl_trans_log_def TransLog;
    char fieldVal[1024 + 1];
    int len;
    ICS_DEBUG(0);
    save_addidata(pub_data_stru, 2);
    memset(&TransLog, 0, sizeof(TransLog));
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "SYS_DATE"),
                         DB_MSG_TYPE, TransLog.sys_date, 2,9);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"),
                         DB_MSG_TYPE, TransLog.acq_insti_code, 2,9);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TRA_NO"),
                         DB_MSG_TYPE, TransLog.acq_tra_no, 2,7);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_DATE"),
                         DB_MSG_TYPE, TransLog.acq_date, 2,9);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID1"),
                         DB_MSG_TYPE, TransLog.acq_term_id1, 2,17);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID2"),
                         DB_MSG_TYPE, TransLog.acq_term_id2, 2,17);
    if(0 >= select_translog(&TransLog)) return -1;
    if(para != NULL)
        strcpy(TransLog.resp_cd_rcv, pub_data_stru->center_result_code);
//  memset(fieldVal, 0, sizeof(fieldVal));
    if(0 < (len=get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "APP_ADDITION"),
                                    DB_MSG_TYPE, fieldVal,sizeof(fieldVal)))) {
        fieldVal[len]=0x00;
        SetTransLog(&TransLog, "APP_ADDITION", fieldVal, 0, pub_data_stru);
    }
//  memset(fieldVal, 0, sizeof(fieldVal));
    if(0 < (len=get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_ADDITION"),
                                    DB_MSG_TYPE, fieldVal,sizeof(fieldVal)))) {
        fieldVal[len]=0x00;
        SetTransLog(&TransLog, "ACQ_ADDITION", fieldVal, 0, pub_data_stru);
    }
//  memset(fieldVal, 0, sizeof(fieldVal));
    if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_RECODE, pub_data_stru->in_msg_type,
                                     fieldVal, 0,sizeof(fieldVal)))) {
        fieldVal[len]=0x00;
        SetTransLog(&TransLog, "RESP_CD_APP", fieldVal, 0, pub_data_stru);
    }
    sprintf(TransLog.step, "%d", pub_data_stru->route_num);
    if(0 > update_translog(&TransLog)) return -1;
    return 1;
}

//更新数据库, APP返回去PAY。flag参见db_update说明。
//para: 保留
int update_db_app_pay(char *para, short flag, glob_msg_stru *pub_data_stru) {
    tl_trans_log_def TransLog;
    char fieldVal[1024 + 1];
    int len;
    ICS_DEBUG(0);
    save_addidata(pub_data_stru, 2);
    memset(&TransLog, 0, sizeof(TransLog));
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "SYS_DATE"),
                         DB_MSG_TYPE, TransLog.sys_date, 2,9);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"),
                         DB_MSG_TYPE, TransLog.acq_insti_code, 2,9);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TRA_NO"),
                         DB_MSG_TYPE, TransLog.acq_tra_no, 2,7);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_DATE"),
                         DB_MSG_TYPE, TransLog.acq_date, 2,9);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID1"),
                         DB_MSG_TYPE, TransLog.acq_term_id1, 2,17);
    _get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID2"),
                         DB_MSG_TYPE, TransLog.acq_term_id2, 2,17);
    if(0 >= select_translog(&TransLog)) return -1;
    if(flag == 1) {
        SetTransLog(&TransLog, "PAY_MSG_TYPE", pub_data_stru->route_msg_type, 0, pub_data_stru);
        SetTransLog(&TransLog, "PAY_TRANS_TYPE", pub_data_stru->route_trans_type, 0, pub_data_stru);
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 <(len=_get_field_data_safe(pub_data_stru, FIELD_CARD_NO, pub_data_stru->in_msg_type,
                                        fieldVal, 1,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "PAY_ACCT_NO", fieldVal, 0, pub_data_stru);
        }
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_INSTI_CODE, pub_data_stru->in_msg_type,
                                         fieldVal, 1,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "PAY_INSTI_CODE", fieldVal, 0, pub_data_stru);
        }
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_TERM_ID1, pub_data_stru->in_msg_type,
                                         fieldVal, 1,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "PAY_TERM_ID1", fieldVal, 0, pub_data_stru);
        }
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_TERM_ID2, pub_data_stru->in_msg_type,
                                         fieldVal, 1,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "PAY_TERM_ID2", fieldVal, 0, pub_data_stru);
        }
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_DATE, pub_data_stru->in_msg_type,
                                         fieldVal, 1,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "PAY_DATE", fieldVal, 0, pub_data_stru);
        }
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_TIME, pub_data_stru->in_msg_type,
                                         fieldVal, 1,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "PAY_TIME", fieldVal, 0, pub_data_stru);
        }
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_TRA_NO, pub_data_stru->in_msg_type,
                                         fieldVal, 1,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "PAY_TRA_NO", fieldVal, 0, pub_data_stru);
        }
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 < (len=get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "APP_ADDITION"),
                                        DB_MSG_TYPE, fieldVal,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "APP_ADDITION", fieldVal, 0, pub_data_stru);
        }
//      memset(fieldVal, 0, sizeof(fieldVal));
        if(0 < (len=get_field_data_safe(pub_data_stru, get_pub_field_id(DB_MSG_TYPE, "PAY_ADDITION"),
                                        DB_MSG_TYPE, fieldVal,sizeof(fieldVal)))) {
            fieldVal[len]=0x00;
            SetTransLog(&TransLog, "PAY_ADDITION", fieldVal, 0, pub_data_stru);
        }
    }
//  memset(fieldVal, 0, sizeof(fieldVal));
    if(0 < (len=_get_field_data_safe(pub_data_stru, FIELD_RECODE, pub_data_stru->in_msg_type,
                                     fieldVal, 0,sizeof(fieldVal)))) {
        fieldVal[len]=0x00;
        SetTransLog(&TransLog, "RESP_CD_PAY", fieldVal, 0, pub_data_stru);
    }
    sprintf(TransLog.step, "%d", pub_data_stru->route_num);
    if(0 > update_translog(&TransLog)) return -1;
    return 1;
}

int get_insti_code(glob_msg_stru *pub_data_stru, char *field_id) {
    int i, inst_field;
    char tmp[30];
    i=0;
    ICS_DEBUG(0);

    if(0>folder_to_insti_code(pub_data_stru->insti_fold_name,
                              pub_data_stru->insti_code, sizeof(pub_data_stru->insti_fold_name)))
        return -1;
//  dcs_debug(0,0,"%s,%d<%s>pre add 33 field",__FILE__,__LINE__,__FUNCTION__);
    if(field_id && field_id[0]) inst_field = atoi(field_id);
    else inst_field = 33;
    if((i=get_field_data_safe(pub_data_stru,inst_field,pub_data_stru->in_msg_type,tmp,sizeof(tmp))) <=0)
        add_pub_field(pub_data_stru,inst_field,pub_data_stru->in_msg_type,
                      strlen(pub_data_stru->insti_code),pub_data_stru->insti_code, 0);
    else {
        tmp[i]=0x00;
        if(strcmp(tmp,pub_data_stru->insti_code)!=0) {
            dcs_log(0,0,"<%s> message insti_code Does not meet expectations ! src code[%s],db code[%s] ",
                    __FUNCTION__,tmp,pub_data_stru->insti_code);
            return -1;
        }

    }
    return 1;
}

int err_set_msg(glob_msg_stru *pub_data_stru) {
    ICS_DEBUG(0);
    snprintf(pub_data_stru->route_insti_code,9,"%s", pub_data_stru->insti_code);
    snprintf(pub_data_stru->route_msg_type  ,5,"%s", pub_data_stru->in_msg_type);
    snprintf(pub_data_stru->route_fold_name ,40,"%s", pub_data_stru->insti_fold_name);
    snprintf(pub_data_stru->route_mac_index ,6 ,"%s",pub_data_stru->in_mac_index);
    snprintf(pub_data_stru->route_mac_key   ,33 ,"%s",pub_data_stru->in_mac_key);
    snprintf(pub_data_stru->route_trans_type, 5,"%s",pub_data_stru->in_trans_type);
    pub_data_stru->route_is_check_mac=pub_data_stru->is_check_mac;
    pub_data_stru->route_insti_work_type=pub_data_stru->insti_work_type;
    if(pub_data_stru->center_result_code[0] == 0)
        strcpy(pub_data_stru->center_result_code, CODE_SYSTEM_ERR);

    if(0 > conver_data(pub_data_stru, 2)) return -1;
    return 1;
}

int genrate_field_conver_handle(char *handle, char *para, short fldid, glob_msg_stru *pub_data_stru) {
    int i;
    ICS_DEBUG(0);
    if(0 > fldid) return 1;
    rtrim(handle);
    for(i = 0; gl_genrate_field_conver[i].handle != NULL; i++) {
        if(strcmp(gl_genrate_field_conver[i].handle, handle) == 0)
            return gl_genrate_field_conver[i].func(para, fldid, pub_data_stru);
    }
    dcs_log(0,0,"<FILE:%s,LINE:%d %s>处理函数[%s]未找到!", __FILE__, __LINE__,__FUNCTION__, handle);
    return -1;
}

int priv_field_conver_handle(char *handle, char *para, short fldid, glob_msg_stru *pub_data_stru) {
    int i;
//  ICS_DEBUG(0);
    if(0 > fldid) return -1;
    rtrim(handle);
    for(i = 0; gl_priv_field_conver[i].handle != NULL; i++) {
        if(strcmp(gl_priv_field_conver[i].handle, handle) == 0) {
            if(0 > gl_priv_field_conver[i].func(para, fldid, pub_data_stru)) {
                dcs_log(0, 0, "<%s>处理函数[%s],para[%s]处理失败!", __FUNCTION__, handle, para);
                return -1;
            } else
                return 1;
        }
    }
    dcs_log(0,0,"<%s>处理函数[%s]未找到!", __FUNCTION__, handle);
    return -1;
}

int special_business(char *handle, char *para, short fldid, glob_msg_stru *pub_data_stru) {
    int i;

    if(0 > fldid) return -1;
    for(i = 0; gl_special_business_handle[i].handle != NULL; i++) {
        if(strcmp(gl_special_business_handle[i].handle, handle) == 0)
            return gl_special_business_handle[i].func(para, fldid, pub_data_stru);
    }
    dcs_log(0,0,"<%s> (%s)处理函数未找到!", __FUNCTION__,handle);
    return -1;
}

void strcpy_safe(char *des, char *src, int len) {
    int i;
    if(0 == src) {
        if(des) *des = 0;
        return;
    }
    if(des == NULL) return;
    for(i=0; i<(len)&&*(src+i)!=0; i++) *(des+i)=*(src+i);
    if(i>0) *(des+i) = 0;
    return;
}

field_define* get_priv_field_def(const char *name,short *id,const message_define *priv_def) {
    int i;
    if(priv_def == NULL || name== NULL) return NULL;

    for(i=0 ; i<priv_def->use_num; i++)
        if(strcmp(name,priv_def->fld_def[i].name)==0) {
            *id=priv_def->fld_def[i].id;
            return (field_define*)&priv_def->fld_def[i];
        }
    return NULL;
}

int discnt_check_list(glob_msg_stru *pub_data_stru,char *check_list) {
    char *p=NULL,*p1,tmp[256],str[64];
    int i;
    if(check_list == NULL || strlen(check_list)==0) return 1;
    dcs_debug(0,0,"<%s> check_list=[%s]",__FUNCTION__,check_list);
    for(p=my_split(check_list,',',str,sizeof(str)); p; p=my_split(p,',',str,sizeof(str))) {
        dcs_debug(0,0,"<%s> p=[%s]",__FUNCTION__,str);
        p1=str;
        while(*p1) {
            if(*p1 =='|') {
                *p1=0x00;
                p1++;
                break;
            }
            p1++;
        }
        snprintf(tmp,sizeof(tmp),"%s",p1);
        for(i = 0; gl_advert_proc[i].func != NULL; i++) {
            if(strcmp(gl_advert_proc[i].handle, str) == 0) {
                dcs_debug(0,0,"<%s> func=[%s]",__FUNCTION__,str);
                if(gl_advert_proc[i].func(tmp,0,pub_data_stru) <0)
                    return -1;
                dcs_debug(0,0,"<%s> func=[%s] succ!",__FUNCTION__,str);
                break;
            }
        }
    }
    return 1;
}

/**
计算交易金额满多少，减多少的算法，且可判断支持的发卡行
para= 发卡行|卡种|起始金额|扣减金额
**/
int cacl_discount_1(char *amount,char *para,glob_msg_stru *pub_data_stru) {
    int i,flag;
    char *p,iss_insti[9],cardtype[2],begin_amount[13],discount_amount[13],card_no[20],tmp[64];
    dcs_debug(0,0,"<%s> begin",__FUNCTION__);
    if(amount == NULL || para == NULL || pub_data_stru==NULL) return -1;
    for(i= strlen(amount); i >0; i--)
        if(amount[i-1] >'9' || amount[i-1]<'0') return -1;
    flag=0;
//  dcs_debug(0,0,"<%s> 1",__FUNCTION__);
    for(rtrim(para),p=my_split(para,'|',tmp,sizeof(tmp)); p; p=my_split(p,'|',tmp,sizeof(tmp))) {
        if(flag == 0) {  //发卡行
            if(strlen(tmp) >8) return -1;
            snprintf(iss_insti,sizeof(iss_insti),"%s",tmp);
            flag=1;
//             dcs_debug(0,0,"<%s> 2",__FUNCTION__);
        } else if(flag == 1) { //卡种
            if(strlen(tmp) > 1) return -1;
            snprintf(cardtype,sizeof(cardtype),"%s",tmp);
            flag=2;
//              dcs_debug(0,0,"<%s> 3",__FUNCTION__);
        } else if(flag == 2) { // 起始金额
            rtrim(begin_amount);

            if(strlen(tmp) >12) return -1;
            snprintf(begin_amount,sizeof(begin_amount),"%s",tmp);
            for(i= strlen(begin_amount); i >0; i--)
                if(begin_amount[i-1] >'9' || begin_amount[i-1]<'0') return -1;
            flag=3;
//              dcs_debug(0,0,"<%s> 4",__FUNCTION__);
        } else if(flag == 3) { //扣减金额
            rtrim(discount_amount);
            if(strlen(tmp) >12) return -1;

            snprintf(discount_amount,sizeof(discount_amount),"%s",tmp);
            for(i= strlen(discount_amount); i >0; i--)
                if(discount_amount[i-1] >'9' || discount_amount[i-1]<'0') return -1;
            flag=4;
//            dcs_debug(0,0,"<%s> 5",__FUNCTION__);
        }

    }

    if(flag == 4) {
//       dcs_debug(0,0,"<%s> 6  -3",__FUNCTION__);
        if(atol(amount) < atol(begin_amount)) return 1;
//          dcs_debug(0,0,"<%s> 6  -2",__FUNCTION__);
        if(atol(begin_amount) <= atol(discount_amount)) return 1;
//       dcs_debug(0,0,"<%s> 6  -1",__FUNCTION__);
        i=get_field_data_safe(pub_data_stru, FIELD_CARD_NO, pub_data_stru->in_msg_type, card_no,sizeof(card_no));
        if(i >0 && i <20) card_no[i]=0x00;
        else return -1;
//       dcs_debug(0,0,"<%s> 6",__FUNCTION__);
        if(cardtype[0]== '1')   //单银行借记卡
            i= check_card_limit(card_no,iss_insti,"1");
        else if(cardtype[0]== '2')   //单银行信用卡
            i= check_card_limit(card_no,iss_insti,"2");
        else if(cardtype[0]== '3')   //单银行所有卡种
            i= check_card_limit(card_no,iss_insti,NULL);
        else if(cardtype[0]== '0')   //所有银行的卡种
            i=1;
        if(i >0) {

            sprintf(begin_amount,"%012ld",atol(amount)-atol(discount_amount));
            add_pub_field(pub_data_stru,FIELD_AMOUNT_REAL,pub_data_stru->in_msg_type,12,begin_amount, 0);
            add_pub_field(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, "AMOUNT_REAL"),
                          DB_MSG_TYPE,12,begin_amount, 2);
//           dcs_debug(0,0,"<%s> real_amount=[%s]",__FUNCTION__,begin_amount);
        }
//       dcs_debug(0,0,"<%s> 7",__FUNCTION__);

    }
    return 1;
}

int conver_data(glob_msg_stru * pub_data_stru,int flag) {
//  ICS_DEBUG(0);
    if(!flag) { //请求交易数据转换
        //1、获取路由交易所需域集合
        //2、特殊数据域的转换(如PIN)
        //新增固定信息的数据域

        if(0>genrate_field_conver(pub_data_stru)) {
            dcs_log(0,0,"<%s> genrate_field_conver fail!",__FUNCTION__);
            return -1;
        }
        if(pub_data_stru->switch_src_flag == 0)
            if(0 > special_business_handle(pub_data_stru)) {
                dcs_log(0,0,"<%s> special_business_handle fail!",__FUNCTION__);
                return -1;
            }
        if(0>get_default_field(pub_data_stru))return -1;
//        dcs_debug(0,0,"<%s> special_business_handle end",__FUNCTION__);
        if(0>priv_field_conver(pub_data_stru)) {
            dcs_log(0,0,"<%s> priv_field_conver fail!",__FUNCTION__);
            return -1;
        }
//       dcs_debug(0,0,"<%s> priv_field_conver end",__FUNCTION__);
        //3、数据库定义的数据域转换(根据业务类型与请求交易类型);
        //4、特殊业务处理
    } else {
        //1、获取路由交易所需域集合
        //2、特殊数据域的转换（根据业务类型与交易步骤）
        //3、数据库定义的数据域转换;
        //4、新增的数据域
        if(0>get_default_field(pub_data_stru))return -1;
//           dcs_debug(0,0,"<%s> resp get_default_field end",__FUNCTION__);
        if(flag == 1)
            if(0>genrate_field_conver(pub_data_stru))return -1;
        if(0>priv_field_conver(pub_data_stru))return -1;
//          dcs_debug(0,0,"<%s> resp priv_field_conver end",__FUNCTION__);
    }
//  dcs_debug(0,0,"<%s> end",__FUNCTION__);
    return 1;
}

int get_route_insti_code(glob_msg_stru * pub_data_stru) {
    int i;
    /*
      1、判断新节点报文是否指定为简单路由还是复杂路由(简单路由为直接指定机构号)；
      2、若为复杂路由则根据下级路由表进行路由，否则按照业务简单路由处理
      3、判断复杂路由的种类：
        A、根据业务类型与下一节点交易类型判断是否存在按照终端信息域路由，若存在：
           (1)、获取报文终端信息；
           (2)、获取终端路由信息表种类；
           (3)、获取重要信息域；
           (4)、调用相关函数处理获取到路由机构
        B、否则直接根据重要信息域路由:
           (1)、获取重要信息域；
           (2)、调用相关函数处理获取到路由机构

    */
//  ICS_DEBUG(0);


    if(0 > (i = route_proc(pub_data_stru))) //特殊路由处理
//      return -1;
        ;
    else if(i > 0) return 1;

    i=get_terminal_route(pub_data_stru); //根据终端进行路由
    if(0>i)
//      return i;
        ;
    else if(i>0) return i;
    i=get_easy_route(pub_data_stru); //获取简单路由
    if(i >0)
        return i; //简单路由成功
    i = get_bin_route(pub_data_stru);
    if(0 > i)
        return i;
    if(0 == i) {
        strcpy(pub_data_stru->center_result_code, CODE_CUPS_NOT_OPERATE);
        return -1;
    }
    return 1;
}

// 获取数据库中的已存储交易信息
int load_db_trans_info(glob_msg_stru *pub_data_stru) {
    int i;
    char msgkey[100 + 1], *p,tmp[64];
//  ICS_DEBUG(0);
    snprintf(msgkey,sizeof(msgkey),"%s", pub_data_stru->timeout_table.first_key);
    rtrim(msgkey);
//  dcs_debug(0,0,"<%s> msgkey=[%s]",__FUNCTION__,msgkey);
    p=my_split(msgkey, ',',tmp,sizeof(tmp));
    if(p == NULL) {
        dcs_log(0, 0, "<FILE:%s,LINE:%d>解析first_key[%s]失败！", __FILE__, __LINE__, pub_data_stru->timeout_table.first_key);
        return -1;
    }
    for(i=0; gl_db_qproc[i].func!=NULL; i++) {
        if(strcmp(tmp, gl_db_qproc[i].name)==0) {
            return gl_db_qproc[i].func(pub_data_stru);
        }
    }
    return db_genrate_query(pub_data_stru);
}

//返回给受理机构
int reply_acq(glob_msg_stru *pub_data_stru) {
    int len
    ICS_DEBUG(0);

    pub_data_stru->switch_src_flag=1;
    if(db_update(pub_data_stru, 0) <0) return -1;
    len=_get_field_data_safe(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, "ACQ_INSTI_CODE"),
                             DB_MSG_TYPE, pub_data_stru->route_insti_code, 2,9);
    len=_get_field_data_safe(pub_data_stru,get_pub_field_id(DB_MSG_TYPE, "ACQ_TRANS_TYPE"),
                             DB_MSG_TYPE, pub_data_stru->route_trans_type, 2,5);
    if(0 > get_route_insti_info(pub_data_stru)) //获取路由机构信息
        return -1;
    if(0 > conver_data(pub_data_stru, 1)) //报文转换
        return -1;
    return 1;
}

field_define* get_priv_field_def_for_id(short id,const message_define *priv_def) {
    int i;
    if(priv_def == NULL) return NULL;

    for(i=0 ; i<priv_def->use_num; i++)
        if(id==priv_def->fld_def[i].id)
            return (field_define*)&priv_def->fld_def[i];
    return NULL;
}

int set_cry_flag(glob_msg_stru *pub_data_stru, int cry_flag) {
    if(!pub_data_stru)  return -1;
    set_in_cry_flag(pub_data_stru, cry_flag);
    set_out_cry_flag(pub_data_stru, cry_flag);
    return 1;
}

//TC上送
//修改TPOS模式查询

int tc_send(char *para, short flag, glob_msg_stru *pub_data_stru) {
    char fieldVal[300 + 1];
    struct  tm *time_tm;
    time_t time_cl;
    char tmp[64];
    int ret, len;
    tl_trans_log_def TransLog;
    ICS_DEBUG(0);
    memset(&TransLog, 0, sizeof(TransLog));
    time(&time_cl) ;
    time_tm = localtime(&time_cl);
    strftime(tmp, 64, "%Y%m%d%H%M%S", time_tm);
//  memset(fieldVal, 0, sizeof(fieldVal));
    if(memcmp(pub_data_stru->in_msg_type, "TPOS", 4)) {
        len = get_field_data_safe(pub_data_stru, FIELD_ORG_TRANS_INFO,
                                  pub_data_stru->in_msg_type, fieldVal,sizeof(fieldVal));
        if(len < 0) {
            strcpy(pub_data_stru->center_result_code, CODE_PACK_ERR);
            dcs_log(0, 0, "<FILE:%s,LINE:%d %s>读原交易信息数据不正确[%d]-[%d]！", __FILE__, __LINE__,__FUNCTION__, FIELD_ORG_TRANS_INFO, len);
            return -1;
        }
        fieldVal[len]=0x00;
        memcpy(TransLog.sys_date, tmp, 8);
        memcpy(TransLog.acq_date, fieldVal, 4);
        memcpy(TransLog.acq_tra_no, fieldVal + 4, 6);
        memcpy(TransLog.acq_term_id1, fieldVal + 10, 20);
        memcpy(TransLog.acq_term_id2, fieldVal + 30, 20);
        len=get_field_data_safe(pub_data_stru, FIELD_INSTI_CODE,
                                pub_data_stru->in_msg_type, TransLog.acq_insti_code,9);
        ret = select_translog(&TransLog);
        if(ret < 0) return -1;
        rtrim(TransLog.resp_cd_rcv);
        if(ret > 0) {

            if(0 > db_to_pub_daba(pub_data_stru, &TransLog)) return -1;
            return 1;
        } else {
            strcpy(pub_data_stru->center_result_code, ret == 1 ? CODE_PROCESSING : CODE_INVALID_TRANS);
            return -1;
        }
    } else {
        if(11 >= (len = get_field_data_safe(pub_data_stru, get_pub_field_id(pub_data_stru->in_msg_type, "3B"),
                                            pub_data_stru->in_msg_type, fieldVal,sizeof(fieldVal)))) {
            strcpy(pub_data_stru->center_result_code, CODE_PACK_ERR);
            return -1;
        }
        bcd_to_asc((unsigned char *)TransLog.acq_tra_no, (unsigned char *)fieldVal, 6, 0);
        bcd_to_asc((unsigned char *)TransLog.acq_mac, (unsigned char *)(fieldVal + 3), 16, 0);
        add_pub_field(pub_data_stru, FIELD_IC_DATA, pub_data_stru->in_msg_type, len - 11, fieldVal + 11, 1);
        get_field_data_safe(pub_data_stru, FIELD_PSAM_NO, pub_data_stru->in_msg_type, TransLog.acq_term_id1,17);
        ret = select_old_tpos_log(&TransLog);
        if(0 > ret) {
            dcs_log(fieldVal, len, "<%s>can not found old  record",__FUNCTION__);
            return -1;
        }
        if(ret > 0) {
            if(0 > db_to_pub_daba(pub_data_stru, &TransLog)) return -1;
            return 1;
        } else {
            strcpy(pub_data_stru->center_result_code, ret == 1 ? CODE_PROCESSING : CODE_INVALID_TRANS);
            return -1;
        }
    }
    return 1;
}

int get_data_from(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[512 + 1], *p, *ps, flag, tmpBuf[1024 + 1], *p1, buf[10 + 1],*msg_type, db_para[40], in_para[40];
    int fieldLen, start, end, offset, n;
    rtrim(para);
    for(p = para; *p && *p !=','; p++);
    *p++ = 0;
    strncpy(in_para, para, sizeof(in_para));
    for(para = p; *p && *p != ','; p++);
    flag = *p;
    *p = 0;
    strncpy(db_para, para, sizeof(db_para));
    if(pub_data_stru->req_flag) {
    		msg_type = pub_data_stru->in_msg_type;
    		para = in_para;
    }
    else {
    		msg_type = DB_MSG_TYPE;
    		para = db_para;
    }
    memset(fieldVal, 0, sizeof(fieldVal));
    fieldLen = get_field_data_safe(pub_data_stru,
                                   get_pub_field_id(msg_type,para),
                                   msg_type,fieldVal,sizeof(fieldVal));
    if(fieldLen <= 0)return 0;
    	
    if(flag && memcmp(pub_data_stru->center_result_code, "00000", strlen(pub_data_stru->center_result_code)) == 0)
        p++;
    else
        return add_pub_field(pub_data_stru, fldid,pub_data_stru->route_msg_type,
                             fieldLen, fieldVal, 1);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    ps = p;
    for(p1 = tmpBuf; *p && p-ps<strlen(ps);) {
        if(*p == '#') {
            p++;
            if (*p == '1') {						// 截取字符
                _ATOI(p+1, 2, start);
                _ATOI(p+3, 2, end);
                offset = 2*2+1;					//2*2为起止， 1结束符
            } else if(*p == '2') {					// 分割字符					
                _ATOI(p+2, 2, start);
                end = start;
                offset = 1+2+1;					// 1分隔符， 2起止符， 1结束符
            }	
            /*else {
                dcs_log(0, 0, "<%s>参数[%s]设置出错", __FUNCTION__, p);
                return -1;
            }*/
	       		n = format_msg_data(fieldVal, fieldLen, p, start, end, p1, sizeof(tmpBuf)-strlen(tmpBuf)-1);
	       		p1 += n;
	       		p += offset+1;
        } else if(*p == '\\') {
            memcpy(buf, p + 1, 2);
            p += 3;
            asc_to_bcd((unsigned char *)p1, (unsigned char *)buf, 2, 0);
            p1++;
        } else {
            *p1++ = *p++;
        }
    }
    *p1 = 0;
    return add_pub_field(pub_data_stru, fldid, pub_data_stru->route_msg_type,
                         strlen(tmpBuf), tmpBuf, 1);
}

int json_generate_list(char *para, short fldid, glob_msg_stru *pub_data_stru) {
		char fieldVal[1024 + 1], tmpBuf[1024 + 1], key[20], text[50];
    int fieldLen, array_len, data_len, i, len_key, len_text;
    struct json_object *obj, *list;
    rtrim(para);
    
    memset(fieldVal, 0, sizeof(fieldVal));
    fieldLen = get_field_data_safe(pub_data_stru,
                                   get_pub_field_id(pub_data_stru->in_msg_type,para),
                                   pub_data_stru->in_msg_type,fieldVal,sizeof(fieldVal));
    if(fieldLen <= 0) return 0;
    
    list = json_tokener_parse(fieldVal);
    if(!list || json_object_get_type(list) != json_type_array) return 0;
    
    data_len = 1;
    array_len = json_object_array_length(list);
    for(i = 0; i < array_len; i++) {
    		obj = json_object_array_get_idx(list, i);
    		memset(key, 0, sizeof(key));
    		memset(text, 0, sizeof(text));
    		len_key = sprintf(key, "%s%s%s", json_object_get_string(json_object_object_get(obj, "bmcode")),
    									json_object_get_string(json_object_object_get(obj, "custcode")),
    									json_object_get_string(json_object_object_get(obj, "sbcode")));
    		len_text = sprintf(text, "%s-编号%s", json_object_get_string(json_object_object_get(obj, "custname")),
    									json_object_get_string(json_object_object_get(obj, "sbcode")));
    		sprintf(tmpBuf+data_len, "%c%s%c%s", len_text, text, len_key, key);
    		data_len += len_text+len_key+2;
    }
    tmpBuf[0] = array_len;
		return add_pub_field(pub_data_stru, fldid, pub_data_stru->route_msg_type,
                         data_len, tmpBuf, 1);
}

