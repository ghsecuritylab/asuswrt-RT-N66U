/*
 * Linux network interface code
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: interface.c,v 1.13 2005/03/07 08:35:32 kanki Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <proto/ethernet.h>

#include <shutils.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmparams.h>
#include <bcmdevs.h>
#include <shared.h>

#include "rc.h"
#include "interface.h"

/* Default switch configs */
static struct switch_config {
	int lanmask;
	int wanmask;
} sw_config[] = {
	SWCFG_INIT(SWCFG_DEFAULT, SW_CPU|SW_L1|SW_L2|SW_L3|SW_L4,        SW_CPU|SW_WAN),
	SWCFG_INIT(SWCFG_STB1,    SW_CPU|      SW_L2|SW_L3|SW_L4,        SW_CPU|SW_WAN|SW_L1),
	SWCFG_INIT(SWCFG_STB2,    SW_CPU|SW_L1|      SW_L3|SW_L4,        SW_CPU|SW_WAN|SW_L2),
	SWCFG_INIT(SWCFG_STB3,    SW_CPU|SW_L1|SW_L2|      SW_L4,        SW_CPU|SW_WAN|SW_L3),
	SWCFG_INIT(SWCFG_STB4,    SW_CPU|SW_L1|SW_L2|SW_L3,              SW_CPU|SW_WAN|SW_L4),
	SWCFG_INIT(SWCFG_STB12,   SW_CPU|            SW_L3|SW_L4,        SW_CPU|SW_WAN|SW_L1|SW_L2),
	SWCFG_INIT(SWCFG_STB34,   SW_CPU|SW_L1|SW_L2,                    SW_CPU|SW_WAN|SW_L3|SW_L4),
	SWCFG_INIT(SWCFG_BRIDGE,  SW_CPU|SW_L1|SW_L2|SW_L3|SW_L4|SW_WAN, SW_CPU),
};

/* Generates switch ports config string
 * char *buf	- pointer to buffer[SWCFG_BUFSIZE] for result string
 * int *ports	- array of phy port numbers in order of SWPORT_ enum, eg. {W,L1,L2,L3,L4,C}
 * int swmask	- mask of logical ports to return
 * char *cputag	- NULL: return config string excluding CPU port
 *		  PSTR: return config string including CPU port, tagged with PSTR, eg. 8|8t|8*
 */
void _switch_gen_config(char *buf, const int ports[SWPORT_COUNT], int swmask, char *cputag)
{
	struct {
		int port;
		char *tag;
	} res[SWPORT_COUNT];
	int i, n, count, mask = swmask;
	char *ptr;

	if (!cputag)
		mask &= ~SW_CPU;

	for (i = count = 0; i < SWPORT_COUNT && mask; mask >>= 1, i++) {
		if ((mask & 1U) == 0)
			continue;
		for (n = count; n > 0 && ports[i] < res[n - 1].port; n--)
			res[n] = res[n - 1];
		res[n].port = ports[i];
		res[n].tag = (i == SWPORT_CPU) ? cputag : "";
		count++;
	}

	for (i = 0, ptr = buf; ptr && i < count; i++)
		ptr += sprintf(ptr, i ? " %d%s" : "%d%s", res[i].port, res[i].tag);
}

/* Generates switch ports config string
 * char *buf	- pointer to buffer[SWCFG_BUFSIZE] for result string
 * int *ports	- array of phy port numbers in order of SWPORT_ enum, eg. {W,L1,L2,L3,L4,C}
 * int index	- config index in default sw_config array
 * int wan	- 0: return config string for lan ports
 *		  1: return config string for wan ports
 * char *cputag	- NULL: return config string excluding CPU port
 *		  PSTR: return config string including CPU port, tagged with PSTR, eg. 8|8t|8*
 */
void switch_gen_config(char *buf, const int ports[SWPORT_COUNT], int index, int wan, char *cputag)
{
	int mask;

	if (!buf || index < SWCFG_DEFAULT || index >= SWCFG_COUNT)
		return;

	mask = wan ? sw_config[index].wanmask : sw_config[index].lanmask;
	_switch_gen_config(buf, ports, mask, cputag);
}

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

