//-----------BIBLIOTECAS QUE UTILIZARE PARA EL PROGRAMA CLIENTE-----------------
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#define serverdireccion "127.0.0.1"//DIRECCION DEL SERVIDOR AL QUE SE CONECTA EL CLIENTE
#define serverpuerto 2021//PUERTO DEL SERVIDOR AL QUE SE CONECTA EL CLIENTE
//-------------------------------------------------------------------------------
//---------------------------FUNCION MAIN() DONDE SE REALIZARA TODO EL PROGRAMA------------------------------
//        LA FUNCION MAIN() RECIBE POR PARAMETROS LO SIGUIENTE
// ./CLIENT ParcialTL2020 <EB> <tipo de solicitud> <EB> <nombre de usuario> <fin de filadelArchivo>
void main(int argc,char *argv[])
{
    int socketDescriptorArchivo;//VARIABLE QUE UTILIZARE PARA EL SOCKET QUE CREARE MAS ADELANTE
    int recibo;//VARIABLE QUE LE ASIGNARE A LA SOCKETCALL RECV() DURANTE LA EJECUCION DEL PROGRAMA
    struct sockaddr_in servidor,cliente;//VARIABLES CON LA ESTRUCTURA DE UN SOCKET
    struct hostent *he;
    char direccion[16];
    unsigned int puerto;//PUERTO CLIENTE
    char puertoo[6];//PUERTO CLIENTE EN CHAR LO USARE PARA CONVERTIR EL PUERTO
    char mensajerecibido[96]="";//ES DONDE ALMACENARE EL MENSAJE QUE LLEGA UNA VEZ HACE UN RECV(SOCKET,MENSAJERECIBIDO,SIZEOF(MENSAJERECIBIDO),0)
    char mensaje[200]="";//ES DONDE ALMACENARE EL MENSAJE QUE ESCRIBE POR CONSOLA EL CLIENTE
    int val=0;//ES UN CONTADOR QUE UTILIZARE PARA LA PRIMERA VEZ QUE EL CLIENTE REALICE LA SOLICITUD

    if(argc!=4)//POR COMANDO COMO DIJE ANTES RECIBE 4 VALORES SI ESTO NO OCURRE SALE DEL PROGRAMA
    {
        printf("ParcialTL2020 <EB> <tipo de solicitud> <EB> <nombre de usuario> <fin de filadelArchivo>");
        exit(1);//PERMITE QUE TERMINE EL PROGRAMA DEBIDO A QUE HUBO UN ERROR
    }
    //USO LA SOCKETCALL SOCKET(INT DOMINIO,INT TIPO,INT PROTOCOLO) PARA CREAR EL SOCKET Y LO IGUALAMOS A SOCKETDESCRIPTORARCHIVO
    if((socketDescriptorArchivo=socket(AF_INET,SOCK_STREAM,0))==-1)//DOMINIO AF_INET FAMILI DE DIRECCIONES, TIPO SOCK_STREAM YA QUE ESTAMOS USANDO TCP,PROTOCOLO 0 COLOCA POR DEFAULT QUE ES PROTOCOLO TCP
    {                                                                                         
        perror("ERROR");
        exit(1);//PERMITE QUE TERMINE EL PROGRAMA DEBIDO A QUE HUBO UN ERROR
    }
    //-----------------CARGO LA INFORMACION DEL SERVER
    servidor.sin_family=AF_INET;//ASIGNO AF_NET FAMILIA DE DIRECCIONES
    servidor.sin_port = htons(serverpuerto);//ASIGNO EL PUERTO PERO USAMOS HTONS() PARA CONVERTIRLO A LENGUAJE DE RED
    servidor.sin_addr.s_addr=inet_addr(serverdireccion);//ASIGNO LA DIRECCION PERO USAMOS INET_ADDR PARA CONVERTIRLO A LENGUAJE DE RED
    bzero(&(servidor.sin_zero),8);//ASIGNO ESTO PARA MANTENER EL TAMANIO DE LA ESTRUCTURA SOCKADDR_IN
    struct timeval posiciondeTiempFinal,posiciondeTiempoInicial;//COLOCA ESTO PARA PODER USAR UNA ESTRUCTURA DE TIEMPO QUE TIENE SEG Y MSEG
    gettimeofday(&posiciondeTiempoInicial,0);//INICIO EL TIEMPO DE LA ESTRUCTURA TIMEVAL CREADA EN LA LINEA ANTERIOR
    //USO LA SOCKET CALL CONNECT() PARA INICIAR LA CONEXION CON EL SERVER SI ES QUE ESTA ABIERTA, LE MANDO UN DESCRIPTORDESOCKET,DIRECCION DEL SOCKET REMOTO,TAMANIO DE LA DIRECCION
    //CUANDO EL SERVIDOR NO ESTA EN PASIVO, TIRARA UN ERROR ACA
    //EN EL SERVIDOR HAY UN BIND PERO ACA COMO ESTAMOS EN EL CLIENTE USAMOS UN CONNECT
    if(connect(socketDescriptorArchivo,(struct sockaddr *)&servidor,sizeof(struct sockaddr))==-1)
    {
        perror("ERROR correct");
        exit(1);//PERMITE QUE TERMINE EL PROGRAMA DEBIDO A QUE HUBO UN ERROR
    }
    gettimeofday(&posiciondeTiempFinal,0);//INICIO EL TIEMPO DE LA ESTRUCTURA TIMEVAL CREADA CON ESTE Y EL ANTERIOR VOY A PODER MEDIR EL TIEMPO TOTAL DE CONEXION DEL CLIENTE
    bzero(&(cliente),sizeof(cliente));//ASIGNO ESTO PARA MANTENER EL TAMANIO DE LA ESTRUCTURA SOCKADDR_IN
    int taminiocliente= sizeof(cliente);//OBTENGO EL TAMANIO DE LA ESTRUCTURA SOCKADDR_IN
    getsockname(socketDescriptorArchivo,(struct sockaddr *)&cliente,&taminiocliente);//ASIGNO PARA OBTENER LA DIRECCION DEL CLIENTE
    inet_ntop(AF_INET,&cliente.sin_addr,direccion,sizeof(direccion));//REVISAR SI ESTA BIEN LA FAMILY AF_INET TAMBIEN SEPUEDE USAR PARA COLOCAR LA DIRECCION DEL CLIENTE
    printf("local direccion:%s\n",inet_ntop(AF_INET,&cliente.sin_addr,direccion,sizeof(direccion)));
    puerto = ntohs(cliente.sin_port);//ASIGNO PARA CAMBIAR DE LENGUAJE RED A LENGUAJE HOST
    printf("local direccion: %s\n",direccion);
    printf("local puerto: %u\n",puerto);
    sprintf(puertoo,"%u",puerto);//PASO EL UNSIGNED INT A CHAR
    //------------inicio
    printf("%s\n",argv[1]);
    printf("%s\n",argv[2]);
    printf("%s\n",argv[3]);
    printf("filadelArchivo:\n");
    strcat(mensaje,argv[1]);//UTILIZO PARA CONCATENAR TODO EL MENSAJE QUE LE MANDARE AL INICIO AL SERVER
    strcat(mensaje," ");
    strcat(mensaje,argv[2]);
    strcat(mensaje," ");
    strcat(mensaje,argv[3]);
    strcat(mensaje," ");
    strcat(mensaje,direccion);
    strcat(mensaje,"-");
    strcat(mensaje,puertoo);
    strcat(mensaje," ");
    strcat(mensaje,"0ms");
    strcat(mensaje," ");
    printf("%s\n",mensaje);//MENSAJE QUE MANDARE AL SERVER AL INICIO DEL PROGRAMA
    //SOCKETCALL QUE UTILIZO PARA ENVIAR SEND() LE MANDO EL DESCRIPTORDESOCKET , EL MENSAJE , EL TAMANIO DEL MENSAJE, Y FLAGS EN 0 PORQUE NO USE
    if(send(socketDescriptorArchivo,mensaje,sizeof(mensaje),0)==-1){
        perror("ERROR send");//EN CASO DE ERROR ENTRA A ESTE IF
        exit(1);
    }    
    else{
    printf("enviado\n");//ACA SEGNIFICA QUE SE PUDO ENVIAR AL SERVER
    }
    val=0;
    while(1){
        
        if(val==1){
        //LUEGO DE LA SOLICITUD PARA INGRESAR ENTRAMOS ACA Y ESCRIBE POR TECLADO EL CLIENTE
            printf("ingrese mensaje para el server:");
            fgets(mensaje,96,stdin);//EL CLIENTE ESCRIBE POR CONSOLA Y ESO CUANDO DA UN \r SE ALMACENA EN MENSAJE
            if(send(socketDescriptorArchivo,mensaje,sizeof(mensaje),0)==-1){
                perror("ERROR send");
                exit(1);
            }    
            else 
                printf("enviado\n");
        }
        if(val==0){
            //SOCKETCALL QUE UTILIZO PARA RECIBIR RECV() LE MANDO EL DESCRIPTORDESOCKET, EL MENSAJERECIBIDO, EL TAMANIO DEL MENSAJERECIBIDO, Y FLAGS EN 0 PORQUE NO USE
            if((recibo=recv(socketDescriptorArchivo,mensajerecibido,96,0))==-1){
                perror("ERROR recv");
                exit(1);//SI HUBO UN ERROR SALE DEL PROGRAMA
            }
            mensajerecibido[recibo]='\0';
            printf("recibido: %s",mensajerecibido);
            val=1;
        }
        if((strcmp(mensaje,"msj-log\n"))==0){//VERIFICA QUE EL COMANDO VALIDO SEA LOG Y REALIZA CIERTA ACCION
            int findearchivo=1;
            printf("contenido del archivo.txt recibido por recv \n");
            while(findearchivo){
                if((recibo=recv(socketDescriptorArchivo,mensajerecibido,96,0))==-1){
                    perror("ERROR recv");
                    exit(1);
                }
                if(strcmp(mensajerecibido,"FIN")==0){
                    findearchivo=0;
                }else{
                    printf("recibido: %s",mensajerecibido);
                }
            }
        }
        if((strcmp(mensaje,"msj-fin\n"))==0){//VERIFICA QUE EL COMANDO VALIDO SEA FIN Y REALIZA CIERTA ACCION
            long long restaParaEncontrarElTiempoDeConexion=(posiciondeTiempFinal.tv_sec-posiciondeTiempoInicial.tv_sec)*1000000+posiciondeTiempFinal.tv_usec-posiciondeTiempoInicial.tv_usec;
            printf("Tiempo de conexion: %lldms\n",restaParaEncontrarElTiempoDeConexion);//ME DA EL TIEMPO DE CONEXION TOTAL DEL CLIENTE
            exit(1);
        }       
    }
    close(socketDescriptorArchivo);//CIERRO EL SOCKET AUNQUE EL SERVIDOR YA LO HACE CON SHUTDOWN DEL OTRO LADO
}