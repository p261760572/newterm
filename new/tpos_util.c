#include "base.h"
#include "var.h"
#include "tpos.h"
#include "general_util.h"
#include "db_qfunc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "secu.h"
#include "db_tools.h"
#include "assert.h"
#include "ibdcs.h"

int tpos_check_mac(glob_msg_stru * pub_data_stru) {
    int len,n,i,l;
    char tmp[1024],mac[17],return_code[3];
    unsigned char *p;
    len=0;
    //ICS_DEBUG(0);
    if(get_field_data_safe(pub_data_stru,
                           get_pub_field_id(pub_data_stru->in_msg_type,"TPDU"),
                           pub_data_stru->in_msg_type,tmp,sizeof(tmp))>0) {
        len=5;
    }
    if(get_field_data_safe(pub_data_stru,
                           get_pub_field_id(pub_data_stru->in_msg_type,"TEL_NO"),
                           pub_data_stru->in_msg_type,
                           tmp,sizeof(tmp))>0) {
        len=len + 33;
    }
    if(get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"0D"),
                           pub_data_stru->in_msg_type,tmp,sizeof(tmp))<=0) {
        dcs_log(0,0,"<%s>can not get 0D data",__FUNCTION__);
        return -1;
    }
    len=len+10; //去掉消息头
    memset(tmp,0,sizeof(tmp));
    if(pub_data_stru->in_cry_flag) {
        assert((pub_data_stru->src_len-16-len-1)<= sizeof(tmp));
        memcpy(tmp,pub_data_stru->src_buffer+len,pub_data_stru->src_len-16-len-1);
        p=(unsigned char *)pub_data_stru->src_buffer+len;
        /*
            if(((pub_data_stru->src_len-16-len-1)%16))
          {
              n=pub_data_stru->src_len-16-len-1 +(16-((pub_data_stru->src_len-16-len-1)%16));
          }
          else */
        n=pub_data_stru->src_len-16-len-1;
    } else {
        assert((pub_data_stru->src_len-8-len-1) <= sizeof(tmp));
        memcpy(tmp,pub_data_stru->src_buffer+len,pub_data_stru->src_len-8-len-1);
        p=(unsigned char *)pub_data_stru->src_buffer+len;
        /*
        if(((pub_data_stru->src_len-8-len-1)%8))
        {
         n=pub_data_stru->src_len-8-len-1 +(8-((pub_data_stru->src_len-8-len-1)%8));
        }
        else */
        n=pub_data_stru->src_len-8-len-1;
    }
    dcs_debug(p,n,"<%s> begin calc mac head len=%d",__FUNCTION__,len);

    if(pub_data_stru->in_cry_flag) {
        memset(tmp, 0, 16);
        for(i=0; i<n;)
            for(l=0; i<n&&l<16; l++,i++)
                tmp[l]=(unsigned char)p[i]^(unsigned char)tmp[l];
    } else {
        memset(tmp, 0, 8);
        for(i=0; i<n;)
            for(l=0; i<n&&l<8; l++,i++)
                tmp[l]=(unsigned char)p[i]^(unsigned char)tmp[l];
    }
    if(pub_data_stru->in_cry_flag) {
        if(CALCMAC(return_code, pub_data_stru->in_mac_index,
                   pub_data_stru->in_mac_key, 0, 16, tmp, tmp)<0) {
            dcs_log(0,0,"<%s> CALCMAC error mac_index=[%s]",__FUNCTION__,
                    pub_data_stru->in_mac_index);
            strcpy(pub_data_stru->center_result_code,"96");
            return -1;
        }
    } else {
        if(DESCALCMAC(return_code,pub_data_stru->in_mac_index,
                      pub_data_stru->in_mac_key,8,tmp,tmp) < 0) {
            dcs_log(0,0,"<%s> DESCALCMAC error mac_index=[%s]",__FUNCTION__,
                    pub_data_stru->in_mac_index);
            strcpy(pub_data_stru->center_result_code,"96");
            return -1;
        }
    }
    dcs_debug(0,0,"<%s> calc mac end",__FUNCTION__);
    if(memcmp(return_code,"00",2)!=0) {
        dcs_log(return_code,2,"<%s> jmj return fail",__FUNCTION__);
        return -1;
    }
    if(pub_data_stru->in_cry_flag) {
        asc_to_bcd((unsigned char *)mac,(unsigned char *)tmp,32,0);
        if(memcmp(pub_data_stru->src_buffer+pub_data_stru->src_len-16-1,mac,16)!=0) {
            strcpy(pub_data_stru->center_result_code,"A0");
            dcs_log(mac,16,"<%s>genrate mac",__FUNCTION__);
            dcs_log(pub_data_stru->src_buffer+pub_data_stru->src_len-16-1,16,
                    "<%s>recv mac",__FUNCTION__);
            return -1;
        }
    } else {
        asc_to_bcd((unsigned char *)mac,(unsigned char *)tmp,16,0);

        if(memcmp(pub_data_stru->src_buffer+pub_data_stru->src_len-8-1,mac,8)!=0) {
            strcpy(pub_data_stru->center_result_code,"A0");
            dcs_log(mac,8,"<%s>genrate mac",__FUNCTION__);
            dcs_log(pub_data_stru->src_buffer+pub_data_stru->src_len-8-1,8,
                    "<%s>recv mac",__FUNCTION__);
            return -1;
        }
    }
    return 1;
}

//修改电话号码处理校验20141011
//POS终端校验
int tpos_check_terminal(glob_msg_stru * pub_data_stru) {
    char psam[17],tmp[256], tel_no[256 + 1];
    struct TPOS_TERM_INFO terminfo;
    int i,id;

    memset(&terminfo,0,sizeof(terminfo));
    if(0>get_psam_no(pub_data_stru ,psam,sizeof(psam))) {
        dcs_log(0,0,"<%s> get_psam_no fail!",__FUNCTION__);
        return -1;
    }
    if(pub_data_stru->insti_work_type) {
        if(0>tpos_get_work_key(psam,pub_data_stru->in_pin_index,
                               pub_data_stru->in_mac_index,
                               pub_data_stru->in_data_index,
                               pub_data_stru->in_pin_key,
                               pub_data_stru->in_mac_key,
                               pub_data_stru->in_data_key)) {
            dcs_log(0,0,"<%s> tpos_get_work_key fail psam=%s ",__FUNCTION__,psam);
            strcpy(pub_data_stru->center_result_code,"96");
            return -1;
        }
        dcs_debug(0,0,"<%s>pin_index=[%s] pin_key=[%s]\n mac_index=[%s] mac_key[%s]\n"
                  "cd_index=[%s] cd_key=[%s]",
                  __FUNCTION__,pub_data_stru->in_pin_index,
                  pub_data_stru->in_pin_key,pub_data_stru->in_mac_index,
                  pub_data_stru->in_mac_key,pub_data_stru->in_data_index,
                  pub_data_stru->in_data_key);
        snprintf(pub_data_stru->route_mac_index,6,"%s",pub_data_stru->in_pin_index);
        snprintf(pub_data_stru->route_mac_key,33,"%s",pub_data_stru->in_mac_key);
    }
    i=get_tpos_info(psam,&terminfo);
    if(0 > i) {
        strcpy(pub_data_stru->center_result_code,"TZ1");
        return -1;
    }
    if(terminfo.status[0] ==0x30) {
        strcpy(pub_data_stru->center_result_code,"TZ1");
        dcs_log(0,0,"<%s> status is %s",__FUNCTION__,terminfo.status);
        return -1;
    }
    if(terminfo.is_bind[0]==0x31) {
        id=get_pub_field_id(pub_data_stru->in_msg_type,"TEL_NO");
        if(0>get_field_data_safe(pub_data_stru,id,pub_data_stru->in_msg_type,
                                 tmp,sizeof(tmp))) {
            strcpy(pub_data_stru->center_result_code,"TZ4");
            dcs_log(0,0,"<%s> tel_no is null ! ",__FUNCTION__);
            return -1;;
        } else {
            char *p;
            memset(tel_no, 0, sizeof(tel_no));

            p=tel_no+4;
            if((unsigned char)tmp[5] <0x30) {

                bcd_to_asc((unsigned char *)p,(unsigned char *)tmp+5,16,0);
                while(*p ==0x30) p++;
                if(strlen(p) <=8) {
                    p-= 4;
                    memcpy(p,"0731",4);
                } else
                    p--;
            } else {
                memcpy(p, tmp + 5, 16);
                rtrim(p);
            }
            rtrim(terminfo.tel_no);
            if(memcmp(p, terminfo.tel_no, strlen(terminfo.tel_no))) {
                if(! is_trust_tel(p)) {
                    strcpy(pub_data_stru->center_result_code,"TZ4");
                    dcs_log(0,0,"<tpos_check_terminal> tel_no1=[%s],tel_no2=[%s] != ", p, terminfo.tel_no);
                    return -1;
                }
                pub_data_stru->tmp_order[2] =0x31; // 安全线路不校验终端sn
            }
        }
    } else {
        id=get_pub_field_id(pub_data_stru->in_msg_type,"TEL_NO");
        if(0<get_field_data_safe(pub_data_stru,id,pub_data_stru->in_msg_type,
                                 tmp,sizeof(tmp))) {
            char *p;
            memset(tel_no, 0, sizeof(tel_no));
            p=tel_no+4;
            if((unsigned char)tmp[5] <0x30) {

                bcd_to_asc((unsigned char *)p,(unsigned char *)tmp+5,16,0);
                while(*p ==0x30) p++;
                if(strlen(p) <=8) {
                    p-= 4;
                    memcpy(p,"0731",4);
                } else
                    p--;
            } else {
                memcpy(p, tmp + 5, 16);
                rtrim(p);
            }
            if(is_trust_tel(p))  pub_data_stru->tmp_order[2] =0x31; // 安全线路不校验终端sn
        }
    }
    pub_data_stru->tmp_order[1]=terminfo.menu_update_flag[0];
    dcs_debug(0,0,"<%s> check pass!",__FUNCTION__);
    return 1;
}

int tpos_check_prog_ver(glob_msg_stru * pub_data_stru) {
    int id;
    char tmp[16];
    ICS_DEBUG(0);
    id=get_pub_field_id(pub_data_stru->in_msg_type,"PROG_VER");
    if(id < 0) return -1;
    memset(tmp,0,sizeof(tmp));
    if(0>get_field_data_safe(pub_data_stru,id,pub_data_stru->in_msg_type,
                             tmp,sizeof(tmp)));
    if(strcmp(tmp,"2014")==0 ||
       strcmp(tmp,"2015")==0 ||
       strcmp(tmp,"2013")==0)
        return 1;
    strcpy(pub_data_stru->center_result_code,"TZ2");
    return -1;
}

int tpos_check_menu_ver(glob_msg_stru * pub_data_stru) {
    ICS_DEBUG(0);
    return 1;
}

int tpos_login(glob_msg_stru * pub_data_stru) {
    char tmp[128],psam[17],tmp1[128];
//  ICS_DEBUG(0);
    dcs_debug(0,0,"<%s>entery tpos login proc",__FUNCTION__);
    /*
        time(&t);
      tm = localtime(&t);

        snprintf(tmp,sizeof(tmp),"%4d%02d%02d%02d%02d%02d",
                tm->tm_year+1900,
                tm->tm_mon + 1,
                tm->tm_mday,
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec);
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type,"DATE"),
                      pub_data_stru->route_msg_type,
                      8,tmp,1);
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type,"TIME"),
                      pub_data_stru->route_msg_type,
                      6,tmp+8,1);
        */
    if(0>get_psam_no(pub_data_stru,psam,sizeof(psam))) {
        dcs_log(0,0,"<%s> can not get psam!",__FUNCTION__);
        return -1;
    }
    memset(tmp1,0,sizeof(tmp1));
    if(get_field_data_safe(pub_data_stru,
                           get_pub_field_id(pub_data_stru->in_msg_type,"02"),
                           pub_data_stru->in_msg_type,tmp1,sizeof(tmp1))<=0) {
        dcs_log(0,0,"<%s> can not get sn!",__FUNCTION__);
        return -1;
    }
    if(pub_data_stru->tmp_order[2] !=0x31) { // 安全线路不交易终端sn
        memset(tmp,0,sizeof(tmp));
        if(0>get_tpos_sn(psam,tmp)) {
            strcpy(pub_data_stru->center_result_code,"TZ1");
            return 1;
        }
        rtrim(tmp);
        if(strlen(tmp)<=0 || (memcmp(tmp,"FFFFFFFFFFFFFFFF",16)==0)) {
            update_tpos_sn(psam,tmp1);
        } else if(strcmp(tmp1,tmp)!=0) {
            dcs_log(0,0,"<%s>  sn不一致! db_sn=[%s] pos_sn=[%s]",__FUNCTION__,tmp,tmp1);
            strcpy(pub_data_stru->center_result_code,"TZ3");
            return 1;
        }
    }
    if(0 >tpos_gen_work_key(pub_data_stru,psam))
        strcpy(pub_data_stru->center_result_code,"96");

    dcs_debug(0,0,"<%s> proc end",__FUNCTION__);
    return 1;
}