int _ifconfig(const char *name, int flags, const char *addr, const char *netmask, const char *dstaddr)
{
	int s;
	struct ifreq ifr;
	struct in_addr in_addr, in_netmask, in_broadaddr;

	_dprintf("%s: name=%s flags=%s addr=%s netmask=%s\n", __FUNCTION__, name, flags == IFUP ? "IFUP" : "0", addr, netmask);

	/* Open a raw socket to the kernel */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return errno;

	/* Set interface name */
	strlcpy(ifr.ifr_name, name, IFNAMSIZ);

	/* Set interface flags */
	ifr.ifr_flags = flags;
	if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0)
		goto ERROR;

	/* Set IP address */
	if (addr) {
		inet_aton(addr, &in_addr);
		sin_addr(&ifr.ifr_addr).s_addr = in_addr.s_addr;
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFADDR, &ifr) < 0)
			goto ERROR;
	}

	/* Set IP netmask and broadcast */
	if (addr && netmask) {
		inet_aton(netmask, &in_netmask);
		sin_addr(&ifr.ifr_netmask).s_addr = in_netmask.s_addr;
		ifr.ifr_netmask.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFNETMASK, &ifr) < 0)
			goto ERROR;

		in_broadaddr.s_addr = (in_addr.s_addr & in_netmask.s_addr) | ~in_netmask.s_addr;
		sin_addr(&ifr.ifr_broadaddr).s_addr = in_broadaddr.s_addr;
		ifr.ifr_broadaddr.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFBRDADDR, &ifr) < 0)
			goto ERROR;
	}

	/* Set IP dst or P-to-P peer address */
	if (dstaddr && *dstaddr) {
		inet_aton(dstaddr, &in_addr);
		sin_addr(&ifr.ifr_dstaddr).s_addr = in_addr.s_addr;
		ifr.ifr_dstaddr.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFDSTADDR, &ifr) < 0)
			goto ERROR;
	}

	close(s);
	return 0;

 ERROR:
	close(s);
	perror(name);
	return errno;
}

static int route_manip(int cmd, char *name, int metric, char *dst, char *gateway, char *genmask)
{
	int s;
	struct rtentry rt;
	
	_dprintf("%s: cmd=%s name=%s addr=%s netmask=%s gateway=%s metric=%d\n",
		__FUNCTION__, cmd == SIOCADDRT ? "ADD" : "DEL", name, dst, genmask, gateway, metric);

	/* Open a raw socket to the kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) return errno;

	/* Fill in rtentry */
	memset(&rt, 0, sizeof(rt));
	if (dst)
		inet_aton(dst, &sin_addr(&rt.rt_dst));
	if (gateway)
		inet_aton(gateway, &sin_addr(&rt.rt_gateway));
	if (genmask)
		inet_aton(genmask, &sin_addr(&rt.rt_genmask));
	rt.rt_metric = metric;
	rt.rt_flags = RTF_UP;
	if (sin_addr(&rt.rt_gateway).s_addr)
		rt.rt_flags |= RTF_GATEWAY;
	if (sin_addr(&rt.rt_genmask).s_addr == INADDR_BROADCAST)
		rt.rt_flags |= RTF_HOST;
	rt.rt_dev = name;

	/* Force address family to AF_INET */
	rt.rt_dst.sa_family = AF_INET;
	rt.rt_gateway.sa_family = AF_INET;
	rt.rt_genmask.sa_family = AF_INET;
		
	if (ioctl(s, cmd, &rt) < 0) {
		perror(name);
		close(s);
		return errno;
	}

	close(s);
	return 0;

}

int route_add(char *name, int metric, char *dst, char *gateway, char *genmask)
{
	return route_manip(SIOCADDRT, name, metric, dst, gateway, genmask);
}

int route_del(char *name, int metric, char *dst, char *gateway, char *genmask)
{
	return route_manip(SIOCDELRT, name, metric, dst, gateway, genmask);
}

