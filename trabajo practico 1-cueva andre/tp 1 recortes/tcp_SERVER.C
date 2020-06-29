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

#define MYPORT 3490  // Puerto al cual nos conectaremos (puerto del servidor)

#define BACKLOG 5 // Cantidad de conexiones en la cola.El LARGO QUE PUEDE TENER LA COLA DE ESPERA
#define MAXDATASIZE 100

main()
{      //                ---definimos variables---
 //nota: sockfd [file descriptor] a nivel desarrollo, en el que vamos a leer y
 //      escribir
 int sockfd, new_fd; // escuchamos en sockfd, nueva coneccion en new_fd
 //hay dos sockets porque sockfd queda escuchando, cuando se hace
//fork y se crea la conexion mediante new_fd que hace una copia del socket
//-----------------estas struct guardan la informacion del socket------------
 struct sockaddr_in my_addr; //--toda la informacion que tiene que ver con el 
                             //servidor---
 struct sockaddr_in their_addr;//--toda la informacion que tiene que ver con el 
                               //cliente---
//---------------------------------------------------------------------------
 socklen_t sin_size;
 int numbytes;
 char buf[MAXDATASIZE];//es el espacio donde vamos a transferir datos

//--el siguiente IF crea el socket, el IF verifica si tira error--
 if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)//si da -1 es que hubo ERROR
	//sockfd llama a la funcion socket(dominio,tipo,protocolo)
	//dominio:   AF_INET es la familia de protocolos tcp/ip
	//tipo:
	//    tcp -> socket_STREAM (ya arma la cabecera deacuerdo a la comunica. que vas a tener
	//    udp -> socket_DGRAM  (ya arma la cabecera deacuerdo a la...)    
    //    OBLIGA QUE el desarrollador programe la cabeza del paquet-> socket_RAW
	//protocolo: 0 porque normalmente solo existe un protocolo
 {
  perror("socket");//printERROR 
  exit(1);
 }
//--Cargamos la informacion del servidor--
//  NOTA  *el servidor debe tener un puerto definido para que los clientes se pueden conectar
//  NOTA  *clientes se van a conectar con un puerto destino CONOCIDO,pero un puerto ORIGEN asignado
//         por el SISTEMA OPERATIVO
 my_addr.sin_family = AF_INET; //nota: estos tres my_addr.. ya entienden la comunicacion no son enteros unicamente.
 my_addr.sin_port   = htons(MYPORT);//MYPORT es el puerto que se define en el servidor
                                    //htons convierte el FORMATOELS.O. al que necesita TCP
									//y a htons le paso el puerto porque lo que navega por TCP es PUERTO y IP. 
 my_addr.sin_addr.s_addr = INADDR_ANY;
 //----------------------------------------
 bzero(&(my_addr.sin_zero), 8);//funcion para rellenar con ceros->poruqe lo que queda de la estructura
                               //hay que llenarlo con ceros. y SON 8 CEROS que se colocan.
//---------------------
 if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
//bind: vincula el socketfd con la estructura my_addr(my_addr.sin_family,etc) pero
//      tambien sizeof()porque debes mandarle la longitud de la estructura.
 {
  perror("bind");
  exit(1);
 }

 if (listen(sockfd, BACKLOG) == - 1)//ya con listen estas en MODOPASIVO
//si se vinculo todo, hay que poner el servidor en MODO PASIVO, a escuchar (listen) conexiones.
// Listen recibe:
//       .sockfd
//       .BACKLOG tambien recibe un tamano de cola porq pueden caer muchas conexiones a la vez.
//       es concurrente este caso, no tiene problemas en atender muchas consultas.Pero el comando
//       que divide los procesos se puede formar una cola.
//NOTA: En MODOPASIVO, si alguien revisa los puertos de la maquina lo ve abierto ese puerto y el cliente
//      ya se va a poder conectar. 
 {
  perror("listen");
  exit(1);
 }
// --- CICLO QUE MANTIENE EL SERVIDOR ABIERTO HASTA QUE aprete CTRL+C ---
 while(1)//
 {
  sin_size = sizeof(struct sockaddr_in);
  if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1)
//nota: cada vez que se acepta una conexion, el new_fd padre, se va sobreescribiendo entonces trabajo 
//      con la copia dada por fork, si hago close solo cierro esa conexion
//accept -> es el que acepta la CONEXION de donde esta entrando
//podriamos poner sockfd =accept(sockfd,..) ESO EN udp , peroestamos usando TCP ENTONCES
//USAMOS UN nuevo socket -> new_fd , porq queremos que sockfd quede en MODOPASIVO
//accetp pide:
//        .sockfd
//        .estructura vacia (struct sockaddr *)&their_addr  ->donde se va a cargar la info. del CLIENTE
  {
   perror("accept");
   continue;
  }
  printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));
  //si NO hubo ERROR, mostramos la direccion IP, se guarda en sin_addr

  if(!fork())//fork GENERA Un NUEVO proceso con una copia de las variables del original
  {
   //_________peilou,  lo q hace el sistema con la conexion que acaba de entrar__________________________
                  
//   while(1) LO QUE HACE ES MANDARLO MUCHAS VECES EL MENSAJE
 //  {
   if (send(new_fd, "Hello, world!\n", 14, 0) == -1) perror("send");
   //Entonces le send ->envia el mensaje "hello world" y en pantalla imprime "Enviado"
   //send devuelve la cantidad de bits enviados o -1 si hubo un error
   //send pide:
   //  .el socket - > new_fd
   //  .el texto o un string que traes de afuera
   //  .la longitud
   //  .0 que seimpre va a estar  flags (mayormente no los usamos mucho)_
   else printf("Enviado\n");
                       //------espere el recibir algo-----
   if ((numbytes=recv(new_fd, buf, MAXDATASIZE, 0)) == -1)//va a estar bloqued hasta que el otro lado lo lea
	   //recv devuelve la cantidad de bits recibidos
	   //recv pide:
	   //			.recibe de new_f
	   //			.lo guarda en buf
	   //			.lee hasta MAXDATASIZE como maximo porque es su limite.
   {
    perror("recv");
    exit(1);
   }

   buf[numbytes] = '\0';//le agrega el \0 para poder imprimirlo
                      //--luego imprime lo recibido----
   printf("Received: %s\n", buf);//si la respuesta es mas larga de lo posible la CORTA
 // } ES DEL WHILE
  }
  //_________________________________________________________
  else close(new_fd);//CIERRA EL SOCKET, se puede cerrar porq cierra por puerto, si tengo 5 conexiones
                     //cierra la q este parado y las demas siguen estando.

  while(waitpid(-1, NULL, WNOHANG) > 0);//es para cerrar bien TODO,cuando alguien hace CTRL+c
 }
}