int tpos_download_para(glob_msg_stru * pub_data_stru) {
    struct TPOS_TERM_INFO  terminfo;
    struct TPOS_MENU_NODE node;
    struct MENU_NODE_INFO node_info;
    struct TPOS_PARA_INFO para_info;
    struct  tm *tm;
    time_t  t;
    char psam[18],menu_ver[9],tmp[512],para_ver[6],tmp1[6][512],node_set[200];
    char str[512],str1[256],*p1,*p2,*p3,*p4;
    int id,stack[30],use,cnt,n,i,k,leaf,s[6],l[6];
    //ICS_DEBUG(0);
    memset(&terminfo,0,sizeof(terminfo));
    memset(psam,0,sizeof(psam));
    memset(node_set,0,sizeof(node_set));
    use=0;
    cnt=0;
    if(0>get_psam_no(pub_data_stru ,psam,sizeof(psam))) {
        dcs_log(0,0,"<%s> get_psam_no fail!",__FUNCTION__);
        return -1;
    }
    if(0>get_tpos_info(psam,&terminfo)) {
        dcs_log(0,0,"<%s> get_tpos_info fail!",__FUNCTION__);
        return -1;
    }

    memset(menu_ver,0,sizeof(menu_ver));
    if(0>get_field_data_safe(pub_data_stru,
                             get_pub_field_id(pub_data_stru->in_msg_type,
                                     "MENU_VER"),
                             pub_data_stru->in_msg_type,menu_ver,sizeof(menu_ver))) {
        dcs_log(0,0,"<%s> get_field_data menu_ver FAIL!",__FUNCTION__);
        return -1;
    }

    time(&t);
    tm = localtime(&t);
    snprintf(tmp,sizeof(tmp),"%4d%02d%02d%02d%02d%02d",
             tm->tm_year+1900,
             tm->tm_mon + 1,
             tm->tm_mday,
             tm->tm_hour,
             tm->tm_min,
             tm->tm_sec);

    add_pub_field(pub_data_stru,
                  get_pub_field_id(pub_data_stru->route_msg_type,"DATE"),
                  pub_data_stru->route_msg_type,
                  8,tmp,1);
    add_pub_field(pub_data_stru,
                  get_pub_field_id(pub_data_stru->route_msg_type,"TIME"),
                  pub_data_stru->route_msg_type,
                  6,tmp+8,1);

    dcs_debug(0,0,"<%s> cmp menu_ver",__FUNCTION__);
    if(strcmp(menu_ver,terminfo.menu_ver)!=0) {
        //检查菜单是否完整
        dcs_debug(0,0,"<%s>traversal menu is complete ?",__FUNCTION__);

        if(0> menu_preorder_traversal(terminfo.menu_ver ,NULL)) {
            dcs_log(0,0,"<%s> menu_preorder_traversal fail!",__FUNCTION__);
            strcpy(pub_data_stru->center_result_code,"96");
            return 1;
        }
        dcs_debug(0,0,"<%s>traversal menu complete end",__FUNCTION__);

        if(0>tpos_reset_download(psam,0)) {
            dcs_log(0,0,"<%s> tpos_reset_download fail!",__FUNCTION__);
            strcpy(pub_data_stru->center_result_code,"96");
            return 1;
        }

        terminfo.last_node_set[0]=0x00;
        terminfo.download_flag[0]=0x31;
        terminfo.last_menu_para_step=0;
        dcs_debug(0,0,"<%s>  tpos_reset_download succ!,menu_ver=[%s]",
                  __FUNCTION__,terminfo.menu_ver);
        if(strcmp(menu_ver,"00000000")!=0) {

            snprintf(menu_ver,sizeof(menu_ver),"%s",terminfo.menu_ver);
            if(0>get_menu_root_node(menu_ver ,&node)) {
                dcs_log(0,0,"<%s> get_menu_root_node fail!",__FUNCTION__);
                strcpy(pub_data_stru->center_result_code,"96");
                return 1;
            }
            if(0>get_node_info(menu_ver,node.node_id,&node_info)) {
                dcs_log(0,0,"<%s> get_node_info fail",__FUNCTION__);
                strcpy(pub_data_stru->center_result_code,"96");
                return 1;
            }
            strcpy(pub_data_stru->center_result_code,"00");
            tmp[0]=1;
            n=1;
            memcpy(tmp+n,"\x30",1);
            n++;
            tmp[n]=node.node_id/256;
            n++;
            tmp[n]=node.node_id%256;
            n++;
            add_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->route_msg_type,"18"),
                          pub_data_stru->route_msg_type,
                          n,tmp,1);
            add_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->route_msg_type,"24"),
                          pub_data_stru->route_msg_type,
                          0,tmp,1);
            add_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->route_msg_type,"25"),
                          pub_data_stru->route_msg_type,
                          0,tmp,1);
            update_pub_field(pub_data_stru,
                             get_pub_field_id(pub_data_stru->route_msg_type,"MENU_VER"),
                             pub_data_stru->route_msg_type,
                             8,"00000000",0);
            dcs_debug(0,0,"<%s> delete old menu root!",__FUNCTION__);
            return 1;
        }
        update_pub_field(pub_data_stru,
                         get_pub_field_id(pub_data_stru->route_msg_type,"MENU_VER"),
                         pub_data_stru->route_msg_type,
                         8,terminfo.menu_ver,0);
        memset(menu_ver,0,sizeof(menu_ver));
        get_field_data_safe(pub_data_stru,
                            get_pub_field_id(pub_data_stru->in_msg_type,"MENU_VER"),
                            pub_data_stru->in_msg_type,menu_ver,sizeof(menu_ver));
        dcs_debug(0,0,"<%s>  menu_ver=[%s]",__FUNCTION__,menu_ver);
        snprintf(menu_ver,sizeof(menu_ver),"%s",terminfo.menu_ver);
    } else {

//          memset(tmp,0,sizeof(tmp));
        if((n=get_field_data_safe(pub_data_stru, get_pub_field_id(pub_data_stru->in_msg_type,"18"),
                                  pub_data_stru->in_msg_type,tmp,sizeof(tmp)))>0) {
            tmp[n]=0x00;
            if(strcmp(tmp,"00")==0) {
                empty_tmp_para(psam);
                terminfo.last_node_set[0]=0x00;
                del_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"18"),
                              pub_data_stru->in_msg_type, 0);
            } else {
                dcs_log(0,0,"<tpos_download_para>  recv para download result code=[%s],psam=[%s]",tmp,psam);
                return -1;
            }

        }
        if((n=get_field_data_safe(pub_data_stru, get_pub_field_id(pub_data_stru->in_msg_type,"19"),
                                  pub_data_stru->in_msg_type,tmp,sizeof(tmp)))>0) {
            tmp[n]=0x00;
            if(strcmp(tmp,"00")==0) {
                empty_tmp_para(psam);
                terminfo.last_node_set[0]=0x00;
                del_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"19"),
                              pub_data_stru->in_msg_type,0);
            } else {
                dcs_log(0,0,"<tpos_download_para>  recv para download result code=[%s],psam=[%s]",tmp,psam);
                return -1;
            }
        }
        if((n=get_field_data_safe(pub_data_stru, get_pub_field_id(pub_data_stru->in_msg_type,"1B"),
                                  pub_data_stru->in_msg_type,tmp,sizeof(tmp)))>0) {
            tmp[n]=0x00;
            if(strcmp(tmp,"00")==0) {
                empty_tmp_para(psam);
                terminfo.last_node_set[0]=0x00;
                del_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"1B"),
                              pub_data_stru->in_msg_type,0);
            } else {
                dcs_log(0,0,"<tpos_download_para>  recv para download result code=[%s],psam=[%s]",tmp,psam);
                return -1;
            }
        }
        if((n=get_field_data_safe(pub_data_stru, get_pub_field_id(pub_data_stru->in_msg_type,"1A"),
                                  pub_data_stru->in_msg_type, tmp, sizeof(tmp)))>0) {
            tmp[n]=0x00;
            if(strcmp(tmp,"00")==0) {
                empty_tmp_para(psam);
                terminfo.last_node_set[0]=0x00;
                del_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"1A"),
                              pub_data_stru->in_msg_type,0);
            } else {
                dcs_log(0,0,"<tpos_download_para>  recv para download result code=[%s],psam=[%s]",tmp,psam);
                return -1;
            }
        }
        if((n=get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"1F"),
                                  pub_data_stru->in_msg_type,tmp,sizeof(tmp)))>0) {
            tmp[n]=0x00;
            if(strcmp(tmp,"00")==0) {
                empty_tmp_para(psam);
                terminfo.last_node_set[0]=0x00;
                del_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"1F"),
                              pub_data_stru->in_msg_type,0);
            } else {
                dcs_log(0,0,"<tpos_download_para>  recv para download result code=[%s],psam=[%s]",tmp,psam);
                return -1;
            }
        }
        if((n=get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"1C"),
                                  pub_data_stru->in_msg_type,tmp,sizeof(tmp)))>0) {
            tmp[n]=0x00;
            if(strcmp(tmp,"00")==0) {
                empty_tmp_para(psam);
                terminfo.last_node_set[0]=0x00;
                del_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"1C"),
                              pub_data_stru->in_msg_type,0);
            } else {
                dcs_log(0,0,"<tpos_download_para>  recv para download result code=[%s],psam=[%s]",tmp,psam);
                return -1;
            }
        }
        if((n=get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"16"),
                                  pub_data_stru->in_msg_type,tmp,sizeof(tmp)))>0) {
            tmp[n]=0x00;
            if(strcmp(tmp,"00")==0) {
                empty_tmp_para(psam);
                terminfo.last_node_set[0]=0x00;
                del_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"16"),
                              pub_data_stru->in_msg_type,0);
            } else {
                dcs_log(0,0,"<tpos_download_para>  recv para download result code=[%s],psam=[%s]",tmp,psam);
                return -1;
            }
        }
        if((n=get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"17"),
                                  pub_data_stru->in_msg_type,tmp,sizeof(tmp)))>0) {
            tmp[n]=0x00;
            if(strcmp(tmp,"00")==0) {
                empty_tmp_para(psam);
                terminfo.last_node_set[0]=0x00;
                del_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"17"),
                              pub_data_stru->in_msg_type,0);
            } else {
                dcs_log(0,0,"<tpos_download_para>  recv para download result code=[%s],psam=[%s]",tmp,psam);
                return -1;
            }
        }
        if((n=get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"3C"),
                                  pub_data_stru->in_msg_type,tmp,sizeof(tmp)))>0) {
            tmp[n]=0x00;
            if(strcmp(tmp,"00")==0) {
                empty_tmp_para(psam);
                terminfo.last_node_set[0]=0x00;
                del_pub_field(pub_data_stru,get_pub_field_id(pub_data_stru->in_msg_type,"3C"),
                              pub_data_stru->in_msg_type,0);
            } else {
                dcs_log(0,0,"<tpos_download_para>  recv para download result code=[%s],psam=[%s]",tmp,psam);
                return -1;
            }
        }
    }
    //检查上次下发的参数是否完成处理
    dcs_debug(0,0,"FILE:%s,%d <tpos_download_para> node_set=[%s]",__FILE__,__LINE__,terminfo.last_node_set);
//  dcs_debug(0,0,"%s,%d:psam=%s",__FILE__,__LINE__,psam);
    if(get_last_part_para(pub_data_stru,psam,menu_ver,terminfo.last_node_set) >0) {
        dcs_debug(0,0,"download para for pre");
        strcpy(pub_data_stru->center_result_code,"00");
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->in_msg_type,"24"),
                      pub_data_stru->in_msg_type,
                      0,tmp,1);
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->in_msg_type,"25"),
                      pub_data_stru->in_msg_type,
                      0,tmp,1);
        return 1;
    }
