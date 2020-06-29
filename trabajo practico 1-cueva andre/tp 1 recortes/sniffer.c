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
#include <sys/wait.h>   //Utiliza constantes para el uso de waitpid();
#include <arpa/inet.h>  //Define operaciones de internet

#include <netinet/ip.h>
#include <netinet/tcp.h>

#define PORT 5500  // Puerto al cual nos conectaremos
#define MAXDATASIZE 100 //

typedef struct packet_s
{
    struct ip IP_header;
    struct tcphdr TCP_header;
    char data[MAXDATASIZE];
} packet;

void reportErrorIfNecessary(int value, char * string){
    if(value == -1){
        perror(string);
        exit(1);
    }
}

int main( int quantityOfArgumentsReceived, char * listOfArguments[] ){
    printf("Flood your Network!!!\n");
    int fileDescriptorRawSocket;
    int returnedInteger = 0;
    int optionValue = 1;

    struct sockaddr_in address;
    socklen_t addressLength = sizeof(address);

    packet packetToReceive;
   
    fileDescriptorRawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    reportErrorIfNecessary(fileDescriptorRawSocket, "socket");

    returnedInteger = setsockopt(fileDescriptorRawSocket, IPPROTO_IP, IP_HDRINCL, &optionValue, sizeof(optionValue) );
    reportErrorIfNecessary(returnedInteger,"setsockopt");

    while (1)
    {
        recvfrom(fileDescriptorRawSocket, &packetToReceive, sizeof(packet), 0, (struct sockaddr *) &address, &addressLength);
        printf("%s", packetToReceive.data);
    }

    close(fileDescriptorRawSocket);
}