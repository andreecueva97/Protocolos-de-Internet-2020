#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 3490

#define MAXDATASIZE 100

int main(int argc, char *argv[])
{
 int sockfd, numbytes;
 char buf[MAXDATASIZE];
 struct hostent *he;
 struct sockaddr_in their_addr;//cliente
 char total[100]="";
 if (argc != 3 )
 {
  fprintf(stderr, "usage: client hostname\n");
  exit(1);
 }

 if ((he=gethostbyname(argv[1])) == NULL)
 {
  herror("gesthostbyname");
  exit(1);
 }

 if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
 {
  perror("socket");
  exit(1);
 }

 their_addr.sin_family = AF_INET;
 their_addr.sin_port   = htons(PORT);
 their_addr.sin_addr   = *((struct in_addr *)he->h_addr);
 bzero(&(their_addr.sin_zero), sizeof(unsigned char) *8);
if (connect(sockfd, (struct sockaddr *)&(their_addr), sizeof(struct sockaddr)) == -1)
 {
  perror("connect");
  exit(1);
 }
 int primeravuelta=0; //COLOCO ESTO SI QUIERO QUE SEA COMO CHAT
 while(1){//COLOCO ESTO SI QUIERO QUE SEA COMO UN CHAT
   
  if(primeravuelta==1){
      printf("\ningrese texto para enviar al Servidor:");// no funciona bien porque cuando coloco ./client localhost a
      fgets(argv[2],20,stdin);                           //                                  ingrese texto para enviar al servidor:  
      int i;                                               //                             aparecio recien el ingrese
      for(i=0;argv[2][i]!='\0';i++);
  }
  if (send(sockfd, argv[2], strlen(argv[2]), 0) == -1) perror("send");
  else printf("Enviado\n");
  if ((numbytes=recv(sockfd, buf, MAXDATASIZE, 0)) == -1)
  {
  perror("recv");
  exit(1);
  }
  else{
    buf[numbytes] = '\0';
    printf("Received: %s\n", buf);
  }
  primeravuelta=1;
 }
 
 close(sockfd);

 return 0;
}