//  dcs_debug(0,0,"%s,%d:psam=%s",__FILE__,__LINE__,psam);
    // 检查菜单项是否需更新
    //检查部分更新标识以及最后更新的iD是否为>0
    if(terminfo.download_flag[0] == 0x31 && terminfo.last_menu_para_step >0) { //部分更新
        //根据更新索引id检查是否还有需更新的节点

        i=0;

        dcs_debug(0,0,"<%s> enter menu section update",__FUNCTION__);
        // 判断菜单参数是否还有未更新的
        cnt =0;
        n=0;
        node_info.step=terminfo.last_menu_para_step;
        while(n<400) {
//         dcs_debug(0,0,"<tpos_download_para> get next menu node, step[%d]",node_info.step);
            if(get_next_menu_node(menu_ver,node_info.step,&node_info)<0) {
                dcs_debug(0,0,"<%s>exit while",__FUNCTION__);
                break;
            }
//         dcs_debug(0,0,"<tpos_download_para> get next menu node info, step[%d]",node_info.step);
            node.node_id=node_info.node_id;
            if(node_info.use[0]==0x31)
                if(0> menu_preorder_traversal(terminfo.menu_ver ,&node)) {
                    dcs_log(0,0,"<%s> menu_preorder_traversal fail!,node_id=%d",
                            __FUNCTION__,node.node_id);
                    strcpy(pub_data_stru->center_result_code,"96");
                    return 1;
                }
            if(cnt == 0) n++;
            cnt++;
            snprintf(node_set+strlen(node_set),sizeof(node_set)-strlen(node_set),
                     "%2.2s%d,","18",node_info.node_id);
            if(node_info.use[0]==0x31)
                memcpy(tmp+n,"\x31",1);
            else
                memcpy(tmp+n,"\x30",1);
            n++;
            tmp[n]=node.node_id/256;
            n++;
            tmp[n]=node.node_id%256;
            n++;
            if(node_info.use[0]==0x30) continue;
            tmp[n]=node.pre_node_id/256;//节点ID
            if(node.is_leaf[0]==0x31) {
                tmp[n]= tmp[n]|0x80; //表示是叶结点
                leaf=1;
            } else {
                tmp[n]= tmp[n]&0x7F;
                leaf=0;
            }
            if(node.pre_flag[0]==0x31)
                tmp[n]= tmp[n] | 0x40; //为前结点的右结点
            else
                tmp[n]= tmp[n] & 0xBF;
            n++;
            tmp[n]=node.pre_node_id%256; //前驱节点ID
            n++;
            if(leaf && node_info.use[0] !=0x30) {
                memcpy(tmp+n,node_info.func_code,3); //功能代码
                n=n+3;
                if(node_info.void_flag[0] == 0x31)
                    tmp[n]=0x80;      //冲正标识
                else
                    tmp[n]=0x00;      //冲正标识
                n++;
                tmp[n]= node_info.func_disp_indx;   //功能提示索引
                n++;
                rtrim(node_info.op_code);
                tmp[n]= strlen(node_info.op_code)/2; //操作代码长度
                n++;
                asc_to_bcd((unsigned char *)tmp+n,(unsigned char *)node_info.op_code,strlen(node_info.op_code),0);//操作代码集合
                n=n+strlen(node_info.op_code)/2;
            }
            if(node.pre_node_id >0) {
                rtrim(node_info.title);
                tmp[n]=strlen(node_info.title);
                n++;
                memcpy(tmp+n,node_info.title,strlen(node_info.title));
                n=n+strlen(node_info.title);
            } else {
                tmp[n]=0x00;
                n++;
            }
        }
        dcs_debug(0,0,"<%s> part update menu node complete",__FUNCTION__);
        if(cnt >0) {
            tmp[0]=cnt;
            add_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->route_msg_type,"18"),
                          pub_data_stru->route_msg_type,n,tmp,1);

            //保存本次下载的参数id集合
            //保存本次更新到了哪一个终端参数(菜单)

            n=_get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->route_msg_type,"18"),pub_data_stru->route_msg_type,tmp,1,sizeof(tmp));

            save_download_para_info(1,node_info.step,tmp,psam);
        }
        //判断其他参数是否还有未更新的
        if(0>get_para_ver(menu_ver,para_ver)) {
            dcs_log(0,0,"<%s> error",__FUNCTION__);
            strcpy(pub_data_stru->center_result_code,"96");
            return 1;
        }
        for(i=0 ; i<6; i++) {
            s[i]=1,l[i]=0;
        }
        para_info.step=terminfo.last_other_para_step;
        dcs_debug(0,0,"<%s> begin get_next_part_para cnt=%d",__FUNCTION__,cnt);
        while(n <310) {
            if(get_next_part_para(para_ver,para_info.step,&para_info)<0) break;

            snprintf(node_set+strlen(node_set),sizeof(node_set)-strlen(node_set)-1,
                     "%2.2s%d,",para_info.para_type,para_info.id);
            if(strcmp(para_info.para_type,"19")==0) { //更新功能提示信息
                tmp1[0][s[0]]=para_info.id;
                s[0]++;
                n++;
                if(para_info.use[0] == 0x31) {
                    tmp1[0][s[0]]=0x31;
                    s[0]++;
                    n++;
                    k=s[0];//保留长度位置
                    s[0]++;
                    n++;
                    p4=NULL;
                    p2=my_split(para_info.control_info,',',str,sizeof(str));
                    if(p2 != NULL) {
                        i=atoi(str);
                        tmp1[0][s[0]]=i; //显示信息条数
                        s[0]++;
                        n++;
                        p3=NULL;
                        p1=my_split(para_info.detail,'|',str1,sizeof(str1));
                        if(p1 == NULL) {
                            dcs_log(0,0,"<%s> 19 para format error: no found detail!",
                                    __FUNCTION__);
                            return -1;
                        }
                        for(; ;) {
                            p2=my_split(p2,',',str,sizeof(str));
                            if(p2 == NULL || strlen(str)!=2) {
                                dcs_log(0,0,"<%s> 19 para format error: control info error!",
                                        __FUNCTION__);
                                return -1;
                            }
                            asc_to_bcd((unsigned char *)tmp1[0]+s[0],(unsigned char *)str,2,0); //显示格式码1字节
                            s[0]++;
                            n++;
                            rtrim(str1);
                            tmp1[0][s[0]]=strlen(str1);   //显示数据每条的长度1字节
                            s[0]++;
                            n++;
                            memcpy(tmp1[0]+s[0],str1,strlen(str1));
                            n=n+strlen(str1);
                            s[0]=s[0]+strlen(str1);
                            i--;
                            if(i >0) {
                                p1=my_split(p1,'|',str1,sizeof(str1));
                                if(p1 == NULL) {
                                    dcs_log(0,0,"<%s> 19 para format error: no found detail!",
                                            __FUNCTION__);
                                    return -1;
                                }
                            } else break;
                        }
                    } else {
                        dcs_log(0,0,"<%s> 19 para format error:can not found control info!",__FUNCTION__);
                        return -1;
                    }
                    tmp1[0][k]=s[0]-k-1;

                } else {
                    tmp1[0][s[0]]=0x30;
                    s[0]++;
                    n++;
                }
                if(l[0]==0) n++;
                l[0]++;
            } else if(strcmp(para_info.para_type,"1A")==0) { //更新操作提示信息

                tmp1[1][s[1]]=para_info.id;
                s[1]++;
                n++;
                if(para_info.use[0] == 0x31) {
                    tmp1[1][s[1]]=0x31;
                    s[1]++;
                    n++;
                    k=s[1]; //保留长度位置
                    s[1]++;
                    n++;
                    p4=NULL;
                    p2=my_split(para_info.control_info,',',str,sizeof(str));
                    if(p2 != NULL) {
                        i=atoi(str);
                        tmp1[1][s[1]]=i; //显示信息条数
                        s[1]++;
                        n++;
                        p3=NULL;
                        p1=my_split(para_info.detail,'|',str1,sizeof(str1));
                        if(p1 == NULL) {
                            dcs_log(0,0,"<%s> 1A para format error: no found detail!",__FUNCTION__);
                            return -1;
                        }
                        for(; ;) {
                            p2=my_split(p2,',',str,sizeof(str));
                            if(p2 == NULL || strlen(str)!=2) {
                                dcs_log(0,0,"<%s> 1A para format error: control info error!",__FUNCTION__);
                                return -1;
                            }
                            asc_to_bcd((unsigned char *)tmp1[1]+s[1],(unsigned char *)str,2,0); //显示格式码1字节
                            s[1]++;
                            n++;
                            rtrim(str1);
                            tmp1[1][s[1]]=strlen(str1);   //显示数据每条的长度1字节
                            s[1]++;
                            n++;
                            memcpy(tmp1[1]+s[1],str1,strlen(str1));
                            n=n+strlen(str1);
                            s[1]=s[1]+strlen(str1);
                            i--;
                            if((i) >0) {
                                p1=my_split(p1,'|',str1,sizeof(str1));
                                if(p1 == NULL) {
                                    dcs_log(0,0,"<%s> 1A para format error: no found detail!",__FUNCTION__);
                                    return -1;
                                }
                            } else break;
                        }
                    }
                    tmp1[1][k]=s[1]-k-1;
                } else {
                    tmp1[1][s[1]]=0x00;
                    s[1]++;
                    n++;
                }
                if(l[1]==0) n++;
                l[1]++;
            } else if(strcmp(para_info.para_type,"1B")==0) { //更新首页信息
                tmp1[2][s[2]]=para_info.id;
                s[2]++;
                n++;
                memcpy(tmp1[2],para_info.detail,strlen(para_info.detail));
                s[2]=strlen(para_info.detail);
                n=n+strlen(para_info.detail);
                if(l[2]==0) n++;
                l[2]++;
            } else if(strcmp(para_info.para_type,"1C")==0) { //更新打印模板信息
                tmp1[3][s[3]]=para_info.id;
                s[3]++;
                n++;

                rtrim(para_info.control_info);
                if(strlen(para_info.control_info)!=2) {
                    dcs_log(0,0,"<%s> 1C para format error: control info error!",__FUNCTION__);
                    return -1;
                }
                asc_to_bcd((unsigned char *)tmp1[3]+s[3],(unsigned char *)para_info.control_info,2,0); //字体格式码1字节
                s[3]++;
                n++;
                tmp1[3][s[3]]=strlen(para_info.detail);   //打印数据每条的长度1字节
                s[3]++;
                n++;
                memcpy(tmp1[3]+s[3],para_info.detail,strlen(para_info.detail));
                s[3]=s[3]+strlen(para_info.detail);
                n=n+strlen(para_info.detail);

                if(l[3]==0) n++;
                l[3]++;
            } else if(strcmp(para_info.para_type,"16")==0) { //更新终端参数信息
                tmp1[4][s[4]]=para_info.id;
                s[4]++;
                n++;
                if(para_info.id == 1) {
                    tmp1[4][s[4]]=0x01;
                    s[4]++;
                    n++;
                    tmp1[4][s[4]]=para_info.detail[0];
                    s[4]++;
                    n++;
                } else if(para_info.id == 2) {
                    tmp1[4][s[4]]=0x01;
                    s[4]++;
                    n++;
                    tmp1[4][s[4]]=atoi(para_info.detail);
                    s[4]++;
                    n++;
                } else if(para_info.id == 6) {
                    tmp1[4][s[4]]=strlen(para_info.detail);
                    s[4]++;
                    n++;
                    snprintf(tmp1[4]+s[4],sizeof(tmp1[4])-s[4],"%s",para_info.detail);
                    s[4]=s[4]+strlen(para_info.detail);
                    n = n + strlen(para_info.detail);
                } else if(para_info.id == 5) {
                    tmp1[4][s[4]]=(strlen(para_info.detail)+1)/2;   //每条的长度1字节
                    s[4]++;
                    n++;
                    asc_to_bcd((unsigned char *)tmp1[4]+s[4],(unsigned char *)para_info.detail,((strlen(para_info.detail)+1)/2)*2,0);
                    n=n+(strlen(para_info.detail)+1)/2;
                    s[4]=s[4]+(strlen(para_info.detail)+1)/2;
                } else if(para_info.id == 3 || para_info.id == 4) {
                    k=s[4];
                    use=1;
                    s[4]++;
                    n++;
                    i=atoi(para_info.control_info);
                    tmp1[4][s[4]]=i; //条数
                    s[4]++;
                    n++;
                    p1=my_split(para_info.detail,'|',str,sizeof(str));
                    if(p1 == NULL) {
                        dcs_log(0,0,"<%s> 16 para format error: no found detail!",__FUNCTION__);
                        return -1;
                    }
                    for(; i>0; i--) {
                        if(p1 == NULL) {
                            dcs_log(0,0,"<%s> 16 para format error: no found detail!",__FUNCTION__);
                            return -1;
                        }
                        rtrim(str);
                        if(para_info.id==3) {
                            tmp1[4][s[4]]=strlen(str);   //每条的长度1字节
                            s[4]++;
                            n++;
                            memcpy(tmp1[4]+s[4],str,strlen(str));
                            n=n+strlen(str);
                            s[4]=s[4]+strlen(str);
                            use=use+strlen(str)+1;
                        } else {
                            tmp1[4][s[4]]=(strlen(str)+1)/2;   //每条的长度1字节
                            s[4]++;
                            n++;
                            asc_to_bcd((unsigned char *)tmp1[4]+s[4],(unsigned char *)str,((strlen(str)+1)/2)*2,0);
                            n=n+(strlen(str)+1)/2;
                            s[4]=s[4]+(strlen(str)+1)/2;
                            use=use+(strlen(str)+1)/2+1;
                        }
                        p1=my_split(p1,'|',str,sizeof(str));
                    }
                    tmp1[4][k]=use;
                } else if(para_info.id == 7) {
                    tmp1[4][s[4]]=strlen(terminfo.name);
                    s[4]++;
                    n++;
                    snprintf(tmp1[4]+s[4],sizeof(tmp1[4])-s[4],"%s",terminfo.name);
                    s[4]=s[4]+strlen(terminfo.name);
                    n = n + strlen(terminfo.name);
                } else {
                    dcs_log(0,0,"<%s> can not recognition para type para_info.id[%d]",
                            __FUNCTION__, para_info.id);
                    return -1;
                }
                dcs_debug(0,0,"<%s>id%d end",__FUNCTION__,para_info.id);
                if(l[4]==0) n++;
                l[4]++;
            } else if(strcmp(para_info.para_type,"17")==0) { //选择模板信息
                tmp1[5][s[5]]=para_info.id;
                s[5]++;
                n++;
                if(para_info.use[0] == 0x31) {
                    tmp1[5][s[5]]=0x31;
                    s[5]++;
                    n++;
                    i=atoi(para_info.control_info);
                    if(i <=0 || i > 60) {
                        dcs_log(0,0,"<%s> 17 para format error: !",__FUNCTION__);
                        return -1;
                    }
                    tmp1[5][s[5]]=i; //信息条数
                    s[5]++;
                    n++;
                    dcs_debug(0,0,"<%s>detail=[%s]i=%d",__FUNCTION__,para_info.detail,i);
                    for(p1=my_split(para_info.detail,'|',str,sizeof(str)) ; i>0; i--) {
                        if(p1 == NULL) {
                            dcs_log(0,0,"<%s> 17 para format error: no found detail! i=%d",
                                    __FUNCTION__,i);
                            return -1;
                        }
                        tmp1[5][s[5]]=strlen(str);
                        s[5]++;
                        n++;
                        memcpy(tmp1[5]+s[5],str,strlen(str));
                        n=n+strlen(str);
                        s[5]=s[5]+strlen(str);
                        p1=my_split(p1,'|',str,sizeof(str));
                        if(p1 == NULL) {
                            dcs_log(0,0,"<%s> 17 para format error: no found detail! i=%d",
                                    __FUNCTION__,i);
                            return -1;
                        }
                        tmp1[5][s[5]]=strlen(str);
                        s[5]++;
                        n++;
                        memcpy(tmp1[5]+s[5],str,strlen(str));
                        n=n+strlen(str);
                        s[5]=s[5]+strlen(str);
                        p1=my_split(p1,'|',str,sizeof(str));
                    }
                } else {
                    tmp1[5][s[5]]=0x30;
                    s[5]++;
                    n++;
                }
                if(l[5]==0) n++;
                l[5]++;
            } else {
                dcs_log(0,0,"<%s> para format error: type[%d]",__FUNCTION__,para_info.para_type);
                return -1;
            }
        }

        if(l[0] >0) {
            tmp1[0][0]=l[0];
            add_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->route_msg_type,"19"),
                          pub_data_stru->route_msg_type,
                          s[0],tmp1[0],1);
            dcs_debug(tmp1[0],s[0],"<%s> 19 data len=%d",__FUNCTION__,s[0]);
        }
        if(l[1] >0) {
            tmp1[1][0]=l[1];
            add_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->route_msg_type,"1A"),
                          pub_data_stru->route_msg_type,
                          s[1],tmp1[1],1);
            dcs_debug(tmp1[1],s[1],"<%s> 1A data len=%d",__FUNCTION__,s[1]);
        }
        if(l[2] >0) {
            add_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->route_msg_type,"1B"),
                          pub_data_stru->route_msg_type,
                          s[2],tmp1[2],1);
            dcs_debug(tmp1[2],s[2],"<%s> 1B data len=%d",__FUNCTION__,s[2]);
        }
        if(l[3] >0) {
            tmp1[3][0]=l[3];
            add_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->route_msg_type,"1C"),
                          pub_data_stru->route_msg_type,
                          s[3],tmp1[3],1);
            dcs_debug(tmp1[3],s[3],"<%s> 1C data len=%d",__FUNCTION__,s[3]);
        }
        if(l[4] >0) {
            tmp1[4][0]=l[4];
            add_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->route_msg_type,"16"),
                          pub_data_stru->route_msg_type,
                          s[4],tmp1[4],1);
            dcs_debug(tmp1[4],s[4],"<%s> 16 data len=%d",__FUNCTION__,s[4]);
        }
        if(l[5] >0) {
            tmp1[5][0]=l[5];
            add_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->route_msg_type,"17"),
                          pub_data_stru->route_msg_type,
                          s[5],tmp1[5],1);
            dcs_debug(tmp1[5],s[5],"<%s> 17 data len=%d",__FUNCTION__,s[5]);
        }
        //保存本次下载的参数id集合
        //保存本次更新到了哪一个终端参数(菜单)
        if(strlen(node_set)>0) node_set[strlen(node_set)-1]=0x00;
        if(para_info.step >terminfo.last_other_para_step)
            save_download_para_info(0,para_info.step,node_set,psam);
        strcpy(pub_data_stru->center_result_code,"00");
        if(strlen(node_set)>0) {
            add_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->in_msg_type,"24"),
                          pub_data_stru->route_msg_type,
                          0,tmp,1);
            add_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->in_msg_type,"25"),
                          pub_data_stru->route_msg_type,
                          0,tmp,1);
        } else if(get_ic_para(pub_data_stru,&terminfo)>0) { //IC卡参数下载
            add_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->in_msg_type,"24"),
                          pub_data_stru->route_msg_type,
                          0,tmp,1);
            add_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->in_msg_type,"25"),
                          pub_data_stru->route_msg_type,
                          0,tmp,1);
        } else {
            del_pub_field(pub_data_stru,
                          get_pub_field_id(pub_data_stru->in_msg_type,"0D"),
                          pub_data_stru->in_msg_type,
                          0);
            update_auto_download_flag(terminfo.psam);
            dcs_debug(0,0,"<%s>para update end!",__FUNCTION__);
        }
        return 1;
    } else { //菜单全部更新处理
        cnt=0;
        dcs_debug(0,0,"<%s> db_init_stack begin!",__FUNCTION__);
        use=db_init_stack(terminfo.stack_detail,stack,sizeof(stack));
        if(use <0) {
            dcs_log(0,0,"<%s> db_init_stack fail!",__FUNCTION__);
            strcpy(pub_data_stru->center_result_code,"96");
            return -1;
        }
        dcs_debug(0,0,"<%s> db_init_stack succ! use=%d",__FUNCTION__,use);
        if(use == 0) {
            if(0> menu_preorder_traversal(terminfo.menu_ver ,NULL)) {
                dcs_log(0,0,"<%s> menu_preorder_traversal fail!",__FUNCTION__);
                strcpy(pub_data_stru->center_result_code,"96");
                return -1;
            }
            //初始状态，获取根节点
            if(0>get_menu_root_node(menu_ver ,&node)) {
                dcs_log(0,0,"<%s> get_menu_root_node fail!",__FUNCTION__);
                strcpy(pub_data_stru->center_result_code,"96");
                return -1;
            }
            dcs_debug(0,0,"<%s>get_menu_root_node succ node_id=%d ",__FUNCTION__,
                      node.node_id);
            if(0>db_push(stack,node.node_id,&use)) {
                dcs_log(0,0,"<%s> db_push fail! use=%d",__FUNCTION__,use);
                strcpy(pub_data_stru->center_result_code,"96");
                return -1;
            }
            dcs_debug(0,0,"<%s>db_push succ node_id=%d,use=%d ",__FUNCTION__,node.node_id,use);
        } else {
            memset(&node,0,sizeof(node));
            node.node_id=stack[use-1];
            //抽检栈缓冲区中的菜单项是否存在本菜单中
            if(0> menu_preorder_traversal(terminfo.menu_ver ,&node)) {
                dcs_log(0,0,"<%s> menu_preorder_traversal fail!,node_id=%d",__FUNCTION__,node.node_id);
                strcpy(pub_data_stru->center_result_code,"96");
                return -1;
            }
        }
        n=1;
        while(1) {
            if(use == 0) { // 判断堆栈是否为空，空则表示菜单下载完毕，break;
                tpos_reset_download(psam,1);
                break;
            }
            if(n >=400) break;
            if(0>db_pop(stack ,&id,&use)) {
                dcs_log(0,0,"<%s> db_pop fail! use=%d",__FUNCTION__,use);
                strcpy(pub_data_stru->center_result_code,"96");
                return -1;
            }
            cnt++;

            //获取下一节点map信息
            if(0>get_menu_node(terminfo.menu_ver ,id , &node)) {
                dcs_log(0,0,"<%s> get_menu_node fail",__FUNCTION__);
                strcpy(pub_data_stru->center_result_code,"96");
                return -1;
            }
            if(node.right_node_id > 0) {
                if(0>db_push(stack,node.right_node_id,&use)) {
                    dcs_log(0,0,"<%s> db_push fail! use=%d",__FUNCTION__,use);
                    strcpy(pub_data_stru->center_result_code,"96");
                    return -1;
                }
            }
            if(node.left_node_id > 0) {
                if(0>db_push(stack,node.left_node_id,&use)) {
                    dcs_log(0,0,"<%s> db_push fail! use=%d",__FUNCTION__,use);
                    strcpy(pub_data_stru->center_result_code,"96");
                    return -1;
                }
            }
            // 打包处理
            if(0>get_node_info(menu_ver,node.node_id,&node_info)) {
                dcs_log(0,0,"<%s> get_node_info fail",__FUNCTION__);
                strcpy(pub_data_stru->center_result_code,"96");
                return -1;
            }
            snprintf(node_set+strlen(node_set),sizeof(node_set)-strlen(node_set),"%2.2s%d,","18",node_info.node_id);
            memcpy(tmp+n,"\x31",1);
            n++;
            tmp[n]=node.node_id/256;
            n++;
            tmp[n]=node.node_id%256;
            n++;
            tmp[n]=node.pre_node_id/256;//节点ID
            dcs_debug(0,0,"<%s> node id[%d] is_leaf[%c]",
                      __FUNCTION__,node.node_id,node.is_leaf[0]);
            if(node.is_leaf[0]==0x31) {
                tmp[n]= tmp[n]|0x80; //表示是叶结点
                leaf=1;
            } else {
                tmp[n]= tmp[n]&0x7F;
                leaf=0;
            }
            if(node.pre_flag[0]==0x31)
                tmp[n]= tmp[n] | 0x40; //为前结点的右结点
            else
                tmp[n]= tmp[n] & 0xBF;
            n++;
            tmp[n]=node.pre_node_id%256; //前驱节点ID
            n++;
            if(leaf) {
                memcpy(tmp+n,node_info.func_code,3); //功能代码
                n=n+3;

                if(node_info.void_flag[0] == 0x31)
                    tmp[n]=0x80;      //冲正标识
                else
                    tmp[n]=0x00;      //冲正标识
                n++;
                tmp[n]= node_info.func_disp_indx;   //功能提示索引
                n++;
                rtrim(node_info.op_code);
                tmp[n]= strlen(node_info.op_code)/2; //操作代码长度
                n++;
                asc_to_bcd((unsigned char *)tmp+n,(unsigned char *)node_info.op_code,strlen(node_info.op_code),0);//操作代码集合
                n=n+strlen(node_info.op_code)/2;
            }
            if(node.pre_node_id >0) {
                rtrim(node_info.title);
                tmp[n]=strlen(node_info.title);
                n++;
                memcpy(tmp+n,node_info.title,strlen(node_info.title));
                n=n+strlen(node_info.title);
            } else {
                tmp[n]=0x00;
                n++;
            }
        }
    }
    if(cnt >0) {
        tmp[0]=cnt;

        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type,"18"),
                      pub_data_stru->route_msg_type,
                      n,tmp,1);
        n=_get_field_data_safe(pub_data_stru,get_pub_field_id(pub_data_stru->route_msg_type,"18"),
                               pub_data_stru->route_msg_type,tmp,1,sizeof(tmp));
        node_set[strlen(node_set)-1]=0x00;
        if(use>0) {
            db_save_stack(psam,stack,use); //保存本终端对应的堆栈空间
            db_save_set(node_set,node_info.step,psam);
        } else {
            db_save_set(node_set,get_max_step(menu_ver),psam);
        }
        strcpy(pub_data_stru->center_result_code,"00");
        dcs_debug(0,0,"<%s> end",__FUNCTION__);
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type,"24"),
                      pub_data_stru->route_msg_type,
                      0,tmp,1);
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type,"25"),
                      pub_data_stru->route_msg_type,
                      0,tmp,1);
        return 1;
    }
    return 1;
}

int tpos_link_test(glob_msg_stru * pub_data_stru) {
    char tmp[64];
    struct  tm *tm;
    time_t  t;

    time(&t);
    tm = localtime(&t);
    snprintf(tmp,sizeof(tmp),"%4d%02d%02d%02d%02d%02d",
             tm->tm_year+1900,
             tm->tm_mon + 1,
             tm->tm_mday,
             tm->tm_hour,
             tm->tm_min,
             tm->tm_sec);
    add_pub_field(pub_data_stru,
                  get_pub_field_id(pub_data_stru->route_msg_type,"DATE"),
                  pub_data_stru->route_msg_type,
                  8,tmp,1);
    add_pub_field(pub_data_stru,
                  get_pub_field_id(pub_data_stru->route_msg_type,"TIME"),
                  pub_data_stru->route_msg_type,
                  6,tmp+8,1);
    strcpy(pub_data_stru->center_result_code,"00");
    return 1;
}

int _tpos_get_key16(char *sek_indx,char *tek_indx,char *tm_key,char *key1,char *key2) {
    char return_code[4],check_value[17];
    int s;
    ICS_DEBUG(0);
    s=DESTMKGETPIKMAK2(return_code,sek_indx,tek_indx,tm_key,key1,key2,check_value,"Y","X");
    if((s!=1) || (memcmp(return_code,"00",2) !=0))    return -1;
    return 1;
}

