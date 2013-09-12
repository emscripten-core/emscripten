#ifndef _NETPACKET_PACKET_H
#define _NETPACKET_PACKET_H

#ifdef __cplusplus
extern "C" {
#endif

struct sockaddr_ll {
	unsigned short sll_family, sll_protocol;
	int sll_ifindex;
	unsigned short sll_hatype;
	unsigned char sll_pkttype, sll_halen;
	unsigned char sll_addr[8];
};

struct packet_mreq {
	int mr_ifindex;
	unsigned short int mr_type,  mr_alen;
	unsigned char mr_address[8];
};

#define PACKET_HOST		0
#define PACKET_BROADCAST	1
#define PACKET_MULTICAST	2
#define PACKET_OTHERHOST	3
#define PACKET_OUTGOING		4
#define PACKET_LOOPBACK		5
#define PACKET_FASTROUTE	6

#define PACKET_ADD_MEMBERSHIP		1
#define PACKET_DROP_MEMBERSHIP		2
#define	PACKET_RECV_OUTPUT		3
#define	PACKET_RX_RING			5
#define	PACKET_STATISTICS		6

#define PACKET_MR_MULTICAST	0
#define PACKET_MR_PROMISC	1
#define PACKET_MR_ALLMULTI	2

#ifdef __cplusplus
}
#endif

#endif
