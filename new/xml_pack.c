#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "base.h"
#include "tools.h"
#include "general_util.h"
typedef struct {
	int top;
	char cache[10][64];
}XML_PSTACK;

int pxml_init_stack(XML_PSTACK *stack)
{
	if( stack == NULL ) return 0;
	stack->top=0;
	return 1;
}

int  pxml_stack_is_empty(XML_PSTACK *stack)
{
	 if( stack == NULL ) return 1;
	 if( stack->top == 0 ) return 1;
	 return 0;
}
char * pxml_pop_stack(XML_PSTACK *stack)
{
	if( stack == NULL ) return NULL;
	if( stack->top <= 0) return NULL;
	stack->top--;
	return stack->cache[stack->top];
}

int pxml_push_stack(XML_PSTACK *stack,char *str)
{
	if( stack == NULL || str == NULL ) return 0;
	if( stack->top >=10 ) return 0;
	if( strlen(str ) >=64 ) return 0;
	strcpy(stack->cache[stack->top],str);
	stack->top++;
	return 1;
}

int xml_pack(const char *msg_type, message_define *priv_def, glob_msg_stru *pub_data_stru, char *buf)
{
	char *p,*s,*s1,*s2,curr_node_name[64],tmp[512],str[128];
	int n,i;
	field_set *p_set;
	XML_PSTACK stack;
	if( buf == NULL ) return -1;
	p=buf;
	p_set=&pub_data_stru->route_set;
  if( strcmp(msg_type,"TXML")!=0)
  {
  		dcs_log(0,0,"<%s> not recognition msgtype[%s]!",__FUNCTION__,msg_type);
  		return -1;
  }
  if( strcmp(msg_type,priv_def->msg_type)!=0)
  {
  		dcs_log(0,0,"<%s> not match!  msgtype[%s], priv msgtype=[%s]",__FUNCTION__,msg_type,priv_def->msg_type);
  		return -1;
  }
  pub_data_stru->route_priv_def=priv_def;
	pxml_init_stack(&stack);
	curr_node_name[0]=0x00;
	for ( i=0 ; i<p_set->num ; i++)
	{
		s=my_split(p_set->field.field_name[i],'|',str,sizeof(str));
		if( s == NULL )
		{	 
			dcs_log(0,0,"<%s> p_set->field_name[%d] is NULL ",__FUNCTION__,i);
			return -1;
	  }
		s1=my_split(s,'|',tmp,sizeof(tmp));
		if( s1 == NULL ) 
		{
			 dcs_log(0,0,"<%s> p_set->field_name[%d] pid is NULL ",__FUNCTION__,i);
			 return -1;
		}
		if( strcmp(tmp,curr_node_name)!=0 && strcmp(tmp,"0")!=0)
		{
			while(1)
			{
				s2=pxml_pop_stack(&stack);
				if( s2 == NULL )
				{
						dcs_log(0,0,"<%s>stack  is unexpected NULL!",__FUNCTION__);
						return -1;
				}
				if(strcmp(s2,tmp)!=0 )
				{
					n=sprintf(p,"</%s>",s2);
					p += n;
				}
				else
				{
					if(! pxml_push_stack(&stack,s2))
					{
							dcs_log(0,0,"<%s>stack  is unexpected overflow!",__FUNCTION__);
							return -1;
					}
					break;
				}
		  }
		}
		
		n=sprintf(p,"<%s>",s);
		p += n;
		strcpy(curr_node_name,str);
	  
		if(! pxml_push_stack(&stack,str))
		{
				 dcs_log(0,0,"<%s>stack  is overflow!",__FUNCTION__);
				 return -1;
		}
    // ÃÌº””Úƒ⁄»›	
    if( 0<(n=get_field_data_safe(pub_data_stru, get_pub_field_id(msg_type,str), 
    															msg_type, tmp,sizeof(tmp)-1)))
    {
    	tmp[n]=0x00;
    	n=sprintf(p,"%s",tmp);
    	p +=n;
    }
  }
  while (1)
  {
  	s1=pxml_pop_stack(&stack);
  	if( s1 == NULL ) break;
  	n=sprintf(p,"</%s>",s1);
		p += n;
  }
  return p - buf;
}