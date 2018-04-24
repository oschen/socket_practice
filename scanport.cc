#include<stdio.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<string.h>
#include<pthread.h>


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

  //error_out("test errot_out",__LINE__);
  pthread_t *thread;
  int max_port;
  int thread_num;
  int seg_len;
  struct in_addr dest_ip;
  int i;
  if(argc!=7)
  {  error_out("Usage: ./scanport -m max_port -a server_addr -n thread_num\n",__LINE__);
     exit(1);
   }
   for(i=1;i<argc;i++)
   {
     if(strcmp(argv[i],"-m")==0)
      {
        max_port=atoi(argv[i+1]);
        if(max_port<=0 || max_port>=65535)
        {  error_out("invalid max port\n",__LINE__);
            exit(1);
          }
        continue;
      }
      if(strcmp("-a",argv[i])==0)
      {
        if(inet_aton(argv[i+1],&dest_ip)==0)
        {
          printf("Usage:invalid dest ip address\n");
          exit(1);
        }
        continue;
      }
      if(strcmp("-n",argv[i])==0)
      {
        thread_num=atoi(argv[i+1]);
        if(thread_num<=0)
        {
          printf("Usage: invalid thread num\n");
          exit(1);
        }
        continue;
      }
   }

   if(max_port<thread_num)
       thread_num=max_port;
  seg_len=max_port/thread_num;
  if((max_port%thread_num)!=0)
     thread_num+=1;
  thread=(pthread_t *)malloc(thread_num*sizeof(pthread_t));
  port_segment *portinfo=(port_segment*)malloc(thread_num*sizeof(port_segment));
  for(i=0;i<thread_num;i++)
  {
    portinfo[i].dest_ip=dest_ip;
    portinfo[i].min_port=i*seg_len+1;
    if(i==thread_num-1)
    {
      portinfo[i].max_port=max_port;
    }
    else
    {
      portinfo[i].max_port=portinfo[i].min_port+seg_len-1;
    }
    if(pthread_create(&thread[i],NULL,scaner,(void *)(&portinfo[i]))!=0)
       error_out("create thread fail\n",__LINE__);
  }
     for(i=0;i<thread_num;i++)
       pthread_join(thread[i],NULL);
    free(portinfo);

  return 0;
}