int _tpos_get_key32(char *sek_indx,char *tek_indx,char *tm_key,char *key1,char *key2) {
    char return_code[4],check_value[17];
    int s;
    memset(return_code,0,sizeof(return_code));
    s=DESTMKGETPIKMAK2(return_code,sek_indx,tek_indx,tm_key,key1,key2,check_value,"Y","Y");

    if((s!=1) || (memcmp(return_code,"00",2) !=0)) {
        dcs_log(0,0,"<%s>DESTMKGETPIKMAK2 error return_code=[%s] ",
                __FUNCTION__,return_code);
        return -1;
    }
    memcpy(key2+32,check_value,8);
    return 1;
}
int _tpos_get_smkey(char *sek_indx,char *tek_indx,char *tm_key,char *key1,char *key2) {
    char return_code[4],check_value[17];
    int s;
    dcs_debug(0,0,"<%s>begin ",__FUNCTION__);

    memset(return_code,0,sizeof(return_code));
    s=GET_PIKorMAK(return_code,sek_indx,tek_indx,tm_key,0,0,key1,key2,check_value);

    if((s!=1) || (memcmp(return_code,"00",2) !=0)) {
        dcs_log(0,0,"<%s>DESTMKGETPIKMAK2 error return_code=[%s] ",
                __FUNCTION__,return_code);
        return -1;
    }
    return 1;
}
int get_psam_no(glob_msg_stru * pub_data_stru ,char *psam, int size) {
    short int id,n;
    ICS_DEBUG(0);
    if(NULL== get_priv_field_def("PSAM_NO", &id, pub_data_stru->in_priv_def)) {
        dcs_log(0,0,"<%s> get_priv_field_def[psam_no] is null!" ,__FUNCTION__);
        return -1;
    }
    n=get_field_data_safe(pub_data_stru,id,pub_data_stru->in_msg_type,psam,size);

    return n;
}
//菜单前序遍历函数
// menu_ver:版本号
// menu_node: NULL 则为检查整个树的完整性，不为空则获取该结构中ID所在的菜单项数据
// 成功：返回值大于0
//
int menu_preorder_traversal(char *menu_ver ,struct TPOS_MENU_NODE * menu_node) {
    struct TPOS_MENU_NODE  node,node1;
    int cnt;
    struct STACK stack;
    ICS_DEBUG(0);
    memset(&node,0,sizeof(node));
    if(0>get_menu_root_node(menu_ver ,&node)) return -1;

    cnt=tpos_init_stack(&stack);

    cnt=tpos_push(&stack,&node);

    cnt=0;
    if(menu_node == NULL) {
        while(!stackempt(&stack)) {

            tpos_pop(&stack,&node);

            cnt++;

            if(node.left_node_id >0) {
                if(0>get_menu_node(menu_ver ,node.left_node_id , &node1)) {
                    dcs_log(0,0,"<%s>get_menu_node left node fail!",__FUNCTION__);
                    return -1;
                }

                tpos_push(&stack,&node1);
            }
            if(node.right_node_id >0) {
                if(0>get_menu_node(menu_ver ,node.right_node_id , &node1)) {
                    dcs_log(0,0,"<%s>get_menu_node right node fail!",__FUNCTION__);
                    return -1;
                }
                tpos_push(&stack,&node1);
            }
        }
        if(cnt!= get_menu_cnt(menu_ver)) {
            dcs_log(0,0,"<%s>get_menu_cnt  fail! cnt=%d",__FUNCTION__,cnt);
            return -1;
        } else return 1;
    }

    while(!stackempt(&stack)) {
        tpos_pop(&stack,&node);

        if(node.node_id == menu_node->node_id) {
            memcpy(menu_node,&node,sizeof(node));
            dcs_debug(0,0,"<%s> node[%d] is exist",__FUNCTION__,menu_node->node_id);
            return 1;
        }
        if(node.left_node_id >0) {
            if(0>get_menu_node(menu_ver ,node.left_node_id , &node1)) return -1;
            tpos_push(&stack,&node1);
        }
        if(node.right_node_id >0) {
            if(0>get_menu_node(menu_ver ,node.right_node_id , &node1)) return -1;
            tpos_push(&stack,&node1);
        }
    }
    dcs_log(0,0,"<%s> can not found node id[%d]",__FUNCTION__,menu_node->node_id);
    return -1;
}

int  tpos_init_stack(struct STACK *stack) {
    if(stack == NULL) return -1;
    stack->nmax=30;
    stack->use=0;
    return 1;
}
int tpos_push(struct STACK *stack,struct TPOS_MENU_NODE *node) {
    if(stack ==NULL || node == NULL) return -1;
    if(stack->use >=stack->nmax) return -1;
    stack->nodes[stack->use]= *node;

    (stack->use)++;

    return 1;
}

int tpos_pop(struct STACK *stack,struct TPOS_MENU_NODE *node) {
    if(stack ==NULL || node == NULL) return -1;
    if(stack->use <=0) return -1 ;

    (stack->use)--;

    *node=stack->nodes[stack->use];

    return 1;
}
int stackempt(struct STACK *stack) {
    if(stack == NULL) return 1;
    if(stack->use ==0) return 1;

    return 0;
}
int db_push(int *stack,int id,int *use) {
    if(*use >=30) return -1;
    stack[*use]=id;
    (*use)++;

    return 1;
}

int db_pop(int *stack , int *id,int *use) {
    if(*use >0)(*use)--;
    else return -1;
    *id=stack[*use];
    return 1;
}

int db_init_stack(char *node_set,int *stack, size_t stack_size) {
    char *p,tmp[64];
    int n;
    n=0;
    if(node_set == NULL || strlen(node_set)==0) return 0;

    p=my_split(node_set,',',tmp,sizeof(tmp));
    while(p != NULL) {

        stack[n]=atoi(tmp);
        n++;
        if(n==stack_size) return -1;
        p=my_split(p,',',tmp,sizeof(tmp));
    }
    return n;
}
//将上次下载的参数重新下载
int get_last_part_para(glob_msg_stru *pub_data_stru,const char *psam,char *menu_ver,char *node_set) {
    char *p,*p1,*p2,*p3,tmp[7][512],para_ver[6],str[64],str1[64];
    int n,s[7],l[7],leaf,i,k,f;
    para_set_stru t[30];
    struct TPOS_MENU_NODE node;
    struct MENU_NODE_INFO node_info;
    struct TPOS_PARA_INFO para_info;
    ICS_DEBUG(0);
    memset(&t,0,sizeof(t));
    for(n=0; n<7; n++) {
        s[n]=1;
        l[n]=0;
    };
    dcs_debug(0,0,"<%s> begin",__FUNCTION__);
    if(strlen(node_set) ==0) {
        dcs_debug(0,0,"<%s> node_set is null",__FUNCTION__);
        return 0;
    }
    if(memcmp(node_set,"3C",2)==0) {
        f=0;
        if((n=get_ic_data(tmp[0]+1, sizeof(tmp[0])-1,node_set+2,&f))<0) return -1;  //修改get_ic_data返回BUG 20140909
        tmp[0][0]=f;
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type,"3C"),
                      pub_data_stru->route_msg_type,
                      n+1, tmp[0], 1);
        return  1;
    }
    memset(para_ver,0,sizeof(para_ver));
    if(0>get_para_ver(menu_ver,para_ver)) return -1;
    n=0;
    dcs_debug(0,0,"<%s>node_set=[%s]",__FUNCTION__,node_set);
    p=my_split(node_set,',',str,sizeof(str));
    while(p!=NULL) {
        memcpy(t[n].para_type,p,2);
        t[n].id=atoi(p+2);
        dcs_debug(0,0,"<%s>t[%d].para_type=[%s],t[%d].id=%d",
                  __FUNCTION__,n,t[n].para_type,n,t[n].id);
        n++;
        p=my_split(p,',',str,sizeof(str));
    }
    if(n==0) return 0;
    for(i=0 ; i<n; i++) {
        dcs_debug(0,0,"<%s> n=%d,node[%d] id[%d] ",__FUNCTION__,n,i,t[i].id);
        if(strcmp(t[i].para_type,"18")==0) {
            if(0>get_menu_node(menu_ver ,t[i].id , &node)) return -1;
            if(0>get_node_info(menu_ver,node.node_id,&node_info))   return -1;
            if(node_info.use[0] == 0x31)   //新增或修改节点信息
                memcpy(tmp[0]+s[0],"\x31",1);
            else //删除节点信息
                memcpy(tmp[0]+s[0],"\x30",1);
            s[0]++;
            if(node_info.use[0] == 0x30) continue;
            tmp[0][s[0]]=node.node_id/256;
            s[0]++;
            tmp[0][s[0]]=node.node_id%256;
            s[0]++;
            tmp[0][s[0]]=node.pre_node_id/256;//节点ID

            if(node.is_leaf[0]==0x31) {
                tmp[0][s[0]]= tmp[0][s[0]]|0x80; //表示是叶结点
                leaf=1;
            } else {
                tmp[0][s[0]]= tmp[0][s[0]]&0x7F;
                leaf=0;
            }
            if(node.pre_flag[0]==0x31)
                tmp[0][s[0]]= tmp[0][s[0]] | 0x40; //为前结点的右结点
            else
                tmp[0][s[0]]= tmp[0][s[0]] & 0xBF;
            s[0]++;
            tmp[0][s[0]]=node.pre_node_id%256; //前驱节点ID
            s[0]++;
            if(leaf && node_info.use[0] !=0x30) {
                memcpy(tmp[0]+s[0],node_info.func_code,3); //功能代码
                s[0]=s[0]+3;

                if(node_info.void_flag[0] == 0x31)
                    tmp[0][s[0]]=0x80;      //冲正标识
                else
                    tmp[0][s[0]]=0x00;      //冲正标识
                s[0]++;
                tmp[0][s[0]]= node_info.func_disp_indx;   //功能提示索引
                s[0]++;
                rtrim(node_info.op_code);
                tmp[0][s[0]]= strlen(node_info.op_code)/2; //操作代码长度
                s[0]++;
                asc_to_bcd((unsigned char *)tmp[0]+s[0],(unsigned char *)node_info.op_code,strlen(node_info.op_code),0);//操作代码集合
                s[0]=s[0]+strlen(node_info.op_code)/2;
            }
            if(node.pre_node_id >0 && node_info.use[0] == 0x31) { //未被删除的非根节点加入实际标题
                rtrim(node_info.title);
                tmp[0][s[0]]=strlen(node_info.title);
                s[0]++;
                memcpy(tmp[0]+s[0],node_info.title,strlen(node_info.title));
                s[0]=s[0]+strlen(node_info.title);
            } else {
                tmp[0][s[0]]=0x00;
                s[0]++;
            }

            l[0]++;
        } else if(strcmp(t[i].para_type,"19")==0) {
            if(0>get_other_para_info("19",t[i].id,para_ver,&para_info)) return -1;
            tmp[1][s[1]]=t[i].id;
            s[1]++;
            if(para_info.use[0] == 0x31) {
                tmp[1][s[1]]=0x31;
                s[1]++;
                k=s[1];//保留长度位置
                s[1]++;
                p2=my_split(para_info.control_info,',',str,sizeof(str));
                if(p2 != NULL) {
                    f=atoi(str);
                    tmp[1][s[1]]=f; //显示信息条数
                    s[1]++;

                    p3=NULL;
                    p1=my_split(para_info.detail,'|',str1,sizeof(str1));
                    if(p1 == NULL) {
                        dcs_log(0,0,"<%s>19 para format error: no found detail!",
                                __FUNCTION__);
                        return -1;
                    }
                    for(; ;) {
                        p2=my_split(p2,',',str,sizeof(str));
                        if(p2 == NULL || strlen(str)!=2) {
                            dcs_log(0,0,"<%s> 19 para format error: control info error!",
                                    __FUNCTION__);
                            return -1;
                        }
                        asc_to_bcd((unsigned char *)tmp[1]+s[1],(unsigned char *)str,2,0); //显示格式码1字节
                        s[1]++;
                        rtrim(str1);
                        tmp[1][s[1]]=strlen(str1);   //显示数据每条的长度1字节
                        s[1]++;

                        memcpy(tmp[1]+s[1],str1,strlen(str1));
                        s[1]=s[1]+strlen(str1);
                        f--;
                        if(f >0) {
                            p1=my_split(p1,'|',str1,sizeof(str1));
                            if(p1 == NULL) {
                                dcs_log(0,0,"<%s> 19 para format error: no found detail!",
                                        __FUNCTION__);
                                return -1;
                            }
                        } else break;
                    }
                }
                tmp[1][k]= s[1]-k-1;
            } else {
                tmp[1][s[1]]=0x00;
                s[1]++;
                tmp[1][s[1]]=0x00;
            }
            l[1]++;
        } else if(strcmp(t[i].para_type,"1A")==0) {
            if(0>get_other_para_info("1A",t[i].id,para_ver,&para_info)) return -1;
            tmp[2][s[2]]=t[i].id;
            s[2]++;


            if(para_info.use[0] == 0x31) {
                tmp[2][s[2]]=0x31;
                s[2]++;
                k=s[2]; //保留长度位置
                s[2]++;
                p2=my_split(para_info.control_info,',',str,sizeof(str));
                if(p2 != NULL) {
                    f=atoi(str);
                    tmp[2][s[2]]=f; //显示信息条数
                    s[2]++;

                    p3=NULL;
                    p1=my_split(para_info.detail,'|',str1,sizeof(str1));
                    if(p1 == NULL) {
                        dcs_log(0,0,"<%s> 1A para format error: no found detail!",
                                __FUNCTION__);
                        return -1;
                    }
                    for(; ;) {
                        p2=my_split(p2,',',str,sizeof(str));
                        if(p2 == NULL || strlen(str)!=2) {
                            dcs_log(0,0,"<%s> 1A para format error: control info error!",
                                    __FUNCTION__);
                            return -1;
                        }
                        asc_to_bcd((unsigned char *)tmp[2]+s[2],(unsigned char *)str,2,0); //显示格式码1字节
                        s[2]++;
                        rtrim(str1);
                        tmp[2][s[2]]=strlen(str1);   //显示数据每条的长度1字节
                        s[2]++;

                        memcpy(tmp[2]+s[2],str1,strlen(str1));
                        s[2]=s[2]+strlen(str1);
                        f--;
                        if(f >0) {
                            p1=my_split(p1,'|',str1,sizeof(str1));
                            if(p1 == NULL) {
                                dcs_log(0,0,"<%s> 1A para format error: no found detail!",
                                        __FUNCTION__);
                                return -1;
                            }
                        } else break;
                    }
                }
                tmp[2][k]=s[2]-k-1;
            } else {
                tmp[2][s[2]]=0x00;
                s[2]++;
                tmp[2][s[2]]=0x00;
            }
            l[2]++;
        } else if(strcmp(t[i].para_type,"1B")==0) {
            tmp[3][s[3]]=para_info.id;
            if(0>get_other_para_info("1B",0,para_ver,&para_info)) return -1;
            snprintf(tmp[3],sizeof(tmp[3]),"%s",para_info.detail);
            s[3]=strlen(para_info.detail);
            l[3]++;
        } else if(strcmp(t[i].para_type,"1C")==0) {

            if(0>get_other_para_info("1C",t[i].id,para_ver,&para_info)) return -1;
            tmp[4][s[4]]=t[i].id;
            s[4]++;

            rtrim(para_info.control_info);
            if(strlen(para_info.control_info)!=2) {
                dcs_log(0,0,"<%s> 1C para format error: control info error!",
                        __FUNCTION__);
                return -1;
            }
            asc_to_bcd((unsigned char *)tmp[4]+s[4],(unsigned char *)para_info.control_info,2,0); //字体格式码1字节
            s[4]++;

            tmp[4][s[4]]=strlen(para_info.detail);   //打印数据每条的长度1字节
            s[4]++;

            memcpy(tmp[4]+s[4],para_info.detail,strlen(para_info.detail));
            s[4]=s[4]+strlen(para_info.detail);
            l[4]++;
        } else if(strcmp(para_info.para_type,"16")==0) { //更新终端参数信息

            if(para_info.use[0] == 0x31) {
                tmp[5][s[5]]=para_info.id;
                s[5]++;
                if(para_info.id == 1) {
                    tmp[5][s[5]]=0x01;
                    s[5]++;
                    tmp[5][s[5]]=para_info.detail[0];
                    s[5]++;
                } else if(para_info.id == 2) {
                    tmp[5][s[5]]=0x01;
                    s[5]++;
                    tmp[5][s[5]]=atoi(para_info.detail);
                    s[5]++;
                } else if(para_info.id == 6) {
                    tmp[5][s[5]]=strlen(para_info.detail);
                    s[5]++;
                    snprintf(tmp[5]+s[5],sizeof(tmp[5])-s[5],"%s",para_info.detail);
                    s[5]=s[5]+strlen(para_info.detail);
                } else if(para_info.id == 5) {
                    f=strlen(para_info.detail);
                    tmp[5][s[5]]=(f+1)/2;
                    s[5]++;
                    asc_to_bcd((unsigned char *)tmp[5]+s[5],(unsigned char *)para_info.detail,((f+1)/2)*2,0);
                    s[5]=s[5]+(f+1)/2;
                } else if(para_info.id == 3 || para_info.id == 4) {
                    f=atoi(para_info.control_info);
                    tmp[5][s[5]]=f; //条数
                    s[5]++;
                    p1=my_split(para_info.detail,'|',str,sizeof(str));
                    if(p1 == NULL) {
                        dcs_log(0,0,"<%s> 16 para format error: no found detail!",
                                __FUNCTION__);
                        return -1;
                    }
                    for(; f>0; f--) {

                        rtrim(p1);
                        if(para_info.id==3) {
                            tmp[5][s[5]]=strlen(str);   //显示数据每条的长度1字节
                            s[5]++;
                            memcpy(tmp[5]+s[5],str,strlen(str));
                            s[5]=s[5]+strlen(str);
                        } else {
                            tmp[5][s[5]]=(strlen(str)+1)/2;   //显示数据每条的长度1字节
                            s[5]++;
                            asc_to_bcd((unsigned char *)tmp[5]+s[5],(unsigned char *)str,((strlen(str)+1)/2)*2,0);
                            s[5]=s[5]+(strlen(str)+1)/2;
                        }
                        p1=my_split(p1,'|',str,sizeof(str));
                        if(p1 == NULL) {
                            dcs_log(0,0,"<%s> 16 para format error: no found detail!",
                                    __FUNCTION__);
                            return -1;
                        }
                    }
                } else if(para_info.id == 7) {
                    struct TPOS_TERM_INFO tpos_inf;
                    memset(&tpos_inf,0,sizeof(tpos_inf));
                    get_tpos_info((char *)psam,&tpos_inf);
                    tmp[5][s[5]]=strlen(tpos_inf.name);
                    s[5]++;
                    snprintf(tmp[5]+s[5],sizeof(tmp[5])-s[5],"%s",tpos_inf.name);
                    s[5]=s[5]+strlen(tpos_inf.name);
                } else {
                    dcs_log(0,0,"<%s> 16 para format error:can not found control info!",__FUNCTION__);
                    return -1;
                }
                l[5]++;
            }
        } else if(strcmp(para_info.para_type,"17")==0) { //更新选择项模板参数信息
            if(0>get_other_para_info("17",t[i].id,para_ver,&para_info)) return -1;
            tmp[6][s[6]]=para_info.id;
            s[6]++;
            if(para_info.use[0] == 0x31) {
                tmp[6][s[6]]=0x31;
                s[6]++;
                i=atoi(para_info.control_info);
                if(i <=0 || i > 60) {
                    dcs_log(0,0,"<%s> 17 para format error: !",__FUNCTION__);
                    return -1;
                }
                tmp[6][s[6]]=atoi(para_info.control_info); //信息条数
                s[6]++;

                for(p1=my_split(para_info.detail,'|',str,sizeof(str)) ; i>0; i--) {
                    if(p1 == NULL) {
                        dcs_log(0,0,"<%s> 17 para format error: no found detail!",__FUNCTION__);
                        return -1;
                    }
                    tmp[6][s[6]]=strlen(str);
                    s[6]++;
                    memcpy(tmp[6]+s[6],str,strlen(str));
                    s[6]=s[6]+strlen(str);
                    p1=my_split(p1,',',str,sizeof(str));
                    if(p1 == NULL) {
                        dcs_log(0,0,"<%s> 17 para format error: no found detail!",__FUNCTION__);
                        return -1;
                    }
                    tmp[6][s[6]]=strlen(str);
                    s[6]++;
                    memcpy(tmp[6]+s[6],str,strlen(str));
                    n=n+strlen(str);
                    s[6]=s[6]+strlen(str);
                    p1=my_split(p1,',',str,sizeof(str));
                }
            } else {
                tmp[6][s[6]]=0x30;
                s[6]++;
            }
            l[6]++;
        } else {
            dcs_log(0,0,"<%s> can not recognition para type [%s]",__FUNCTION__,t[i].para_type);
            return -1;
        }
    }
    dcs_debug(0,0,"<%s> data pack end");
    if(l[0] >0) {
        tmp[0][0]=l[0];
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type,"18"),
                      pub_data_stru->route_msg_type,
                      s[0],tmp[0],1);
    }
    if(l[1] >0) {
        tmp[1][0]=l[1];
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type,"19"),
                      pub_data_stru->route_msg_type,
                      s[1],tmp[1],1);
    }
    if(l[2] >0) {
        tmp[2][0]=l[2];
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type,"1A"),
                      pub_data_stru->route_msg_type,
                      s[2],tmp[2],1);
    }
    if(l[3] >0) {
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type,"1B"),
                      pub_data_stru->route_msg_type,
                      s[3],tmp[3],1);
    }
    if(l[4] >0) {
        tmp[4][0]=l[4];
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type,"1C"),
                      pub_data_stru->route_msg_type,
                      s[4],tmp[4],1);
    }
    if(l[5] >0) {
        tmp[5][0]=l[5];
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type,"16"),
                      pub_data_stru->route_msg_type,
                      s[5],tmp[5],1);
    }
    if(l[6] >0) {
        tmp[6][0]=l[6];
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type,"17"),
                      pub_data_stru->route_msg_type,
                      s[6],tmp[6],1);
    }
    dcs_debug(0,0,"<%s> end",__FUNCTION__);
    return 1;
}

