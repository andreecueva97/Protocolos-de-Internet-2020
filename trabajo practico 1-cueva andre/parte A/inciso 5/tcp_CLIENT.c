#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
//argc cantidad de argumentos
//argv vector donde estas los argumentos por posicion
#define PORT 3490

#define MAXDATASIZE 100

int main(int argc, char *argv[])
{
  printf("------------------cliente----------------");
 int sockfd, numbytes;//tenemos un solo socket,porq no tenemos dos procesos(entonces no uso FORK)
                      //LO MISMO OCURRE EN SERVIDORES ITERATIVOS
 char buf[MAXDATASIZE];
 struct hostent *he;
 struct sockaddr_in their_addr;

 if (argc != 2 )//pide dos parametros porq el 1 es el nombre del ejecutable
 {
  fprintf(stderr, "usage: client hostname\n");
  // lo vamos a ejecutar dandole por parametros nombre de hosh
  exit(1);
 }

 if ((he=gethostbyname(argv[1])) == NULL)
	 //gethostbyname ->consulta la tabla de host, hace un recuest de hcp
     //                lo necesario para cargar la direccion ip
	 // he carga hostent(direccion ip, nombre host y la info. q obtenga por hcp)
 {
  herror("gesthostbyname");
  exit(1);
 }

 if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	 //creamos el socket igual 	que en el server
 {
  perror("socket");
  exit(1);
 }
//------------------cargo la informacion del servidor------------------
 their_addr.sin_family = AF_INET;
 their_addr.sin_port   = htons(PORT);
 their_addr.sin_addr   = *((struct in_addr *)he->h_addr);//tenemos que pasarle la direcc. ip del CLIENT
                                                         //QUE LA OBTUVIMOS EN he que tiene todo del host
//---------------------------------------------------------------------
 bzero(&(their_addr.sin_zero), 8);

 if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
	 // NOTA :connect inicia la conexion si their_addr(el servicio levantado aca) esta ABIERTA
     //       ES DECIR,si esto lo ejecuto cuando en servidor NO esta en PASIVO, TIRA UN ERROR ACA.
	 //en el servidor habia un bind pero aca lo hacemos con un connect ya qu estamos con CLIENT
     //connect pide:
	 //      .sockfd
	 //      .estructura
 {
  perror("connect");
  exit(1);
 }

while(1) //LO QUE HACE ES MANDARLO MUCHAS VECES EL MENSAJE
{
 char message[20];
 int i;
  printf("\ningrese texto:");
  fgets(message,20,stdin);
  for(i=0;message[i]!='\0';i++); //opcional sino usar strlen(message);
   //_________peilou,  lo q hace el sistema con la conexion que acaba de entrar__________________________
  //char snum[5];
  //snprintf(snum,10,"%d",i);
	printf("\nbytes ENVIADOS %d\n\n",i);
 if (send(sockfd,message,i, 0) == -1) perror("send");
     //send
 else printf("Enviado\n");
 
 if ((numbytes=recv(sockfd, buf, MAXDATASIZE, 0)) == -1)
	 //recv
 {
  perror("recv");
  exit(1);
 }

 buf[numbytes] = '\0';
 printf("Received: %s\n", buf);
 int e;
 for(e=0;buf[e]!='\0';e++);
 printf("\nbytes recibidos %d\n\n",e);
 
 
}
 close(sockfd);

 return 0;
}

