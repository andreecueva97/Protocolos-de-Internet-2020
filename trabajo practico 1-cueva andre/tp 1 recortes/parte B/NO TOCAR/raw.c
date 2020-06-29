#include <sys/socket.h>	
#include <netinet/in.h>	
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void ProcessPacket(unsigned char* , int);
void print_ip_header(unsigned char* , int);
void print_tcp_packet(unsigned char* , int);
void print_udp_packet(unsigned char * , int);
void print_icmp_packet(unsigned char* , int);
void PrintData (unsigned char* , int);

int sock_raw;
FILE *logfile;
int tcp=0,udp=0,icmp=0,others=0,igmp=0,total=0,i,j;
struct sockaddr_in source,dest;

int main()
{
	int saddr_size , data_size;
	struct sockaddr saddr;
	struct in_addr in;
	
	unsigned char *buffer = (unsigned char *)malloc(65536); //Its Big!
	
	logfile=fopen(&quot;log.txt&quot;,&quot;w&quot;);
	if(logfile==NULL) printf(&quot;Unable to create file.&quot;);
	printf(&quot;Starting...\n&quot;);
	//Create a raw socket that shall sniff
	sock_raw = socket(AF_INET , SOCK_RAW , IPPROTO_TCP);
	if(sock_raw &lt; 0)
	{
		printf(&quot;Socket Error\n&quot;);
		return 1;
	}
	while(1)
	{
		saddr_size = sizeof saddr;
		//Receive a packet
		data_size = recvfrom(sock_raw , buffer , 65536 , 0 , &amp;saddr , &amp;saddr_size);
		if(data_size &lt;0 )
		{
			printf(&quot;Recvfrom error , failed to get packets\n&quot;);
			return 1;
		}
		//Now process the packet
		ProcessPacket(buffer , data_size);
	}
	close(sock_raw);
	printf(&quot;Finished&quot;);
	return 0;
}
void ProcessPacket(unsigned char* buffer, int size)
{
	//Get the IP Header part of this packet
	struct iphdr *iph = (struct iphdr*)buffer;
	++total;
	switch (iph-&gt;protocol) //Check the Protocol and do accordingly...
	{
		case 1:  //ICMP Protocol
			++icmp;
			//PrintIcmpPacket(Buffer,Size);
			break;
		
		case 2:  //IGMP Protocol
			++igmp;
			break;
		
		case 6:  //TCP Protocol
			++tcp;
			print_tcp_packet(buffer , size);
			break;
		
		case 17: //UDP Protocol
			++udp;
			print_udp_packet(buffer , size);
			break;
		
		default: //Some Other Protocol like ARP etc.
			++others;
			break;
	}
	printf(&quot;TCP : %d   UDP : %d   ICMP : %d   IGMP : %d   Others : %d   Total : %d\r&quot;,tcp,udp,icmp,igmp,others,total);
}
void print_ip_header(unsigned char* Buffer, int Size)
{
	unsigned short iphdrlen;
		
	struct iphdr *iph = (struct iphdr *)Buffer;
	iphdrlen =iph-&gt;ihl*4;
	
	memset(&amp;source, 0, sizeof(source));
	source.sin_addr.s_addr = iph-&gt;saddr;
	
	memset(&amp;dest, 0, sizeof(dest));
	dest.sin_addr.s_addr = iph-&gt;daddr;
	
	fprintf(logfile,&quot;\n&quot;);
	fprintf(logfile,&quot;IP Header\n&quot;);
	fprintf(logfile,&quot;   |-IP Version        : %d\n&quot;,(unsigned int)iph-&gt;version);
	fprintf(logfile,&quot;   |-IP Header Length  : %d DWORDS or %d Bytes\n&quot;,(unsigned int)iph-&gt;ihl,((unsigned int)(iph-&gt;ihl))*4);
	fprintf(logfile,&quot;   |-Type Of Service   : %d\n&quot;,(unsigned int)iph-&gt;tos);
	fprintf(logfile,&quot;   |-IP Total Length   : %d  Bytes(Size of Packet)\n&quot;,ntohs(iph-&gt;tot_len));
	fprintf(logfile,&quot;   |-Identification    : %d\n&quot;,ntohs(iph-&gt;id));
	//fprintf(logfile,&quot;   |-Reserved ZERO Field   : %d\n&quot;,(unsigned int)iphdr-&gt;ip_reserved_zero);
	//fprintf(logfile,&quot;   |-Dont Fragment Field   : %d\n&quot;,(unsigned int)iphdr-&gt;ip_dont_fragment);
	//fprintf(logfile,&quot;   |-More Fragment Field   : %d\n&quot;,(unsigned int)iphdr-&gt;ip_more_fragment);
	fprintf(logfile,&quot;   |-TTL      : %d\n&quot;,(unsigned int)iph-&gt;ttl);
	fprintf(logfile,&quot;   |-Protocol : %d\n&quot;,(unsigned int)iph-&gt;protocol);
	fprintf(logfile,&quot;   |-Checksum : %d\n&quot;,ntohs(iph-&gt;check));
	fprintf(logfile,&quot;   |-Source IP        : %s\n&quot;,inet_ntoa(source.sin_addr));
	fprintf(logfile,&quot;   |-Destination IP   : %s\n&quot;,inet_ntoa(dest.sin_addr));
}