/* configure loopback interface */
void config_loopback(void)
{
	/* Bring up loopback interface */
	ifconfig("lo", IFUP, "127.0.0.1", "255.0.0.0");

	/* Add to routing table */
	route_add("lo", 0, "127.0.0.0", "0.0.0.0", "255.0.0.0");
}

#ifdef RTCONFIG_IPV6
int ipv6_mapaddr4(struct in6_addr *addr6, int ip6len, struct in_addr *addr4, int ip4mask)
{
	int i = ip6len >> 5;
	int m = ip6len & 0x1f;
	int ret = ip6len + 32 - ip4mask;
	u_int32_t addr = 0;
	u_int32_t mask = 0xffffffffUL << ip4mask;

	if (ip6len > 128 || ip4mask > 32 || ret > 128)
		return 0;
	if (ip4mask == 32)
		return ret;

	if (addr4)
		addr = ntohl(addr4->s_addr) << ip4mask;

	addr6->s6_addr32[i] &= ~htonl(mask >> m);
	addr6->s6_addr32[i] |= htonl(addr >> m);
	if (m) {
		i++;
		addr6->s6_addr32[i] &= ~htonl(mask << (32 - m));
		addr6->s6_addr32[i] |= htonl(addr << (32 - m));
	}

	return ret;
}
#endif

/* configure/start vlan interface(s) based on nvram settings */
int start_vlan(void)
{
	int s;
	struct ifreq ifr;
	int i, j;
	char ea[ETHER_ADDR_LEN];

	if ((strtoul(nvram_safe_get("boardflags"), NULL, 0) & BFL_ENETVLAN) == 0) return 0;
	
	/* set vlan i/f name to style "vlan<ID>" */
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");

	/* create vlan interfaces */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return errno;

	for (i = 0; i <= VLAN_MAXVID; i ++) {
		char nvvar_name[16];
		char vlan_id[16];
		char *hwname, *hwaddr;
		char prio[8];
		/* get the address of the EMAC on which the VLAN sits */
		snprintf(nvvar_name, sizeof(nvvar_name), "vlan%dhwname", i);
		if (!(hwname = nvram_get(nvvar_name)))
			continue;
		snprintf(nvvar_name, sizeof(nvvar_name), "%smacaddr", hwname);
		if (!(hwaddr = nvram_get(nvvar_name)))
			continue;
		ether_atoe(hwaddr, ea);
		/* find the interface name to which the address is assigned */
		for (j = 1; j <= DEV_NUMIFS; j ++) {
			ifr.ifr_ifindex = j;
			if (ioctl(s, SIOCGIFNAME, &ifr))
				continue;
			if (ioctl(s, SIOCGIFHWADDR, &ifr))
				continue;
			if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER)
				continue;
			if (!bcmp(ifr.ifr_hwaddr.sa_data, ea, ETHER_ADDR_LEN))
				break;
		}
		if (j > DEV_NUMIFS)
			continue;
		if (ioctl(s, SIOCGIFFLAGS, &ifr))
			continue;
		if (!(ifr.ifr_flags & IFF_UP))
			ifconfig(ifr.ifr_name, IFUP, 0, 0);
		/* create the VLAN interface */
		snprintf(vlan_id, sizeof(vlan_id), "%d", i);
		eval("vconfig", "add", ifr.ifr_name, vlan_id);

		/* setup ingress map (vlan->priority => skb->priority) */
		snprintf(vlan_id, sizeof(vlan_id), "vlan%d", i);
		for (j = 0; j < VLAN_NUMPRIS; j ++) {
			snprintf(prio, sizeof(prio), "%d", j);
			eval("vconfig", "set_ingress_map", vlan_id, prio, prio);
		}
	}
	close(s);

#ifdef CONFIG_BCMWL5
	if(nvram_invmatch("switch_wantag", "none"))
		set_wan_tag(&ifr.ifr_name);
