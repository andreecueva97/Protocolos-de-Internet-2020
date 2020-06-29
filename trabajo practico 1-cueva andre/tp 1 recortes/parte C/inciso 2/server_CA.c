#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <time.h>

#define PORT 2020
int main(int argc, char *argv[])
{
	fd_set master; /* Archivo de descriptores maestro */
	fd_set read_fds;fd_set write_fds; /* Archivo temporario de descriptores para el select() */
	struct sockaddr_in my_addr; /* direcci�n del servidor */
	struct sockaddr_in clientaddr; /* direcci�n del cliente */
	int fdmax; /* numero max de descriptores */
	int sockfd; /* desxriptor de socket para escuchar pedidos */
	int newfd; /* nuevo descriptor de socket para accept() */
	char buf[1024]; /* buffer cliente */
	int nbytes; /* setsockopt() SO_REUSEADDR, ver abajo */
	int yes = 1;
	int addrlen;
	int i, j, k=-3;//k cliente que envia mensaje para que le llegue a los demas sockets

	FD_ZERO(&master);
	FD_ZERO(&read_fds);FD_ZERO(&write_fds);

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)///----------------------socket
	{
		perror("Server-socket() error lol!");exit(1);
	}
	printf("Server-socket() OK...\n");//-----------------------------------------------
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)//--setsockopt
	{
		perror("Server-setsockopt() error lol!");exit(1);
	}
	printf("Server-setsockopt() OK...\n");//-------------------------------------------
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero), '\0', 8);
	//-------------------
	clock_t before;
	//-------------------------
	if(bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1)//------bind
	{
    		perror("Server-bind() error lol!");exit(1);
	}
	printf("Server-bind() is OK...\n");//--------------------------------------------
	if(listen(sockfd, 10) == -1)//-----------------------------------------------listen
	{
     	  perror("Server-listen() error lol!");exit(1);
	}
	printf("Server-listen() OK...\n");//---------------------------------------------
	FD_SET(sockfd, &master); /* agrego el sockfd al listado maestro */
	fdmax = sockfd; /* guardo el descriptor mayor */
	int selec;
	for(;;)/* loop */
	{
		read_fds = master;
		write_fds = master;
		if((selec=select(fdmax+1, &read_fds, &write_fds, NULL, NULL)) == -1)//-----------------select
		{
			perror("Server-select() error lol!");exit(1);
		}
		//printf("%d",selec);
		//printf("Server-select() OK...\n");//----------------------------------------
		/* recorro las conexiones existentes */
		for(i = 0; i <= fdmax; i++){//recorrer todos los sockets
    		if(FD_ISSET(i, &read_fds)){
				if(i == sockfd){		/* nuevas conexiones */
        			addrlen = sizeof(clientaddr);
					if((newfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen)) == -1){
    					perror("Server-accept() error lol!");//si no se acepta
					}
					else{
						printf("Server-select() OK...\n");
   	  					printf("Server-accept() OK...\n");//acepta conexion correcta
						//-------tiempo de inicio
						before=clock();
						//-----------
						FD_SET(newfd, &master);//se agrega el socketdelcliente al LISTA MASTER
	  					if(newfd > fdmax)//ocurre si el nuevocliente es > que el fdmax
		 					fdmax = newfd;
						printf("%s: Nueva conexion desde %s en socket %d\n", argv[0], inet_ntoa	(clientaddr.sin_addr), newfd);
						//sleep(10);
						//-----------le envio al cliente su tiempo
						char str[100];
						sprintf(str,"el numero de espera es %d",newfd);
						if(send(newfd,str,strlen(str),0)==-1)perror("send");
						//-------------
					}
    			}else{//si ya esta el cliente entonces
				    if((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0)//este server puede recibir mensajes
				    {
                        //printf("%d\n", i); /* error o conexion cerrada por cliente */

                        if(nbytes == 0){//DESCONECTADO cliente
							//printf("Transcurrido: %d\n", cronosec(1));
                            printf("%s: socket %d desconectado \n", argv[0], i);
							//--------sock desconectado,tiempo conectado es:
							clock_t difference=clock()-before;
							int msec=0;
							msec= difference*1000/CLOCKS_PER_SEC;
							printf("%s: socket %d desconectado ->time que estuvo conectado al server:%d seconds\n",argv[0],i,msec/1000);
							//---------------------------------------------------
						}else//error en recv()
                            perror("recv() error lol!");
                        close(i);//cierra el socket
                                /* la saco del maestro */
                        FD_CLR(i, &master);
				    }else{
						char str2[100];
						int timee=0;
						timee=(clock()-before)/CLOCKS_PER_SEC;
						sprintf(str2,"tiempo de conexion es :%d",timee);
						if(send(newfd,str2,strlen(str2),0)==-1)perror("send");
						k=i;		
						// clock_t differenc=clock()-before;
						// 	int msec=0;
						// 	msec= differenc*1000/CLOCKS_PER_SEC;
						// printf("tiempo:%d",msec/1000);
					}
			    }
			}
		}
		for(j = 0; j <= fdmax; j++)//toma todos los otros sockets
		{
			//////////////////////////////////////////////////////////////
            if(k!=-3){                /* lo mando a todos */
            	if(FD_ISSET(j, &write_fds))//seteado en master entonces entro una conexion nueva newfd
            	{
               		              /* excepto al sockfd y a nosotros mismos */
					if(j != sockfd && j != k)
					{
						sleep(1);
						if(send(j, buf, nbytes, 0) == -1)//le envia un mensaje a todos los otros sockets
							
							perror("send() error lol!");
					}
				}
            }
		}
		k=-3;
	}
	return 0;
}

