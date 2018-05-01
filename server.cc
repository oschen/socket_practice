#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>

#define BUFSIZE 1024
#define MAX_USERNAME 100
#define MAX_PASSWD 100
#define SERV_PORT 12345
#define LISTENQ 10

enum recv_flags{USERNAME,PASSWD};

extern void error_out(const char *,int);

struct userinfo{
  char username[MAX_USERNAME];
  char passwd[MAX_PASSWD];
};

struct userinfo users[]={
  {"oscar","050513"},
  {"chenzhijie","050507"},
  {" "," "}
};

int find_name(const char *name)
{
  int i;
  if(name==NULL)
  {
    printf("in find name , name is NULL");
    return -2;
  }
  for(i=0;users[i].username[0]!=' ';i++)
  {
    if(strncmp(users[i].username,name,strlen(users[i].username))==0)
      return i;
  }
  return -1;
}

void send_data(int conn_fd,const char *string)
{
  if(send(conn_fd,string,strlen(string),0)<0)
    error_out("send_data at server side",__LINE__);
}

int main(int argc,char *argv[])
{
  int socket_fd,conn_fd;
  pid_t pid;
  int res,name_num;
  int optval,cliaddr_len;
  enum recv_flags flag_recv=USERNAME;
  struct sockaddr_in cli_addr,serv_addr;
  char recv_buf[BUFSIZE];
  socket_fd=socket(AF_INET,SOCK_STREAM,0);
  if(socket_fd<0)
    error_out("create socket",__LINE__);
  optval=1;
  if(setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,(void *)&optval,sizeof(int))<0)
     error_out("setsocket",__LINE__);
   memset(&serv_addr,0,sizeof(struct sockaddr_in));
   serv_addr.sin_family=AF_INET;
   serv_addr.sin_port=htons(SERV_PORT);
   serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

   //printf("ip is : %s\n",inet_ntoa(serv_addr.sin_addr));

   if(bind(socket_fd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in))<0)
     error_out("bind socket",__LINE__);
   if(listen(socket_fd,LISTENQ)<0)
     error_out("listen",__LINE__);
  cliaddr_len=sizeof(struct sockaddr_in);
  while(true)
  {

      conn_fd=accept(socket_fd,(struct sockaddr *)&cli_addr,(socklen_t *)&cliaddr_len);
      if(conn_fd<0)
        error_out("accept",__LINE__);
      printf("accept a new client, ip:%s\n",inet_ntoa(cli_addr.sin_addr));
      if((pid=fork())==0)
        {
          while(true)
          {
              memset(recv_buf,0,BUFSIZE);
              if((res=recv(conn_fd,recv_buf,sizeof(recv_buf),0))<0)
                error_out("recv from client",__LINE__);
              if(res==0)
              {  printf("connection has been closed or reset\n");
                 exit(0);
            }
              recv_buf[res-1]='\0';
              if(flag_recv==USERNAME)
            {    name_num=find_name(recv_buf);
                 switch(name_num)
                 {
                   case -1:
                     printf("user info %s is not found in database\n",recv_buf);
                     send_data(conn_fd,"n\n");
                     break;
                  case -2:
                     exit(1);
                     break;
                  default:
                    send_data(conn_fd,"y\n");
                    flag_recv=PASSWD;
                    break;
                 }
}
            else if(flag_recv==PASSWD)
            {
              if(strncmp(users[name_num].passwd,recv_buf,strlen(users[name_num].passwd))==0)
              {
                send_data(conn_fd,"y\n");
                send_data(conn_fd,"welcome login tcp server\n");
                printf("%s login\n",users[name_num].username);
                break;

              }
              else
                send_data(conn_fd,"n\n");
            }

          }
          close(conn_fd);
          close(socket_fd);
          printf("child process exit\n");
          sleep(5);
          exit(0);

        }
        else
          close(conn_fd);
  }
  return 0;



}
