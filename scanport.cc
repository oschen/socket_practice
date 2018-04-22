#include<stdio.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<string.h>

typedef struct _port_segment{
  struct in_addr dest_ip;
  unsigned short int min_port;
  unsigned short int max_port;
} port_segment;

void error_out(const char* err_string,int lineno)
{
  fprintf(stderr,"line: %d  ",lineno);
  perror(err_string);
  exit(1);
}

int do_scan(struct sockaddr_in serv_addr)
{
  int conn_fd;
  int ret;

  conn_fd=socket(AF_INET,SOCK_STREAM,0);
  if(conn_fd<0)
    error_out("create socket fail",__LINE__);
  if((ret=connect(conn_fd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))<0))
  {  if(errno==ECONNREFUSED)
    {
      close(conn_fd);
      exit(0);
    }
    else
    {
      close(conn_fd);
      return -1;
    }
  }
  else if(ret==0)
  {
    printf("port %d found in %s\n",ntohs(serv_addr.sin_port),inet_ntoa(serv_addr.sin_addr));
    close(conn_fd);
    return 1;
  }
    return -1;
}

void *scaner(void *arg)
{
   unsigned short int i;
   struct sockaddr_in serv_addr;
   port_segment portinfo;

   memcpy(&portinfo,arg,sizeof(port_segment));
   memset(&serv_addr,0,sizeof(struct sockaddr_in));
   serv_addr.sin_family=AF_INET;
   serv_addr.sin_addr.s_addr=portinfo.dest_ip.s_addr;
   for(i=portinfo.min_port;i<=portinfo.max_port;i++)
   {
     serv_addr.sin_port=htons(i);
     if(do_scan(serv_addr)<0)
       continue;
   }
   return NULL;

}

int main(int argc,char *argv[])
{

  error_out("test errot_out",__LINE__);
  return 0;
}
