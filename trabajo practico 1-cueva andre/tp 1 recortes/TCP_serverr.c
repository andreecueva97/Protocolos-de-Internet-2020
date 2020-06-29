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
#include <sys/select.h> //Define lo necesario para el uso de select
#include <sys/time.h>   //Lo usamos por el 'sleep'

#include <arpa/inet.h>  //Define operaciones de internet

#define PORT 5007
#define BACKLOG 20 //Define el largo maximo de la cola de conexiones pendientes del servidor
#define MAXDATASIZE 100

void reportErrorIfNecessary(int value, char * string){
    if(value == -1){
        perror(string);
        exit(1);
    }
}

int main(){
    printf("Servidor TCP con Concurrencia Aparente\n");

    int fileDescriptorListenerSocket = 0;
    int fileDescriptorNewClientSocket = 0;
    int clientConnectionOrder = 0;
    int maxFileDescriptor = 0;
    int fileDescriptorIndex = 0;
    int auxiliarFileDescriptorIndex = 0;

    fd_set fileDescriptorsSet;
    fd_set readFileDescriptorsSet;

    struct sockaddr_in serverSocketAddress;
    struct sockaddr_in newClientSocketAddress;

    socklen_t socketAddressSize;

    char messageToSent[MAXDATASIZE];
    char messageReceive[MAXDATASIZE];

    int returnedInteger; //Lo uso para controlar errores
    int quantityOfBytesReceived;
    
    FD_ZERO(&fileDescriptorsSet);
    FD_ZERO(&readFileDescriptorsSet);

    //El cero indica que solamente un solo protocolo estará usando este socket
    fileDescriptorListenerSocket = socket(AF_INET, SOCK_STREAM, 0);
    reportErrorIfNecessary(fileDescriptorListenerSocket, "socket");

    serverSocketAddress.sin_family = AF_INET;
    serverSocketAddress.sin_addr.s_addr = INADDR_ANY;
    serverSocketAddress.sin_port = htons(PORT);
    bzero(&(serverSocketAddress.sin_zero), sizeof(unsigned char) * 8);

    //Se podria haber usado
    //memset(&(serverSocketAddress.sin_zero), 0, sizeof(unsigned char) * 8); 
    //Con memset llenamos un espacio de memoria con una constante

    returnedInteger = bind( fileDescriptorListenerSocket, (struct sockaddr *) &(serverSocketAddress), sizeof(struct sockaddr) );
    reportErrorIfNecessary(returnedInteger, "bind");
    
    returnedInteger = listen( fileDescriptorListenerSocket, BACKLOG); //Ponemos en modo pasivo al servidor
    reportErrorIfNecessary(returnedInteger, "listen");

    //El procedimiento HASTA esta linea es igual a la de un servidor de concurrencia real. Aca arrancan las diferencias.
    //Configuramos los sets.

    FD_SET(fileDescriptorListenerSocket, &fileDescriptorsSet);
    FD_SET(fileDescriptorListenerSocket, &readFileDescriptorsSet);
    maxFileDescriptor = fileDescriptorListenerSocket;

    socketAddressSize = sizeof(struct sockaddr_in);

    while(1){
        readFileDescriptorsSet = fileDescriptorsSet;

        returnedInteger = select(maxFileDescriptor+1, &readFileDescriptorsSet, NULL, NULL, NULL);
        reportErrorIfNecessary(returnedInteger,"select");

        for( fileDescriptorIndex = 0 ; fileDescriptorIndex <= maxFileDescriptor; fileDescriptorIndex++ ){

            if(FD_ISSET(fileDescriptorIndex, &readFileDescriptorsSet)){

                //En el caso que sea el socket que esta en 'listening', aceptamos la nueva conexión
                if(fileDescriptorIndex == fileDescriptorListenerSocket){
                    fileDescriptorNewClientSocket = accept( fileDescriptorListenerSocket, (struct sockaddr * ) &newClientSocketAddress, &socketAddressSize ); //Aceptamos una conexión con el socket
                    reportErrorIfNecessary(fileDescriptorNewClientSocket, "accept");
                    clientConnectionOrder++;

                    printf("Server: I've got connection from: %s. It's the #%d client with %d socket.\n", inet_ntoa(newClientSocketAddress.sin_addr), clientConnectionOrder, fileDescriptorNewClientSocket);

                    FD_SET(fileDescriptorNewClientSocket, &fileDescriptorsSet);

                    if (fileDescriptorNewClientSocket > fileDescriptorListenerSocket)
                        maxFileDescriptor = fileDescriptorNewClientSocket;
                }

                //Un cliente manda datos. Nos fijamos si nos manda un mensaje o simplemente quiere desconexión
                else{
                    quantityOfBytesReceived = recv(fileDescriptorIndex, messageReceive, MAXDATASIZE, 0);
                    reportErrorIfNecessary(quantityOfBytesReceived, "recv");

                    //Se realiza desconexión
                    if( quantityOfBytesReceived <= 0 ){
                        printf("Client assigned to socket #%d closed connection\n", fileDescriptorIndex);
                        returnedInteger = close(fileDescriptorIndex);
                        reportErrorIfNecessary(returnedInteger,"close");
					    FD_CLR(fileDescriptorIndex, &fileDescriptorsSet);
                    }
                    
                    //Se obtiene datos y se reenvia a todos los clientes
                    else{
                        printf("Message Received from client with socket #%d: '%s'. Resending to all clients...\n", fileDescriptorIndex, messageReceive);
                        for( auxiliarFileDescriptorIndex = 0 ; auxiliarFileDescriptorIndex <= maxFileDescriptor; auxiliarFileDescriptorIndex++ ){

                            if ( FD_ISSET(auxiliarFileDescriptorIndex, &fileDescriptorsSet) && auxiliarFileDescriptorIndex != fileDescriptorIndex && auxiliarFileDescriptorIndex != fileDescriptorListenerSocket){
                                sleep(1);
                                send(auxiliarFileDescriptorIndex, messageReceive, strlen(messageReceive) * sizeof(char), 0);
                            }

                        }
                    }
                
                }

            }

        }



    }





    return 0;
}