#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
 
#define MYPORT 3490  // Puerto al cual nos conectaremos

#define MAXDATASIZE 100

int main()
{
 int sockfd; // escuchamos en sockfd
 struct sockaddr_in my_addr;//servidor
  struct sockaddr_in their_addr;//cliente
 socklen_t sin_size;
 int numbytes;
 char buf[MAXDATASIZE];

//SOCK_DGRAM ES PARA DATAGRAMAS, SE ACLARA Q ES UDP
 if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
 {
  perror("socket");
  exit(1);
 }

 my_addr.sin_family = AF_INET;
 my_addr.sin_port   = htons(MYPORT);
 my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
 memset(&(my_addr.sin_zero),'\0',8);


if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
 {
  perror("bind");
  exit(1);
 }
 

 while(1)
 {
  
 
   sin_size = sizeof(struct sockaddr_in);


    while(1) //COLOCO ESTO SI QUIERO QUE SEA COMO UN CHAT
     {
    
    
    if ((numbytes=recvfrom(sockfd, buf, MAXDATASIZE, 0,(struct sockaddr *) &(their_addr), (socklen_t *)& sin_size)) == -1)
    {
     perror("recv");
     exit(1);
    }
    
    buf[numbytes] = '\0';
    
    printf("Received: %s    ip:%s\n", buf,inet_ntoa(their_addr.sin_addr));
    char message[20];
    printf("ingrese texto para enviar al Cliente:");
    fgets(message,20,stdin);
    int i;
    for(i=0;message[i]!='\0';i++);
	  
    if (connect(sockfd, (struct sockaddr *)&(their_addr), sizeof(struct sockaddr_in)) == -1)
 {
  perror("connect");
  exit(1);
 }

   if (send(sockfd,message,i, 0) == -1) perror("send");
    else printf("Enviado\n");
    }//while para el chat
   close(sockfd);
   while(waitpid(-1, NULL, WNOHANG) > 0);
 }
}