//增加60域数据处理 20140924
int tpos_field_pre_conv(char *para, short flag, glob_msg_stru *pub_data_stru) {
    //磁道信息解密
    char tmp[512],tmp1[600],return_code[4];
    unsigned char *p;
    int n,outlen, i;
    ICS_DEBUG(0);
//  dcs_debug(0,0,"<%s> begin ",__FUNCTION__);
    if((n=get_field_data_safe(pub_data_stru,
                              get_pub_field_id(pub_data_stru->in_msg_type,"04"),
                              pub_data_stru->in_msg_type,
                              tmp,sizeof(tmp)))>0) {
        dcs_debug(tmp,n,"<%s> track mdata",__FUNCTION__);
        if(tmp[0] == 0x31) { //磁条卡
            if(pub_data_stru->in_cry_flag)
                i=Track_Decryption(return_code, pub_data_stru->in_data_index, pub_data_stru->in_data_key,0, n-1-8, tmp+1, &outlen, tmp);
            else
                i=DecTrackPrg(return_code, pub_data_stru->in_data_index, pub_data_stru->in_data_key, n-1-8, tmp+1, &outlen, tmp);
            if(i>0) {
                if(memcmp(return_code,"00",2)!=0) return -1;
                bcd_to_asc((unsigned char *)tmp1,(unsigned char *)tmp,outlen*2,0);
                tmp1[outlen*2]=0x00;
                n=strlen(tmp1);
                for(; n>0; n--)
                    if(tmp1[n-1]=='D' || tmp1[n-1]=='d') tmp1[n-1]='=';
                memcpy(tmp,tmp1,48);
                tmp[48]=0x00;
                rtrim_c(tmp,'F');
                add_pub_field(pub_data_stru, FIELD_TRACK2,
                              pub_data_stru->in_msg_type,
                              strlen(tmp),tmp,1);
                for(i = 0; tmp[i] && tmp[i] != '='; i++);
                if(tmp[i]  == '=' && i + 5 < strlen(tmp)) {
                    if(tmp[i + 5] == '2' || tmp[i + 5] == '6') {
                        add_pub_field(pub_data_stru, 60,
                                      pub_data_stru->in_msg_type,
                                      15, "000005200900000", 1);
                    } else {
                        add_pub_field(pub_data_stru, 60,
                                      pub_data_stru->in_msg_type,
                                      15, "000005000900000", 1);
                    }
                } else
                    add_pub_field(pub_data_stru, 60,
                                  pub_data_stru->in_msg_type,
                                  15, "000005000900000", 1);
                tmp[i] = 0x00;
                add_pub_field(pub_data_stru, FIELD_CARD_NO,
                              pub_data_stru->in_msg_type,
                              strlen(tmp), tmp, 1);
                dcs_debug(0,0,"<%s>card_no=[%s]",__FUNCTION__,tmp);
                if((outlen*2-48) >0) {
                    memcpy(tmp,tmp1+48,outlen*2-48);
                    tmp[outlen*2-48]=0x00;
                    rtrim_c(tmp,'F');
                    add_pub_field(pub_data_stru,
                                  FIELD_TRACK3,
                                  pub_data_stru->in_msg_type,
                                  strlen(tmp),tmp,1);
                }
            } else {
                dcs_log(0,0,"<%s> convert track date failed! ",__FUNCTION__);
                return -1;
            }
        } else if(tmp[0] == 0x35 || tmp[0] == 0x37) { //IC卡
            p = (unsigned char *)tmp + 1;
            add_pub_field(pub_data_stru,
                          23,
                          pub_data_stru->in_msg_type,
                          3, (char *)p, 1);
            p += 3;
            add_pub_field(pub_data_stru,
                          FIELD_CARD_NO,
                          pub_data_stru->in_msg_type,
                          *p, (char *)p + 1, 1);
            p += *p + 1;
            add_pub_field(pub_data_stru,
                          FIELD_IC_DATA,
                          pub_data_stru->in_msg_type,
                          *p, (char *)p + 1, 1);
            p += *p + 1;
            if(pub_data_stru->in_cry_flag)
                i=Track_Decryption(return_code, pub_data_stru->in_data_index, pub_data_stru->in_data_key,0, *p, (char *)p+1, &outlen, tmp1);
            else
                i= DecTrackPrg(return_code, pub_data_stru->in_data_index, pub_data_stru->in_data_key, *p, (char *)p+1, &outlen, tmp1);
//          dcs_debug(0,0,"<%s> IC card track Decryption end",__FUNCTION__);
            if(i >0) {
                if(memcmp(return_code,"00",2)!=0) return -1;
                bcd_to_asc((unsigned char *)tmp,(unsigned char *) tmp1, outlen*2, 0);
                tmp[outlen*2]=0x00;
                n = strlen(tmp);
                for(; n>0; n--)
                    if(tmp[n-1]=='D' || tmp[n-1]=='d') tmp[n-1]='=';
                memcpy(tmp1, tmp, 48);
                tmp1[48] = 0x00;
                rtrim_c(tmp1, 'F');
                add_pub_field(pub_data_stru,
                              FIELD_TRACK2,
                              pub_data_stru->in_msg_type,
                              strlen(tmp1), tmp1, 1);
            } else {
                dcs_log(0, 0, "<%s> convert track date failed! ",__FUNCTION__);
                return -1;
            }
          	add_pub_field(pub_data_stru, 60, pub_data_stru->in_msg_type, 15, "000005100900000", 1);
        } else {
            dcs_log(tmp,n,"<%s> can not prase 04",__FUNCTION__);
            return -1;
        }
    }

    return 1;
}

//密钥转换
//para: 只能省略后面的参数。
//第一个参数，输出PIN格式, 0:BIN格式(默认)，1:ASC格式
//第二个参数，输出PIN是否带PAN, 0:不带 1：带(默认)

int tpos_gen_field_conver(char *para, short fldid, glob_msg_stru *pub_data_stru) {
//  int n,i;
    char return_code[4],tmp[128],tmp1[128],card_no[20+1];
    char outPan[20 + 1];
    unsigned char outMode, outPanFlag;
    int l;
    char *p;

    outMode = 0;
    outPanFlag = 1;
    if(fldid ==52) {
        p=my_split(para, ',',tmp,sizeof(tmp));
        if(p != NULL) {
            if(tmp[0] == '1') outMode = 1;
            p=my_split(p,',',tmp,sizeof(tmp));
        }
        if(p != NULL) {
            if(tmp[0] == '0') outPanFlag = 0;
            p=my_split(p,',',tmp,sizeof(tmp));
        }
        if(0 >= get_field_data_safe(pub_data_stru,
                                   FIELD_PIN,
                                   pub_data_stru->in_msg_type,
                                   tmp,sizeof(tmp))) return 1;
        l=get_field_data_safe(pub_data_stru,
                              FIELD_CARD_NO,
                              pub_data_stru->in_msg_type,
                              tmp1,sizeof(tmp1));
        if(l >0) tmp1[l]=0x00;
        if(13 >=l)
            strcpy(tmp1,"0000000000000000");
        del_pub_field(pub_data_stru, FIELD_PIN,pub_data_stru->in_msg_type,0);
        if(memcmp(tmp,"FFFFFFFF",8)==0) return 1;
        memset(card_no,'0',4);
        memcpy(card_no+4,tmp1+(strlen(tmp1)-13),12);
        card_no[12+4]=0x00;
        if(outPanFlag)
            snprintf(outPan,sizeof(outPan),"%s", card_no);
        else
            memset(outPan, '0', 16);
        outPan[16] = 0;
        dcs_debug(0,0,"<%s> route_pin_index=[%s]",__FUNCTION__,pub_data_stru->route_pin_index);
        if(pub_data_stru->in_cry_flag && pub_data_stru->out_cry_flag) {
            if(TRANS_PIN(return_code,pub_data_stru->in_pin_index,pub_data_stru->route_pin_index,pub_data_stru->in_pin_key,pub_data_stru->route_pin_key, 0,0, tmp, card_no,tmp, outPan)<0) {
                dcs_log(0,0,"<%s> TRANS_PIN error!",__FUNCTION__);
                return -1;
            }
        } else if(pub_data_stru->in_cry_flag && !pub_data_stru->out_cry_flag) {
            if(TRANS_PIN(return_code,pub_data_stru->in_pin_index,pub_data_stru->route_pin_index,pub_data_stru->in_pin_key,pub_data_stru->route_pin_key, 0,2, tmp, card_no,tmp, outPan)<0) {
                dcs_log(0,0,"<%s> TRANS_PIN error!",__FUNCTION__);
                return -1;
            }
        } else if(!pub_data_stru->in_cry_flag && pub_data_stru->out_cry_flag) {
            if(TRANS_PIN(return_code,pub_data_stru->in_pin_index,pub_data_stru->route_pin_index,pub_data_stru->in_pin_key,pub_data_stru->route_pin_key, 2,0, tmp, card_no,tmp, outPan)<0) {
                dcs_log(0,0,"<%s> TRANS_PIN error!",__FUNCTION__);
                return -1;
            }
        } else {
            dcs_debug(0,0,"<%s> tmp=[%s],tmp1=[%s],card_no=[%s]",__FUNCTION__,tmp,tmp1,card_no);
            if(DESTRANSPIN2(return_code,pub_data_stru->in_pin_index,pub_data_stru->route_pin_index,pub_data_stru->in_pin_key,pub_data_stru->route_pin_key,tmp,card_no,tmp,outPan) < 0) {
                dcs_log(0,0,"<%s> DESTRANSPIN error!",__FUNCTION__);
                return -1;
            }
        }
        dcs_debug(0,0,"<%s> pin conver end",__FUNCTION__);
        if(memcmp(return_code,"00",2)==0) {
            if(!pub_data_stru->out_cry_flag) {
                dcs_debug(0,0,"<%s> !out_cry_flag begin ",__FUNCTION__);
                if(outMode) {
                    asc_to_bcd((unsigned char *)tmp1, (unsigned char *)tmp, 16, 0);
                    add_pub_field(pub_data_stru, FIELD_PIN,
                                  pub_data_stru->route_msg_type,
                                  8, tmp1, 1);
                } else
                    add_pub_field(pub_data_stru,FIELD_PIN,
                                  pub_data_stru->route_msg_type,
                                  16,tmp,1);
                dcs_debug(0,0,"<%s> !out_cry_flag end ",__FUNCTION__);
            } else {
                dcs_debug(0,0,"<%s> out_cry_flag begin ",__FUNCTION__);
                if(outMode) {
                    asc_to_bcd((unsigned char *)tmp1, (unsigned char *)tmp, 32, 0);
                    add_pub_field(pub_data_stru, FIELD_PIN,
                                  pub_data_stru->route_msg_type,
                                  16, tmp1, 1);
                } else
                    add_pub_field(pub_data_stru,FIELD_PIN,
                                  pub_data_stru->route_msg_type,
                                  32,tmp,1);
                dcs_debug(0,0,"<%s> out_cry_flag end ",__FUNCTION__);
            }
        } else {
            dcs_log(0,0,"<%s> fail! return_code=[%s]",__FUNCTION__,return_code);
            return -1;
        }
    } else {
        dcs_log(0,0,"<%s> error field_id  [%d]",__FUNCTION__,fldid);
        return -1;
    }
    dcs_debug(0,0,"<%s> end",__FUNCTION__);
    return 1;
}

int _tpos_get_work_key(glob_msg_stru * pub_data_stru) {
    char psam[20], msg_type[5];
    int i, id;
    struct TPOS_TERM_INFO terminfo;
    ICS_DEBUG(0);
//  i=_get_field_data(get_pub_field_id(pub_data_stru->route_msg_type, "PSAM_NO"),pub_data_stru,psam, pub_data_stru->route_num == 0 ? 0 : 1);
    if(pub_data_stru->route_num == 0) {
        id = get_pub_field_id(pub_data_stru->in_msg_type, "PSAM_NO");
        memcpy(msg_type, pub_data_stru->in_msg_type, 4);
    } else {
        id = get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID1");
        memcpy(msg_type, DB_MSG_TYPE, 4);
    }
    i=get_field_data_safe(pub_data_stru,id,msg_type,psam,17);
    if(i<=0) {
        dcs_log(0,0,"<%s> can not got psam_no!",__FUNCTION__);
        return -1;
    }
    psam[i]=0x00;
    memset(&terminfo,0,sizeof(terminfo));
    if(get_tpos_info(psam,&terminfo)>0) {
        pub_data_stru->tmp_order[1]=terminfo.menu_update_flag[0];
    }
    return tpos_get_work_key(psam,pub_data_stru->route_pin_index,
                             pub_data_stru->route_mac_index,
                             pub_data_stru->route_data_index,
                             pub_data_stru->route_pin_key,
                             pub_data_stru->route_mac_key,
                             pub_data_stru->route_data_key);
}

