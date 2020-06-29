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

uint16_t
checksum (uint16_t *addr, int len)
{
  int count = len;
  register uint32_t sum = 0;
  uint16_t answer = 0;
  // Sum up 2-byte values until none or only one byte left.
  while (count > 1) {
    sum += *(addr++);
    count -= 2;
  }
  // Add left-over byte, if any.
  if (count > 0) {
    sum += *(uint8_t *) addr;
  }
  // Fold 32-bit sum into 16 bits; we lose information by doing this,
  // increasing the chances of a collision.
  // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }
  // Checksum is one's compliment of sum.
  answer = ~sum;
  return (answer);
}
// Build IPv4 TCP pseudo-header and call checksum function.
uint16_t
tcp4_checksum (struct ip iphdr, struct tcphdr tcphdr)
{
  uint16_t svalue;
  char buf[IP_MAXPACKET], cvalue;
  char *ptr;
  int chksumlen = 0;
  // ptr points to beginning of buffer buf
  ptr = &buf[0];
  // Copy source IP address into buf (32 bits)
  memcpy (ptr, &iphdr.ip_src.s_addr, sizeof (iphdr.ip_src.s_addr));
  ptr += sizeof (iphdr.ip_src.s_addr);
  chksumlen += sizeof (iphdr.ip_src.s_addr);
  // Copy destination IP address into buf (32 bits)
  memcpy (ptr, &iphdr.ip_dst.s_addr, sizeof (iphdr.ip_dst.s_addr));
  ptr += sizeof (iphdr.ip_dst.s_addr);
  chksumlen += sizeof (iphdr.ip_dst.s_addr);
  // Copy zero field to buf (8 bits)
  *ptr = 0; ptr++;
  chksumlen += 1;
  // Copy transport layer protocol to buf (8 bits)
  memcpy (ptr, &iphdr.ip_p, sizeof (iphdr.ip_p));
  ptr += sizeof (iphdr.ip_p);
  chksumlen += sizeof (iphdr.ip_p);
  // Copy TCP length to buf (16 bits)
  svalue = htons (sizeof (tcphdr));
  memcpy (ptr, &svalue, sizeof (svalue));
  ptr += sizeof (svalue);
  chksumlen += sizeof (svalue);
  // Copy TCP source port to buf (16 bits)
  memcpy (ptr, &tcphdr.th_sport, sizeof (tcphdr.th_sport));
  ptr += sizeof (tcphdr.th_sport);
  chksumlen += sizeof (tcphdr.th_sport);
  // Copy TCP destination port to buf (16 bits)
  memcpy (ptr, &tcphdr.th_dport, sizeof (tcphdr.th_dport));
  ptr += sizeof (tcphdr.th_dport);
  chksumlen += sizeof (tcphdr.th_dport);
  // Copy sequence number to buf (32 bits)
  memcpy (ptr, &tcphdr.th_seq, sizeof (tcphdr.th_seq));
  ptr += sizeof (tcphdr.th_seq);
  chksumlen += sizeof (tcphdr.th_seq);
  // Copy acknowledgement number to buf (32 bits)
  memcpy (ptr, &tcphdr.th_ack, sizeof (tcphdr.th_ack));
  ptr += sizeof (tcphdr.th_ack);
  chksumlen += sizeof (tcphdr.th_ack);
  // Copy data offset to buf (4 bits) and
  // copy reserved bits to buf (4 bits)
  cvalue = (tcphdr.th_off << 4) + tcphdr.th_x2;
  memcpy (ptr, &cvalue, sizeof (cvalue));
  ptr += sizeof (cvalue);
  chksumlen += sizeof (cvalue);
  // Copy TCP flags to buf (8 bits)
  memcpy (ptr, &tcphdr.th_flags, sizeof (tcphdr.th_flags));
  ptr += sizeof (tcphdr.th_flags);
  chksumlen += sizeof (tcphdr.th_flags);
  // Copy TCP window size to buf (16 bits)
  memcpy (ptr, &tcphdr.th_win, sizeof (tcphdr.th_win));
  ptr += sizeof (tcphdr.th_win);
  chksumlen += sizeof (tcphdr.th_win);
  // Copy TCP checksum to buf (16 bits)
  // Zero, since we don't know it yet
  *ptr = 0; ptr++;
  *ptr = 0; ptr++;
  chksumlen += 2;
  // Copy urgent pointer to buf (16 bits)
  memcpy (ptr, &tcphdr.th_urp, sizeof (tcphdr.th_urp));
  ptr += sizeof (tcphdr.th_urp);
  chksumlen += sizeof (tcphdr.th_urp);
  return checksum ((uint16_t *) buf, chksumlen);
}
void reportErrorIfNecessary(int value, char * string){
    if(value == -1){
        perror(string);
        exit(1);
    }
}
void waitForEnter(){
    fflush(stdin);
    char character = getchar();
    while(character != 10){
        character = getchar();
    }
}
void buildIPHeader(struct ip * ipHeader){
    ipHeader->ip_v = 4;
    ipHeader->ip_hl = 5;
    ipHeader->ip_tos = 0;
    ipHeader->ip_len = sizeof(packet);
    ipHeader->ip_id = 0;
    ipHeader->ip_off = 0;
    ipHeader->ip_ttl = 255;
    ipHeader->ip_p = IPPROTO_TCP;
    inet_pton(AF_INET, "127.0.0.1", &(ipHeader->ip_src) );
    inet_pton(AF_INET, "127.0.0.1", &(ipHeader->ip_dst) );
    ipHeader->ip_sum = checksum((uint16_t *) &ipHeader, sizeof(struct iphdr));
}
void buildTCPHeaderWithoutChecksum(struct tcphdr * tcpHeader){

    tcpHeader->th_sport = htons(50);
    tcpHeader->th_dport = htons(23);
    tcpHeader->th_seq = htonl(0);
    tcpHeader->th_ack = htonl(0);
    tcpHeader->th_x2 = 0;
    tcpHeader->th_off = 0;
    tcpHeader->th_flags = 0b00000010;
    tcpHeader->th_win = htons(65535);
    tcpHeader->th_urp = 0;

}
void buildPacket(packet * packetToBuild){
    buildIPHeader(&(packetToBuild->IP_header));
    buildTCPHeaderWithoutChecksum(&(packetToBuild->TCP_header));
    (packetToBuild->TCP_header).th_sum = tcp4_checksum(packetToBuild->IP_header, packetToBuild->TCP_header);
}
int main( int quantityOfArgumentsReceived, char * listOfArguments[] ){
    int st;
    int returnedInteger = 0;
    int optionValue = 1;
    struct sockaddr_in destinationAddress;
    packet packetToSend;
    //Limpiamos las estructuras por seguridad
    bzero(&packetToSend, sizeof(packet) );
    bzero(&destinationAddress, sizeof(struct sockaddr_in) );
    st = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    reportErrorIfNecessary(st, "socket");
    returnedInteger = setsockopt(st, IPPROTO_IP, IP_HDRINCL, &optionValue, sizeof(optionValue) );
    reportErrorIfNecessary(returnedInteger,"setsockopt");
    buildPacket(&packetToSend);
    destinationAddress.sin_family = AF_INET;
    destinationAddress.sin_port = 0;
    inet_pton(AF_INET, "127.0.0.1", &(destinationAddress.sin_addr) );
    strcpy(packetToSend.data,"A Raw Message Bro!\n");

    while (1)
    {
        printf("Press ENTER and watch Wireshark!!!\n");
        waitForEnter();
        sendto(st, &(packetToSend.IP_header), sizeof(packet), 0, (struct sockaddr *) &destinationAddress, sizeof(struct sockaddr));
    }
    close(st);
}