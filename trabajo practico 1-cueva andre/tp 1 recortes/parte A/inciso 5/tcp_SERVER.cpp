#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <termios.h>

#define MYPORT 3490  // Puerto al cual nos conectaremos (puerto del servidor)

#define BACKLOG 5 // Cantidad de conexiones en la cola.El LARGO QUE PUEDE TENER LA COLA DE ESPERA
#define MAXDATASIZE 200

//------------------------SHAREMEMORY----------------------------------------
typedef char shm_message[200]; //tamano de cada mensaje
struct shm_segment
{
   int first, last, count;
   shm_message messages[1024];//hay espacio hasta 1024 mensajes
};
int segment_id;
struct shm_segment *shared_memory;

int flag = 1;
void Free_SHM (int signal_number)//permite liberar la sharememory
{
//  shmdt (shared_memory);
  /* Deallocate the shared memory segment.  */
  shmctl (segment_id, IPC_RMID, 0);
  flag = 0;
}
//-----------------------------------------------------------------------------
int main()
{      //                ---definimos variables---
printf("-----------servidor---------------");
 int fds;
 pid_t pid;
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
//-----------------sharememory--------------------------------
// Registrar funcion que limpie SHM segment si Ctrl+C--------------
  struct sigaction sa;//que si le llegue una accion del sistema
  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = &Free_SHM;
  sigaction (SIGINT, &sa, NULL);
//-----------------------------------------------------------------
  struct shmid_ds shmbuffer;
  int segment_size;
  const int shared_segment_size = 0x3200C;
  key_t lv_key;//llave, es un identificador que tiene que tiene que ser igual de los dos lados

  lv_key = ftok("producer.cpp",'B');//toma el nombre y genera la llave
  /* Allocate a shared memory segment.  */
  segment_id = shmget (lv_key, shared_segment_size,//variable que usamos para leer y escribir el mensaje dentro del segmento
		       IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  //IPC_CREAT crea el segmento
  //IPC_EXCL asegura que el segmento se cree
  //S_IRUSR lectura
  //S_IWUSR escritura  
  /* Attach the shared memory segment.  */
  shared_memory = (struct shm_segment*) shmat (segment_id, 0, 0);//coloca el mensaje dentro del segmento
  printf ("shared memory attached at address %p\n", shared_memory);//imprime a direccion de memoria compatida

  shared_memory->first = 0;
  shared_memory->count = 0;
  shared_memory->last = 1;

//------------------------------------------------------------




// --- CICLO QUE MANTIENE EL SERVIDOR ABIERTO HASTA QUE aprete CTRL+C ---
 
 while(1)//
 {
  sin_size = sizeof(struct sockaddr_in);
  //_________________________ACCEPT____________________________________________________________
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
  //______________________________________________________________________________________________
  printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));
 
  //si NO hubo ERROR, mostramos la direccion IP, se guarda en sin_addr
  
if(!fork())//fork GENERA Un NUEVO proceso con una copia de las variables del original
{
  while(1)// LO QUE HACE ES MANDARLO MUCHAS VECES EL MENSAJE
  {
    while(flag){
      if(shared_memory->count <200)
      {
         if ((numbytes=recv(new_fd, buf, MAXDATASIZE, 0)) == -1)//va a estar bloqued hasta que el otro lado lo lea
        {
          perror("recv");
          exit(1);
        }
        buf[numbytes] = '\0';
        //-----------------------sharememory---------------------
        shared_memory->first = ( shared_memory->first + 1 ) % 200;//hace el movimiento necesario para pasar al otro mensaje [0]->[201]->[401]
        int lv_aux = shared_memory->first;//ahora que se movio al siguiente mensaje lo guardo en un aux.
        strcpy(shared_memory->messages[lv_aux],buf);
        shared_memory->count++;
        //----------------------sharememory----------------------
        printf("Received: %s\n", buf);//si la respuesta es mas larga de lo posible la CORTA
        int e;
        for(e=0;buf[e]!='\0';e++);
        printf("\nbytes recibidos %d\n\n",e);
        //--------------------------sharememory------------------------
        shared_memory->first = ( shared_memory->first + 1 ) % 200;//hace el movimiento necesario para pasar al otro mensaje [0]->[201]->[401]
        int lv_aux2 = shared_memory->first;
        printf("ingrese texto:");
        fgets(shared_memory->messages[lv_aux2], 200, stdin);//toma mensaje por entrada de teclado
        int i=0;
        for(i=0;shared_memory->messages[lv_aux2][i]!='\0';i++);
        printf("\nbytes Enviados %d\n\n",i);
        shared_memory->count++;//mensaje agregado entonces suma +1
        //------------------------sharememory-----------------------
        if (send(new_fd,shared_memory->messages[lv_aux2],i, 0) == -1) perror("send");
        else printf("Enviado\n");
      }
    }
	
  } //ES DEL WHILE
}
else close(new_fd);//CIERRA EL SOCKET, se puede cerrar porq cierra por puerto, si tengo 5 conexiones
                     //cierra la q este parado y las demas siguen estando.

  while(waitpid(-1, NULL, WNOHANG) > 0);//es para cerrar bien TODO,cuando alguien hace CTRL+c
}
 return 0;
}