int tpos_trans_cancle(char *para, short flag, glob_msg_stru *pub_data_stru) {
    char tmp[64];
    char fieldVal[30 + 1];
    struct  tm *time_tm;
    time_t time_cl;
    int len, ret;
    tl_trans_log_def tPosLog;
    time(&time_cl) ;
    time_tm = localtime(&time_cl);
    strftime(tmp, 64, "%Y%m%d%H%M%S", time_tm);
    ICS_DEBUG(0);
    memset(fieldVal, 0, sizeof(fieldVal));
    len = get_field_data_safe(pub_data_stru,
                              get_pub_field_id(pub_data_stru->in_msg_type, "09"),
                              pub_data_stru->in_msg_type, fieldVal,sizeof(fieldVal));
    if(len < 0) {
        dcs_log(0, 0, "<%s>读原交易信息数据不正确[%d]-[%d]！",__FUNCTION__,
                FIELD_ORG_TRANS_INFO, len);
        return -1;
    }
    memset(&tPosLog, 0, sizeof(tPosLog));
    memcpy(tPosLog.sys_date, tmp, 8);
    memcpy(tPosLog.acq_date, "NULL", 4);
    get_field_data_safe(pub_data_stru,
                        get_pub_field_id(pub_data_stru->in_msg_type, "PSAM_NO"),
                        pub_data_stru->in_msg_type, tPosLog.acq_term_id1,17);
    sprintf(tPosLog.acq_term_id2, "NULL");
    get_field_data_safe(pub_data_stru,
                        FIELD_INSTI_CODE,
                        pub_data_stru->in_msg_type, tPosLog.acq_insti_code,9);
    memcpy(tPosLog.acq_tra_no, fieldVal, 6);
    dcs_debug(0,0,"<%s> field[09]=[%s]",__FUNCTION__,fieldVal);
    ret = select_translog(&tPosLog);
    if(ret < 0)
        return -1;
    if(ret > 0 && memcmp("00", tPosLog.resp_cd_rcv,2)==0
       && tPosLog.void_flag[0]=='0') {
        memset(fieldVal, 0, sizeof(fieldVal));
        get_field_data_safe(pub_data_stru,
                            FIELD_AMOUNT,
                            pub_data_stru->in_msg_type,
                            fieldVal,sizeof(fieldVal));
        if(atol(fieldVal) != atol(tPosLog.amount_pay)) {
            strcpy(pub_data_stru->center_result_code, CODE_INVALID_TRANS);
            dcs_log(0,0,"<%s> 撤销金额和原始交易金额不一致!",__FUNCTION__);
            return -1;
        }
        update_pub_field(pub_data_stru, FIELD_AMOUNT_REAL,
                         pub_data_stru->in_msg_type,
                         12, tPosLog.amount_real, 1);
        len=get_field_data_safe(pub_data_stru,
                                FIELD_CARD_NO,
                                pub_data_stru->in_msg_type,
                                fieldVal,sizeof(fieldVal));
        if(len >0) {
            rtrim(tPosLog.pay_acct_no);
            if(strcmp(fieldVal,tPosLog.pay_acct_no)!=0) {
                strcpy(pub_data_stru->center_result_code, CODE_INVALID_TRANS);
                dcs_log(0,0,"<%s> 撤销卡号和原始交易卡号不一致! [%s] [%s]",
                        __FUNCTION__,tPosLog.pay_acct_no,fieldVal);
                return -1;
            }
        }
        if(0 > db_to_pub_daba(pub_data_stru, &tPosLog)) return -1;
        // 获取原交易路由支付机构信息
        len=_get_field_data_safe(pub_data_stru,
                                 get_pub_field_id(DB_MSG_TYPE, "PAY_INSTI_CODE"),
                                 DB_MSG_TYPE, fieldVal, 2,sizeof(fieldVal));
        if(len <=0) {
            strcpy(pub_data_stru->center_result_code, "96");
            dcs_log(0,0,"<%s> not get old PAY_INSTI_CODE! ",__FUNCTION__);
            return -1;
        }
        memcpy(pub_data_stru->route_insti_code,fieldVal,8);
        if(0 > get_route_insti_info(pub_data_stru)) {
            strcpy(pub_data_stru->center_result_code, "96");
            dcs_log(0,0,"<%s> get_route_insti_info fail! ",__FUNCTION__);
            return -1;
        }
        pub_data_stru->is_route=3;
        return 1;
    } else {
        strcpy(pub_data_stru->center_result_code, ret == 1 ? CODE_PROCESSING : CODE_INVALID_TRANS);
        return -1;
    }
}

//删除金额判定 20140926
int tpos_reversed(glob_msg_stru * pub_data_stru) {
    char tmp[64];
    char fieldVal[30 + 1];
    struct  tm *time_tm;
    time_t time_cl;
    int len, ret, field_id;
    tl_trans_log_def tPosLog;
    time(&time_cl) ;
    time_tm = localtime(&time_cl);
    strftime(tmp, 64, "%Y%m%d%H%M%S", time_tm);
    ICS_DEBUG(0);
    dcs_debug(0,0,"<%s> begin",__FUNCTION__);
    snprintf(pub_data_stru->route_insti_code,9,"%s",pub_data_stru->insti_code);
    snprintf(pub_data_stru->route_trans_type,5,"%s",pub_data_stru->in_trans_type);
    if(0 > (field_id = get_pub_field_id(pub_data_stru->in_msg_type, "0F"))) {
        dcs_log(0, 0, "<%s>读取冲正信息域失败！", __FUNCTION__);
        return -1;
    }
    len = get_field_data_safe(pub_data_stru,
                              field_id,
                              pub_data_stru->in_msg_type, fieldVal,sizeof(fieldVal));
    if(len != 11) {
        dcs_log(0, 0, "<%s>读取冲正信息数据不正确[%d]-[%d]！", __FUNCTION__, field_id, len);
        return -1;
    }
    memset(&tPosLog, 0, sizeof(tPosLog));
    memcpy(tPosLog.sys_date, tmp, 8);
    memcpy(tPosLog.acq_date, "NULL", 4);
    get_field_data_safe(pub_data_stru,FIELD_PSAM_NO,
                        pub_data_stru->in_msg_type,
                        tPosLog.acq_term_id1,17);
    sprintf(tPosLog.acq_term_id2, "NULL");
    get_field_data_safe(pub_data_stru,FIELD_INSTI_CODE,
                        pub_data_stru->in_msg_type,
                        tPosLog.acq_insti_code,9);
    bcd_to_asc((unsigned char *)tPosLog.acq_tra_no, (unsigned char *)fieldVal, 6, 0);
    bcd_to_asc((unsigned char *)tmp, (unsigned char *)fieldVal + 3, 16, 0);
    ret = select_translog(&tPosLog);
    if(ret < 0) {
        strcpy(pub_data_stru->center_result_code,"96");
        return 1;
    }
    if(memcmp(tmp, tPosLog.acq_mac, 16)) ret = 0;
    rtrim(tPosLog.resp_cd_rcv);
    if(ret == 1 && memcmp("00", tPosLog.resp_cd_rcv, 2)==0 &&
       tPosLog.void_flag[0] == '0' && tPosLog.permit_void[0] == '1') {
        memset(pub_data_stru->timeout_table.first_key, 0, sizeof(pub_data_stru->timeout_table.first_key));
        rtrim(tPosLog.acq_insti_code);
        rtrim(tPosLog.acq_term_id1);
        rtrim(tPosLog.acq_term_id2);
        rtrim(tPosLog.acq_tra_no);
        rtrim(tPosLog.acq_date);
        snprintf(pub_data_stru->timeout_table.first_key,sizeof(pub_data_stru->timeout_table.first_key), "%s,%s,%s,%s,%s,%s,%s",
                 pub_data_stru->in_msg_type, tPosLog.sys_date,tPosLog.acq_insti_code,tPosLog.acq_tra_no,tPosLog.acq_date,tPosLog.acq_term_id1,tPosLog.acq_term_id2);
        memcpy(pub_data_stru->timeout_table.sys_date, tmp, 8);
        memcpy(pub_data_stru->timeout_table.sys_time, tmp + 8, 6);
        strcpy(pub_data_stru->timeout_table.foldname, SYSTEM_FOLDNAME);
        snprintf(pub_data_stru->timeout_table.key,sizeof(pub_data_stru->timeout_table.key),"%s", pub_data_stru->timeout_table.first_key);
        pub_data_stru->timeout_table.num = 30;
        strcpy(pub_data_stru->timeout_table.flag, "2");
        //更新原始交易的冲正标识，表示已发起冲正
        if(0 > db_to_pub_daba(pub_data_stru, &tPosLog)) return -1;
        if(update_db_voidflag(&tPosLog, '1', NULL)<0) return -1;
        write_voidtrans_to_fold(&pub_data_stru->timeout_table);
    }
    dcs_debug(0,0,"<%s> ret=%d",__FUNCTION__,ret);
    if(ret > 0) {
        if(tPosLog.resp_cd_rcv[0] || tPosLog.permit_void[0] == '0')
            strcpy(pub_data_stru->center_result_code, "00");
        else
            strcpy(pub_data_stru->center_result_code, CODE_PROCESSING);
    } else
        strcpy(pub_data_stru->center_result_code, CODE_NOT_EXIST);
    return 1;
}
//修改格式化金额取金额数据域BUG 20141013
/*格式化终端打印数据

终端打印信息配置格式定义：
1 打印份数  N1  ASC
2 打印记录  VAR

打印记录格式定义
1       打印控制符  AN3 ASC  %Bn表示第n份标题（第一行居中）；%FF为正文；
    %En表示第n份落款（最后一行居中）；
2       模板索引号  N1  HEX 指明使用的模板记录号(0x01－0XFF)，打印时，
    取出该号码对应记录号的打印信息内容替换。
3       打印信息    VAR     打印信息若为“FFFF”，表示使用菜单显示内容替换



    打印份数
    |打印控制符
    模板索引号
    打印信息
    |打印控制符
    模板索引号
    打印信息
    ......

例：1|%B101|%E103|%FF06FFFF|%FF07#07|%FF0D#02|%FF15#20|%FF09#04
*/
int print_format(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fmtMsgBuf[512 + 1], prtCtl[4], templetIndex[3], msgbuf[200], tmpbuf[100], fieldVal[512+1];
    char advert_head[200],advert_inf[200],advert_tail[200],tmp[256];
    char *p,*p1;
    int len, i,  fieldLen;
    time_t time_cl ;
    struct tm *time_tm;
    struct TPOS_TERM_INFO terminfo;
    ICS_DEBUG(0);
    if(memcmp("00000", pub_data_stru->center_result_code,
              strlen(pub_data_stru->center_result_code))) return 1;
    len = 0;
//  i = 0;
    rtrim(para);
    p1 =  my_split(para,'|',tmp,sizeof(tmp));
    if(p1) fmtMsgBuf[len++] =  tmp[0]; //打印份数

    while((p1=my_split(p1,'|',tmp,sizeof(tmp)))) {
        p=tmp;
        memcpy(prtCtl, p, 3);
        p +=3;

        if(memcmp(prtCtl,"%B",2)!=0
           && memcmp(prtCtl,"%E",2)!=0
           && memcmp(prtCtl,"%FD",2)!=0
           && memcmp(prtCtl,"%FE",2)!=0
           && memcmp(prtCtl,"%FF",3)!=0) {
            dcs_log(0, 0, "<%s>print_format para controlling words[%s]-[%s]出错！",
                    __FUNCTION__, para, prtCtl);
            return -1;
        }
        if(memcmp(prtCtl,"%FD",3) ==0
           || memcmp(prtCtl,"%FE",3)==0) {
            // 判断是否需要过滤掉此指令
            if(strcmp(pub_data_stru->route_msg_type,"TPOS")==0) {
                char term_id[30];
                i=get_field_data_safe(pub_data_stru,
                                      get_pub_field_id(DB_MSG_TYPE,
                                                       "ACQ_TERM_ID1"),
                                      DB_MSG_TYPE,term_id,
                                      sizeof(term_id));
                if(i >0) term_id[i]=0x00;
                else term_id[0]=0x00;
                if(0< get_advert_inf(pub_data_stru,term_id,advert_head,sizeof(advert_head),advert_inf,
					                 sizeof(advert_inf),advert_tail,sizeof(advert_tail))) {
//                  advert_inf[i]=0x00;
                    dcs_debug(0,0,"<%s>advert_head[%s],advert_inf[%s],advert_tail[%s]",__FUNCTION__,advert_head,advert_inf,advert_tail);
                    if(strlen(advert_head) >0 && strlen(advert_inf) >0) {
                        memcpy(fmtMsgBuf + len, "%FF", 3);
                        len += 3;
                        fmtMsgBuf[len++] = 0x00;
                        p=advert_head;
                        while(*p) {
                            fmtMsgBuf[len] = *p;
                            len++;
                            p++;
                        }
                        fmtMsgBuf[len++] = 0;
                        dcs_debug(0,0,"<%s>advert_head succ!",__FUNCTION__);
                    }
                    if(strlen(advert_inf) >0) {
                        memcpy(fmtMsgBuf + len, prtCtl, 3);
                        len += 3;
                        fmtMsgBuf[len++] = 0x00;
                        p=advert_inf;
                        while(*p) {
                            fmtMsgBuf[len] = *p;
                            len++;
                            p++;
                        }
                        fmtMsgBuf[len++] = 0;
                        dcs_debug(0,0,"<%s>advert_inf succ![%s]",__FUNCTION__,advert_inf);
                    }
//                if( strlen( advert_tail) >0 && strlen( advert_inf) >0 )
                    if(strlen(advert_tail) >0) {
                        memcpy(fmtMsgBuf + len, "%FF", 3);
                        len += 3;
                        fmtMsgBuf[len++] = 0x00;
                        p=advert_tail;
                        while(*p) {
                            fmtMsgBuf[len] = *p;
                            len++;
                            p++;
                        }
                        dcs_debug(0,0,"<%s>advert_tail succ!",__FUNCTION__);
//                    fmtMsgBuf[len++] = 0;
                    }
//          continue;
                } else continue;
            }
        } else {
            memcpy(templetIndex, p , 2);
            p += 2;
            templetIndex[2] = 0;
            memcpy(fmtMsgBuf + len, prtCtl, 3);
            len += 3;
            fmtMsgBuf[len++] = atoi(templetIndex);
        }

        if(len > 512) {
            dcs_log(0, 0, "<%s>打印信息超长para[%s]-[%d]+[%d]！", __FUNCTION__,para, len, 1);
            return -1;
        }

        while(*p) {
            if(*p == '#') {
                p++;
                if(memcmp(p, "EEE", 3) == 0) { //商户名称
                    fieldLen = _get_field_data_safe(pub_data_stru,
                                                    get_pub_field_id(DB_MSG_TYPE,
                                                            "ACQ_TERM_ID1"),
                                                    DB_MSG_TYPE,
                                                    fieldVal,2,sizeof(fieldVal));
                    if(fieldLen <= 0) {
                        dcs_log(0, 0, "<%s>取PSAM_NO出错[%d][%d]！", __FUNCTION__,
                                get_pub_field_id(DB_MSG_TYPE, "ACQ_TERM_ID1"),
                                pub_data_stru->route_num);
                        return -1;
                    }
                    fieldVal[fieldLen]=0x00;
                    if(0 > get_tpos_info(fieldVal, &terminfo)) {
                        dcs_log(0, 0, "<%s>取PSAM_NO[%]终端信息出错！", __FUNCTION__,fieldVal);
                        return -1;
                    }
                    fieldLen = strlen(terminfo.name);
                    if(512 < len + fieldLen) {
                        dcs_log(0, 0, "<%s>打印信息超长para[%s]-[%d]+[%d]！",
                                __FUNCTION__,para, len, fieldLen);
                        return -1;
                    }
                    dcs_log(0, 0, "<%s>打印信息[%s]-[%d]+[%d]！",
                            __FUNCTION__,terminfo.name, len, fieldLen);
                    memcpy(fmtMsgBuf + len, terminfo.name, fieldLen);
                    len += fieldLen;
                    p=p+3;
                } else {
                    if(strlen(p) >3) {
                        memcpy(msgbuf,p,3);
                        msgbuf[3]=0x00;
                    } else {
                        snprintf(msgbuf,sizeof(msgbuf),"%s",p);
                        msgbuf[strlen(p)]=0x00;
                    }
                    switch(atoi(msgbuf)) {
                        case FIELD_CARD_NO:
                            fieldLen = get_field_data_safe(pub_data_stru,
                                                           FIELD_CARD_NO,
                                                           pub_data_stru->in_msg_type,
                                                           fieldVal,sizeof(fieldVal));
                            if(fieldLen > 0) {
                                memset(tmpbuf,0,sizeof(tmpbuf));
                                memcpy(tmpbuf, fieldVal, fieldLen);
                                if(fieldLen > 10)
                                    memset(tmpbuf + 6, '*', fieldLen - 10);
                                if(512 < len + fieldLen) {
                                    dcs_log(0, 0, "<%s>打印信息超长para[%s]-[%d]+[%d]！",
                                            __FUNCTION__,para, len, fieldLen);
                                    return -1;
                                }
                                memcpy(fmtMsgBuf + len, tmpbuf, fieldLen);
                                len += fieldLen;
                            }
                            break;
                        case FIELD_DATE_TIME_Y:
                            if(0 > (fieldLen=get_field_data_safe(pub_data_stru,
                                                                 FIELD_DATE_TIME_Y,
                                                                 pub_data_stru->in_msg_type,
                                                                 fieldVal,sizeof(fieldVal)))) {
                                fieldVal[fieldLen]=0x00;
                                time(&time_cl) ;
                                time_tm = localtime(&time_cl);
                                if(strftime(fieldVal, 20, "%Y%m%d%H%M%S",time_tm) == 0) return 0;
                                add_pub_field(pub_data_stru,
                                              FIELD_DATE_TIME_Y,
                                              pub_data_stru->route_msg_type,
                                              14, fieldVal, 1);
                            }
                            fieldLen = 19;
                            if(512 < len + fieldLen) {
                                dcs_log(0, 0, "<%s>打印信息超长para[%s]-[%d]+[%d]！",
                                        __FUNCTION__,para, len, fieldLen);
                                return -1;
                            }
                            memcpy(fmtMsgBuf + len, fieldVal, 4);
                            len += 4;
                            fmtMsgBuf[len++] = '-';
                            memcpy(fmtMsgBuf + len, fieldVal + 4, 2);
                            len += 2;
                            fmtMsgBuf[len++] = '-';
                            memcpy(fmtMsgBuf + len, fieldVal + 6, 2);
                            len += 2;
                            fmtMsgBuf[len++] = ' ';
                            memcpy(fmtMsgBuf + len, fieldVal + 8, 2);
                            len += 2;
                            fmtMsgBuf[len++] = ':';
                            memcpy(fmtMsgBuf + len, fieldVal + 10, 2);
                            len += 2;
                            fmtMsgBuf[len++] = ':';
                            memcpy(fmtMsgBuf + len, fieldVal + 12, 2);
                            len += 2;
                            break;
                        case FIELD_PAY_FEE:
                        case 910: //上送金额
                        case 917: //扣款金额
                            fieldLen = get_field_data_safe(pub_data_stru,atoi(p),
                                                           DB_MSG_TYPE, fieldVal,sizeof(fieldVal));
                            if(fieldLen <=0) break;
                            fieldVal[fieldLen]=0x00;
                            if(fieldVal[0] >= '0' && fieldVal[0] <= '9')
                                sprintf(tmpbuf, "%.2f", atof(fieldVal)/100);
                            else
                                sprintf(tmpbuf, "%.2f", atof(fieldVal+1)/100);

                            dcs_debug(0,0, "<%s>fieldval=[%s],tmpbuf=[%s]",__FUNCTION__,fieldVal,tmpbuf);
                            fieldLen = strlen(tmpbuf);
                            if(512 < len + fieldLen) {
                                dcs_log(0, 0, "<%s>打印信息超长para[%s]-[%d]+[%d]！",
                                        __FUNCTION__,para, len, fieldLen);
                                return -1;
                            }
                            memcpy(fmtMsgBuf + len, tmpbuf, fieldLen);
                            len += fieldLen;
                            break;
                        case 907: //终端流水号
                            fieldLen = get_field_data_safe(pub_data_stru,atoi(p),
                                                           DB_MSG_TYPE, fieldVal,sizeof(fieldVal));
                            if(fieldLen <=0) break;

                            if(512 < len + fieldLen) {
                                dcs_log(0, 0, "<%s>打印信息超长para[%s]-[%d]+[%d]！",
                                        __FUNCTION__,para, len, fieldLen);
                                return -1;
                            }
                            memcpy(fmtMsgBuf + len, fieldVal, fieldLen);
                            len += fieldLen;
                            break;
                        case 61: // 持卡人信息
                            fieldLen = get_field_data_safe(pub_data_stru,61,
                                                           pub_data_stru->in_msg_type,
                                                           fieldVal,sizeof(fieldVal));
                            if(fieldLen <=0) break;
                            if(512 < len + 30) {
                                dcs_log(0, 0, "<%s>打印信息超长para[%s]-[%d]+[%d]！", __FUNCTION__,para, len, fieldLen);
                                return -1;
                            }
                            fieldVal[fieldLen]=0x00;
                            if(memcmp(fieldVal+2+1+1+7+1,"NM",2)==0) {
                                fieldVal[2+1+1+7+1+30]=0x00;
                                rtrim(fieldVal+2+1+1+7+1);
                                memcpy(fmtMsgBuf + len, fieldVal, fieldLen);
                                len += fieldLen;
                            }
                            break;
                        default:
                            fieldLen = get_field_data_safe(pub_data_stru, atoi(msgbuf),
                                                           pub_data_stru->in_msg_type,
                                                           fieldVal,sizeof(fieldVal));
                            if(fieldLen <=0) break;
                            if(512 < len + fieldLen) {
                                dcs_log(0, 0, "<%s>打印信息超长para[%s]-[%d]+[%d]！",
                                        __FUNCTION__,para, len, fieldLen);
                                return -1;
                            }
                            memcpy(fmtMsgBuf + len, fieldVal, fieldLen);
                            len += fieldLen;
                            break;
                    }
                    if(strlen(p) >3) p=p+3;
                    else    p=p+strlen(p);
                }
            } else if(*p == '\\') {
                memcpy(msgbuf, p + 1, 2);
                asc_to_bcd((unsigned char *)fmtMsgBuf + len, (unsigned char *)msgbuf, 2, 1);
                len += 1;
                p=p+3;
            } else {
                fmtMsgBuf[len] = *p;
                len++;
                p++;
            }
        }
        fmtMsgBuf[len++] = 0;
    }
    add_pub_field(pub_data_stru, fldid,
                  pub_data_stru->route_msg_type,
                  len, fmtMsgBuf, 1);
    return 1;
}