#endif
#if 0
		eval("vconfig", "rem", "vlan2");
		//config wan port to vlan500(port id=500)
		eval("et", "robowr", "0x34", "0x10", "0x01f4");
		eval("vconfig", "add", ifr.ifr_name, "500");
		//Set vlan table entry:500
		eval("et", "robowr", "0x05", "0x83", "0x0101");
		eval("et", "robowr", "0x05", "0x81", "0x01f4");
		eval("et", "robowr", "0x05", "0x80", "0x0000");
		eval("et", "robowr", "0x05", "0x80", "0x0080");
		if(nvram_match("switch_stb_x", "1")) {
			//config LAN port 1 to vlan600(port id=600)
			eval("et", "robowr", "0x34", "0x12", "0x0258");
			//Set vlan table entry:600
	                eval("et", "robowr", "0x05", "0x83", "0x0403");
       		        eval("et", "robowr", "0x05", "0x81", "0x0258");
       	         	eval("et", "robowr", "0x05", "0x80", "0x0000");
                	eval("et", "robowr", "0x05", "0x80", "0x0080");
		}
                else if(nvram_match("switch_stb_x", "2")) {
                        //config LAN port 2 to vlan600(port id=600)
                        eval("et", "robowr", "0x34", "0x14", "0x0258");
                        //Set vlan table entry:600
                        eval("et", "robowr", "0x05", "0x83", "0x0805");
                        eval("et", "robowr", "0x05", "0x81", "0x0258");
                        eval("et", "robowr", "0x05", "0x80", "0x0000");
                        eval("et", "robowr", "0x05", "0x80", "0x0080");
                }
                else if(nvram_match("switch_stb_x", "3")) {
                        //config LAN port  to vlan600(port id=600)
                        eval("et", "robowr", "0x34", "0x16", "0x0258");
                        //Set vlan table entry:600
                        eval("et", "robowr", "0x05", "0x83", "0x1009");
                        eval("et", "robowr", "0x05", "0x81", "0x0258");
                        eval("et", "robowr", "0x05", "0x80", "0x0000");
                        eval("et", "robowr", "0x05", "0x80", "0x0080");
                }
                else if(nvram_match("switch_stb_x", "4")) {
                        //config LAN port  to vlan600(port id=600)
                        eval("et", "robowr", "0x34", "0x18", "0x0258");
                        //Set vlan table entry:600
                        eval("et", "robowr", "0x05", "0x83", "0x2011");
                        eval("et", "robowr", "0x05", "0x81", "0x0258");
                        eval("et", "robowr", "0x05", "0x80", "0x0000");
                        eval("et", "robowr", "0x05", "0x80", "0x0080");
                }
                else if(nvram_match("switch_stb_x", "5")) {
                        //config LAN port  to vlan600(port id=600)
                        eval("et", "robowr", "0x34", "0x16", "0x0258");
			eval("et", "robowr", "0x34", "0x18", "0x0258");
                        //Set vlan table entry:600
                        eval("et", "robowr", "0x05", "0x83", "0x3019");
                        eval("et", "robowr", "0x05", "0x81", "0x0258");
                        eval("et", "robowr", "0x05", "0x80", "0x0000");
                        eval("et", "robowr", "0x05", "0x80", "0x0080");
                }
#endif

	return 0;
}

/* stop/rem vlan interface(s) based on nvram settings */
int stop_vlan(void)
{
	int i;
	char nvvar_name[16];
	char vlan_id[16];
	char *hwname;

	if ((strtoul(nvram_safe_get("boardflags"), NULL, 0) & BFL_ENETVLAN) == 0) return 0;
	
	for (i = 0; i <= VLAN_MAXVID; i ++) {
		/* get the address of the EMAC on which the VLAN sits */
		snprintf(nvvar_name, sizeof(nvvar_name), "vlan%dhwname", i);
		if (!(hwname = nvram_get(nvvar_name)))
			continue;

		/* remove the VLAN interface */
		snprintf(vlan_id, sizeof(vlan_id), "vlan%d", i);
		eval("vconfig", "rem", vlan_id);
	}

	return 0;
}

