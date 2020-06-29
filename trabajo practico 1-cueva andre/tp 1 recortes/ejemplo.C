#include <stdio.h>  
#include <stdlib.h> 
#include <errno.h>  
                   
#include <string.h> 
#include <netdb.h>  
#include <netinet/in.h> 
#include <unistd.h> 
#include <sys/types.h>  
#include <sys/socket.h> 
#include <sys/wait.h>  
#include <arpa/inet.h>  

#include <netinet/ip.h>
#include <netinet/tcp.h>

#define _BSD_SOURCE
#define __USE_BSD 
// usar encabezado TCP BSD
#define __FAVOR_BSD     
// para inundar el puerto 25 de sendmail 
#define P 25            

int main (void)
{
	int sd;//initialize socket
	// buffer para guardar encabezados IP, TCP y datos (payload)
	// se apunta al comienzo de la estructura del encabezado IP, luego la estructura del encabezado TCP para escribir los valores de los mismos
	char buffer[8192];
	struct ip *iph = (struct ip *) buffer;
	struct tcphdr *tcph = (struct tcphdr *) buffer + sizeof (struct ip);

	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons (P);
	sin.sin_addr.s_addr = inet_addr ("127.0.0.1");
	memset (buffer, 0, 8192);
	int one = 1;
    const int *val = &one;
	socklen_t addresslength = sizeof(sin);

	if((sd=socket (PF_INET, SOCK_RAW, IPPROTO_TCP))<0){//create socket
		printf("error create socket");exit(1);
	}

    if (setsockopt (sd, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)  {
        printf ("Atención: no se pueder setear HDRINCL!\n");exit(1);
    }

	while (1)
    {
		if(recvfrom(sd,buffer,sizeof(buffer),0,(struct sockaddr *)&sin,&addresslength)) > 0) 
			printf ("Paquete TCP: %s\n", buffer+sizeof(struct iphdr)+sizeof(struct tcphdr)); 
    }

	close(sd);
    return 0;
}
