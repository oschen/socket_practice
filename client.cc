#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define VALID_USER 'y'
#define INVALID_USER 'n'
#define MAX_USERNAME 100
#define MAX_PASSWD 100
#define BUFSIZE 1024

int my_recv(int,char *,int);
extern int get_userinfo(char *,int);
extern void error_out(const char *err_string,int linenum);

void input_userinfo(int conn_fd,const char *string)
{
  char input[MAX_USERNAME];
  char recv_buf[BUFSIZE];
  int flag=0;
  int max_retry=3;
  int count=0;
  do{
   printf("%s: ",string);
   if(get_userinfo(input,MAX_USERNAME)<0)
     error_out("get input from user",__LINE__);
   if(send(conn_fd,input,strlen(input),0)<0)
     error_out("send",__LINE__);
   if(my_recv(conn_fd,recv_buf,BUFSIZE)<0)
      error_out("receive data",__LINE__);
   if(recv_buf[0]==VALID_USER)
      flag=1;
  else
  {
    count++;
    if(count>=max_retry)
    {
      printf("input user info retry times exceed\n");
      exit(1);

  }
    printf("%s error, please retry !\n",string);
  }
}while(flag==0);


}

int main(int argc,char *argv[])
{
   int conn_fd,serv_port,res;
   struct sockaddr_in serv_addr;
   char receive[BUFSIZE];

   if(argc!=5)
   {
     printf("Usage: [-p] [port] [-a] [serv_address]\n");
     exit(1);
   }
   memset(&serv_addr,0,sizeof(struct sockaddr_in));
   serv_addr.sin_family=AF_INET;
   for(int i=1;i<argc;i++)
   {
     if(strcmp("-p",argv[i])==0)
     {
       serv_port=atoi(argv[i+1]);
       if(serv_port<0 || serv_port>65535)
         error_out("invalid port",__LINE__);
       else
         serv_addr.sin_port=htons(serv_port);

      continue;
     }
     if(strcmp("-a",argv[i])==0)
     {
       if(inet_aton(argv[i+1],&serv_addr.sin_addr)==0)
          error_out("input server addres error",__LINE__);
        continue;
     }

   }
     conn_fd=socket(AF_INET,SOCK_STREAM,0);
     if(conn_fd<0)
       error_out("creat socket error",__LINE__);
     if(connect(conn_fd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in))<0)
       error_out("connect server fail",__LINE__);


     input_userinfo(conn_fd,"username");
     input_userinfo(conn_fd,"passwd");

     if((res=my_recv(conn_fd,receive,BUFSIZE))<0)
       error_out("receive from server error",__LINE__);
     for(int i=0;i<res;i++)
       printf("%c",receive[i]);
     putchar('\n');
     close(conn_fd);
     return 0;





}
