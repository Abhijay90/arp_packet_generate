#include <iostream>
//#include <conio.h>
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

}__attribute__((packed));

struct eth_head{
  unsigned char  dest[6];
  unsigned char  src[6];
  unsigned short eth_type;
  arp_head     arp;
}__attribute__((packed));
eth_head eth_pkt;
//#define ETH_P_ARP 0x0806
//#define ETH_P_ALL 0x0C52
int place_val(){
memcpy(eth_pkt.dest,(void *)ether_aton("FF:FF:FF:FF:FF:FF"),6);
memcpy(eth_pkt.src,(void *)ether_aton("AA:AA:AA:AA:AA:AA"),6);
eth_pkt.eth_type=htons(ETH_P_ARP);
memcpy(eth_pkt.arp.sha,(void *)ether_aton("AA:AA:AA:AA:AA:AA"),6);
inet_pton(AF_INET,"192.168.186.2",&eth_pkt.arp.spa);
memcpy(eth_pkt.arp.tha,(void *)ether_aton("00:00:00:00:00:00"),6);
inet_pton(AF_INET,"192.168.2.4",&eth_pkt.arp.tpa);
cout<<sizeof(eth_pkt.arp.spa);
eth_pkt.arp.oper=htons(1);
eth_pkt.arp.htype=htons(1);
eth_pkt.arp.ptype=htons(ETH_P_IP);
eth_pkt.arp.hlen=6;
eth_pkt.arp.plen=4;
//cout<<eth_pkt.arp.oper;
}
/*void pkt_rcv(){
struct sockaddr_ll *ptr;
}*/

int main()
{
char frame[42];
struct sockaddr_ll addr;
struct ifreq iface;
struct sockaddr_ll f_bind;
//initializing
bzero(frame,42);
bzero(&iface,sizeof(iface));
bzero(&eth_pkt,sizeof(eth_pkt));
bzero(&addr,sizeof(addr));
bzero(&f_bind,sizeof(f_bind));
int socketfd,f_size,sent;

//raw socket
if((socketfd=socket(AF_PACKET,SOCK_RAW,addr.sll_protocol))==-1)
{cout<<"hello";
perror("socket");
return -1;
}
//pulling interface index
char* device;
memcpy(device,"eth0",4);
strncpy(iface.ifr_name,device,IFNAMSIZ);
if(ioctl(socketfd,SIOCGIFINDEX,&iface)==-1)
{
perror("ioctl");
close(socketfd);
return -1;
}

//local addr
addr.sll_ifindex=iface.ifr_ifindex;//addr
addr.sll_protocol=htons(ETH_P_ARP);//protocol

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
return-1;
}

//sockopt for broadcast
if(setsockopt(socketfd,SOL_SOCKET,SO_BROADCAST,&opt,sizeof(opt))==-1)
{
close(socketfd);
perror("setsocketopt");
return-1;
}
place_val();

//whole packet copying to a buffer or a string
memcpy(frame,&eth_pkt,sizeof(eth_pkt));

//sock binding to local addr
f_bind.sll_family=AF_PACKET;
f_bind.sll_protocol=addr.sll_protocol;
f_bind.sll_ifindex=addr.sll_ifindex;

if(bind(socketfd,(struct sockaddr*) &f_bind,sizeof(f_bind))==-1)
{
perror("bind");
cout<<"hello"<<strerror(errno);
return -1;
}
f_size=sizeof(frame);
//address to broadcast
addr.sll_family=AF_PACKET;//addr family
addr.sll_halen=sizeof(eth_pkt.arp.sha);
memcpy(addr.sll_addr,eth_pkt.arp.sha,addr.sll_halen);

//sending msg
if((sent=write(socketfd, frame, f_size))!=f_size)
{
cout<<"only sent "<<sent<<"some bytes from packet of "<<f_size<<" size";
perror("send");
close(socketfd);
return -1;
}
/*cout<<"hello world"<<sizeof(*eth_pkt->arp.sha);<<offsetof(eth_head,dest)<<offsetof(eth_head,src)<<offsetof(eth_head,eth_type);*/
//cout<<sizeof(eth_pkt);
cout<<close(socketfd);
cout<<sizeof(frame);
return 0;
}
