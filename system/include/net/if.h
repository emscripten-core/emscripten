
#ifndef _NET_IF_H
#define _NET_IF_H

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif

struct if_nameindex {
  unsigned  if_index;
  char     *if_name;
};

#define IFHWADDRLEN 6
#define IFNAMSIZ 16

struct ifmap {
  unsigned long int mem_start;
  unsigned long int mem_end;
  unsigned short int base_addr;
  unsigned char irq;
  unsigned char dma;
  unsigned char port;
};

struct ifreq {
  union {
    char ifrn_name[IFNAMSIZ];
  } ifr_ifrn;
  union {
    struct sockaddr ifru_addr;
    struct sockaddr ifru_destaddr;
    struct sockaddr ifru_broadaddr;
    struct sockaddr ifru_netmask;
    struct sockaddr ifru_hwaddr;
    short int ifru_flags;
    int ifru_ivalue;
    int ifru_mtu;
    struct ifmap ifru_map;
    char ifru_slave[IFNAMSIZ];
    char ifru_newname[IFNAMSIZ];
    caddr_t ifru_data;
  } ifr_ifru;
};
#define ifr_name ifr_ifrn.ifrn_name
#define ifr_addr ifr_ifru.ifru_addr
#define ifr_destaddr ifr_ifru.ifru_destaddr
#define ifr_broadaddr ifr_ifru.ifru_broadaddr
#define ifr_netmask ifr_ifru.ifru_netmask
#define ifr_hwaddr ifr_ifru.ifru_hwaddr
#define ifr_flags ifr_ifru.ifru_flags
#define ifr_ivalue ifr_ifru.ifru_ivalue
#define ifr_mtu ifr_ifru.ifru_mtu
#define ifr_map ifr_ifru.ifru_map
#define ifr_slave ifr_ifru.ifru_slave
#define ifr_newname ifr_ifru.ifru_newname
#define ifr_data ifr_ifru.ifru_data

struct ifconf {
  int ifc_len;
  union {
    caddr_t ifcu_buf;
    struct ifreq* ifcu_req;
  } ifc_ifcu;
};
#define ifc_buf ifc_ifcu.ifcu_buf
#define ifc_req ifc_ifcu.ifcu_req

#define IF_NAMESIZE abort(0);

unsigned             if_nametoindex(const char *a);
char                *if_indextoname(unsigned int a, char *b);
struct if_nameindex *if_nameindex();
void                 if_freenameindex(struct if_nameindex *a);



#ifdef __cplusplus
}
#endif

#endif

