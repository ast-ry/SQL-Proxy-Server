#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

int main(){
  int listener,server,client,i;
  struct sockaddr_in addr,sa;
  char buf[1024] = {'\0'};
  fd_set fds;
  int countq = 0,count_sq = 0,count_lp = 0,count_rp = 0,count_dq = 0,size = 0;
  unsigned int len;

  listener = socket(AF_INET,SOCK_STREAM,0);

  memset(&addr,0,sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(5432);  
  addr.sin_addr.s_addr = INADDR_ANY;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(5432);
  sa.sin_addr.s_addr = inet_addr("192.168.247.131");
  
  if(bind(listener,(struct sockaddr *)&addr,sizeof(addr)) < 0)
    perror("bind");
  if(listen(listener,5) < 0)
    perror("listen");

  for(;;){
    len = sizeof(addr);
    client = accept(listener,(struct sockaddr *)&addr,&len);
    if(client < 0)
      perror("accept");
    server = socket(AF_INET,SOCK_STREAM,0);
    if(connect(server,(struct sockaddr *)&sa,sizeof(sa)) < 0)
      perror("connect");

    do{
      FD_ZERO(&fds);
      FD_SET(client,&fds);
      FD_SET(server,&fds);
      int count = select(FD_SETSIZE,&fds,NULL,NULL,NULL);
      if(count > 0) {
        if(FD_ISSET(client, &fds)) {
          size = read(client,buf,sizeof(buf));
          if(size == 0)break;
          if(strcmp(buf,"Q") == 0){
            countq++;
            int cnt_sq = 0,cnt_dq = 0,cnt_lp = 0,cnt_rp = 0;
            for(i=5;i<sizeof(buf);i++){
              if(*(buf+i) == '\0')break;
              if(buf[i] == 39)cnt_sq++;
              if(buf[i] == 34)cnt_dq++;
              if(buf[i] == 40)cnt_lp++;
              if(buf[i] == 41)cnt_rp++;
            }
            if(cnt_dq > 0)count_dq++;
            if(cnt_lp > 0)count_lp++;
            if(cnt_rp > 0)count_rp++;
            if(cnt_sq > 4)count_sq++;
            printf("Received: %s\n count=%d\n single quote=%d\n double quote=%d\n lp=%d\n rp=%d\n",&buf[5],countq,count_sq,count_dq,count_lp,count_rp);
          }
          write(server,buf,size);
        }
        if(FD_ISSET(server, &fds)) {
          size = read(server,buf,sizeof(buf));
          if(size == 0)break;
          write(client,buf,size);
        }
      }
    }while(size !=0);
    close(server);    
  }
  return 0;
}
