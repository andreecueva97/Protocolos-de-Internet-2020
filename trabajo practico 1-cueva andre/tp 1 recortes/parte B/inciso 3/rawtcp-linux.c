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
#define PCKT_LEN 8192// Packet length
// headers' structures
// IP header's structure-------------------------------------------
struct ipheader {
 unsigned char      iph_ihl:5, /* Little-endian */
                    iph_ver:4;
 unsigned char      iph_tos;
 unsigned short int iph_len;
 unsigned short int iph_ident;
 unsigned short int iph_offset;
 unsigned char      iph_ttl;
 unsigned char      iph_protocol;
 unsigned short int iph_chksum;
 unsigned int       iph_sourceip;//origen 127.0.0.1
 unsigned int       iph_destip;//destino  127.0.0.1
};
//-----------------------------------------------------------------
/* Structure of a TCP header ---------------------------------------------*/
struct tcpheader {
 unsigned short int tcph_srcport;
 unsigned short int tcph_destport;
 unsigned int       tcph_seqnum;
 unsigned int       tcph_acknum;
 unsigned char      tcph_reserved:4;
 unsigned char      tcph_offset:1;
 unsigned char      tcph_flags;
 unsigned int
       tcp_res1:4,      /*little-endian*/
       tcph_hlen:4,     /*length of tcp header in 32-bit words*/
       tcph_fin:1,      /*Finish flag "fin"*/
       tcph_syn:1,       /*Synchronize sequence numbers to start a connection*/
       tcph_rst:1,      /*Reset flag */
       tcph_psh:1,      /*Push, sends data to the application*/
       tcph_ack:1,      /*acknowledge*/
       tcph_urg:1,      /*urgent pointer*/
       tcph_res2:2;
 unsigned short int tcph_win;
 unsigned short int tcph_chksum;
 unsigned short int tcph_urgptr;
};
//------------------------------------------------------------------------
// Simple checksum function, may use others such as Cyclic Redundancy Check, CRC
unsigned short csum(unsigned short *buf, int len)
{
        unsigned long sum;
        for(sum=0; len>0; len--)
                sum += *buf++;
        sum = (sum >> 16) + (sum &0xffff);
        sum += (sum >> 16);
        return (unsigned short)(~sum);
}

int main(int argc, char *argv[])
{
   int sd;//socket initialize
   char buffer[PCKT_LEN];// No data, just datagram
   // The size of the headers-------------------------------
   struct ipheader *ip = (struct ipheader *) buffer;//header ip
   struct tcpheader *tcp = (struct tcpheader *) (buffer + sizeof(struct ipheader));//header tcp
   //--------------------------------------------------------
   struct sockaddr_in destinyaddress, originaddress;
   int one = 1;
   const int *val = &one;
   memset(buffer, 0, PCKT_LEN);
if(argc != 5)
{
   printf("- Invalid parameters!!!\n");
   printf("- Usage: %s <source hostname/IP> <source port> <target hostname/IP> <target port>\n", argv[0]);
   exit(1);
}
if((sd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP)) < 0)//create socket
{
   perror("socket() error");exit(1);
}
else
   printf("socket()-SOCK_RAW and tcp protocol is OK.\n");
// The source is redundant, may be used later if needed
// Address family
destinyaddress.sin_family = AF_INET;
originaddress.sin_family = AF_INET;
// Source port, can be any, modify as needed
destinyaddress.sin_port = htons(atoi(argv[2]));
originaddress.sin_port = htons(atoi(argv[4]));
// Source IP, can be any, modify as needed
destinyaddress.sin_addr.s_addr = inet_addr(argv[1]);
originaddress.sin_addr.s_addr = inet_addr(argv[3]);
// IP structure---------------------------------------------
ip->iph_ihl = 5;
ip->iph_ver = 4;
ip->iph_tos = 0;
ip->iph_len = sizeof(struct ipheader) + sizeof(struct tcpheader);//size of packet == ipheader+tcpheader
ip->iph_ident = 0;
ip->iph_offset = 0;
ip->iph_ttl = 255;
ip->iph_protocol = IPPROTO_TCP; // use TCP
ip->iph_chksum = 0; 
ip->iph_sourceip = inet_addr(argv[1]);//Origin IP, modify in command
ip->iph_destip = inet_addr(argv[3]);//Destination Ip, modify in commmand
//----------------------------------------------------------
//TCP structure--------------------------------------------------
tcp->tcph_srcport = htons(atoi(argv[2]));//modify in command
// The destination port, we accept through command line
tcp->tcph_destport = htons(atoi(argv[4]));//modify in comand
tcp->tcph_seqnum = htonl(0);
tcp->tcph_acknum = htonl(0);
tcp->tcph_offset = 0;
tcp->tcph_syn = 1;//
tcp->tcph_ack = htonl(0);
tcp->tcph_win = htons(65535);
tcp->tcph_chksum = 0; // Done by kernel
tcp->tcph_flags= 0b00000010;
tcp->tcph_urgptr = 0;
//------------------------------------------------------------------
// IP checksum calculation
ip->iph_chksum = csum((unsigned short *) buffer, (sizeof(struct ipheader) + sizeof(struct tcpheader)));


if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
{
    perror("setsockopt() error");exit(1);
}
else
   printf("setsockopt() is OK\n");
printf("Udestinyg:::::Source IP: %s port: %u, Target IP: %s port: %u.\n", argv[1], atoi(argv[2]), argv[3], atoi(argv[4]));
// sendto() loop, send every 2 second ----------------------
while(1)
{
   if(sendto(sd, buffer, ip->iph_len, 0, (struct sockaddr *)&destinyaddress, sizeof(destinyaddress)) < 0)
   {
      perror("sendto() error");exit(1);
   }
   else
      printf("sendto() is OK\n");
   sleep(5);
}
//---------------------------------------------------------
close(sd);
return 0;
}
