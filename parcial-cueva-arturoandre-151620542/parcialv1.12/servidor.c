//-----------BIBLIOTECAS QUE UTILIZARE PARA EL PROGRAMA SERVIDOR-----------------------
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
//--------------------------------------------------------------------------------------
#define puerto 2021//PUERTO DEL SERVIDOR
#define filasdeconexiones 3 //CANTIDAD DE CONEXIONES EN LA COLA OSEA SERIA EL LARGO QUE PUEDE TENER LA COLA DE ESPERA
int main(){
    char nombreArchivo[30]="";//VARAIBLE QUE UTILIZAR PARA ALMACENAR EL NOMBRE.TXT DEL ARCHIVO DONDE SE GUARDARA CADA MENSAJE QUE ENVIA EL CLIENTE A ESTE SERVER
    char mensajerecibido[96]="";//VARIABLE QUE UTILIZARE PARA ALMACENAR LO RECIBIDO DEL CLIENTE  MEDIANTE AL SOCKETCALL RECV
    char mensajeAEnviar[96]="";//VARIABLE QUE UTILIZARE PARA ALMCACENAR LO QUE ENVIARE AL CLIENTE CON LA SOCKETCALL SEND
    int socketDescriptorArchivo,newSocketDescriptorArchivo;//VARIABLE QUE UTILIZARE PARA EL SOCKET QUE CREARE MAS ADELANTE
    struct sockaddr_in servidor;//VARIABLES CON LA ESTRUCTURA DE UN SOCKET
    struct sockaddr_in cliente;//VARIABLES CON LA ESTRUCTURA DE UN SOCKET
    socklen_t tamanio;
    int recibo;
    int val=0;
    int valorWhile=1;
    if((socketDescriptorArchivo=socket(AF_INET,SOCK_STREAM,0))==-1){
        perror("ERROR socket");
        exit(1);
    }
    servidor.sin_family=AF_INET;
    servidor.sin_port=htons(puerto);
    servidor.sin_addr.s_addr=inet_addr("127.0.0.1");
    bzero(&(servidor.sin_zero),8);
    //USO BIND PARA VINCULAR EL DESCRIPTORDESOCKET CON LA ESTRUCTURA SOCKADDR Y TAMBIEN LE PASO EL TAMANIO DE LA ESTRUCTURA
    if(bind(socketDescriptorArchivo,(struct sockaddr *)&servidor,sizeof(struct sockaddr))==-1){
        perror("ERROR bind");
        exit(1);
    }
    //USO LISTEN ESTAS EN MODO PASIVO O SEA SI SE VINCULO SE PONE EN MODO PASIVO,A ESCUCHAR CONEXIONES
    //LISTEN RECIBE EL DESCRIPTORDESOCKET Y FILADECONEXIONES, ESTO ES PORQUE PUEDEN CAERLE VARIAS CONEXIONES A LA VEZ
    //DEBIDO A ESTAR EN MODO PASIVO, SI ALGUIEN REVISA LOS PUERTOS DE LA MAQUINA LO VE ABIERTO ESE PUERTO Y EL CLIENTE YA SE VA A PODER CONECTAR
    if(listen(socketDescriptorArchivo,filasdeconexiones)==-1){
        perror("ERROR listen");
        exit(1);
    }
    //VA A MANTENER EL SERVIDOR ABIERTO
    while(1){
        tamanio=sizeof(struct sockaddr_in);
        struct timeval inicio,fin;
        //ACCEPT TIENE EL DESCRIPTORDESOCKET, ESTRCUTURA SOCKADDR Y EL TAMANIO DE LA ESTRUCTURA SOCKADDR
        //ACCEPT ES EL QUE ACEPTA LA CONEXION DE DONDE ESTE ENTRANDO
        //USAMOS UN NUEVO SOCKET PORQUE QUEREMOS QUE EL SOCKETDESCRIPTORARCHIVO ESTE EN MODOPASIVO
        if((newSocketDescriptorArchivo=accept(socketDescriptorArchivo,(struct sockaddr *)&cliente,&tamanio))==-1){
            perror("ERROR accept");exit(1);
        }
        //ACA UN RECV CON EL TIMER QUELLEGA DE CLIENTE
        gettimeofday(&inicio,0);
        if((recibo=recv(newSocketDescriptorArchivo,mensajerecibido,sizeof(mensajerecibido),0))==-1){
            perror("ERROR recv");exit(1);
        }else{//ESTO LO HACE SOLO UNA VEZ QUE SE CONECTA EL CLIENTE A ESTE SERVER
            gettimeofday(&fin,0);
            mensajerecibido[recibo]='\0';}
            printf("%s\n",mensajerecibido);
            int espacio=0;
            char parcial[20]="";
            char solicitud[10]="";
            char usuario[20]="";
            char extremo[20]="";
            char tiv[14]="";
            int k=0;
            for(int i=0;mensajerecibido[i]!='\0';i++){
                if(mensajerecibido[i]==' ')
                    espacio++;
                if(espacio==0){
                    strncat(parcial,&mensajerecibido[i],1);
                }
                if(espacio==1){
                    strncat(solicitud,&mensajerecibido[i],1);
                }
                if(espacio==2){
                    if(k==0){
                        i++;k=1;
                    }
                    strncat(usuario,&mensajerecibido[i],1);
                }
                if(espacio==3){
                    strncat(extremo,&mensajerecibido[i],1);
                }
                if(espacio==4){
                    strncat(tiv,&mensajerecibido[i],1);
                }
            }
            if(espacio==5){
                valorWhile=1;
                printf("%s\n",parcial);
                printf("%s\n",solicitud);
                printf("%s\n",usuario);
                printf("%s\n",extremo);
                printf("%s\n",tiv);
                val=0;
                strcat(mensajeAEnviar,"hola ");
                strcat(mensajeAEnviar,usuario);//CON ESTA FUNCION CONCATENO EN MENSAJE QUE ENVIARE
                strcat( mensajeAEnviar," ");
                strcat(mensajeAEnviar,extremo);
                strcat(mensajeAEnviar,".Bienvenido al servidor de mensajes del ");
                strcat(mensajeAEnviar,parcial);
                strcat(mensajeAEnviar,"\r\n");
                strcat(nombreArchivo,usuario);
                strcat(nombreArchivo,".txt");
                printf(">%s\n",nombreArchivo);
                long long diferencia=(fin.tv_sec-inicio.tv_sec)*1000+(fin.tv_usec-inicio.tv_usec)/1000.0;//ME DARA EL TIEMPO MS QUE TARDA EL ENVIADO
                printf("Tiempo de recibir mensaje: %lldms\n",diferencia);
                //ENVIO POR UNICA VEZ EL MENSAJE DE BIENVENIDA AL CLIENTE QUE SE CONECTO A ESTE SERVIDOR
                if(send(newSocketDescriptorArchivo,mensajeAEnviar,sizeof(mensajeAEnviar),0)==-1){
                    perror("send");
                    exit(1);
                }
            }else{
                printf("ERROR en solicitud");
            }
        while(valorWhile){
            if((recibo=recv(newSocketDescriptorArchivo,mensajerecibido,sizeof(mensajerecibido),0))==-1){
                perror("ERROR recv");
                exit(1);
            }else{
                mensajerecibido[recibo]='\0';
                char msj[5]="";
                char segundaparte[91]="";
                int cont=0;
                int e=0;
                for(e=0;mensajerecibido[e]!='\n';e++){
                    if(mensajerecibido[e]=='-'){
                        cont=1;
                        strcat(msj,"-");
                    }
                    if(cont==0){
                        strncat(msj,&mensajerecibido[e],1);
                    }
                    if(cont==1){
                         if(mensajerecibido[e]!='-')
                             strncat(segundaparte,&mensajerecibido[e],1);
                    }
                }
                if((strcmp(msj,"msj-"))==0){
                    if((strcmp(segundaparte,"log"))==0){
                        printf("> %s\n",nombreArchivo);
                        printf("<<<<log>>>>>>>\n");
                        char hola[20]="";
                        strcat(hola,usuario);
                        strcat(hola,".txt");
                        FILE* archivito;//PUNTERO AL ARCHIVO
                        char filadelArchivo[96]="";
                        archivito=fopen(hola,"r");//ABRO EL ARCHIVO EN MODO LECTURA
                        printf("----contenido del archivo.txt que enviaremos  linea por linea usando send al cliente----\n");//mandadole el nombre del archivo
                        if(archivito){
                        // Lo que quieras hacer con el archivo
                        }else{
                            printf("Error al abrir el archivo");
                            exit(1);
                        }
                        while(fgets(filadelArchivo,96,archivito)!=NULL){
                            printf("linea:%s\n",filadelArchivo);
                            if(send(newSocketDescriptorArchivo,filadelArchivo,sizeof(filadelArchivo),0)==-1){
                                perror("send");
                                exit(1);
                            }
                        }
                        if(send(newSocketDescriptorArchivo,"FIN",sizeof("FIN"),0)==-1){
                            perror("send");
                            exit(1);
                        }
                        fclose(archivito);//CIERRO EL ARCHIVITO.TXT
                    }
                    else if(strcmp(segundaparte,"fin")==0){//MENSAJE QUE MOSTRARA AL DESCONECTARSE EL CLIENTE Y ESPERARA EL PROXIMO CLIENTE ESTE SERVIDOR
                        printf("Conexion con %s del%s finalizada\n",usuario,extremo);
                        shutdown(newSocketDescriptorArchivo,1);//CIERRO EL SOCKET DESDE EL LADO DEL SERVIDOR
                        //CIERRO SOBRE EL QUE ESTOY PARADO Y LAS DEMAS CONEXIONES SIGUEN ESTANDO
                        valorWhile=0;
                        strcpy(nombreArchivo,"");
                    }
                    else{//agregar a arhcivCueva.txt
                        FILE *desArchivo;
                        printf("archivo: %s\n",nombreArchivo);
                        if ((desArchivo=fopen(nombreArchivo, "a")) == NULL){}//ABRE EL ARCHIVO EN MODO APPEND OSEA SI NO EXISTE LO CREA Y EN CASO QUE EXISTA SIGUE CONCATENANDO LO ENVIA EL CLIENTE
                        else{    
                            printf("%s\n",segundaparte);
                            strcat(segundaparte,"\n");
                            fputs(segundaparte,desArchivo);
                            fclose(desArchivo);//CIERRO EL ARCHIVO.TXT
                        }
                    }
                }
            }
        }//while
    }//while
}