//raw no es tcp ni udp, la cabeza esta en control del desarrollador
//wireshark, programa con biblioteca gignte de protocolos. Lo que hace
//es traducir cadenas de0 y 1 del encabezado, en information ordenada
//de los paquetes.
//VAMOS a capturar ceros y unos puestos por algun mas, y lo vas 
//ver y mostrar.
#define __USE_BSD	
#include <sys/socket.h>	
#include <netinet/in.h>	
#include <netinet/ip.h>
#include <arpa/inet.h>
#define __FAVOR_BSD	
#include <netinet/tcp.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
struct tcp_options
{
  u_int8_t op0;
  u_int8_t op1;
  u_int8_t op2;
  u_int8_t op3;
  u_int8_t op4;
  u_int8_t op5;
  u_int8_t op6;
  u_int8_t op7;
};
char datagram[4096]; /* datagram buffer */
char pheader[1024]; /* pseudoheader buffer for computing tcp checksum */
uint16_t  csum (uint16_t * addr, int len)//checksum
{
  int nleft = len;
  uint32_t sum = 0;
  uint16_t *w = addr;
  uint16_t answer = 0;
  while( nleft > 1 ) {
    sum += *w++;
    nleft -= 2;
  }
  if (nleft == 1) {
    *(unsigned char *)  (&answer) = *(unsigned char *) w;
    sum += answer;
  }
  sum = (sum >> 16)+(sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}
int main(int argc, char **argv)
{

  int flags = 0, c, numtries=90;
  char src_ip[17];
  char dst_ip[17];
  short dst_port=80;
  short th_sport=1234;
  short tcp_flags=TH_SYN;
  short pig_ack=0;
  struct ip *iph = (struct ip *) datagram;
  struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct ip));
  struct tcp_options *tcpopt = (struct tcp_options *) (datagram + sizeof(struct ip) + sizeof(struct tcphdr));
  struct sockaddr_in servaddr;
  memset(datagram, 0, 4096); /* zero out the buffer */
  fprintf(stderr,"sizeof (struct ip)= %d\n",sizeof(struct ip));
  snprintf(src_ip,16,"%s","192.168.8.12");  //default
  while ((c = getopt(argc, argv, "s:d:n:p:f:a:q:w:x:y:z:g:h:i:j:")) != -1) {
    switch (c) {
    case 's':
      flags |= 0x1;
      snprintf(src_ip,16,"%s",optarg);
      break;
    case 'd':
      flags |= 0x2;
      snprintf(dst_ip,16,"%s",optarg);
      break;
    case 'n':
      flags |= 0x4;
      numtries=atoi(optarg);
      break;
    case 'p':
      flags |= 0x8;
      dst_port=atoi(optarg);
      break;
    case 'f':
      tcp_flags=atoi(optarg);
      break;
    case 'a':
      pig_ack=atoi(optarg);
      break;
    case 'q':
      th_sport=atoi(optarg);
      break;
    case 'w':
      tcpopt->op0=atoi(optarg);
      break;
    case 'x':
      tcpopt->op1=atoi(optarg);
      break;
    case 'y':
      tcpopt->op2=atoi(optarg);
      break;
    case 'z':
      tcpopt->op3=atoi(optarg);
      break;
    case 'g':
      tcpopt->op4=atoi(optarg);
      break;
    case 'h':
      tcpopt->op5=atoi(optarg);
      break;
    case 'i':
      tcpopt->op6=atoi(optarg);
      break;
    case 'j':
      tcpopt->op7=atoi(optarg);
      break;
    case '?':
      flags |= 0x10;
      fprintf(stderr, "Unrecognized option  \n");
      break;
    }
  }

  if(! ( flags & 0x2  ) ) {
    fprintf(stderr,"\nrawsockets -s <source ip> -d <destination ip> -p <port> -n <number of SYN floods>\n");
    fprintf(stderr,"\nYou must give me a destination\n");
    fprintf(stderr,"  Example:\n\t\t./rawsockets -s 192.168.1.81 -d 192.168.1.71 -p 80 -n 1\n");
    fprintf(stderr,"  Or with SackOK :\n\t\t./rawsockets -s 192.168.1.81 -d 192.168.1.71 -p 80 -n 1 -w 4 -x 2\n");
    fprintf(stderr,"  SackOK and mss 1460:\n\t\t./rawsockets -s 192.168.1.81 -d 192.168.1.71 -p 80 -n 1 -w 4 -x 2 -y 2 -z 4 -g 5 -h 180\n");
    return 1;
  }
  if(getuid( )){
    fprintf(stderr,"\n Also, you must be root to run this program:  ./su -c \"./rawsockets -s <source ip> -d <destination ip> -p 80 -n 2000\"\n");
    return 1;
  }
  fprintf(stdout,"src %s dst %s number of tries %d\n",src_ip,dst_ip,numtries);
  int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP); /* open raw socket */
  //2 PARAM,el SOCKET ES RAW, la estrucutra del SOCKET la defino a mano.
  //3 PARAME,el protocolo es TCP ESTONCES CAPTURA TODO LO TCP, osea tcp como transporte
  //incluye los encabezados IP,PROTOCOLOS Y DATOS SUBSECUENTES.
  //OSEA vas a ver la cabecera de red y transporte y tambien para escribirla
  //la cabecera la podes poblar o llenar con los mecanismos vistos antes
  //VamoS A USAR RAWSOCKET PARA leer AHORA!!!
  //Vamos a poder crear un sniffer que muestre el contenido de package TCP
  // que reciba. 
  servaddr.sin_family = AF_INET;
  // servaddr.sin_port = htons (10000);
  //OLD WAY  servaddr.sin_addr.s_addr = inet_addr (dst_ip); /* destination ip */
  inet_pton(AF_INET, dst_ip,&servaddr.sin_addr);
  int tcphdr_size = sizeof(struct tcphdr);

  iph->ip_hl = 5;
  iph->ip_v = 4;
  iph->ip_tos = 0;
  iph->ip_len = sizeof (struct ip) + sizeof (struct tcphdr) + 8 +6 +6; /* data size = 0, but tcp using option flags */
  iph->ip_id = htons (31337);
  iph->ip_off = 0;
  iph->ip_ttl = 250;
  iph->ip_p = 6;
  iph->ip_sum = 0;
  //OLD WAY iph->ip_src.s_addr = inet_addr (src_ip);/* source ip  */
  inet_pton(AF_INET, src_ip, &(iph->ip_src));
  iph->ip_dst.s_addr = servaddr.sin_addr.s_addr;

  tcph->th_sport = htons (th_sport); /* source port */
  tcph->th_dport = htons (dst_port); /* destination port */
  tcph->th_seq = htonl(31337);
  tcph->th_ack = htonl(pig_ack);/* in first SYN packet, ACK is not present */
  tcph->th_x2 = 0;
  // tcph->th_off = sizeof(struct tcphdr)/4; /* data position in the packet */
  // Special chirico adjustment to give 2x32
  tcph->th_off = 7+2+1 ;

  fprintf(stderr,"Data offset %d  sizeof(struct tcphdr)=%d\n",tcph->th_off,sizeof(struct tcphdr));

  tcph->th_flags = tcp_flags; /* initial connection request */
  tcph->th_win = htons (57344); /* FreeBSD uses this value too */
  tcph->th_sum = 0; /* we will compute it later */
  tcph->th_urp = 0;
  if (tcphdr_size % 4 != 0) /* takes care of padding to 32 bits */
    tcphdr_size = ((tcphdr_size % 4) + 1) * 4;
  fprintf(stderr,"tcphdr_size %d\n",tcphdr_size);
  tcphdr_size=40;
  fprintf(stderr,"tcphdr_size %d\n",tcphdr_size);

  memset(pheader,0x0,sizeof(pheader));
  memcpy(&pheader,&(iph->ip_src.s_addr),4);
  memcpy(&pheader[4],&(iph->ip_dst.s_addr),4);
  pheader[8]=0; // just to underline this zero byte specified by rfc
  pheader[9]=(u_int16_t)iph->ip_p;
  pheader[10]=(u_int16_t)(tcphdr_size & 0xFF00)>>8;
  pheader[11]=(u_int16_t)(tcphdr_size & 0x00FF);

  /* tcpopt->op0=4;   sackOK 
     tcpopt->op1=2;
  */
  memcpy(&pheader[12], tcph, sizeof(struct tcphdr));
  memcpy(&pheader[12+ sizeof(struct tcphdr)], tcpopt, sizeof(struct tcp_options));

  fprintf(stderr,"12+sizeof(struct tcphdr)= %d    %d\n",12+sizeof(struct tcphdr),sizeof(struct tcp_options));
  /* This is an example of setting SackOK we need to set it in the
     header for checksum and in the actual data.  This should only get
     sent when using SYN? */
  //pheader[32]=4;
  //  pheader[33]=2;
    //datagram[40]=4;
    //datagram[41]=2;
    fprintf(stderr,"********** %d %d\n",tcpopt->op0,datagram[40]);
    fprintf(stderr,"********** %d %d\n",tcpopt->op1,datagram[41]);
    fprintf(stderr,"csum size is %d\n",tcphdr_size+12);
  tcph->th_sum = csum ((uint16_t *) (pheader),tcphdr_size+12);
  //  iph->ip_sum = csum ((unsigned short *) datagram, iph->ip_len >> 1);
  int one = 1; const int *val = &one;
  //indico que voy a trabajar a NIVEL IPPROTO_IP, e incluir AL HEADER ip con IP_HDRINCL
  if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0){
    fprintf(stderr,"Error: setsockopt. You need to run this program as root\n");
      return -1;
  }
  int modval= numtries / 15;
  if (modval < 2) modval = 2;
  int ft=0;
  while(numtries-- > 0)
    {
     if (sendto (s,datagram,iph->ip_len ,0,(struct sockaddr *) &servaddr, sizeof (servaddr)) < 0)
      {
        fprintf(stderr,"Error in sendto\n");
        exit(1);
      } 
     else
      {
        if(ft==0) {
          fprintf(stderr,"[****************]\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
          ft=1;
	      }
	       if ( (numtries % modval) == 0 )
	        fprintf(stderr,".");

      }
    }
  fprintf(stderr,"sizeof(struct tcp_options)=%d\n",sizeof(struct tcp_options));
  return 0;
}
