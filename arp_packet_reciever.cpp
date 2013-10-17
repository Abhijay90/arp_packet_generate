#include <iostream>
//#include <conio.h>
#include<cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <string.h>
#include <arpa/inet.h>
#include <cstdio>
#include <errno.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ctime>
using namespace std;
struct arp_head
{   u_int16_t htype;    /* Hardware Type           */
    u_int16_t ptype;    /* Protocol Type           */
    u_char hlen;        /* Hardware Address Length */ 
    u_char plen;        /* Protocol Address Length */
    u_int16_t oper;     /* Operation Code          */
    u_char sha[6];      /* Sender hardware address */
    u_char spa[4];      /* Sender IP address       */
    u_char tha[6];      /* Target hardware address */ 
    u_char tpa[4];      /* Target IP address       */
 
}arp;
 

struct eth_head{
  unsigned char  dest[6];
  unsigned char  src[6];
  unsigned short eth_type;
}__attribute__((packed));
eth_head eth_pkt;

//#define ETH_P_ARP 0x0806
//#define ETH_P_ALL 0x0C52


int main()
{
struct sockaddr_ll addr;
struct sockaddr_ll f_bind;
struct ifreq iface;
int socketfd,len;
bzero(&iface,sizeof(iface));
bzero(&arp,sizeof(arp));
bzero(&eth_pkt,sizeof(eth_pkt));

//raw socket
if((socketfd=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ARP)))==-1)
{cout<<"hello";
perror("socket");
return -1;
}

///pulling interface index
char* device;
memcpy(device,"eth0",4);//Interface Device manual upload
strncpy(iface.ifr_name,device,IFNAMSIZ);
if(ioctl(socketfd,SIOCGIFINDEX,&iface)==-1)
{
perror("ioctl");
close(socketfd);
return -1;
}


//local addr
/*addr.sll_ifindex=iface.ifr_ifindex;//interface index
addr.sll_protocol=htons(ETH_P_ARP);//protocol*/
 
//sock options for binding to eth0
if(setsockopt(socketfd,SOL_SOCKET,SO_BINDTODEVICE,&iface,sizeof(iface))!=0)// check if socketbinding to device is properly executed.
{
close(socketfd);
cout<<"error binding to interface";
perror("setsocketopt");
return -1;
}
//cout<<socketfd;

//sockopt for addr reusability
int opt=1;
if(setsockopt(socketfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))==-1)
{
close(socketfd);
perror("setsocketopt");
return -1;
}

//allow broadcast
if(setsockopt(socketfd,SOL_SOCKET,SO_BROADCAST,&opt,sizeof(opt))==-1)
{
close(socketfd);
perror("setsocketopt");
return-1;
}

//sock binding to local addr
f_bind.sll_family=AF_PACKET;
f_bind.sll_protocol=htons(ETH_P_ARP);
f_bind.sll_ifindex=iface.ifr_ifindex;

if(bind(socketfd,(struct sockaddr*) &f_bind,sizeof(f_bind))==-1)
{
perror("bind");
cout<<"hello"<<strerror(errno);
return -1;
}


//recieving packet
bool condition=true;
unsigned char packet_buffer[42];

bzero(&packet_buffer,42);

double seconds,start;
start=time(NULL);
while(condition)
{
cout<<"sniffer starting \n";
seconds=time(NULL)-start;
cout<<seconds<<"\n";
if(seconds>5)
{
close(socketfd);
return 0;
}
bzero(packet_buffer,42);
len=read(socketfd,packet_buffer,42);//reading from socket
cout<<"packet length "<<len<<"\n";
if(len==-1)
{
perror("read");
close(socketfd);
return -1;
}
else{
int sz=(sizeof(eth_pkt)+sizeof(arp));
if(len<sz)
{
cout<<"short packet \n";
continue;
}
/*ptr_eth=*(eth_head*)(packet_buffer);
eth_pkt=*ptr_eth;
*/
eth_pkt=*(eth_head*)(packet_buffer);
//cout<<eth_pkt.eth_type;
if(eth_pkt.eth_type==htons(ETH_P_ARP))
{
arp=*(arp_head*)(packet_buffer + sizeof(eth_pkt));
if(arp.oper==htons(1))
{cout<<"packet acquired request";
condition=false;}
else{
if(arp.oper==htons(2))
{
cout<<"packet acquired response";
condition=false;}}}
}
}
close(socketfd);
return 0;
}
