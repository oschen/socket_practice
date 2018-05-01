#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>

#define BUFSIZE 1024

void error_out(const char *err_string,int linenum)
{
  fprintf(stderr,"line:%d",linenum);
  perror(err_string);
  exit(1);
}

int my_recv(int conn_fd,char *data_buf,int len)
{
  static char mybuf[BUFSIZE];
  static char *pread;
  static int len_remain=0;
  int i;
  if(len_remain<=0)
  {
    if((len_remain=recv(conn_fd,mybuf,BUFSIZE,0))<0)
      error_out("recv",__LINE__);
    else if(len_remain==0)
      return 0;
    pread=mybuf;

  }

   for(i=0;*pread!='\n';i++)
   {
     if(i>=len-1)
      {
        data_buf[i]=*pread++;
        return len;

      }

    data_buf[i]=*pread++;
    len_remain--;
   }

   len_remain--;
   pread++;
   return i;

}

int get_userinfo(char *buf,int len)
{
  int i=0;
  int c;
  if(buf==NULL)
  {
    return -1;
  }
  while(((c=getchar())!='\n')&&(c!=EOF)&&(i<len-2))
  {
    buf[i++]=c;
  }
  buf[i++]='\n';
  buf[i]='\0';
  return 0;

}