//终端ic卡参数下载()
int get_ic_para(glob_msg_stru *pub_data_stru,struct TPOS_TERM_INFO *terminfo) {
    int n,l,cnt,cnt1;
    char buf[512],node_set[200];
    ICS_DEBUG(0);
    //1、在ic参数表中查找大于last_ic_para_step的记录，不存在则类参数已下完。
    dcs_debug(0,0,"<%s> begin",__FUNCTION__);
    memset(node_set,0,sizeof(node_set));
    cnt =0;
    memcpy(node_set,"3C",2);
    n= get_aid_data(buf+1,400,node_set+2,&terminfo->last_ic_para_step, &cnt);
    buf[n+1]=0x00;
    dcs_debug(buf, n + 1, "<FILE:%s,LINE:%d>打印信息[%d]", __FILE__, __LINE__, n);
    if(n <0) return n;
    if(n !=0)
        db_save_set_ic(node_set,terminfo->last_ic_para_step, terminfo->psam,0);
    dcs_debug(buf+1,n,"<%s> getaid_data buf n=%d",__FUNCTION__,n);
    if(n>400) {
        buf[0]=cnt;
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type, "3C"),
                      pub_data_stru->route_msg_type,
                      n+1, buf, 1);
        return 1;
    }
    cnt1=0;
    l=n;
    //2、在ic pub key表中查找大于last_ic_key_step的记录，不存则此类参数已下完。
    if(pub_data_stru->in_cry_flag)
        n=get_pubkey_data_sm(buf+n+1,400-n,node_set+strlen(node_set),&terminfo->last_ic_key_step,&cnt1);
    else
        n=get_pubkey_data(buf+n+1,400-n,node_set+strlen(node_set),&terminfo->last_ic_key_step,&cnt1);
    dcs_log(buf + l, n, "<%s>打印信息[%d]", __FUNCTION__, n);
    if(n <0) return n;
    if(n !=0)
        db_save_set_ic(node_set,terminfo->last_ic_key_step,terminfo->psam,1);
    dcs_debug(buf+1+l,n,"<%s> get_pubkey_data buf n=%d",__FUNCTION__,n);
    l=l+n+1;

    buf[0]=cnt+cnt1;
    if(l >1)
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type,"3C"),
                      pub_data_stru->route_msg_type,l, buf, 1);
    dcs_debug(buf,l,"<%s> end",__FUNCTION__);
    return (l-1);
}


//电力缴费48域处理
int electricity_payment_48(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[256 + 1], tmpBuf[512 + 1], nFieldVal[256 + 1], fld2F[256 + 1], *p,tmp[256];
    int fieldLen, i, l, fid, fld2FLen;
    fid = get_pub_field_id(pub_data_stru->in_msg_type, para);
    fieldLen = get_field_data_safe(pub_data_stru, fid, pub_data_stru->in_msg_type,
                                   fieldVal,sizeof(fieldVal));
    if(fieldLen < 0)return 1;
    if(memcmp("00000", pub_data_stru->center_result_code,
              strlen(pub_data_stru->center_result_code))) return 1;
    memset(tmpBuf, 0, sizeof(tmpBuf));
    memcpy(tmpBuf, "10", 2);
    l = 2;
    memcpy(tmpBuf + l, pub_data_stru->center_result_code, 3);
    l += 3;
    p = my_split(fieldVal, '|',tmp,sizeof(tmp));
    fieldLen = 0;
    fld2FLen = 0;
    switch(tmp[0]) {
        case '1':
            p = my_split(p, '|',tmp,sizeof(tmp));
            l += snprintf(tmpBuf + l,sizeof(tmpBuf)-l, "户名:%s\n未欠费！", tmp);
            fieldLen = sprintf(nFieldVal + fieldLen, "1");
            break;
        case '0':
            p = my_split(p, '|',tmp,sizeof(tmp));
            l += snprintf(tmpBuf + l,sizeof(tmpBuf)-l, "户名:%s\n欠费金额:", tmp);
            if(p == NULL) break;
            p = my_split(p, '|',tmp,sizeof(tmp));   //欠费总金额
            if(p != NULL)
                l += snprintf(tmpBuf + l,sizeof(tmpBuf)-l, "%.2f元", atof(tmp)/100.00);
            else
                break;
            fieldLen += snprintf(nFieldVal + fieldLen, sizeof(nFieldVal)-fieldLen,"%s|", tmp);
            memset(fld2F, 0, sizeof(fld2F));
            fld2F[fld2FLen++] = 3;
            fld2F[fld2FLen++] = 1;
            fld2F[fld2FLen++] = 8;
            memcpy(fld2F + fld2FLen, "应缴金额", 8);
            fld2FLen += 8;
            fld2F[fld2FLen++] = 4;
            fld2F[fld2FLen] = snprintf(fld2F + fld2FLen + 1,sizeof(fld2F)-fld2FLen-1, "%.2f", atof(tmp)/100.00);
            fld2FLen += fld2F[fld2FLen] + 1;
//          memcpy(fld2F + fld2FLen, p, strlen(p)); fld2FLen += strlen(p);
            fld2F[fld2FLen++] = 0x52;
            fld2F[fld2FLen++] = 0x0d;
            memcpy(fld2F + fld2FLen, "确认键继续...", 13);
            fld2FLen += 13;
            i = get_pub_field_id(pub_data_stru->route_msg_type, "2F");
            if(i < 0) {
                dcs_log(0, 0, "<%s>取报文%s[2F]ID失败！", __FUNCTION__, pub_data_stru->route_msg_type);
                return -1;
            }
            add_pub_field(pub_data_stru, i, pub_data_stru->route_msg_type,
                          fld2FLen, fld2F, 1);
            p = my_split(p, '|',tmp,sizeof(tmp));//滞纳金
            if(p== NULL) break;
            for(i = 0; i < 20; i++) { //欠费总月数+欠费明细
                p = my_split(p, '|',tmp,sizeof(tmp));
                if(p==NULL)break;
                if(strlen(tmp)>0)
                    fieldLen += snprintf(nFieldVal + fieldLen,sizeof(nFieldVal)-fieldLen, "%s|", tmp);
            }


            break;
        default:
            fieldLen = snprintf(nFieldVal + fieldLen, sizeof(nFieldVal)-fieldLen, "1");
            snprintf(tmpBuf + 5,sizeof(tmpBuf)-5,"%s", fieldVal);
            break;
    }
    add_pub_field(pub_data_stru, fldid, pub_data_stru->route_msg_type,
                  l, tmpBuf, 1);
    fid = get_pub_field_id(DB_MSG_TYPE, "PAY_ADDITION");
    if(fid < 0) {
        dcs_log(0, 0, "<%s>取数据库[%s]ID失败！",__FUNCTION__, "PAY_ADDITION");
        return -1;
    }
    if(0 > (l = get_pub_field_id(DB_MSG_TYPE, "FLD_48"))) {
        dcs_log(0, 0, "<%s>取数据库[%s]ID失败！",__FUNCTION__,"FLD_48");
        return -1;
    }

    i=_get_field_data_safe(pub_data_stru,fid, DB_MSG_TYPE,
                           tmpBuf, 2,sizeof(tmpBuf));
    if(i > 0) tmpBuf[i]=0x00;
    else tmpBuf[0]=0x00;

    if(0 > SetFieldData(tmpBuf, l, nFieldVal, sizeof(tmpBuf), 1)) {
        dcs_log(0, 0, "<%s>设置数据失败[%d]失败！",__FUNCTION__,  i);
        return -1;
    }
    add_pub_field(pub_data_stru, fid, DB_MSG_TYPE,
                  strlen(tmpBuf), tmpBuf, 2);

    return 1;
}


/*格式化终端显示数据

终端显示信息配置格式定义：
参数前两个字节为显示时长用16进制表示，显示记录格式定义，成功返回配置在前，失败返回配置在后。用^分隔
例：交易金额：00#002
可用余额用954域，余额用054域
*/
int show_format(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[256 + 1], *p,*p1,tmp[512],fmtMsgBuf[512 + 1],tmpbuf[100];
    int len,fieldLen;

    dcs_debug(0,0,"<%s> begin",__FUNCTION__);
    if(memcmp("00000", pub_data_stru->center_result_code,
              strlen(pub_data_stru->center_result_code))) return 1;

    if(para == NULL) {
        dcs_log(0,0,"<%s> para is null",__FUNCTION__);
        return -1;
    }
    len=0;

    rtrim(para);
    p1=my_split(para,'|',tmp,sizeof(tmp));
    if(p1 == NULL) return -1;
    fmtMsgBuf[len++] = 0x31; //刷新后显示
    // 显示时间的长短
    asc_to_bcd((unsigned char *)fmtMsgBuf + len, (unsigned char *)tmp, 2, 0);
    len++;
    // 响应码
    memcpy(fmtMsgBuf + len, pub_data_stru->center_result_code, 3);
    len += 3;
//  flag=0;
    for(p1=my_split(p1,'|',tmp,sizeof(tmp)); p; p=my_split(p1,'|',tmp,sizeof(tmp))) {
        p=tmp;
        while(*p) {
            if(*p == '#') {
                p++;
                {
                    if(strlen(p) >3) {
                        memcpy(tmp,p,3);
                        tmp[3]=0x00;
                    } else {
                        snprintf(tmp,sizeof(tmp),"%s",p);
                        tmp[strlen(p)]=0x00;
                    }
                    switch(atoi(tmp)) {
                        case FIELD_AMOUNT:
                            fieldLen = get_field_data_safe(pub_data_stru,FIELD_AMOUNT,
                                                           pub_data_stru->in_msg_type,
                                                           fieldVal,sizeof(fieldVal));
                            if(fieldLen <=0) break;
                            fieldVal[fieldLen]=0x00;
                            sprintf(tmpbuf, "%.2f", atof(fieldVal)/100);
                            fieldLen = strlen(tmpbuf);
                            if(512 < len + fieldLen) {
                                dcs_log(0, 0, "<%s>显示信息超长para[%s]-[%d]+[%d]！",
                                        __FUNCTION__,p, len, fieldLen);
                                return -1;
                            }
                            memcpy(fmtMsgBuf + len, tmpbuf, fieldLen);
                            len += fieldLen;
                            break;
                        case 28:
                            fieldLen = get_field_data_safe(pub_data_stru,28,
                                                           pub_data_stru->in_msg_type,
                                                           fieldVal,sizeof(fieldVal));
                            if(fieldLen <=0) break;
                            fieldVal[fieldLen]=0x00;
                            if(fieldVal[0]=='C')
                                sprintf(tmpbuf, "%.2f", atof(fieldVal+1)/100);
                            else if(fieldVal[0]=='D')
                                sprintf(tmpbuf, "-%.2f", atof(fieldVal+1)/100);
                            else tmpbuf[0]=0x00;
                            fieldLen = strlen(tmpbuf);
                            if(512 < len + fieldLen) {
                                dcs_log(0, 0, "<%s>显示信息超长para[%s]-[%d]+[%d]！",
                                        __FUNCTION__,p, len, fieldLen);
                                return -1;
                            }
                            memcpy(fmtMsgBuf + len, tmpbuf, fieldLen);
                            len += fieldLen;
                            break;
                        case FIELD_BALANCE:
                        case FIELD_BALANCE_1:
                            fieldLen = get_field_data_safe(pub_data_stru,atoi(tmp),
                                                           pub_data_stru->in_msg_type,
                                                           fieldVal,sizeof(fieldVal));
                            if(fieldLen <=0) break;
                            fieldVal[fieldLen]=0x00;
                            sprintf(tmpbuf, "%.2f", (fieldVal[27] == 'D' ? -1 : 1) * atof(fieldVal + 28)/100);
                            fieldLen = strlen(tmpbuf);
                            if(512 < len + fieldLen) {
                                dcs_log(0, 0, "<%s>显示信息超长para[%s]-[%d]+[%d]！",
                                        __FUNCTION__,p, len, fieldLen);
                                return -1;
                            }
                            memcpy(fmtMsgBuf + len, tmpbuf, fieldLen);
                            len += fieldLen;
                            break;
                        default:
                            fieldLen = get_field_data_safe(pub_data_stru,atoi(tmp),
                                                           pub_data_stru->in_msg_type,
                                                           fieldVal,sizeof(fieldVal));
                            if(fieldLen <=0) break;
                            if(512 < len + fieldLen) {
                                dcs_log(0, 0, "<%s>打印信息超长para[%s]-[%d]+[%d]！",
                                        __FUNCTION__,para, len, fieldLen);
                                return -1;
                            }
                            memcpy(fmtMsgBuf + len, fieldVal, fieldLen);
                            len += fieldLen;
                            break;
                    }
                    if(strlen(p) >3) p=p+3;
                    else    p=p+strlen(p);
                }
            } else if(*p == '\\') {
                memcpy(tmp, p + 1, 2);
                asc_to_bcd((unsigned char *)fmtMsgBuf + len, (unsigned char *)tmp, 2, 1);
                len += 1;
                p=p+3;
            } else {
                fmtMsgBuf[len] = *p;
                len++;
                p++;
            }
        }
    }

    if(len  > 0)
        add_pub_field(pub_data_stru, fldid,pub_data_stru->route_msg_type,
                      len, fmtMsgBuf, 1);
    dcs_debug(fmtMsgBuf,len,"<%s> [%s]end",__FUNCTION__,fmtMsgBuf+5);
    return len;
}

