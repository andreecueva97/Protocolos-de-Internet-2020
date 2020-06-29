#include <stdio.h>  //Libreria estandar de C
#include <stdlib.h> //Libreria que permite el manejo de la memoria, entre otras cosas mas
#include <errno.h>  //Define la variable 'int errno' (Error Number)
                    //Es usado por 'System Calls'/Funciones para indicar que 'algo salio mal'
#include <string.h> //Nos da funciones para manejar Strings
#include <netdb.h>  //Deficiones para operacion de base de datos de red
#include <netinet/in.h> //Define in_port_t, in_addr_t, sin_family, sin_port, entre otros.
#include <unistd.h> //Define constantes y declara funciones

#include <sys/types.h>  //Define los tipos de datos como lo es 'id_t'
#include <sys/socket.h> //Define los tipos de datos como lo es 'sockaddr'
#include <sys/select.h> //Define lo necesario para el uso de select


#define PORT 2020

#define MAXDATASIZE 100

int main(int argc, char *argv[])
{
 int sockfd, numbytes, bytesenv;
 char buf[MAXDATASIZE];
 struct hostent *he;
 struct sockaddr_in their_addr;
 int i=0;
 char txt[MAXDATASIZE];
 int fdmax; /* numero max de descriptores */

 fd_set read_fds;

 FD_ZERO(&read_fds);

 if (argc != 2 ) 
 {
  fprintf(stderr, "usage: client hostname\n");
  exit(1);
 }

 if ((he=gethostbyname(argv[1])) == NULL)
 {
  herror("gesthostbyname");
  exit(1);
 }

 if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
 {
  perror("socket");
  exit(1);
 }

 their_addr.sin_family = AF_INET;
 their_addr.sin_port   = htons(PORT);
 their_addr.sin_addr   = *((struct in_addr *)he->h_addr);
 bzero(&(their_addr.sin_zero), 8);

 if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
 {
  perror("connect");
  exit(1);
 }
 if(sockfd > STDIN_FILENO)
        fdmax = sockfd;
 else{
        fdmax = STDIN_FILENO;
 }


while(1){

        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
			perror("Server-select() error lol!");
			exit(1);
		}

		printf("Server-select() K...\n");

		if( FD_ISSET(sockfd, &read_fds) ){

            if ((numbytes=recv(sockfd, buf, MAXDATASIZE, 0)) <= 0)
             {
              perror("recv");
              exit(1);
             }
            else{
                 buf[numbytes] = '\0';
                 printf("Received: %s\n", buf);
            }

        }

        if( FD_ISSET(STDIN_FILENO, &read_fds) ){
             fgets(txt,MAXDATASIZE,stdin);
             send(sockfd, txt, strlen(txt) * sizeof(char), 0 );
             bzero(txt,MAXDATASIZE);

        }
}

 close(sockfd);

 return 0;
}
