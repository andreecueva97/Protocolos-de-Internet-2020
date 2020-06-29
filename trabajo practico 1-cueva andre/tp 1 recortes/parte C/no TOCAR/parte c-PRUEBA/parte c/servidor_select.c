#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* puerto en el que escuchamos */

#define PORT 2020

int main(int argc, char *argv[])

{

	fd_set master; /* Archivo de descriptores maestro */
	fd_set read_fds; /* Archivo temporario de descriptores para el select() */
	struct sockaddr_in serveraddr; /* direcci�n del servidor */
	struct sockaddr_in clientaddr; /* direcci�n del cliente */
	int fdmax; /* numero max de descriptores */
	int listener; /* desxriptor de socket para escuchar pedidos */
	int newfd; /* nuevo descriptor de socket para accept() */
	char buf[1024]; /* buffer cliente */
	int nbytes; /* setsockopt() SO_REUSEADDR, ver abajo */
	int yes = 1;
	int addrlen;
	int i, j;

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Server-socket() error lol!");
		exit(1);
	}
	printf("Server-socket() OK...\n");

	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("Server-setsockopt() error lol!");
		exit(1);
	}

	printf("Server-setsockopt() OK...\n");


	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(PORT);
	memset(&(serveraddr.sin_zero), '\0', 8);

	if(bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
	{
    		perror("Server-bind() error lol!");
    		exit(1);
	}

	printf("Server-bind() is OK...\n");

	/* listen */
	if(listen(listener, 10) == -1)
	{
     	  perror("Server-listen() error lol!");
     	  exit(1);
	}

	printf("Server-listen() OK...\n");

	FD_SET(listener, &master); /* agrego el listener al listado maestro */

	fdmax = listener; /* guardo el descriptor mayor */

	/* loop */

	for(;;)
	{

		read_fds = master;

		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
			perror("Server-select() error lol!");
			exit(1);

		}

		printf("Server-select() K...\n");

		/* recorro las conexiones existentes */

		for(i = 0; i <= fdmax; i++)
		{

            if(FD_ISSET(i, &read_fds))
            {
				if(i == listener)
				{

         				/* nuevas conexiones */

        				addrlen = sizeof(clientaddr);

					if((newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
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

	  				printf("%s: Nueva conexi�n desde %s en socket %d\n", argv[0], inet_ntoa	(clientaddr.sin_addr), newfd);

                }else{

                /* datos de cliente */

                    if((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0)
                    {
                    /* error o conexi�n cerrada por cliente */
                        printf("%d", i);
                        if(nbytes == 0)
                            printf("%s: socket %d desconectado \n", argv[0], i);

                        else
                            perror("recv() error lol!");

                        close(i);

                        /* la saco del maestro */
                        FD_CLR(i, &master);

                    }else{

                    /* tengo un dato de cliente */

                    for(j = 0; j <= fdmax; j++)
                    {

                        /* lo mando a todos */

                        if(FD_ISSET(j, &master))
                        {
                                /* excepto al listener y a nosotros mismos */
                                if(j != listener && j != i)
                                {
                                    sleep(1);
                                    if(send(j, buf, nbytes, 0) == -1)
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