int tpos_get_last_addidata(char *para, short flag, glob_msg_stru * pub_data_stru) {
    tl_trans_log_def TransLog;
//  int i, j, fldLen;
    char fldVal[255 + 1];
    struct  tm *tm;
    time_t  t;
    memset(&TransLog, 0, sizeof(TransLog));
    if(0 > get_field_data_safe(pub_data_stru, FIELD_TRA_NO, pub_data_stru->in_msg_type,
                               TransLog.acq_tra_no,7)) {
        dcs_log(0, 0, "<%s>取数据域[%d]出错", __FUNCTION__, FIELD_TRA_NO);
        return -1;
    }
    if(0 > get_field_data_safe(pub_data_stru, FIELD_PSAM_NO, pub_data_stru->in_msg_type,
                               TransLog.acq_term_id1,17)) {
        dcs_log(0, 0, "<%s>取数据域[%d]出错", __FUNCTION__, FIELD_PSAM_NO);
        return -1;
    }
    if(0 > get_field_data_safe(pub_data_stru,FIELD_INSTI_CODE,
                               pub_data_stru->in_msg_type,
                               TransLog.acq_insti_code,9)) {
        dcs_log(0, 0, "<%s>取数据域[%d]出错", __FUNCTION__, FIELD_INSTI_CODE);
        return -1;
    }
    time(&t);
    tm = localtime(&t);
    snprintf(fldVal,sizeof(fldVal),"%4d%02d%02d",
             tm->tm_year+1900,
             tm->tm_mon + 1,
             tm->tm_mday);
//  memcpy(TransLog.acq_date,fldVal,8);
    memcpy(TransLog.sys_date,fldVal,8);
    sprintf(TransLog.acq_tra_no, "%06d", atoi(TransLog.acq_tra_no) - 1);
    if(0 > select_translog(&TransLog))
        return -1;
    if(0 > db_to_pub_daba(pub_data_stru, &TransLog)) return -1;
    return 1;
}

int use_tpos_check_terminal(glob_msg_stru * pub_data_stru, char * field_name) {
    char psam[17];
    struct TPOS_TERM_INFO terminfo;
    short i,id;
    ICS_DEBUG(0);
    memset(psam,0,sizeof(psam));
    memset(&terminfo,0,sizeof(terminfo));
    if(field_name ==  NULL) {
        dcs_log(0,0,"<%s> field_name is null",__FUNCTION__);
        return -1;
    }
    if(NULL== get_priv_field_def(field_name, &id,pub_data_stru->in_priv_def)) {
        dcs_log(0,0,"<%s> get_priv_field_def[%s] is null!",__FUNCTION__,field_name);
        return -1;
    }
    if(get_field_data_safe(pub_data_stru,id,pub_data_stru->in_msg_type,psam,17)<=0) {
        return -1;
    }

    i=get_tpos_info(psam,&terminfo);
    if(0 > i) {
        strcpy(pub_data_stru->center_result_code,"03");
        return -1;
    }
    if(terminfo.status[0] ==0x30) {
        strcpy(pub_data_stru->center_result_code,"03");
        dcs_log(0,0,"<%s> status is %s",__FUNCTION__,terminfo.status);
        return -1;
    }
    if(pub_data_stru->insti_work_type) {
        if(0>tpos_get_work_key(psam,pub_data_stru->in_pin_index,
                               pub_data_stru->in_mac_index,
                               pub_data_stru->in_data_index,
                               pub_data_stru->in_pin_key,
                               pub_data_stru->in_mac_key,
                               pub_data_stru->in_data_key)) {
            dcs_log(0,0,"<%s> tpos_get_work_key fail psam=%s ",__FUNCTION__,psam);
            strcpy(pub_data_stru->center_result_code,"96");
            return -1;
        }
        dcs_debug(0,0,"pin_index=[%s] pin_key=[%s]\n "
                  "mac_index=[%s] mac_key[%s]\ncd_index=[%s] cd_key=[%s]",
                  pub_data_stru->in_pin_index,pub_data_stru->in_pin_key,
                  pub_data_stru->in_mac_index,pub_data_stru->in_mac_key,
                  pub_data_stru->in_data_index,pub_data_stru->in_data_key);
    }
    dcs_debug(0,0,"<%s> check pass!",__FUNCTION__);
    return 1;
}


//IC卡55域返回
int ic55_ret(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[256 + 1];
    int fieldLen;
    fieldLen = get_field_data_safe(pub_data_stru,
                                   get_pub_field_id(pub_data_stru->in_msg_type,
                                           para),
                                   pub_data_stru->in_msg_type,
                                   fieldVal + 2,sizeof(fieldVal));
    if(fieldLen <= 0)return 1;
    if(memcmp(pub_data_stru->center_result_code, "00000",
              strlen(pub_data_stru->center_result_code))) return 1;
    memcpy(fieldVal, "00", 2);
    return add_pub_field(pub_data_stru, fldid,pub_data_stru->route_msg_type,
                         fieldLen + 2, fieldVal, 1);
}


//格式化回显反向输入函数
int format_echo_input(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char fieldVal[256 + 1], *p,tmp[256];
    int fldLen,i,flag,s;

    dcs_debug(0,0,"<%s> begin",__FUNCTION__);
    if(memcmp("00000", pub_data_stru->center_result_code,
              strlen(pub_data_stru->center_result_code))) return 1;

    if(para == NULL) {
        dcs_log(0,0,"<%s> para is null",__FUNCTION__);
        return -1;
    }
    fldLen=0;

    rtrim(para);
    p=my_split(para,'|',tmp,sizeof(tmp));
    if(p == NULL) return -1;
    i=atoi(tmp);
    fieldVal[fldLen++]=i;
    for(; i >0 ; i--) {
        flag=0;
        for(p=my_split(p,'|',tmp,sizeof(tmp)); p; p=my_split(p,'|',tmp,sizeof(tmp))) {
            if(flag == 0) {
                asc_to_bcd((unsigned char *)(fieldVal+fldLen),(unsigned char *)tmp,2,0);
                fldLen++;
                flag=1;
                dcs_debug(fieldVal,fldLen,"<%s> 数据显示格式 [p=%s]",__FUNCTION__,tmp);
            } else if(flag == 1) {
                if(tmp[0] == '0') { //无需格式化
                    fieldVal[   fldLen++]=strlen(tmp+1);
                    memcpy(fieldVal+fldLen,tmp+1,strlen(tmp+1));
                    fldLen +=strlen(tmp+1);
                    dcs_debug(fieldVal,fldLen,"<%s> 固定数据显示 ",__FUNCTION__);
                } else if(tmp[0]=='1') { //金额、需格式化
                    s = get_field_data_safe(pub_data_stru,
                                            get_pub_field_id(pub_data_stru->route_msg_type,
                                                    tmp+1),
                                            pub_data_stru->route_msg_type, tmp,sizeof(tmp));
                    if(s >0) {
                        tmp[s]=0x00;
                        fieldVal[fldLen] = sprintf(fieldVal + fldLen + 1, "%.2f", atof(tmp)/100.00);
                        fldLen +=(unsigned char)fieldVal[fldLen]+1;
                    } else {
                        dcs_log(0,0,"<%s> get_field_data no data [%s]",__FUNCTION__,tmp+1);
                        return -1;
                    }
                    dcs_debug(fieldVal,fldLen,"<%s> 数据内容 金额 ",__FUNCTION__);
                } else if(tmp[0]=='2') { //从数据域去数据显示
                    s = get_field_data_safe(pub_data_stru,
                                            get_pub_field_id(pub_data_stru->route_msg_type,
                                                    tmp+1),
                                            pub_data_stru->route_msg_type,
                                            tmp,sizeof(tmp));
                    if(s >0) {
                        tmp[s]=0x00;
                        fieldVal[   fldLen++]=strlen(p);
                        snprintf(fieldVal + fldLen , sizeof(fieldVal)-fldLen,"%s", tmp);
                        fldLen +=s;
                    } else {
                        dcs_log(0,0,"<%s> get_field_data no data [%s]",__FUNCTION__,p+1);
                        return -1;
                    }
                    dcs_debug(fieldVal,fldLen,"<%s> 从字段域获取数据",__FUNCTION__);
                }
                break;
            } else break;
        }
    }
    if(fldLen  > 0)
        add_pub_field(pub_data_stru,
                      get_pub_field_id(pub_data_stru->route_msg_type, "2F"),
                      pub_data_stru->route_msg_type,
                      fldLen, fieldVal, 1);
    dcs_debug(fieldVal,fldLen,"<%s> end",__FUNCTION__);
    return fldLen;
}

/**
商家促销折扣计算
**/
int tpos_discount_result(char *para, short flag, glob_msg_stru *pub_data_stru) {
    char tmp[64],src_amount[13];
    DISCNT_INFO discnt_info;
    int len;

    memset(&discnt_info,0,sizeof(discnt_info));
    len = get_field_data_safe(pub_data_stru,
                              get_pub_field_id(pub_data_stru->in_msg_type,
                                      "PSAM_NO"),
                              pub_data_stru->in_msg_type, discnt_info.term_id,17);
    if(len < 0) {
        dcs_log(0, 0, "<%s>不能读 PSAM_NO 数据！", __FUNCTION__);
        strcpy(pub_data_stru->center_result_code,"30");
        return -1;
    }
    discnt_info.term_id[len]=0x00;
    len=get_term_discount_info(&discnt_info);
    if(len < 0) {
        dcs_log(0, 0, "<%s>get_term_discount_info fail！", __FUNCTION__);
        strcpy(pub_data_stru->center_result_code,"96");
        return len;
    }
    if(len == 0) return 1;
    time_t t;
    struct tm *p;
    time(&t);
    p=localtime(&t); /*取得当地时间*/
    sprintf(tmp,"%04d%02d%02d",(1900+p->tm_year),p->tm_mon+1,p->tm_mday);
    if(atol(tmp)<atol(discnt_info.begin_date) ||
       atol(tmp)>atol(discnt_info.end_date)) return 1;
    if(discnt_check_list(pub_data_stru,discnt_info.check_list) < 0) return 1;
    if(0 >= (len=get_field_data_safe(pub_data_stru,FIELD_AMOUNT,
                                     pub_data_stru->in_msg_type,
                                     src_amount,sizeof(src_amount)))) {
        dcs_log(0, 0, "<%s>get_field_data FIELD_AMOUNT fail！", __FUNCTION__);
        strcpy(pub_data_stru->center_result_code,"96");
        return -1;
    }
    src_amount[len]=0x00;
    if(discnt_info.type[0]=='1') { // 金额满多少，送多少
        if(cacl_discount_1(src_amount,discnt_info.para,pub_data_stru) < 0) {
            dcs_log(0, 0, "<%s>cacl_discount_1 fail！", __FUNCTION__);
            strcpy(pub_data_stru->center_result_code,"96");
            return -1;
        }
    } else if(discnt_info.type[0]=='2') {

    }
    return 1;
}

int save_tc_value(glob_msg_stru * pub_data_stru) {
    strcpy(pub_data_stru->center_result_code,"00");
    return 1;
}

int tpos_trans_refund(char *para, short flag, glob_msg_stru *pub_data_stru) {
    char tmp[64],month[3],odate[5],*p;
    char fieldVal[30 + 1];
    struct  tm *time_tm;
    time_t time_cl;
    int len, ret;
    tl_trans_log_def tPosLog;
    time(&time_cl) ;
    time_tm = localtime(&time_cl);
    strftime(tmp, 64, "%Y%m%d%H%M%S", time_tm);
    memcpy(month,tmp+4,2);
    month[3]=0x00;
    ICS_DEBUG(0);
    memset(fieldVal, 0, sizeof(fieldVal));
    len = get_field_data_safe(pub_data_stru,
                              get_pub_field_id(pub_data_stru->in_msg_type, "09"),
                              pub_data_stru->in_msg_type,
                              fieldVal,sizeof(fieldVal));
    if(len < 0) {
        dcs_log(0, 0, "<%s>读原交易信息数据不正确！", __FUNCTION__);
        return -1;
    }
    if(len < 5) {
        dcs_log(fieldVal, len, "<%s>读原交易信息数据不正确！", __FUNCTION__);
        return -1;
    }
    dcs_debug(fieldVal,len,"<%s> field[09]=[%s]",__FUNCTION__,fieldVal);
    p=fieldVal;
    memcpy(odate,p,4);
    odate[4]=0x00;
    dcs_debug(0,0,"<%s> odate=[%s]",__FUNCTION__,p);
    p +=strlen(p)+1;

    dcs_debug(0,0,"<%s> acq_tra_no=[%s]",__FUNCTION__,p);
    memset(&tPosLog, 0, sizeof(tPosLog));

    tmp[8]=0x00;
    if(atoi(odate) >= atoi(tmp+4))
        memcpy(tPosLog.sys_date, tmp, 8);
    else {
        tmp[4]=0x00;
        snprintf(tPosLog.sys_date,sizeof(tPosLog.sys_date),"%04d%4.4s",atoi(tmp)-1, odate);
    }
    memcpy(tPosLog.acq_date, "NULL", 4);
    memcpy(tPosLog.acq_tra_no,p,6);
    get_field_data_safe(pub_data_stru,
                        get_pub_field_id(pub_data_stru->in_msg_type, "PSAM_NO"),
                        pub_data_stru->in_msg_type, tPosLog.acq_term_id1,17);
    sprintf(tPosLog.acq_term_id2, "NULL");
    get_field_data_safe(pub_data_stru,FIELD_INSTI_CODE,
                        pub_data_stru->in_msg_type, tPosLog.acq_insti_code,9);

    ret = select_translog(&tPosLog);
    if(ret < 0)
        return -1;
    if(ret > 0 && memcmp("00", tPosLog.resp_cd_rcv,2)==0 &&
       tPosLog.void_flag[0]=='0') {
        memset(fieldVal, 0, sizeof(fieldVal));
        get_field_data_safe(pub_data_stru,
                            FIELD_AMOUNT,
                            pub_data_stru->in_msg_type,
                            fieldVal,sizeof(fieldVal));
        if(atol(fieldVal) != atol(tPosLog.amount_pay)) {
            strcpy(pub_data_stru->center_result_code, CODE_INVALID_TRANS);
            dcs_log(0,0,"<%s> 撤销金额和原始交易金额不一致!",__FUNCTION__);
            return -1;
        }
        len=get_field_data_safe(pub_data_stru,FIELD_CARD_NO,
                                pub_data_stru->in_msg_type,
                                fieldVal,sizeof(fieldVal));
        if(len >0) {
            fieldVal[len]=0x00;
            rtrim(tPosLog.pay_acct_no);
            if(strcmp(fieldVal,tPosLog.pay_acct_no)!=0) {
                strcpy(pub_data_stru->center_result_code, CODE_INVALID_TRANS);
                dcs_log(0,0,"<%s> 撤销卡号和原始交易卡号不一致! [%s] [%s]",
                        __FUNCTION__,tPosLog.pay_acct_no,fieldVal);
                return -1;
            }
        }
        if(0 > db_to_pub_daba(pub_data_stru, &tPosLog)) return -1;
        return 1;
    } else {
        strcpy(pub_data_stru->center_result_code, ret == 1 ? CODE_PROCESSING : CODE_INVALID_TRANS);
        return -1;
    }
}

int tpos_settle(glob_msg_stru * pub_data_stru) {
    char psam[17];
    struct TPOS_TERM_INFO terminfo;

    if(0>get_psam_no(pub_data_stru ,psam,sizeof(psam))) {
        dcs_log(0,0,"<%s> get_psam_no fail!",__FUNCTION__);
        return -1;
    }
    memset(&terminfo,0,sizeof(terminfo));
    if(0>get_tpos_info(psam,&terminfo)) {
        dcs_log(0,0,"<%s> get_tpos_info fail!",__FUNCTION__);
        return -1;
    }

    if(0>tpos_count_trans(pub_data_stru,psam,terminfo.settle_time)) {
        dcs_log(0,0,"<%s> tpos_count_trans fail!",__FUNCTION__);
        return -1;
    }
    return 1;
}

//设置控制信息：控制信息与数据用,分隔
int set_ctrl_info(char *para, short fldid, glob_msg_stru *pub_data_stru) {
    char ctrl_info[5 + 1], *p,tmp[64];
    p = my_split(para, ',',tmp,sizeof(tmp));
    memset(ctrl_info, 0, sizeof(ctrl_info));
    strncpy(ctrl_info, tmp, 5);
    ctrl_info[5]=0x00;
    p = my_split(p, ',',tmp,sizeof(tmp));
    if(p == NULL)
        add_pub_field(pub_data_stru, fldid, pub_data_stru->route_msg_type, 0, para, 1);
    else
        add_pub_field(pub_data_stru, fldid, pub_data_stru->route_msg_type, strlen(tmp), tmp, 1);
    return 1;
}