void print_tcp_packet(unsigned char* Buffer, int Size)
{
	unsigned short iphdrlen;
	
	struct iphdr *iph = (struct iphdr *)Buffer;
	iphdrlen = iph-&gt;ihl*4;
	
	struct tcphdr *tcph=(struct tcphdr*)(Buffer + iphdrlen);
			
	fprintf(logfile,&quot;\n\n***********************TCP Packet*************************\n&quot;);	
		
	print_ip_header(Buffer,Size);
		
	fprintf(logfile,&quot;\n&quot;);
	fprintf(logfile,&quot;TCP Header\n&quot;);
	fprintf(logfile,&quot;   |-Source Port      : %u\n&quot;,ntohs(tcph-&gt;source));
	fprintf(logfile,&quot;   |-Destination Port : %u\n&quot;,ntohs(tcph-&gt;dest));
	fprintf(logfile,&quot;   |-Sequence Number    : %u\n&quot;,ntohl(tcph-&gt;seq));
	fprintf(logfile,&quot;   |-Acknowledge Number : %u\n&quot;,ntohl(tcph-&gt;ack_seq));
	fprintf(logfile,&quot;   |-Header Length      : %d DWORDS or %d BYTES\n&quot; ,(unsigned int)tcph-&gt;doff,(unsigned int)tcph-&gt;doff*4);
	//fprintf(logfile,&quot;   |-CWR Flag : %d\n&quot;,(unsigned int)tcph-&gt;cwr);
	//fprintf(logfile,&quot;   |-ECN Flag : %d\n&quot;,(unsigned int)tcph-&gt;ece);
	fprintf(logfile,&quot;   |-Urgent Flag          : %d\n&quot;,(unsigned int)tcph-&gt;urg);
	fprintf(logfile,&quot;   |-Acknowledgement Flag : %d\n&quot;,(unsigned int)tcph-&gt;ack);
	fprintf(logfile,&quot;   |-Push Flag            : %d\n&quot;,(unsigned int)tcph-&gt;psh);
	fprintf(logfile,&quot;   |-Reset Flag           : %d\n&quot;,(unsigned int)tcph-&gt;rst);
	fprintf(logfile,&quot;   |-Synchronise Flag     : %d\n&quot;,(unsigned int)tcph-&gt;syn);
	fprintf(logfile,&quot;   |-Finish Flag          : %d\n&quot;,(unsigned int)tcph-&gt;fin);
	fprintf(logfile,&quot;   |-Window         : %d\n&quot;,ntohs(tcph-&gt;window));
	fprintf(logfile,&quot;   |-Checksum       : %d\n&quot;,ntohs(tcph-&gt;check));
	fprintf(logfile,&quot;   |-Urgent Pointer : %d\n&quot;,tcph-&gt;urg_ptr);
	fprintf(logfile,&quot;\n&quot;);
	fprintf(logfile,&quot;                        DATA Dump                         &quot;);
	fprintf(logfile,&quot;\n&quot;);
		
	fprintf(logfile,&quot;IP Header\n&quot;);
	PrintData(Buffer,iphdrlen);
		
	fprintf(logfile,&quot;TCP Header\n&quot;);
	PrintData(Buffer+iphdrlen,tcph-&gt;doff*4);
		
	fprintf(logfile,&quot;Data Payload\n&quot;);	
	PrintData(Buffer + iphdrlen + tcph-&gt;doff*4 , (Size - tcph-&gt;doff*4-iph-&gt;ihl*4) );
						
	fprintf(logfile,&quot;\n###########################################################&quot;);
}

void print_udp_packet(unsigned char *Buffer , int Size)
{
	
	unsigned short iphdrlen;
	
	struct iphdr *iph = (struct iphdr *)Buffer;
	iphdrlen = iph-&gt;ihl*4;
	
	struct udphdr *udph = (struct udphdr*)(Buffer + iphdrlen);
	
	fprintf(logfile,&quot;\n\n***********************UDP Packet*************************\n&quot;);
	
	print_ip_header(Buffer,Size);			
	
	fprintf(logfile,&quot;\nUDP Header\n&quot;);
	fprintf(logfile,&quot;   |-Source Port      : %d\n&quot; , ntohs(udph-&gt;source));
	fprintf(logfile,&quot;   |-Destination Port : %d\n&quot; , ntohs(udph-&gt;dest));
	fprintf(logfile,&quot;   |-UDP Length       : %d\n&quot; , ntohs(udph-&gt;len));
	fprintf(logfile,&quot;   |-UDP Checksum     : %d\n&quot; , ntohs(udph-&gt;check));
	
	fprintf(logfile,&quot;\n&quot;);
	fprintf(logfile,&quot;IP Header\n&quot;);
	PrintData(Buffer , iphdrlen);
		
	fprintf(logfile,&quot;UDP Header\n&quot;);
	PrintData(Buffer+iphdrlen , sizeof udph);
		
	fprintf(logfile,&quot;Data Payload\n&quot;);	
	PrintData(Buffer + iphdrlen + sizeof udph ,( Size - sizeof udph - iph-&gt;ihl * 4 ));
	
	fprintf(logfile,&quot;\n###########################################################&quot;);
}

void print_icmp_packet(unsigned char* Buffer , int Size)
{
	unsigned short iphdrlen;
	
	struct iphdr *iph = (struct iphdr *)Buffer;
	iphdrlen = iph-&gt;ihl*4;
	
	struct icmphdr *icmph = (struct icmphdr *)(Buffer + iphdrlen);
			
	fprintf(logfile,&quot;\n\n***********************ICMP Packet*************************\n&quot;);	
	
	print_ip_header(Buffer , Size);
			
	fprintf(logfile,&quot;\n&quot;);
		
	fprintf(logfile,&quot;ICMP Header\n&quot;);
	fprintf(logfile,&quot;   |-Type : %d&quot;,(unsigned int)(icmph-&gt;type));
			
	if((unsigned int)(icmph-&gt;type) == 11) 
		fprintf(logfile,&quot;  (TTL Expired)\n&quot;);
	else if((unsigned int)(icmph-&gt;type) == ICMP_ECHOREPLY) 
		fprintf(logfile,&quot;  (ICMP Echo Reply)\n&quot;);
	fprintf(logfile,&quot;   |-Code : %d\n&quot;,(unsigned int)(icmph-&gt;code));
	fprintf(logfile,&quot;   |-Checksum : %d\n&quot;,ntohs(icmph-&gt;checksum));
	//fprintf(logfile,&quot;   |-ID       : %d\n&quot;,ntohs(icmph-&gt;id));
	//fprintf(logfile,&quot;   |-Sequence : %d\n&quot;,ntohs(icmph-&gt;sequence));
	fprintf(logfile,&quot;\n&quot;);

	fprintf(logfile,&quot;IP Header\n&quot;);
	PrintData(Buffer,iphdrlen);
		
	fprintf(logfile,&quot;UDP Header\n&quot;);
	PrintData(Buffer + iphdrlen , sizeof icmph);
		
	fprintf(logfile,&quot;Data Payload\n&quot;);	
	PrintData(Buffer + iphdrlen + sizeof icmph , (Size - sizeof icmph - iph-&gt;ihl * 4));
	
	fprintf(logfile,&quot;\n###########################################################&quot;);
}

void PrintData (unsigned char* data , int Size)
{
	
	for(i=0 ; i &lt; Size ; i++)
	{
		if( i!=0 &amp;&amp; i%16==0)   //if one line of hex printing is complete...
		{
			fprintf(logfile,&quot;         &quot;);
			for(j=i-16 ; j&lt;i ; j++)
			{
				if(data[j]&gt;=32 &amp;&amp; data[j]&lt;=128)
					fprintf(logfile,&quot;%c&quot;,(unsigned char)data[j]); //if its a number or alphabet
				
				else fprintf(logfile,&quot;.&quot;); //otherwise print a dot
			}
			fprintf(logfile,&quot;\n&quot;);
		} 
		
		if(i%16==0) fprintf(logfile,&quot;   &quot;);
			fprintf(logfile,&quot; %02X&quot;,(unsigned int)data[i]);
				
		if( i==Size-1)  //print the last spaces
		{
			for(j=0;j&lt;15-i%16;j++) fprintf(logfile,&quot;   &quot;); //extra spaces
			
			fprintf(logfile,&quot;         &quot;);
			
			for(j=i-i%16 ; j&lt;=i ; j++)
			{
				if(data[j]&gt;=32 &amp;&amp; data[j]&lt;=128) fprintf(logfile,&quot;%c&quot;,(unsigned char)data[j]);
				else fprintf(logfile,&quot;.&quot;);
			}
			fprintf(logfile,&quot;\n&quot;);
		}
	}
}