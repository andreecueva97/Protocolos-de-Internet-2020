#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 2020
/* puerto en el que escuchamos */
int main(int argc, char *argv[])
{
	fd_set master; /* Archivo de descriptores maestro */
	fd_set read_fds; /* Archivo temporario de descriptores para el select() */
	struct sockaddr_in my_addr; /* direcci�n del servidor */
	struct sockaddr_in clientaddr; /* direcci�n del cliente */
	int fdmax; /* numero max de descriptores */
	int sockfd; /* desxriptor de socket para escuchar pedidos */
	int newfd; /* nuevo descriptor de socket para accept() */
	char buf[1024]; /* buffer cliente */
	int nbytes; /* setsockopt() SO_REUSEADDR, ver abajo */
	int yes = 1;
	int addrlen;
	int i, j;

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

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
	for(;;)/* loop */
	{
		read_fds = master;
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)//-----------------select
		{
			perror("Server-select() error lol!");exit(1);
		}
		printf("Server-select() OK...\n");//----------------------------------------
		/* recorro las conexiones existentes */
		for(i = 0; i <= fdmax; i++)//recorro todos los sockets
		{
    			if(FD_ISSET(i, &read_fds))
    			{
				    if(i == sockfd)
				    {
         				/* nuevas conexiones */
        			    addrlen = sizeof(clientaddr);
					    if((newfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
					    {
    						perror("Server-accept() error lol!");
					    }
					    else
					    {
   	  					    printf("Server-accept() OK...\n");
						    FD_SET(newfd, &master);
	  					    if(newfd > fdmax)
		 					    fdmax = newfd;
  	  				    }
	  				    printf("%s: Nueva conexion desde %s en socket %d\n", argv[0], inet_ntoa	(clientaddr.sin_addr), newfd);
    				}
  			    else
  			    {
  		                	/* datos de cliente */
				    if((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0)
				    {
                        printf("%d", i); /* error o conexi�n cerrada por cliente */
                        if(nbytes == 0)
                            printf("%s: socket %d desconectado \n", argv[0], i);
                        else
                            perror("recv() error lol!");
                        close(i);
                                /* la saco del maestro */
                        FD_CLR(i, &master);
				    }

				    else      //me esta tomando el conjunto de write y exception
				    {
				            /* tengo un dato de cliente */
				        for(j = 0; j <= fdmax; j++)//toma todos los otros sockets
				        {
                            /* lo mando a todos */
                            if(FD_ISSET(j, &master))//seteado en master entonces entro una conexion nueva newfd
                            {
                                /* excepto al sockfd y a nosotros mismos */
                                if(j != sockfd && j != i)
                                {
                                    if(send(j, buf, nbytes, 0) == -1)//le envia un mensaje a todos los otros sockets
                                        perror("send() error lol!");
                                }
                            }

				        }
				    }
  			    }
			}
		}
	}
	return 0;
}

