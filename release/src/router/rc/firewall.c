/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <rc.h>

#include <stdarg.h>
#include <netdb.h>	// for struct addrinfo

#define WEBSTRFILTER 1

#define foreach_x(x)	for (i=0; i<atoi(nvram_safe_get(x)); i++)

#ifdef RTCONFIG_IPV6
char wan6face[IFNAMSIZ + 1];
// RFC-4890, sec. 4.3.1
const int allowed_icmpv6[] = { 1, 2, 3, 4, 128, 129 };
#endif

char *mac_conv(char *mac_name, int idx, char *buf);	// oleg patch

#ifdef WEB_REDIRECT
void redirect_setting();
#endif

char *g_buf;
char g_buf_pool[1024];

void g_buf_init()
{
	g_buf = g_buf_pool;
}

char *g_buf_alloc(char *g_buf_now)
{
	g_buf += strlen(g_buf_now)+1;

	return (g_buf_now);
}

int host_addr_info(const char *name, int af, struct sockaddr_storage *buf)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *p;
	int err;
	int addrtypes = 0;

	memset(&hints, 0, sizeof hints);
#ifdef RTCONFIG_IPV6
	switch (af & (IPT_V4 | IPT_V6)) {
	case IPT_V4:
		hints.ai_family = AF_INET;
		break;
	case IPT_V6:
		hints.ai_family = AF_INET6;
		break;
	//case (IPT_V4 | IPT_V6):
	//case 0: // error?
	default:
		hints.ai_family = AF_UNSPEC;
	}
#else
	hints.ai_family = AF_INET;
#endif
	hints.ai_socktype = SOCK_RAW;

	if ((err = getaddrinfo(name, NULL, &hints, &res)) != 0) {
		return addrtypes;
	}

	for(p = res; p != NULL; p = p->ai_next) {
		switch(p->ai_family) {
		case AF_INET:
			addrtypes |= IPT_V4;
			break;
		case AF_INET6:
			addrtypes |= IPT_V6;
			break;
		}
		if (buf && (hints.ai_family == p->ai_family) && res->ai_addrlen) {
			memcpy(buf, res->ai_addr, res->ai_addrlen);
		}
	}

	freeaddrinfo(res);
	return (addrtypes & af);
}

inline int host_addrtypes(const char *name, int af)
{
	return host_addr_info(name, af, NULL);
}

int ipt_addr_compact(const char *s, int af, int strict)
{
	char p[INET6_ADDRSTRLEN * 2];
	int r = 0;

	if ((s) && (*s))
	{
		if (sscanf(s, "%[0-9.]-%[0-9.]", p, p) == 2) {
			r = IPT_V4;
		}
#ifdef RTCONFIG_IPV6
		else if (sscanf(s, "%[0-9A-Fa-f:]-%[0-9A-Fa-f:]", p, p) == 2) {
			r = IPT_V6;
		}
#endif
		else {
			if (sscanf(s, "%[^/]/", p)) {
#ifdef RTCONFIG_IPV6
				r = host_addrtypes(p, strict ? af : (IPT_V4 | IPT_V6));
#else
				r = host_addrtypes(p, IPT_V4);
#endif
			}
		}
	}
	else
	{
		r = (IPT_V4 | IPT_V6);
	}

	return (r & af);
}

/*
void nvram_unsets(char *name, int count)
{
	char itemname_arr[32];
	int i;

	for (i=0; i<count; i++)
	{
		sprintf(itemname_arr, "%s%d", name, i);
		nvram_unset(itemname_arr);
	}
}
*/

char *proto_conv(char *proto, char *buf)
{			
	if (!strncasecmp(proto, "Both", 3)) strcpy(buf, "both");
	else if (!strncasecmp(proto, "TCP", 3)) strcpy(buf, "tcp");
	else if (!strncasecmp(proto, "UDP", 3)) strcpy(buf, "udp");
	else if (!strncasecmp(proto, "OTHER", 5)) strcpy(buf, "other");
	else strcpy(buf,"tcp");
	return buf;
}

char *protoflag_conv(char *proto, char *buf, int isFlag)
{
	if (!isFlag)
	{		
		if (strncasecmp(proto, "UDP", 3)==0) strcpy(buf, "udp");
		else strcpy(buf, "tcp");
	}
	else
	{	
		if (strlen(proto)>3)
		{
			strcpy(buf, proto+4);
		}			
		else strcpy(buf,"");
	}	
	return (buf);
}
/*
char *portrange_ex_conv(char *port_name, int idx)
{
	char *port, *strptr;
	char itemname_arr[32];
	
	sprintf(itemname_arr,"%s%d", port_name, idx);
	port=nvram_get(itemname_arr);

	strcpy(g_buf, "");
	
	if (!strncmp(port, ">", 1)) {
		sprintf(g_buf, "%d-65535", atoi(port+1) + 1);
	}
	else if (!strncmp(port, "=", 1)) {
		sprintf(g_buf, "%d-%d", atoi(port+1), atoi(port+1));
	}
	else if (!strncmp(port, "<", 1)) {
		sprintf(g_buf, "1-%d", atoi(port+1) - 1);
	}
	//else if (strptr=strchr(port, ':'))
	else if ((strptr=strchr(port, ':')) != NULL) //2008.11 magic oleg patch
	{		
		strcpy(itemname_arr, port);
		strptr = strchr(itemname_arr, ':');
		sprintf(g_buf, "%d-%d", atoi(itemname_arr), atoi(strptr+1));		
	}
	else if (*port)
	{
		sprintf(g_buf, "%d-%d", atoi(port), atoi(port));
	}
	else
	{
		//sprintf(g_buf, "");
		g_buf[0] = 0;	// oleg patch
	}
	
	return (g_buf_alloc(g_buf));
}
*/

char *portrange_ex2_conv(char *port_name, int idx, int *start, int *end)
{
	char *port, *strptr;
	char itemname_arr[32];
	
	sprintf(itemname_arr,"%s%d", port_name, idx);
	port=nvram_get(itemname_arr);

	strcpy(g_buf, "");
	
	if (!strncmp(port, ">", 1)) 
	{
		sprintf(g_buf, "%d-65535", atoi(port+1) + 1);
		*start=atoi(port+1);
		*end=65535;
	}
	else if (!strncmp(port, "=", 1)) 
	{
		sprintf(g_buf, "%d-%d", atoi(port+1), atoi(port+1));
		*start=*end=atoi(port+1);
	}
	else if (!strncmp(port, "<", 1)) 
	{
		sprintf(g_buf, "1-%d", atoi(port+1) - 1);
		*start=1;
		*end=atoi(port+1);
	}
	//else if (strptr=strchr(port, ':'))
	else if ((strptr=strchr(port, ':')) != NULL) //2008.11 magic oleg patch
	{		
		strcpy(itemname_arr, port);
		strptr = strchr(itemname_arr, ':');
		sprintf(g_buf, "%d-%d", atoi(itemname_arr), atoi(strptr+1));	
		*start=atoi(itemname_arr);
		*end=atoi(strptr+1);
	}
	else if (*port)
	{
		sprintf(g_buf, "%d-%d", atoi(port), atoi(port));
		*start=atoi(port);
		*end=atoi(port);
	}
	else
	{
		//sprintf(g_buf, "");
		 g_buf[0] = 0;	// oleg patch
		*start=0;
		*end=0;
	}
	
	return (g_buf_alloc(g_buf));
}

char *portrange_ex2_conv_new(char *port_name, int idx, int *start, int *end)
{
	char *port, *strptr;
	char itemname_arr[32];
	
	sprintf(itemname_arr,"%s%d", port_name, idx);
	port=nvram_get(itemname_arr);

	strcpy(g_buf, "");
	
	if (!strncmp(port, ">", 1)) 
	{
		sprintf(g_buf, "%d-65535", atoi(port+1) + 1);
		*start=atoi(port+1);
		*end=65535;
	}
	else if (!strncmp(port, "=", 1)) 
	{
		sprintf(g_buf, "%d-%d", atoi(port+1), atoi(port+1));
		*start=*end=atoi(port+1);
	}
	else if (!strncmp(port, "<", 1)) 
	{
		sprintf(g_buf, "1-%d", atoi(port+1) - 1);
		*start=1;
		*end=atoi(port+1);
	}
	else if ((strptr=strchr(port, ':')) != NULL)
	{		
		strcpy(itemname_arr, port);
		strptr = strchr(itemname_arr, ':');
		sprintf(g_buf, "%d:%d", atoi(itemname_arr), atoi(strptr+1));	
		*start=atoi(itemname_arr);
		*end=atoi(strptr+1);
	}
	else if (*port)
	{
		sprintf(g_buf, "%d:%d", atoi(port), atoi(port));
		*start=atoi(port);
		*end=atoi(port);
	}
	else
	{
		//sprintf(g_buf, "");
		 g_buf[0] = 0;	// oleg patch
		*start=0;
		*end=0;
	}
	
	return (g_buf_alloc(g_buf));
}

char *portrange_conv(char *port_name, int idx)
{
	char itemname_arr[32];
	
	sprintf(itemname_arr,"%s%d", port_name, idx);
	strcpy(g_buf, nvram_safe_get(itemname_arr));	
	
	return (g_buf_alloc(g_buf));
}
/*
char *iprange_conv(char *ip_name, int idx)
{
	char *ip;
	char itemname_arr[32];
	char startip[16], endip[16];
	int i, j, k;
	
	sprintf(itemname_arr,"%s%d", ip_name, idx);
	ip=nvram_safe_get(itemname_arr);
	//strcpy(g_buf, "");
	 g_buf[0] = 0;	// 0313
	
	// scan all ip string
	i=j=k=0;
	
	while (*(ip+i))
	{
		if (*(ip+i)=='*') 
		{
			startip[j++] = '1';
			endip[k++] = '2';
			endip[k++] = '5';
			endip[k++] = '4';
			// 255 is for broadcast
		}
		else 
		{
			startip[j++] = *(ip+i);
			endip[k++] = *(ip+i);
		}
		i++;
	}	
	
	startip[j++] = 0;
	endip[k++] = 0;

	if (strcmp(startip, endip)==0)
		sprintf(g_buf, "%s", startip);
	else
		sprintf(g_buf, "%s-%s", startip, endip);
	return (g_buf_alloc(g_buf));
}

char *iprange_ex_conv(char *ip_name, int idx)
{
	char *ip;
	char itemname_arr[32];
	char startip[16], endip[16];
	int i, j, k;
	int mask;
	
	sprintf(itemname_arr,"%s%d", ip_name, idx);
	ip=nvram_safe_get(itemname_arr);
	strcpy(g_buf, "");
	
	// scan all ip string
	i=j=k=0;
	mask=32;
	
	while (*(ip+i))
	{
		if (*(ip+i)=='*') 
		{
			startip[j++] = '0';
			endip[k++] = '0';
			// 255 is for broadcast
			mask-=8;
		}
		else 
		{
			startip[j++] = *(ip+i);
			endip[k++] = *(ip+i);
		}
		i++;
	}	
	
	startip[j++] = 0;
	endip[k++] = 0;

	if (mask==32)
		sprintf(g_buf, "%s", startip);
	else if (mask==0)
		strcpy(g_buf, "");
	else sprintf(g_buf, "%s/%d", startip, mask);

	return (g_buf_alloc(g_buf));
}
*/
char *ip_conv(char *ip_name, int idx)
{
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	sprintf(g_buf, "%s", nvram_safe_get(itemname_arr));
	return (g_buf_alloc(g_buf));
}

char *general_conv(char *ip_name, int idx)
{
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	sprintf(g_buf, "%s", nvram_safe_get(itemname_arr));
	return (g_buf_alloc(g_buf));
}

char *filter_conv(char *proto, char *flag, char *srcip, char *srcport, char *dstip, char *dstport)
{
	char newstr[64];

	_dprintf("filter : %s,%s,%s,%s,%s,%s\n", proto, flag, srcip, srcport, dstip, dstport);
	
	strcpy(g_buf, "");	
										
	if (strcmp(proto, "")!=0)
	{
		sprintf(newstr, " -p %s", proto);
		strcat(g_buf, newstr);
	}				

	if (strcmp(flag, "")!=0)
	{
		//sprintf(newstr, " --tcp-flags %s RST", flag);
		sprintf(newstr, " --tcp-flags %s %s", flag, flag);
		strcat(g_buf, newstr);
	}			
		 
	if (strcmp(srcip, "")!=0)
	{
		if (strchr(srcip , '-'))
			sprintf(newstr, " --src-range %s", srcip);
		else	
			sprintf(newstr, " -s %s", srcip);
		strcat(g_buf, newstr);
	}				

	if (strcmp(srcport, "")!=0)
	{
		sprintf(newstr, " --sport %s", srcport);
		strcat(g_buf, newstr);
	}			

	if (strcmp(dstip, "")!=0)
	{
		if (strchr(dstip, '-'))
			sprintf(newstr, " --dst-range %s", dstip);
		else	
			sprintf(newstr, " -d %s", dstip);
		strcat(g_buf, newstr);
	}
			
	if (strcmp(dstport, "")!=0)
	{
		sprintf(newstr, " --dport %s", dstport);
		strcat(g_buf, newstr);
	}
	return (g_buf);
	//printf("str: %s\n", g_buf);
}

/* 
 * ret : 0 : invalid format
 * ret : 1 : valid format
 */

int timematch_conv(char *mstr, char *nv_date, char *nv_time)
{
	char *datestr[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	char timestart[6], timestop[6];
	char *time, *date;
	int i, head;
	int ret = 1;

	date = nvram_safe_get(nv_date);
	time = nvram_safe_get(nv_time);


	if (strlen(date)!=7||strlen(time)!=8) {
		ret = 0;
		goto no_match;
	}

	if (strncmp(date, "0000000", 7)==0) {
		ret = 0;
		goto no_match;
	}
	
	if (strncmp(date, "1111111", 7)==0 &&
	    strncmp(time, "00002359", 8)==0) goto no_match;
	
	i=0;
	strncpy(timestart, time, 2);
	i+=2;
	timestart[i++] = ':';
	strncpy(timestart+i, time+2, 2);
	i+=2;
	timestart[i]=0;
	i=0;
	strncpy(timestop, time+4, 2);
	i+=2;
	timestop[i++] = ':';
	strncpy(timestop+i, time+6, 2);
	i+=2;
	timestop[i]=0;


	if(strcmp(timestart, timestop)==0) {
		ret = 0;
		goto no_match;
	}

	//sprintf(mstr, "-m time --timestart %s:00 --timestop %s:00 --days ",
	sprintf(mstr, "-m time --timestart %s --timestop %s --days ",
			timestart, timestop);

	head=1;

	for (i=0;i<7;i++)
	{
		if (*(date+i)=='1')
		{
			if (head)
			{
				sprintf(mstr, "%s %s", mstr, datestr[i]);
				head=0;
			}
			else
			{	
				sprintf(mstr, "%s,%s", mstr, datestr[i]);
			}
		}
	}
	return ret;
	
no_match:
	//sprintf(mstr, "");
	mstr[0] = 0;	// oleg patch
	return ret;
}

char *iprange_ex_conv(char *ip_name, int idx)
{
	char *ip;
	char itemname_arr[32];
	char startip[16], endip[16];
	int i, j, k;
	int mask;

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	ip=nvram_safe_get(itemname_arr);
	strcpy(g_buf, "");

	//printf("## iprange_ex_conv: %s, %d, %s\n", ip_name, idx, ip);	// tmp test
	// scan all ip string
	i=j=k=0;
	mask=32;

	while (*(ip+i))
	{
		if (*(ip+i)=='*')
		{
			startip[j++] = '0';
			endip[k++] = '0';
			// 255 is for broadcast
			mask-=8;
		}
		else
		{
			startip[j++] = *(ip+i);
			endip[k++] = *(ip+i);
		}
		++i;
	}

	startip[j++] = 0;
	endip[k++] = 0;

	if (mask==32)
		sprintf(g_buf, "%s", startip);
	else if (mask==0)
		strcpy(g_buf, "");
	else sprintf(g_buf, "%s/%d", startip, mask);

	//printf("\nmask is %d, g_buf is %s\n", mask, g_buf);	// tmp test
	return (g_buf_alloc(g_buf));
}

void
p(int step)
{
	_dprintf("P: %d %s\n", step, g_buf);
}

void 
ip2class(char *lan_ip, char *netmask, char *buf)
{
	unsigned int val, ip;
	struct in_addr in;
	int i=0;

	// only handle class A,B,C	
	val = (unsigned int)inet_addr(netmask);
	ip = (unsigned int)inet_addr(lan_ip);
/*
	in.s_addr = ip & val;
	if (val==0xff00000) sprintf(buf, "%s/8", inet_ntoa(in));
	else if (val==0xffff0000) sprintf(buf, "%s/16", inet_ntoa(in));
	else sprintf(buf, "%s/24", inet_ntoa(in));
*/
	// oleg patch ~
	in.s_addr = ip & val;

	for (val = ntohl(val); val; i++) 
		val <<= 1;

	sprintf(buf, "%s/%d", inet_ntoa(in), i);
	// ~ oleg patch
	//_dprintf("ip2class output: %s\n", buf);	
}

void convert_routes(void)
{
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	/* Disable Static if it's not enable */	
	if (nvram_match("sr_enable_x", "0"))
	{
		nvram_set("lan_route", "");
		
		for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);

			nvram_set(strcat_r(prefix, "route", tmp), "");
		}
		return;
	}

	int i;
	char *nv, *nvp, *b;
	char *ip, *netmask, *gateway, *matric, *interface;
	//char wroutes[400], lroutes[400];
	char wroutes[1024], lroutes[1024], mroutes[1024];	// oleg patch

	wroutes[0] = 0;
	lroutes[0] = 0;	
	mroutes[0] = 0;	// oleg patch

	if (nvram_match("sr_enable_x", "1")) {
		nv = nvp = strdup(nvram_safe_get("sr_rulelist"));
		if(nv) {
			while ((b = strsep(&nvp, "<")) != NULL) {
				if((vstrsep(b, ">", &ip, &netmask, &gateway, &matric, &interface) != 5)) continue;

				_dprintf("%x %s %s %s %s %s\n", i, ip, netmask, gateway, matric, interface);

				if (!strcmp(interface, "WAN")) {		
					sprintf(wroutes, "%s %s:%s:%s:%d", wroutes, ip, netmask, gateway, atoi(matric)+1);
				}
				else if (!strcmp(interface, "MAN"))	// oleg patch
				{
					sprintf(mroutes, "%s %s:%s:%s:%d", mroutes, ip, netmask, gateway, atoi(matric)+1);
				}	 
				else if (!strcmp(interface, "LAN"))
				{
					sprintf(lroutes, "%s %s:%s:%s:%d", lroutes, ip, netmask, gateway, atoi(matric)+1);
				}
			}
			free(nv);
		}
	}

	//Roly
	/* Disable Static if it's not enable */
/*	oleg patch
	if (nvram_match("sr_enable_x", "0"))
	{
		wroutes[0] = 0;
		lroutes[0] = 0;
	}
*/
	//printf("route: %s %s\n", lroutes, wroutes);
	nvram_set("lan_route", lroutes);

	for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		nvram_set(strcat_r(prefix, "route", tmp), wroutes);
		nvram_set(strcat_r(prefix, "mroute", tmp), mroutes);	// oleg patch
	}
}

void
//write_upnp_forward(FILE *fp, FILE *fp1, char *wan_if, char *wan_ip, char *lan_if, char *lan_ip, char *lan_class, char *logaccept, char *logdrop)
write_upnp_forward(FILE *fp, char *wan_if, char *wan_ip, char *lan_if, char *lan_ip, char *lan_class, char *logaccept, char *logdrop)	// oleg patch
{
	char name[] = "forward_portXXXXXXXXXX", value[512];
	char *wan_port0, *wan_port1, *lan_ipaddr, *lan_port0, *lan_port1, *proto;
	char *enable, *desc;
	int i;

	/* Set wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1,proto,enable,desc */
	for (i=0 ; i<15 ; i++)
	{
		snprintf(name, sizeof(name), "forward_port%d", i);

		strncpy(value, nvram_safe_get(name), sizeof(value));

		/* Check for LAN IP address specification */
		lan_ipaddr = value;
		wan_port0 = strsep(&lan_ipaddr, ">");
		if (!lan_ipaddr)
			continue;

		/* Check for LAN destination port specification */
		lan_port0 = lan_ipaddr;
		lan_ipaddr = strsep(&lan_port0, ":");
		if (!lan_port0)
			continue;

		/* Check for protocol specification */
		proto = lan_port0;
		lan_port0 = strsep(&proto, ":,");
		if (!proto)
			continue;

		/* Check for enable specification */
		enable = proto;
		proto = strsep(&enable, ":,");
		if (!enable)
			continue;

		/* Check for description specification (optional) */
		desc = enable;
		enable = strsep(&desc, ":,");

		/* Check for WAN destination port range (optional) */
		wan_port1 = wan_port0;
		wan_port0 = strsep(&wan_port1, "-");
		if (!wan_port1)
			wan_port1 = wan_port0;

		/* Check for LAN destination port range (optional) */
		lan_port1 = lan_port0;

		lan_port0 = strsep(&lan_port1, "-");
		if (!lan_port1)
			lan_port1 = lan_port0;

		/* skip if it's disabled */
		if ( strcmp(enable, "off") == 0 )
			continue;

		/* -A PREROUTING -p tcp -m tcp --dport 823 -j DNAT 
				 --to-destination 192.168.1.88:23  */
		if ( !strcmp(proto,"tcp") || !strcmp(proto,"both") )
		{
			//fprintf(fp, "-A PREROUTING -p tcp -m tcp -d %s --dport %s "
			fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %s "	// oleg patch
				  "-j DNAT --to-destination %s:%s\n"
					//, wan_ip, wan_port0, lan_ipaddr, lan_port0);	// oleg patch

			//fprintf(fp1, "-A FORWARD -p tcp "		// oleg patch
			//	 "-m tcp -d %s --dport %s -j %s\n"	// oleg patch
			//	 , lan_ipaddr, lan_port0, logaccept);	// oleg patch
			, wan_port0, lan_ipaddr, lan_port0);		// oleg patch
		}
		if ( !strcmp(proto,"udp") || !strcmp(proto,"both") ) {
			//fprintf(fp, "-A PREROUTING -p udp -m udp -d %s --dport %s "	// oleg patch
			fprintf(fp, "-A VSERVER -p udp -m udp --dport %s "		// oleg patch
				  "-j DNAT --to-destination %s:%s\n"
				  	//, wan_ip, wan_port0, lan_ipaddr, lan_port0);	// oleg patch

			//fprintf(fp1, "-A FORWARD -p udp -m udp -d %s --dport %s -j %s\n"	// oleg patch
			//	 , lan_ipaddr, lan_port0, logaccept);				// oleg patch
			, wan_port0, lan_ipaddr, lan_port0);					// oleg patch
		}
	}
}
/*
char *ipoffset(char *ip, int offset, char *tmp)
{
	unsigned int ip1, ip2, ip3, ip4;

	sscanf(ip, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
	sprintf(tmp, "%d.%d.%d.%d", ip1, ip2, ip3, ip4+offset);

	_dprintf("ip : %s\n", tmp);
	return (tmp);
}
*/

#ifdef RTCONFIG_WIRELESSREPEATER
void repeater_nat_setting(){
	FILE *fp;
	char *lan_ip = nvram_safe_get("lan_ipaddr");

	if((fp = fopen("/tmp/nat_rules", "w")) == NULL)
		return;

	fprintf(fp, "*nat\n"
		":PREROUTING ACCEPT [0:0]\n"
		":POSTROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n");

	fprintf(fp, "-A PREROUTING -d 10.0.0.1 -p tcp --dport 80 -j DNAT --to-destination %s:80\n", lan_ip);
	fprintf(fp, "-A PREROUTING -d %s -p tcp --dport 80 -j DNAT --to-destination %s:80\n", nvram_default_get("lan_ipaddr"), lan_ip);
	fprintf(fp, "-A PREROUTING -p udp --dport 53 -j DNAT --to-destination %s:18018\n", lan_ip);
	fprintf(fp, "COMMIT\n");

	fclose(fp);
	return;
}
#endif

void nat_setting(char *wan_if, char *wan_ip, char *wanx_if, char *wanx_ip, char *lan_if, char *lan_ip, char *logaccept, char *logdrop)	// oleg patch
{
	FILE *fp;		// oleg patch
	char lan_class[32];	// oleg patch
	int i;
	int wan_port;
	char dstips[32], dstports[12];
	char *proto, *protono, *port, *lport, *dstip, *desc;
	char *nv, *nvp, *b;
	int n;
	char *p;

	if ((fp=fopen("/tmp/nat_rules", "w"))==NULL) return;	// oleg patch

	fprintf(fp, "*nat\n"
		":PREROUTING ACCEPT [0:0]\n"
		":POSTROUTING ACCEPT [0:0]\n"
	  	/* ":OUTPUT ACCEPT [0:0]\n"); */
		":OUTPUT ACCEPT [0:0]\n"	// oleg patch
		":VSERVER - [0:0]\n"		// oleg patch
		":VUPNP - [0:0]\n");

	_dprintf("writting prerouting %s %s %s %s %s %s\n", wan_if, wan_ip, wanx_if, wanx_ip, lan_if, lan_ip);

	//Log	
	//if (nvram_match("misc_natlog_x", "1"))
	// 	fprintf(fp, "-A PREROUTING -i %s -j LOG --log-prefix ALERT --log-level 4\n", wan_if);
// oleg patch ~
	/* VSERVER chain */
	if (inet_addr_(wan_ip)) {
		fprintf(fp, "-A PREROUTING -d %s -j VSERVER\n", wan_ip);
		fprintf(fp, "-A PREROUTING -d %s -j VUPNP\n", wan_ip);
	}

	// wanx_if != wan_if means DHCP+PPP exist?
	if (strcmp(wan_if, wanx_if) && inet_addr_(wanx_ip)) {
		fprintf(fp, "-A PREROUTING -d %s -j VSERVER\n", wanx_ip);
// ~ oleg patch
		fprintf(fp, "-A PREROUTING -d %s -j VUPNP\n", wanx_ip);
	}

	// need multiple instance for tis?
	if (nvram_match("misc_http_x", "1"))
	{
		wan_port=8080;
		if (!nvram_match("misc_httpport_x", ""))
			wan_port=atoi(nvram_safe_get("misc_httpport_x")); 	
		//fprintf(fp, "-A PREROUTING -p tcp -m tcp -d %s --dport %d -j DNAT --to-destination %s:80\n", wan_ip, wan_port, lan_ip);
		fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %d -j DNAT --to-destination %s:%s\n",
			wan_port, lan_ip, nvram_safe_get("lan_port"));	// oleg patch
#ifdef RTCONFIG_HTTPS
		wan_port=8443;
		if (!nvram_match("misc_httpsport_x", ""))
			wan_port=atoi(nvram_safe_get("misc_httpsport_x")); 	
		//fprintf(fp, "-A PREROUTING -p tcp -m tcp -d %s --dport %d -j DNAT --to-destination %s:80\n", wan_ip, wan_port, lan_ip);
		fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %d -j DNAT --to-destination %s:%s\n",
			wan_port, lan_ip, nvram_safe_get("https_lanport"));	// oleg patch
#endif	
	}

#ifdef RTCONFIG_WEBDAV
	if (nvram_match("enable_webdav", "1"))
	{
		fprintf(fp, "-A VSERVER -p tcp -m tcp --dport 443 -j DNAT --to-destination %s:%s\n",
			lan_ip, nvram_safe_get("webdav_https_port"));	// oleg patch
	}
#endif	

	if (is_nat_enabled() && nvram_match("upnp_enable", "1"))
	{
		// upnp port forward
		//write_upnp_forward(fp, fp1, wan_if, wan_ip, lan_if, lan_ip, lan_class, logaccept, logdrop);
		write_upnp_forward(fp, wan_if, wan_ip, lan_if, lan_ip, lan_class, logaccept, logdrop);	// oleg patch
	}
	
	// Port forwarding or Virtual Server
	if (is_nat_enabled() && nvram_match("vts_enable_x", "1"))
	{
		nvp = nv = strdup(nvram_safe_get("vts_rulelist"));
		if (nv) { 
		while ((b = strsep(&nvp, "<")) != NULL) {
			if ((vstrsep(b, ">", &desc, &port, &dstip, &lport, &proto) != 5)) continue;

			if (strlen(lport)!=0) 
			{
				sprintf(dstips, "%s:%s", dstip, lport);
				sprintf(dstports, "%s", lport);
			}
			else
			{
				sprintf(dstips, "%s:%s", dstip, port);
				sprintf(dstports, "%s", port);
			}

			if (strcmp(proto, "TCP")==0 || strcmp(proto, "BOTH")==0)
			{
				if (lport!=NULL && strlen(lport)!=0) 
				{
					//fprintf(fp, "-A PREROUTING -p tcp -m tcp -d %s --dport %s -j DNAT --to-destination %s\n", 
					//wan_ip, port, dstips);
					fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %s -j DNAT --to-destination %s\n",
					port, dstips);	// oleg patch
				}
				else
				{
					//fprintf(fp, "-A PREROUTING -p tcp -m tcp -d %s --dport %s -j DNAT --to %s\n", 
					//wan_ip, port, dstip);
					fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %s -j DNAT --to %s\n",
					port, dstip);	// oleg patch
				}

				//fprintf(fp1, "-A FORWARD -p tcp -m tcp -d %s --dport %s -j %s\n", 
				//	dstip, dstports, logaccept);
				//fprintf(fp, "-A FORWARD -p tcp -m tcp -d %s --dport %s -j %s\n",  dstip, dstports, logaccept);	// add back for conntrack patch
			}		
				
			if (strcmp(proto, "UDP")==0 || strcmp(proto, "BOTH")==0)
			{
				if (strlen(lport)!=0) 
				{
					//fprintf(fp, "-A PREROUTING -p udp -m udp -d %s --dport %s -j DNAT --to-destination %s\n", 
					//wan_ip, port, dstips);
					fprintf(fp, "-A VSERVER -p udp -m udp --dport %s -j DNAT --to-destination %s\n",
					port, dstips);	// oleg patch
				}
				else
				{
					//fprintf(fp, "-A PREROUTING -p udp -m udp -d %s --dport %s -j DNAT --to %s\n", 
					//wan_ip, port, dstip);
					fprintf(fp, "-A VSERVER -p udp -m udp --dport %s -j DNAT --to %s\n", port, dstip);	// oleg patch

				}
				//fprintf(fp1, "-A FORWARD -p udp -m udp -d %s --dport %s -j %s\n", 
				//	dstip, dstports, logaccept);	// oleg patch
				//fprintf(fp, "-A FORWARD -p udp -m udp -d %s --dport %s -j %s\n", dstip, dstports, logaccept);	// add back for conntrack patch
			}						
			//if (strcmp(proto, "OTHER")==0)
			//{
			//
			//	//fprintf(fp, "-A PREROUTING -p %s -d %s -j DNAT --to %s\n",
			//	//	protono, wan_ip, dstip);
			//
			//	//fprintf(fp, "-A FORWARD -p %s -d %s -j %s\n", protono, dstip, logaccept);	// add back for conntrack patch
			//
			//	fprintf(fp, "-A VSERVER -p %s -j DNAT --to %s\n",
			//		protono, dstip);	// oleg patch
			//}
		 }
		 free(nv);	
		 }
	}	

	if (is_nat_enabled() && nvram_match("autofw_enable_x", "1"))
	{
		/* Trigger port setting */
		write_porttrigger(fp, wan_if, 1);
	}


#if 0	
	if (is_nat_enabled() && !nvram_match("sp_battle_ips", "0") && inet_addr_(wan_ip))	// oleg patch
	{
		#define BASEPORT 6112
		#define BASEPORT_NEW 10000

		ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);

		/* run starcraft patch anyway */
		fprintf(fp, "-A PREROUTING -p udp -d %s --sport %d -j NETMAP --to %s\n", wan_ip, BASEPORT, lan_class);

		fprintf(fp, "-A POSTROUTING -p udp -s %s --dport %d -j NETMAP --to %s\n", lan_class, BASEPORT, wan_ip);

		//fprintf(fp, "-A FORWARD -p udp --dport %d -j %s\n",
		//			BASEPORT, logaccept);	// oleg patch
	}
#endif

//#endif
TRACE_PT("writing dmz\n");

	// Exposed station	
	if (is_nat_enabled() && !nvram_match("dmz_ip", ""))
	{		
/*	oleg patch
		fprintf(fp, "-A PREROUTING -d %s -j DNAT --to %s\n", 
			wan_ip, nvram_safe_get("dmz_ip"));

 		fprintf(fp1, "-A FORWARD -d %s -j %s\n", 
			nvram_safe_get("dmz_ip"), logaccept);
*/
		fprintf(fp, "-A VSERVER -j DNAT --to %s\n", nvram_safe_get("dmz_ip"));	// oleg patch
	}

	if (is_nat_enabled())
	{
		p = "";
#ifdef RTCONFIG_IPV6
		switch (get_ipv6_service()) {
		case IPV6_6IN4:
			// avoid NATing proto-41 packets when using 6in4 tunnel
			p = "-p ! 41";
			break;
		}
#endif

		// oleg patch ~
		if (inet_addr_(wan_ip))
			fprintf(fp, "-A POSTROUTING %s -o %s ! -s %s -j MASQUERADE\n", p, wan_if, wan_ip); 


		/* masquerade physical WAN port connection */
		if (strcmp(wan_if, wanx_if) && inet_addr_(wanx_ip))
			fprintf(fp, "-A POSTROUTING %s -o %s ! -s %s -j MASQUERADE\n", 
				p, wanx_if, wanx_ip);
		// ~ oleg patch


		// masquerade lan to lan
		ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);
		fprintf(fp, "-A POSTROUTING %s -o %s -s %s -d %s -j MASQUERADE\n", p, lan_if, lan_class, lan_class);
	}

	fprintf(fp, "COMMIT\n");
	
	fclose(fp);
	//fclose(fp1);	// oleg patch

	// force nat update
	nvram_set_int("nat_state", NAT_STATE_UPDATE);

	start_nat_rules();
}

#ifdef RTCONFIG_USB_MODEM
void nat_setting2(char *lan_if, char *lan_ip, char *logaccept, char *logdrop)	// oleg patch
{
	FILE *fp;		// oleg patch
	char lan_class[32];	// oleg patch
	int i;
	int wan_port;
	char dstips[32], dstports[12];
	char *proto, *protono, *port, *lport, *dstip, *desc;
	char *nv, *nvp, *b;
	int n;
	char *wan_if, *wan_ip;
	char *wanx_if, *wanx_ip;
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *p;

	if ((fp=fopen("/tmp/nat_rules", "w"))==NULL) return;	// oleg patch

	fprintf(fp, "*nat\n"
		":PREROUTING ACCEPT [0:0]\n"
		":POSTROUTING ACCEPT [0:0]\n"
	  	/* ":OUTPUT ACCEPT [0:0]\n"); */
		":OUTPUT ACCEPT [0:0]\n"	// oleg patch
		":VSERVER - [0:0]\n"
		":VUPNP - [0:0]\n");		// oleg patch

	for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
			continue;

		wan_if = get_wan_ifname(unit);
		wan_ip = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));
		wanx_if = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		wanx_ip = nvram_safe_get(strcat_r(prefix, "xipaddr", tmp));

		_dprintf("writting prerouting %s %s %s %s %s %s\n", wan_if, wan_ip, wanx_if, wanx_ip, lan_if, lan_ip);

		//Log	
		//if (nvram_match("misc_natlog_x", "1"))
		//	fprintf(fp, "-A PREROUTING -i %s -j LOG --log-prefix ALERT --log-level 4\n", wan_if);

// oleg patch ~
		/* VSERVER chain */
		if (inet_addr_(wan_ip)) {
			fprintf(fp, "-A PREROUTING -d %s -j VSERVER\n", wan_ip);
			fprintf(fp, "-A PREROUTING -d %s -j VUPNP\n", wan_ip);
		}

		// wanx_if != wan_if means DHCP+PPP exist?
		if (unit == WAN_UNIT_WANPORT1 && strcmp(wan_if, wanx_if) && inet_addr_(wanx_ip)) {
			fprintf(fp, "-A PREROUTING -d %s -j VSERVER\n", wanx_ip);
// ~ oleg patch
			fprintf(fp, "-A PREROUTING -d %s -j VUPNP\n", wanx_ip);
		}
	}

	// need multiple instance for tis?
	if (nvram_match("misc_http_x", "1"))
	{
		wan_port=8080;
		if (!nvram_match("misc_httpport_x", ""))
			wan_port=atoi(nvram_safe_get("misc_httpport_x")); 	
		//fprintf(fp, "-A PREROUTING -p tcp -m tcp -d %s --dport %d -j DNAT --to-destination %s:80\n", wan_ip, wan_port, lan_ip);
		fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %d -j DNAT --to-destination %s:%s\n",
				wan_port, lan_ip, nvram_safe_get("lan_port"));	// oleg patch
#ifdef RTCONFIG_HTTPS
		wan_port=8443;
		if (!nvram_match("misc_httpsport_x", ""))
			wan_port=atoi(nvram_safe_get("misc_httpsport_x")); 	
		//fprintf(fp, "-A PREROUTING -p tcp -m tcp -d %s --dport %d -j DNAT --to-destination %s:80\n", wan_ip, wan_port, lan_ip);
		fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %d -j DNAT --to-destination %s:%s\n",
			wan_port, lan_ip, nvram_safe_get("https_lanport"));	// oleg patch
#endif
	}

#ifdef RTCONFIG_WEBDAV
	if (nvram_match("enable_webdav", "1"))
	{
		fprintf(fp, "-A VSERVER -p tcp -m tcp --dport 443 -j DNAT --to-destination %s:%s\n",
				lan_ip, nvram_safe_get("webdav_https_port"));	// oleg patch
	}
#endif	

	for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
			continue;

		wan_if = get_wan_ifname(unit);
		wan_ip = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));

		if (is_nat_enabled() && nvram_match("upnp_enable", "1"))
		{
			// upnp port forward
			//write_upnp_forward(fp, fp1, wan_if, wan_ip, lan_if, lan_ip, lan_class, logaccept, logdrop);
			write_upnp_forward(fp, wan_if, wan_ip, lan_if, lan_ip, lan_class, logaccept, logdrop);	// oleg patch
		}
	}

	// Port forwarding or Virtual Server
	if (is_nat_enabled() && nvram_match("vts_enable_x", "1"))
	{
		nvp = nv = strdup(nvram_safe_get("vts_rulelist"));
		if (nv) { 
			while ((b = strsep(&nvp, "<")) != NULL) {
				if ((vstrsep(b, ">", &desc, &port, &dstip, &lport, &proto) != 5)) continue;

				if (strlen(lport)!=0) 
				{
					sprintf(dstips, "%s:%s", dstip, lport);
					sprintf(dstports, "%s", lport);
				}
				else
				{
					sprintf(dstips, "%s:%s", dstip, port);
					sprintf(dstports, "%s", port);
				}

				if (strcmp(proto, "TCP")==0 || strcmp(proto, "BOTH")==0)
				{
					if (lport!=NULL && strlen(lport)!=0) 
					{
						//fprintf(fp, "-A PREROUTING -p tcp -m tcp -d %s --dport %s -j DNAT --to-destination %s\n", 
						//wan_ip, port, dstips);
						fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %s -j DNAT --to-destination %s\n",
						port, dstips);	// oleg patch
					}
					else
					{
						//fprintf(fp, "-A PREROUTING -p tcp -m tcp -d %s --dport %s -j DNAT --to %s\n", 
						//wan_ip, port, dstip);
						fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %s -j DNAT --to %s\n",
						port, dstip);	// oleg patch
					}

					//fprintf(fp1, "-A FORWARD -p tcp -m tcp -d %s --dport %s -j %s\n", 
					//	dstip, dstports, logaccept);
					//fprintf(fp, "-A FORWARD -p tcp -m tcp -d %s --dport %s -j %s\n",  dstip, dstports, logaccept);	// add back for conntrack patch
				}

				if (strcmp(proto, "UDP")==0 || strcmp(proto, "BOTH")==0)
				{
					if (strlen(lport)!=0) 
					{
						//fprintf(fp, "-A PREROUTING -p udp -m udp -d %s --dport %s -j DNAT --to-destination %s\n", 
						//wan_ip, port, dstips);
						fprintf(fp, "-A VSERVER -p udp -m udp --dport %s -j DNAT --to-destination %s\n",
						port, dstips);	// oleg patch
					}
					else
					{
						//fprintf(fp, "-A PREROUTING -p udp -m udp -d %s --dport %s -j DNAT --to %s\n", 
						//wan_ip, port, dstip);
						fprintf(fp, "-A VSERVER -p udp -m udp --dport %s -j DNAT --to %s\n", port, dstip);	// oleg patch
					}

					//fprintf(fp1, "-A FORWARD -p udp -m udp -d %s --dport %s -j %s\n", 
					//	dstip, dstports, logaccept);	// oleg patch
					//fprintf(fp, "-A FORWARD -p udp -m udp -d %s --dport %s -j %s\n", dstip, dstports, logaccept);	// add back for conntrack patch
				}

				//if (strcmp(proto, "OTHER")==0)
				//{
				//	fprintf(fp, "-A PREROUTING -p %s -d %s -j DNAT --to %s\n", protono, wan_ip, dstip);
				//	fprintf(fp, "-A FORWARD -p %s -d %s -j %s\n", protono, dstip, logaccept);	// add back for conntrack patch
				//	fprintf(fp, "-A VSERVER -p %s -j DNAT --to %s\n", protono, dstip);	// oleg patch
				//}
			}
			free(nv);	
		}
	}

	for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
			continue;

		wan_if = get_wan_ifname(unit);

		if (is_nat_enabled() && nvram_match("autofw_enable_x", "1"))
		{
			/* Trigger port setting */
			write_porttrigger(fp, wan_if, 1);
		}
	}

#if 0
	if (is_nat_enabled() && !nvram_match("sp_battle_ips", "0") && inet_addr_(wan_ip))	// oleg patch
	{
#define BASEPORT 6112
#define BASEPORT_NEW 10000

		ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);

		/* run starcraft patch anyway */
		fprintf(fp, "-A PREROUTING -p udp -d %s --sport %d -j NETMAP --to %s\n", wan_ip, BASEPORT, lan_class);

		fprintf(fp, "-A POSTROUTING -p udp -s %s --dport %d -j NETMAP --to %s\n", lan_class, BASEPORT, wan_ip);

		//fprintf(fp, "-A FORWARD -p udp --dport %d -j %s\n",
		//			BASEPORT, logaccept);	// oleg patch
	}
#endif

//#endif
TRACE_PT("writing dmz\n");

	// Exposed station	
	if (is_nat_enabled() && !nvram_match("dmz_ip", ""))
	{
/*	oleg patch
		fprintf(fp, "-A PREROUTING -d %s -j DNAT --to %s\n", 
				wan_ip, nvram_safe_get("dmz_ip"));

		fprintf(fp1, "-A FORWARD -d %s -j %s\n", 
				nvram_safe_get("dmz_ip"), logaccept);
*/
		fprintf(fp, "-A VSERVER -j DNAT --to %s\n", nvram_safe_get("dmz_ip"));	// oleg patch
	}

	if (is_nat_enabled())
	{
#ifdef RTCONFIG_IPV6
		switch (get_ipv6_service()) {
		case IPV6_6IN4:
			// avoid NATing proto-41 packets when using 6in4 tunnel
			p = "-p ! 41";
			break;
		}
#endif

		for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
				continue;

			wan_if = get_wan_ifname(unit);
			wan_ip = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));
			wanx_if = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
			wanx_ip = nvram_safe_get(strcat_r(prefix, "xipaddr", tmp));

			// oleg patch ~
			if (inet_addr_(wan_ip))
				fprintf(fp, "-A POSTROUTING %s -o %s ! -s %s -j MASQUERADE\n", p, wan_if, wan_ip); 

			/* masquerade physical WAN port connection */
			if (unit == WAN_UNIT_WANPORT1 && strcmp(wan_if, wanx_if) && inet_addr_(wanx_ip))
				fprintf(fp, "-A POSTROUTING %s -o %s ! -s %s -j MASQUERADE\n", p, wanx_if, wanx_ip);
			// ~ oleg patch

		}

		// masquerade lan to lan
		ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);
		fprintf(fp, "-A POSTROUTING %s -o %s -s %s -d %s -j MASQUERADE\n", p, lan_if, lan_class, lan_class);
	}

	fprintf(fp, "COMMIT\n");
	
	fclose(fp);
	//fclose(fp1);	// oleg patch

	// force nat update
	nvram_set_int("nat_state", NAT_STATE_UPDATE);
	
	start_nat_rules();
}
#endif

#ifdef WEB_REDIRECT
void redirect_setting()
{
	FILE *nat_fp, *redirect_fp;
	char tmp_buf[1024];
	char http_rule[256], dns_rule[256];
	char *lan_ipaddr_t = nvram_safe_get("lan_ipaddr");
	char *lan_netmask_t = nvram_safe_get("lan_netmask");

	if ((redirect_fp = fopen("/tmp/redirect_rules", "w+")) == NULL) {
		fprintf(stderr, "*** Can't make the file of the redirect rules! ***\n");
		return;
	}

	if(nvram_get_int("sw_mode") == SW_MODE_ROUTER && (nat_fp = fopen("/tmp/nat_rules", "r")) != NULL) {
		memset(tmp_buf, 0, sizeof(tmp_buf));
		while ((fgets(tmp_buf, sizeof(tmp_buf), nat_fp)) != NULL
				&& strncmp(tmp_buf, "COMMIT", 6) != 0) {
			fprintf(redirect_fp, "%s", tmp_buf);
			memset(tmp_buf, 0, sizeof(tmp_buf));
		}

		fclose(nat_fp);
	}
	else{
		fprintf(redirect_fp, "*nat\n"
				":PREROUTING ACCEPT [0:0]\n"
				":POSTROUTING ACCEPT [0:0]\n"
				":OUTPUT ACCEPT [0:0]\n"
				":VSERVER - [0:0]\n");
	}

	memset(http_rule, 0, sizeof(http_rule));
	memset(dns_rule, 0, sizeof(dns_rule));
	sprintf(http_rule, "-A PREROUTING -d ! %s/%s -p tcp --dport 80 -j DNAT --to-destination %s:18017\n", lan_ipaddr_t, lan_netmask_t, lan_ipaddr_t);
	sprintf(dns_rule, "-A PREROUTING -p udp --dport 53 -j DNAT --to-destination %s:18018\n", lan_ipaddr_t);

	fprintf(redirect_fp, "%s%s", http_rule, dns_rule);
	fprintf(redirect_fp, "COMMIT\n");

	fclose(redirect_fp);

	// notice if wanduck need to re-build the rules.
	// kill_pidfile_s("/var/run/wanduck.pid", SIGUSR2);
	// do it by stop_nat_rules directly or other event trigger by wanduck
}
#endif

/* Rules for LW Filter and MAC Filter
 * MAC ACCEPT
 *     ACCEPT -> MACS
 *             -> LW Disabled
 *                MACS ACCEPT
 *             -> LW Default Accept: 
 *                MACS DROP in rules
 *                MACS ACCEPT Default
 *             -> LW Default Drop: 
 *                MACS ACCEPT in rules
 *                MACS DROP Default
 *     DROP   -> FORWARD DROP 
 *
 * MAC DROP
 *     DROP -> FORWARD DROP
 *     ACCEPT -> FORWARD ACCEPT 
 */

int 	// 0928 add
start_default_filter(int lanunit)
{
	//TODO handle multiple lan 
	FILE *fp;

	printf("\nset default filter settings\n");	// tmp test
	if ((fp=fopen("/tmp/filter.default", "w"))==NULL) return -1;

	fprintf(fp, "*filter\n:INPUT ACCEPT [0:0]\n:FORWARD ACCEPT [0:0]\n:OUTPUT ACCEPT [0:0]\n:logaccept - [0:0]\n:logdrop - [0:0]\n");
	fprintf(fp, "-A INPUT -m state --state INVALID -j DROP\n");
	fprintf(fp, "-A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT\n");
	fprintf(fp, "-A INPUT -i lo -m state --state NEW -j ACCEPT\n");
	fprintf(fp, "-A INPUT -i br0 -m state --state NEW -j ACCEPT\n");
	fprintf(fp, "-A INPUT -j DROP\n");
	fprintf(fp, "-A FORWARD -m state --state INVALID -j DROP\n");
	fprintf(fp, "-A FORWARD -m state --state ESTABLISHED,RELATED -j ACCEPT\n");
	fprintf(fp, "-A FORWARD -i br0 -o br0 -j ACCEPT\n");
	fprintf(fp, "-A FORWARD -i lo -o lo -j ACCEPT\n");
	fprintf(fp, "-A FORWARD -j DROP\n");
	fprintf(fp, "-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logaccept -j ACCEPT\n");

	fprintf(fp,"-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP\" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logdrop -j DROP\n");
	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	//system("iptables -F"); 
	xstart("iptables-restore", "/tmp/filter.default");
}

#ifdef WEBSTRFILTER
/* url filter corss midnight patch start */
int makeTimestr(char *tf)
{
	char *url_time = nvram_get("url_time_x");
	char *url_date = nvram_get("url_date_x");
	static const char *days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	int i, comma = 0;

	memset(tf, 0, 256);

	if (!nvram_match("url_enable_x", "1"))
		return -1;

	if ((!url_date) || strlen(url_date) != 7 || !strcmp(url_date, "0000000"))
	{
		printf("url filter get time fail\n");
		return -1;
	}

	sprintf(tf, "-m time --timestart %c%c:%c%c --timestop %c%c:%c%c --days ", url_time[0], url_time[1], url_time[2], url_time[3], url_time[4], url_time[5], url_time[6], url_time[7]);
//	sprintf(tf, " -m time --timestart %c%c:%c%c --timestop %c%c:%c%c --days ", url_time[0], url_time[1], url_time[2], url_time[3], url_time[4], url_time[5], url_time[6], url_time[7]);

	for (i=0; i<7; ++i)
	{
		if (url_date[i] == '1')
		{
			if (comma == 1)
				strncat(tf, ",", 1);

			strncat(tf, days[i], 3);
			comma = 1;
		}
	}

	_dprintf("# url filter time module str is [%s]\n", tf);	// tmp test
	return 0;
}

int makeTimestr2(char *tf)
{
	char *url_time = nvram_get("url_time_x_1");
	char *url_date = nvram_get("url_date_x");
	static const char *days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	int i, comma = 0;

	memset(tf, 0, 256);

	if (!nvram_match("url_enable_x_1", "1"))
		return -1;

	if ((!url_date) || strlen(url_date) != 7 || !strcmp(url_date, "0000000"))
	{
		printf("url filter get time fail\n");
		return -1;
	}

	sprintf(tf, "-m time --timestart %c%c:%c%c --timestop %c%c:%c%c --days ", url_time[0], url_time[1], url_time[2], url_time[3], url_time[4], url_time[5], url_time[6], url_time[7]);

	for (i=0; i<7; ++i)
	{
		if (url_date[i] == '1')
		{
			if (comma == 1)
				strncat(tf, ",", 1);

			strncat(tf, days[i], 3);
			comma = 1;
		}
	}

	_dprintf("# url filter time module str is [%s]\n", tf);	// tmp test
	return 0;
}

int
valid_url_filter_time()
{
	char *url_time1 = nvram_get("url_time_x");
	char *url_time2 = nvram_get("url_time_x_1");
	char starttime1[5], endtime1[5];
	char starttime2[5], endtime2[5];

	memset(starttime1, 0, 5);
	memset(endtime1, 0, 5);
	memset(starttime2, 0, 5);
	memset(endtime2, 0, 5);

	if (!nvram_match("url_enable_x", "1") && !nvram_match("url_enable_x_1", "1"))
		return 0;

	if (nvram_match("url_enable_x", "1"))
	{
		if ((!url_time1) || strlen(url_time1) != 8)
			goto err;

		strncpy(starttime1, url_time1, 4);
		strncpy(endtime1, url_time1 + 4, 4);
		_dprintf("starttime1: %s\n", starttime1);
		_dprintf("endtime1: %s\n", endtime1);

		if (atoi(starttime1) > atoi(endtime1))
			goto err;
	}

	if (nvram_match("url_enable_x_1", "1"))
	{
		if ((!url_time2) || strlen(url_time2) != 8)
			goto err;

		strncpy(starttime2, url_time2, 4);
		strncpy(endtime2, url_time2 + 4, 4);
		_dprintf("starttime2: %s\n", starttime2);
		_dprintf("endtime2: %s\n", endtime2);

		if (atoi(starttime2) > atoi(endtime2))
			goto err;
	}

	if (nvram_match("url_enable_x", "1") && nvram_match("url_enable_x_1", "1"))
	{
		if ((atoi(starttime1) > atoi(starttime2)) && 
			((atoi(starttime2) > atoi(endtime1)) || (atoi(endtime2) > atoi(endtime1))))
			goto err;

		if ((atoi(starttime2) > atoi(starttime1)) && 
			((atoi(starttime1) > atoi(endtime2)) || (atoi(endtime1) > atoi(endtime2))))
			goto err;
	}

	return 1;

err:
	_dprintf("invalid url filter time setting!\n");
	return 0;
}
/* url filter corss midnight patch end */
#endif

#ifdef CONTENTFILTER
int makeTimestr_content(char *tf)
{
	char *keyword_time = nvram_get("keyword_time_x");
	char *keyword_date = nvram_get("keyword_date_x");
	static const char *days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	int i, comma = 0;

	memset(tf, 0, 256);

	if (!nvram_match("keyword_enable_x", "1"))
		return -1;

	if ((!keyword_date) || strlen(keyword_date) != 7 || !strcmp(keyword_date, "0000000"))
	{
		printf("content filter get time fail\n");
		return -1;
	}

	sprintf(tf, "-m time --timestart %c%c:%c%c --timestop %c%c:%c%c --days ", keyword_time[0], keyword_time[1], keyword_time[2], keyword_time[3], keyword_time[4], keyword_time[5], keyword_time[6], keyword_time[7]);

	for (i=0; i<7; ++i)
	{
		if (keyword_date[i] == '1')
		{
			if (comma == 1)
				strncat(tf, ",", 1);

			strncat(tf, days[i], 3);
			comma = 1;
		}
	}

	printf("# content filter time module str is [%s]\n", tf);	// tmp test
	return 0;
}

int makeTimestr2_content(char *tf)
{
	char *keyword_time = nvram_get("keyword_time_x_1");
	char *keyword_date = nvram_get("keyword_date_x");
	static const char *days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	int i, comma = 0;

	memset(tf, 0, 256);

	if (!nvram_match("keyword_enable_x_1", "1"))
		return -1;

	if ((!keyword_date) || strlen(keyword_date) != 7 || !strcmp(keyword_date, "0000000"))
	{
		printf("content filter get time fail\n");
		return -1;
	}

	sprintf(tf, "-m time --timestart %c%c:%c%c --timestop %c%c:%c%c --days ", keyword_time[0], keyword_time[1], keyword_time[2], keyword_time[3], keyword_time[4], keyword_time[5], keyword_time[6], keyword_time[7]);

	for (i=0; i<7; ++i)
	{
		if (keyword_date[i] == '1')
		{
			if (comma == 1)
				strncat(tf, ",", 1);

			strncat(tf, days[i], 3);
			comma = 1;
		}
	}

	printf("# content filter time module str is [%s]\n", tf);	// tmp test
	return 0;
}

int
valid_keyword_filter_time()
{
	char *keyword_time1 = nvram_get("keyword_time_x");
	char *keyword_time2 = nvram_get("keyword_time_x_1");
	char starttime1[5], endtime1[5];
	char starttime2[5], endtime2[5];

	memset(starttime1, 0, 5);
	memset(endtime1, 0, 5);
	memset(starttime2, 0, 5);
	memset(endtime2, 0, 5);

	if (!nvram_match("keyword_enable_x", "1") && !nvram_match("keyword_enable_x_1", "1"))
		return 0;

	if (nvram_match("keyword_enable_x", "1"))
	{
		if ((!keyword_time1) || strlen(keyword_time1) != 8)
			goto err;

		strncpy(starttime1, keyword_time1, 4);
		strncpy(endtime1, keyword_time1 + 4, 4);
		printf("starttime1: %s\n", starttime1);
		printf("endtime1: %s\n", endtime1);

		if (atoi(starttime1) > atoi(endtime1))
			goto err;
	}

	if (nvram_match("keyword_enable_x_1", "1"))
	{
		if ((!keyword_time2) || strlen(keyword_time2) != 8)
			goto err;

		strncpy(starttime2, keyword_time2, 4);
		strncpy(endtime2, keyword_time2 + 4, 4);
		printf("starttime2: %s\n", starttime2);
		printf("endtime2: %s\n", endtime2);

		if (atoi(starttime2) > atoi(endtime2))
			goto err;
	}

	if (nvram_match("keyword_enable_x", "1") && nvram_match("keyword_enable_x_1", "1"))
	{
		if ((atoi(starttime1) > atoi(starttime2)) && 
			((atoi(starttime2) > atoi(endtime1)) || (atoi(endtime2) > atoi(endtime1))))
			goto err;

		if ((atoi(starttime2) > atoi(starttime1)) && 
			((atoi(starttime1) > atoi(endtime2)) || (atoi(endtime1) > atoi(endtime2))))
			goto err;
	}

	return 1;

err:
	printf("invalid content filter time setting!\n");
	return 0;
}
#endif

int
filter_setting(char *wan_if, char *wan_ip, char *lan_if, char *lan_ip, char *logaccept, char *logdrop)
{
	FILE *fp;	// oleg patch
#ifdef RTCONFIG_IPV6
	FILE *fp_ipv6;
#endif
	char *proto, *flag, *srcip, *srcport, *dstip, *dstport, *mac;
	char *nv, *nvp, *b;
	char *setting, line[256];
	char macaccept[32], chain[32];
	char *ftype, *dtype, *fftype;
	int num;
	int i;
	char prefix[32], tmp[100], *wan_proto, *wan_ipaddr;
#ifdef RTCONFIG_OLD_PARENTALCTRL
	char parental_ctrl_timematch[128];
	char parental_ctrl_nv_date[128];
	char parental_ctrl_nv_time[128];
	char parental_ctrl_enable_status[128];
#endif	/* RTCONFIG_OLD_PARENTALCTRL */
#ifdef RTCONFIG_IPV6
	char s[128];
	char t[512];
	char *en;
	char *sec;
	char *hit;
	int n;	
	char *p, *c;
#endif
	int v4v6_ok;

//2008.09 magic{
#ifdef WEBSTRFILTER
	char nvname[36], timef[32], timef2[32], *filterstr;
#endif
//2008.09 magic}
	int unit = get_wan_unit(wan_if);
	
	if(wan_prefix(wan_if, prefix) < 0)
		sprintf(prefix, "wan%d_", WAN_UNIT_WANPORT1);

	wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));
	wan_ipaddr = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));

	//if(!strlen(wan_proto)) return -1;

	if ((fp=fopen("/tmp/filter_rules", "w"))==NULL) return -1;
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled() && (fp_ipv6=fopen("/tmp/filter_rules_ipv6", "w"))==NULL) return -2;
#endif

	fprintf(fp, "*filter\n:INPUT ACCEPT [0:0]\n:FORWARD ACCEPT [0:0]\n:OUTPUT ACCEPT [0:0]\n:FUPNP - [0:0]\n:MACS - [0:0]\n:logaccept - [0:0]\n:logdrop - [0:0]\n");
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	fprintf(fp_ipv6, "*filter\n:INPUT ACCEPT [0:0]\n:FORWARD ACCEPT [0:0]\n:OUTPUT ACCEPT [0:0]\n:MACS - [0:0]\n:logaccept - [0:0]\n:logdrop - [0:0]\n");
#endif

	strcpy(macaccept, "");

#ifdef RTCONFIG_OLD_PARENTALCTRL
	parental_ctrl();
#endif	/* RTCONFIG_OLD_PARENTALCTRL */

#ifdef RTCONFIG_PARENTALCTRL
	if(nvram_get_int("MULTIFILTER_ALL") != 0){
TRACE_PT("writing Parental Control\n");
		config_daytime_string(fp);
	}
#endif

	// FILTER from LAN to WAN Source MAC
	if (!nvram_match("macfilter_enable_x", "0"))
	{		
		// LAN/WAN filter		
		
		if (nvram_match("macfilter_enable_x", "2"))
		{
			dtype = logaccept;
			ftype = logdrop;
			fftype = logdrop;
		}
		else
		{
			dtype = logdrop;
			ftype = logaccept;

			strcpy(macaccept, "MACS");
			fftype = macaccept;
		}

#ifdef RTCONFIG_OLD_PARENTALCTRL
		num = atoi(nvram_safe_get("macfilter_num_x"));

		for(i = 0; i < num; ++i)
		{
			snprintf(parental_ctrl_nv_date, sizeof(parental_ctrl_nv_date),
						"macfilter_date_x%d", i);
			snprintf(parental_ctrl_nv_time, sizeof(parental_ctrl_nv_time),
						"macfilter_time_x%d", i);
			snprintf(parental_ctrl_enable_status,
						sizeof(parental_ctrl_enable_status),
						"macfilter_enable_status_x%d", i);
			if(!nvram_match(parental_ctrl_nv_date, "0000000") &&
				nvram_match(parental_ctrl_enable_status, "1")){
				timematch_conv(parental_ctrl_timematch,
									parental_ctrl_nv_date, parental_ctrl_nv_time);
				fprintf(fp, "-A INPUT -i %s %s -m mac --mac-source %s -j %s\n", lan_if, parental_ctrl_timematch, nvram_get_by_seq("macfilter_list_x", i), ftype);
				fprintf(fp, "-A FORWARD -i %s %s -m mac --mac-source %s -j %s\n", lan_if, parental_ctrl_timematch, nvram_get_by_seq("macfilter_list_x", i), fftype);
			}
		}
#else	/* RTCONFIG_OLD_PARENTALCTRL */
		nv = nvp = strdup(nvram_safe_get("macfilter_rulelist"));	

		if(nv) {
		while((b = strsep(&nvp, "<")) != NULL) {
			if((vstrsep(b, ">", &mac) != 1)) continue;			
			if(strlen(mac)==0) continue;
		
	 		fprintf(fp, "-A INPUT -i %s -m mac --mac-source %s -j %s\n", lan_if, mac, ftype);
	 		fprintf(fp, "-A FORWARD -i %s -m mac --mac-source %s -j %s\n", lan_if, mac, fftype);

		}
		free(nv);
		}
#endif	/* RTCONFIG_OLD_PARENTALCTRL */
	}

	if (!nvram_match("fw_enable_x", "1"))
	{
		if (nvram_match("macfilter_enable_x", "1"))
		{
			/* Filter known SPI state */
			fprintf(fp, "-A INPUT -i %s -m state --state NEW -j %s\n"
			,lan_if, logdrop);
		}
	}
	else
	{	
		if (nvram_match("macfilter_enable_x", "1"))
		{
			/* Filter known SPI state */
			fprintf(fp, "-A INPUT -m state --state INVALID -j %s\n"
			  "-A INPUT -m state --state RELATED,ESTABLISHED -j %s\n"
			  "-A INPUT -i lo -m state --state NEW -j %s\n"
			  "-A INPUT -i %s -m state --state NEW -j %s\n"
			,logdrop, logaccept, "ACCEPT", lan_if, logdrop);
		}
		else
		{
			/* Filter known SPI state */
			fprintf(fp, "-A INPUT -m state --state INVALID -j %s\n"
			  "-A INPUT -m state --state RELATED,ESTABLISHED -j %s\n"
			  "-A INPUT -i lo -m state --state NEW -j %s\n"
			  "-A INPUT -i %s -m state --state NEW -j %s\n"
			,logdrop, logaccept, "ACCEPT", lan_if, "ACCEPT");
		}

// oleg patch ~
		/* Pass multicast */
		if (nvram_match("mr_enable_x", "1") || nvram_invmatch("udpxy_enable_x", "0")) {
			fprintf(fp, "-A INPUT -p 2 -d 224.0.0.0/4 -j %s\n", logaccept);
			fprintf(fp, "-A INPUT -p udp -d 224.0.0.0/4 ! --dport 1900 -j %s\n", logaccept);
		}
// ~ oleg patch
		/* enable incoming packets from broken dhcp servers, which are sending replies
		 * from addresses other than used for query, this could lead to lower level
		 * of security, but it does not work otherwise (conntrack does not work) :-( 
		 */
		if (!strcmp(wan_proto, "dhcp") || !strcmp(wan_proto, "bigpond") ||
		    !strcmp(wan_ipaddr, "0.0.0.0"))	// oleg patch
		{
			fprintf(fp, "-A INPUT -p udp --sport 67 --dport 68 -j %s\n", logaccept);
		}
		// Firewall between WAN and Local
		if (nvram_match("misc_http_x", "1"))
		{
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %s -j %s\n", lan_ip, nvram_safe_get("lan_port"), logaccept);
#ifdef RTCONFIG_HTTPS
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %s -j %s\n", lan_ip, nvram_safe_get("https_lanport"), logaccept);
#endif		
		}
		
		if (!nvram_match("enable_ftp", "0"))
		{	
			//fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %s -j %s\n", wan_ip, nvram_safe_get("usb_ftpport_x"), logaccept);
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport 21 -j %s\n", logaccept);	// oleg patch
		}

#ifdef RTCONFIG_WEBDAV
		if (nvram_match("enable_webdav", "1"))
		{	
			//fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %s -j %s\n", wan_ip, nvram_safe_get("usb_ftpport_x"), logaccept);
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %s -j %s\n", nvram_safe_get("webdav_https_port"), logaccept);	// oleg patch
		}
#endif

		if (!nvram_match("misc_ping_x", "0"))	// qq
		{
			//fprintf(fp, "-A INPUT -p icmp -d %s -j %s\n", wan_ip, logaccept);
			fprintf(fp, "-A INPUT -p icmp -j %s\n", logaccept);	// oleg patch
		}

		if (!nvram_match("misc_lpr_x", "0"))
		{
/*
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %d -j %s\n", wan_ip, 515, logaccept);
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %d -j %s\n", wan_ip, 9100, logaccept);
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %d -j %s\n", wan_ip, 3838, logaccept);
*/
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 515, logaccept);	// oleg patch
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 9100, logaccept);	// oleg patch
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 3838, logaccept);	// oleg patch
		}

		//Add for pptp server
		if (nvram_match("pptpd_enable", "1")) {
			fprintf(fp, "-A INPUT -i %s -p tcp --dport %d -j %s\n", wan_if, 1723, logaccept);
			fprintf(fp, "-A INPUT -p 47 -j %s\n",logaccept);
		}

		fprintf(fp, "-A INPUT -j %s\n", logdrop);
	}

#ifdef RTCONFIG_IPV6
	i = get_ipv6_service();
	switch (i) {
	case IPV6_ANYCAST_6TO4:
	case IPV6_6IN4:
		// Accept ICMP requests from the remote tunnel endpoint
		if (i == IPV6_ANYCAST_6TO4)
			sprintf(tmp, "192.88.99.%d", nvram_get_int("ipv6_relay"));
		else
			strlcpy(tmp, nvram_safe_get("ipv6_tun_v4end"), sizeof(tmp));
		if (*tmp && strcmp(tmp, "0.0.0.0") != 0)
			fprintf(fp, "-A INPUT -p icmp -s %s -j %s\n", tmp, "ACCEPT");
		fprintf(fp, "-A INPUT -p 41 -j %s\n", "ACCEPT");
		break;
	}
#endif

/* apps_dm DHT patch */
	if (nvram_match("apps_dl_share", "1"))
	{
		fprintf(fp, "-I INPUT -p udp --dport 6881 -j ACCEPT\n");	// DHT port
		// port range
		fprintf(fp, "-I INPUT -p udp --dport %s:%s -j ACCEPT\n", nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"));
		fprintf(fp, "-I INPUT -p tcp --dport %s:%s -j ACCEPT\n", nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"));
	}

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	fprintf(fp_ipv6,
		"-A FORWARD -m rt --rt-type 0 -j DROP\n");
#endif

// oleg patch ~
	/* Pass multicast */
	if (nvram_match("mr_enable_x", "1"))
	{
		fprintf(fp, "-A FORWARD -p udp -d 224.0.0.0/4 -j ACCEPT\n");
		if (strlen(macaccept)>0)
			fprintf(fp, "-A MACS -p udp -d 224.0.0.0/4 -j ACCEPT\n");
	}

	/* Clamp TCP MSS to PMTU of WAN interface before accepting RELATED packets */
	// TODO handle for multiple WAN
	if (!strcmp(wan_proto, "pptp") || !strcmp(wan_proto, "pppoe") || !strcmp(wan_proto, "l2tp"))
	{
		fprintf(fp, "-A FORWARD -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");
		if (strlen(macaccept)>0)
			fprintf(fp, "-A MACS -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");

#ifdef RTCONFIG_IPV6
		switch (get_ipv6_service()) {
		case IPV6_ANYCAST_6TO4:
		case IPV6_6IN4:
			fprintf(fp_ipv6, "-A FORWARD -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");
			if (strlen(macaccept)>0)
			fprintf(fp_ipv6, "-A MACS -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");
			break;
		}
#endif
	}

// ~ oleg patch
	fprintf(fp, "-A FORWARD -m state --state ESTABLISHED,RELATED -j %s\n", logaccept);
	if (strlen(macaccept)>0)
		fprintf(fp, "-A MACS -m state --state ESTABLISHED,RELATED -j %s\n", logaccept);
// ~ oleg patch
	/* Filter out invalid WAN->WAN connections */
	fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", wan_if, lan_if, logdrop); 
	if(unit == WAN_UNIT_WANPORT1 && !nvram_match(strcat_r(prefix, "ifname", tmp), wan_if))
		fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", nvram_get(tmp), lan_if, logdrop);
// oleg patch ~
	/* Drop the wrong state, INVALID, packets */
	fprintf(fp, "-A FORWARD -m state --state INVALID -j %s\n", logdrop);
	if (strlen(macaccept)>0)
		fprintf(fp, "-A MACS -m state --state INVALID -j %s\n", logdrop);

	/* Accept the redirect, might be seen as INVALID, packets */
	fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);	
	if (strlen(macaccept)>0)
	{
		fprintf(fp, "-A MACS -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);
	}

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	{
		fprintf(fp_ipv6, "-A FORWARD -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);
		if (strlen(macaccept)>0)
		fprintf(fp_ipv6, "-A MACS -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);

		// Filter out invalid WAN->WAN connections
		if (*wan6face)
			fprintf(fp_ipv6, "-A FORWARD -o %s ! -i %s -j %s\n", wan6face, lan_if, logdrop);

		fprintf(fp_ipv6, "-A FORWARD -p ipv6-nonxt -m length --length 40 -j ACCEPT\n");

		// ICMPv6 rules
		for (i = 0; i < sizeof(allowed_icmpv6)/sizeof(int); ++i) {
			fprintf(fp_ipv6, "-A FORWARD -p ipv6-icmp --icmpv6-type %i -j %s\n", allowed_icmpv6[i], logaccept);
		}
	}
#endif

#ifdef RTCONFIG_IPV6
	// RFC-4890, sec. 4.4.1
	const int allowed_local_icmpv6[] =
		{ 130, 131, 132, 133, 134, 135, 136,
		  141, 142, 143,
		  148, 149, 151, 152, 153 };
	if (ipv6_enabled())
	{
		fprintf(fp_ipv6,
			"-A INPUT -m rt --rt-type 0 -j %s\n"
			/* "-A INPUT -m state --state INVALID -j DROP\n" */
			"-A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT\n",
			logaccept);

		fprintf(fp_ipv6, "-A INPUT -p ipv6-nonxt -m length --length 40 -j ACCEPT\n");

		fprintf(fp_ipv6,
			"-A INPUT -i %s -j ACCEPT\n" // anything coming from LAN
			"-A INPUT -i lo -j ACCEPT\n",
				lan_if);

		switch (get_ipv6_service()) {
		case IPV6_ANYCAST_6TO4:
		case IPV6_NATIVE_DHCP:
			// allow responses from the dhcpv6 server
			fprintf(fp_ipv6, "-A INPUT -p udp --dport 546 -j %s\n", logaccept);
			break;
		}

		// ICMPv6 rules
		for (n = 0; n < sizeof(allowed_icmpv6)/sizeof(int); n++) {
			fprintf(fp_ipv6, "-A INPUT -p ipv6-icmp --icmpv6-type %i -j %s\n", allowed_icmpv6[n], logaccept);
		}
		for (n = 0; n < sizeof(allowed_local_icmpv6)/sizeof(int); n++) {
			fprintf(fp_ipv6, "-A INPUT -p ipv6-icmp --icmpv6-type %i -j %s\n", allowed_local_icmpv6[n], logaccept);
		}

		// if logging
		if (nvram_match("macfilter_enable_x", "2"))
			fprintf(fp_ipv6, "-A INPUT -j %s\n", logdrop);

		// default policy: DROP

		fprintf(fp_ipv6, "-A OUTPUT -m rt --rt-type 0 -j %s\n", logdrop);
	}
#endif

	/* Clamp TCP MSS to PMTU of WAN interface */
/* oleg patch mark off
	if ( nvram_match("wan_proto", "pppoe"))
	{
		fprintf(fp, "-I FORWARD -p tcp --tcp-flags SYN,RST SYN -m tcpmss --mss %d: -j TCPMSS "
			  "--set-mss %d\n", atoi(nvram_safe_get("wan_pppoe_mtu"))-39, atoi(nvram_safe_get("wan_pppoe_mtu"))-40);
		
		if (strlen(macaccept)>0)
			fprintf(fp, "-A MACS -p tcp --tcp-flags SYN,RST SYN -m tcpmss --mss %d: -j TCPMSS "
			  "--set-mss %d\n", atoi(nvram_safe_get("wan_pppoe_mtu"))-39, atoi(nvram_safe_get("wan_pppoe_mtu"))-40);
	}
	if (nvram_match("wan_proto", "pptp"))
	{
		fprintf(fp, "-A FORWARD -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");
		if (strlen(macaccept)>0)
			fprintf(fp, "-A MACS -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");
 	}
*/
	//if (nvram_match("fw_enable_x", "1"))
	if ( nvram_match("fw_enable_x", "1") && nvram_match("misc_ping_x", "0") )	// ham 0902 //2008.09 magic
		fprintf(fp, "-A FORWARD -i %s -p icmp -j DROP\n", wan_if);

	if (nvram_match("fw_enable_x", "1") && !nvram_match("fw_dos_x", "0"))	// oleg patch
	{
		// DoS attacks
		// sync-flood protection	
		fprintf(fp, "-A FORWARD -i %s -p tcp --syn -m limit --limit 1/s -j %s\n", wan_if, logaccept);
		// furtive port scanner
		fprintf(fp, "-A FORWARD -i %s -p tcp --tcp-flags SYN,ACK,FIN,RST RST -m limit --limit 1/s -j %s\n", wan_if, logaccept);
		// ping of death
		fprintf(fp, "-A FORWARD -i %s -p icmp --icmp-type echo-request -m limit --limit 1/s -j %s\n", wan_if, logaccept);
	}

	// FILTER from LAN to WAN
	// Rules for MAC Filter and LAN to WAN Filter
	// Drop rules always before Accept
	if (nvram_match("macfilter_enable_x", "1"))
		strcpy(chain, "MACS");
	else strcpy(chain, "FORWARD");

	if (nvram_match("fw_lw_enable_x", "1"))
	{		
		char lanwan_timematch[128];
		char ptr[32], *icmplist;
		char *ftype, *dtype;
		char protoptr[16], flagptr[16];
		int apply;
		
		apply = timematch_conv(lanwan_timematch, "filter_lw_date_x", "filter_lw_time_x");
 
		if (nvram_match("filter_lw_default_x", "DROP"))
		{
			dtype = logdrop;
			ftype = logaccept;

		}
		else
		{
			dtype = logaccept;
			ftype = logdrop;
		}
		
		if(apply) {
			v4v6_ok = IPT_V4;

			// LAN/WAN filter		
			nv = nvp = strdup(nvram_safe_get("filter_lwlist"));
		
			if(nv) {
				while ((b = strsep(&nvp, "<")) != NULL) {
					if((vstrsep(b, ">", &srcip, &srcport, &dstip, &dstport, &proto) !=5 )) continue;
					(void)protoflag_conv(proto, protoptr, 0);
					(void)protoflag_conv(proto, flagptr, 1);
					g_buf_init(); // need to modified
					setting = filter_conv(protoptr, flagptr, srcip, srcport, dstip, dstport);
					if (srcip) v4v6_ok = ipt_addr_compact(srcip, v4v6_ok, (v4v6_ok == IPT_V4));
					if (dstip) v4v6_ok = ipt_addr_compact(dstip, v4v6_ok, (v4v6_ok == IPT_V4));
					if (v4v6_ok & IPT_V4)
					fprintf(fp, "-A %s %s -i %s -o %s %s -j %s\n", chain, lanwan_timematch, lan_if, wan_if, setting, ftype);
#ifdef RTCONFIG_IPV6
					if (ipv6_enabled() && (v4v6_ok & IPT_V6) && *wan6face)
					fprintf(fp_ipv6, "-A %s %s -i %s -o %s %s -j %s\n", chain, lanwan_timematch, lan_if, wan6face, setting, ftype);
#endif
				}
				free(nv);
			}
		}
		// ICMP	
		foreach(ptr, nvram_safe_get("filter_lw_icmp_x"), icmplist)
		{
			fprintf(fp, "-A %s %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", chain, lanwan_timematch, lan_if, wan_if, ptr, ftype);
#ifdef RTCONFIG_IPV6
			if (ipv6_enabled() && *wan6face)
			fprintf(fp_ipv6, "-A %s %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", chain, lanwan_timematch, lan_if, wan6face, ptr, ftype);
#endif
		}	

		// Default
		fprintf(fp, "-A %s -i %s -o %s -j %s\n", chain, lan_if, wan_if, dtype);
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled() && *wan6face)
		fprintf(fp_ipv6, "-A %s -i %s -o %s -j %s\n", chain, lan_if, wan6face, dtype);
#endif
	}
	else if (nvram_match("macfilter_enable_x", "1"))
	{
	 	fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", lan_if, wan_if, logdrop);
	 	fprintf(fp, "-A MACS -i %s -o %s -j %s\n", lan_if, wan_if, logaccept);
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled() && *wan6face)
		{
	 		fprintf(fp_ipv6, "-A FORWARD -i %s -o %s -j %s\n", lan_if, wan6face, logdrop);
	 		fprintf(fp_ipv6, "-A MACS -i %s -o %s -j %s\n", lan_if, wan6face, logaccept);
		}
#endif
	}

	// Block VPN traffic
	if (nvram_match("fw_pt_pptp", "0"))	
		fprintf(fp, "-I %s -i %s -o %s -p tcp --dport %d -j %s\n", chain, lan_if, wan_if, 1723, "DROP");
	if (nvram_match("fw_pt_l2tp", "0"))
		fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 1701, "DROP");
	if (nvram_match("fw_pt_ipsec", "0"))
	{
		fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 500, "DROP");
		fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 4500, "DROP");
	}
	if (nvram_match("fw_pt_pptp", "0"))
		fprintf(fp, "-I %s -i %s -o %s -p 47 -j %s\n", chain, lan_if, wan_if, "DROP");
	if (nvram_match("fw_pt_ipsec", "0"))
	{
		fprintf(fp, "-I %s -i %s -o %s -p 50 -j %s\n", chain, lan_if, wan_if, "DROP");
		fprintf(fp, "-I %s -i %s -o %s -p 51 -j %s\n", chain, lan_if, wan_if, "DROP");
	}

	// Filter from WAN to LAN
	if (nvram_match("fw_wl_enable_x", "1"))
	{
		char wanlan_timematch[128];
		char ptr[32], *icmplist;
		char *dtype, *ftype;
		int apply;

		apply = timematch_conv(wanlan_timematch, "filter_wl_date_x", "filter_wl_time_x");

		if (nvram_match("filter_wl_default_x", "DROP"))
		{
			dtype = logdrop;
			ftype = logaccept;
		}
		else
		{
			dtype = logaccept;
			ftype = logdrop;
		}

		if(apply) {
			v4v6_ok = IPT_V4;

			// WAN/LAN filter		
			nv = nvp = strdup(nvram_safe_get("filter_wllist"));
		
			if(nv) {
				while ((b = strsep(&nvp, "<")) != NULL) {
					if ((vstrsep(b, ">", &proto, &flag, &srcip, &srcport, &dstip, &dstport) != 5)) continue;

					setting=filter_conv(proto, flag, srcip, srcport, dstip, dstport);
					if (srcip) v4v6_ok = ipt_addr_compact(srcip, v4v6_ok, (v4v6_ok == IPT_V4));
					if (dstip) v4v6_ok = ipt_addr_compact(dstip, v4v6_ok, (v4v6_ok == IPT_V4));
					if (v4v6_ok & IPT_V4)
		 			fprintf(fp, "-A FORWARD %s -i %s -o %s %s -j %s\n", wanlan_timematch, wan_if, lan_if, setting, ftype);
#ifdef RTCONFIG_IPV6
					if (ipv6_enabled() && (v4v6_ok & IPT_V6) && *wan6face)
					fprintf(fp_ipv6, "-A FORWARD %s -i %s -o %s %s -j %s\n", wanlan_timematch, wan6face, lan_if, setting, ftype);
#endif
				}
				free(nv);
			}
		}

		// ICMP	
		foreach(ptr, nvram_safe_get("filter_wl_icmp_x"), icmplist)
		{
			fprintf(fp, "-A FORWARD %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", wanlan_timematch, wan_if, lan_if, ptr, ftype);
#ifdef RTCONFIG_IPV6
			if (ipv6_enabled() && *wan6face)
			fprintf(fp_ipv6, "-A FORWARD %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", wanlan_timematch, wan6face, lan_if, ptr, ftype);
#endif
		}	
	 	
		// thanks for Oleg
		// Default
		// fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", wan_if, lan_if, dtype);
	}

TRACE_PT("writing vts_enable_x\n");
	
	/* Write forward chain rules of NAT */
	//if ((fp1 = fopen("/tmp/nat_forward_rules", "r"))!=NULL)
	// oleg patch ~
	/* Enable Virtual Servers */
	// fprintf(fp, "-A FORWARD -m conntrack --ctstate DNAT -j %s\n", logaccept);	// disable for tmp 

	// add back vts forward rules
	if (is_nat_enabled() && nvram_match("vts_enable_x", "1"))
	{
		char *proto, *port, *lport, *dstip, *desc, *protono;
		char dstips[256], dstports[256];
		// WAN/LAN filter		
		nv = nvp = strdup(nvram_safe_get("vts_rulelist"));
		
		if(nv) {
		while ((b = strsep(&nvp, "<")) != NULL) {
			if ((vstrsep(b, ">", &desc, &port, &dstip, &lport, &proto, &protono) != 6)) continue;

			if (lport!=NULL && strlen(lport)!=0)
			{
				sprintf(dstips, "%s:%s", dstip, lport);
				sprintf(dstports, "%s", lport);
			}
			else
			{
				sprintf(dstips, "%s:%s", dstip, port);
				sprintf(dstports, "%s", port);
			}

			if (strcmp(proto, "TCP")==0 || strcmp(proto, "BOTH")==0)
			{
				fprintf(fp, "-A FORWARD -p tcp -m tcp -d %s --dport %s -j %s\n",  dstip, dstports, logaccept);  // add back for conntrack patch
			}

			if (strcmp(proto, "UDP")==0 || strcmp(proto, "BOTH")==0)
			{
				fprintf(fp, "-A FORWARD -p udp -m udp -d %s --dport %s -j %s\n", dstip, dstports, logaccept);	// add back for conntrack patch
			}

			if (strcmp(proto, "OTHER")==0)
			{
				fprintf(fp, "-A FORWARD -p %s -d %s -j %s\n", protono, dstip, logaccept);	// add back for conntrack patch
			}
		}
		free(nv);
		}
	}
TRACE_PT("write porttrigger\n");
	if (is_nat_enabled() && nvram_match("autofw_enable_x", "1")) 
		write_porttrigger(fp, wan_if, 0);

	if (is_nat_enabled())
		write_upnp_filter(fp, wan_if);
	// ~ add back
#if 0
	if (is_nat_enabled() && !nvram_match("sp_battle_ips", "0"))
	// ~ oleg patch

	{
/* oleg patch mark off
		while (fgets(line, sizeof(line), fp1))
		{
			fprintf(fp, "%s", line);
		}

		fclose(fp1);
*/
		fprintf(fp, "-A FORWARD -p udp --dport %d -j %s\n", BASEPORT, logaccept);	// oleg patch
	}
#endif

TRACE_PT("write wl filter\n");

	if (nvram_match("fw_wl_enable_x", "1")) // Thanks for Oleg
	{
		// Default
		fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", wan_if, lan_if, 
			nvram_match("filter_wl_default_x", "DROP") ? logdrop : logaccept);
	}
	// logaccept chain
	fprintf(fp, "-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logaccept -j ACCEPT\n");

	// logdrop chain
	fprintf(fp,"-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP\" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logdrop -j DROP\n");

TRACE_PT("write_url filter\n");

#ifdef WEBSTRFILTER
/* url filter corss midnight patch start */
	if (valid_url_filter_time()) {
		if (!makeTimestr(timef)) {
			nv = nvp = strdup(nvram_safe_get("url_rulelist"));

			if(nv) {
			while ((b = strsep(&nvp, "<")) != NULL) {
TRACE_PT("filterstr %s\n", b);
				if(vstrsep(b, ">", &filterstr) != 1) continue;		
				if(strlen(filterstr)==0) continue;
TRACE_PT("filterstr %s %s\n", timef, filterstr);
				fprintf(fp,"-I FORWARD -p tcp %s -m webstr --url \"%s\" -j DROP\n", timef, filterstr); //2008.10 magic
			}
			free(nv);
			}
		}

		if (!makeTimestr2(timef2)) {
			nv = nvp = strdup(nvram_safe_get("url_rulelist"));

			if(nv) {
			while ((b = strsep(&nvp, "<")) != NULL) {
				if(vstrsep(b, ">", &filterstr) != 1) continue;		
				if (strlen(filterstr)==0) continue;
				fprintf(fp,"-I FORWARD -p tcp %s -m webstr --url \"%s\" -j DROP\n", timef2, filterstr); //2008.10 magic
			}
			free(nv);
			}
		}
	}
/* url filter corss midnight patch end */
#endif

#ifdef CONTENTFILTER
	if (valid_keyword_filter_time())
	{
		if (!makeTimestr_content(timef)) {
			nv = nvp = strdup(nvram_safe_get("keyword_rulelist"));
			if(nv) {
			while ((b = strsep(&nvp, "<")) != NULL) {
				if(vstrsep(b, ">", &filterstr) != 1) continue;		
				if (strcmp(filterstr, ""))
					fprintf(fp,"-I FORWARD -p tcp --sport 80 %s -m string --string \"%s\" --algo bm -j DROP\n", timef, filterstr);
			}
			free(nv);
			}
		}
		if (!makeTimestr2_content(timef2)) {
			nv = nvp = strdup(nvram_safe_get("keyword_rulelist"));
			if(nv) {
			while ((b = strsep(&nvp, "<")) != NULL) {
				if(vstrsep(b, ">", &filterstr) != 1) continue;		
				if (strcmp(filterstr, ""))
					fprintf(fp,"-I FORWARD -p tcp --sport 80 %s -m string --string \"%s\" --algo bm -j DROP\n", timef, filterstr);
			}
			free(nv);
			}
		}
	}
#endif

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	//system("iptables -F");
	xstart("iptables-restore","/tmp/filter_rules");

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	{
		fprintf(fp_ipv6, "COMMIT\n\n");
		fclose(fp_ipv6);
		xstart("ip6tables-restore","/tmp/filter_rules_ipv6");
	}
#endif
}

#ifdef RTCONFIG_USB_MODEM
int
filter_setting2(char *lan_if, char *lan_ip, char *logaccept, char *logdrop)
{
	FILE *fp;	// oleg patch
#ifdef RTCONFIG_IPV6
	FILE *fp_ipv6;
#endif
	char *proto, *flag, *srcip, *srcport, *dstip, *dstport, *mac;
	char *nv, *nvp, *b;
	char *setting, line[256];
	char macaccept[32], chain[32];
	char *ftype, *dtype, *fftype;
	int num;
	int i;
	char *wan_proto;
#ifdef RTCONFIG_OLD_PARENTALCTRL
	char parental_ctrl_timematch[128];
	char parental_ctrl_nv_date[128];
	char parental_ctrl_nv_time[128];
	char parental_ctrl_enable_status[128];
#endif	/* RTCONFIG_OLD_PARENTALCTRL */
//2008.09 magic{
#ifdef WEBSTRFILTER
	char nvname[36], timef[32], timef2[32], *filterstr;
#endif
//2008.09 magic}
	char *wan_if, *wan_ip;
	char *wanx_if, *wanx_ip;
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
#ifdef RTCONFIG_IPV6
	char s[128];
	char t[512];
	char *en;
	char *sec;
	char *hit;
	int n;	
	char *p, *c;
#endif
	int v4v6_ok;

	if(wan_prefix(wan_if, prefix) < 0)
		sprintf(prefix, "wan%d_", WAN_UNIT_WANPORT1);

	wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));
	wan_ip = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));

	//if(!strlen(wan_proto)) return -1;

	if ((fp=fopen("/tmp/filter_rules", "w"))==NULL) return -1;
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled() && (fp_ipv6=fopen("/tmp/filter_rules_ipv6", "w"))==NULL) return -2;
#endif

	fprintf(fp, "*filter\n:INPUT ACCEPT [0:0]\n:FORWARD ACCEPT [0:0]\n:OUTPUT ACCEPT [0:0]\n:FUPNP - [0:0]\n:MACS - [0:0]\n:logaccept - [0:0]\n:logdrop - [0:0]\n");
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	fprintf(fp_ipv6, "*filter\n:INPUT ACCEPT [0:0]\n:FORWARD ACCEPT [0:0]\n:OUTPUT ACCEPT [0:0]\n:MACS - [0:0]\n:logaccept - [0:0]\n:logdrop - [0:0]\n");
#endif

	strcpy(macaccept, "");

#ifdef RTCONFIG_OLD_PARENTALCTRL
	parental_ctrl();
#endif	/* RTCONFIG_OLD_PARENTALCTRL */

#ifdef RTCONFIG_PARENTALCTRL
	if(nvram_get_int("MULTIFILTER_ALL") != 0){
TRACE_PT("writing Parental Control\n");
		config_daytime_string(fp);
	}
#endif

	// FILTER from LAN to WAN Source MAC
	if (!nvram_match("macfilter_enable_x", "0"))
	{		
		// LAN/WAN filter		
		
		if (nvram_match("macfilter_enable_x", "2"))
		{
			dtype = logaccept;
			ftype = logdrop;
			fftype = logdrop;
		}
		else
		{
			dtype = logdrop;
			ftype = logaccept;

			strcpy(macaccept, "MACS");
			fftype = macaccept;
		}

#ifdef RTCONFIG_OLD_PARENTALCTRL
		num = atoi(nvram_safe_get("macfilter_num_x"));

		for(i = 0; i < num; ++i)
		{
			snprintf(parental_ctrl_nv_date, sizeof(parental_ctrl_nv_date),
						"macfilter_date_x%d", i);
			snprintf(parental_ctrl_nv_time, sizeof(parental_ctrl_nv_time),
						"macfilter_time_x%d", i);
			snprintf(parental_ctrl_enable_status,
						sizeof(parental_ctrl_enable_status),
						"macfilter_enable_status_x%d", i);
			if(!nvram_match(parental_ctrl_nv_date, "0000000") &&
				nvram_match(parental_ctrl_enable_status, "1")){
				timematch_conv(parental_ctrl_timematch,
									parental_ctrl_nv_date, parental_ctrl_nv_time);
				fprintf(fp, "-A INPUT -i %s %s -m mac --mac-source %s -j %s\n", lan_if, parental_ctrl_timematch, nvram_get_by_seq("macfilter_list_x", i), ftype);
				fprintf(fp, "-A FORWARD -i %s %s -m mac --mac-source %s -j %s\n", lan_if, parental_ctrl_timematch, nvram_get_by_seq("macfilter_list_x", i), fftype);
			}
		}
#else	/* RTCONFIG_OLD_PARENTALCTRL */
		nv = nvp = strdup(nvram_safe_get("macfilter_rulelist"));	

		if(nv) {
		while((b = strsep(&nvp, "<")) != NULL) {
			if((vstrsep(b, ">", &mac) != 1)) continue;			
			if(strlen(mac)==0) continue;
		
	 		fprintf(fp, "-A INPUT -i %s -m mac --mac-source %s -j %s\n", lan_if, mac, ftype);
	 		fprintf(fp, "-A FORWARD -i %s -m mac --mac-source %s -j %s\n", lan_if, mac, fftype);
		}
		free(nv);
		}
#endif	/* RTCONFIG_OLD_PARENTALCTRL */
	}

	if (!nvram_match("fw_enable_x", "1"))
	{
		if (nvram_match("macfilter_enable_x", "1"))
		{
			/* Filter known SPI state */
			fprintf(fp, "-A INPUT -i %s -m state --state NEW -j %s\n"
			,lan_if, logdrop);
		}
	}
	else
	{	
		if (nvram_match("macfilter_enable_x", "1"))
		{
			/* Filter known SPI state */
			fprintf(fp, "-A INPUT -m state --state INVALID -j %s\n"
			  "-A INPUT -m state --state RELATED,ESTABLISHED -j %s\n"
			  "-A INPUT -i lo -m state --state NEW -j %s\n"
			  "-A INPUT -i %s -m state --state NEW -j %s\n"
			,logdrop, logaccept, "ACCEPT", lan_if, logdrop);
		}
		else
		{
			/* Filter known SPI state */
			fprintf(fp, "-A INPUT -m state --state INVALID -j %s\n"
			  "-A INPUT -m state --state RELATED,ESTABLISHED -j %s\n"
			  "-A INPUT -i lo -m state --state NEW -j %s\n"
			  "-A INPUT -i %s -m state --state NEW -j %s\n"
			,logdrop, logaccept, "ACCEPT", lan_if, "ACCEPT");
		}

// oleg patch ~
		/* Pass multicast */
		if (nvram_match("mr_enable_x", "1") || nvram_invmatch("udpxy_enable_x", "0")) {
			fprintf(fp, "-A INPUT -p 2 -d 224.0.0.0/4 -j %s\n", logaccept);
			fprintf(fp, "-A INPUT -p udp -d 224.0.0.0/4 ! --dport 1900 -j %s\n", logaccept);
		}
// ~ oleg patch
		/* enable incoming packets from broken dhcp servers, which are sending replies
		 * from addresses other than used for query, this could lead to lower level
		 * of security, but it does not work otherwise (conntrack does not work) :-( 
		 */
		if (!strcmp(wan_proto, "dhcp") || !strcmp(wan_proto, "bigpond") ||
		    !strcmp(wan_ip, "0.0.0.0"))	// oleg patch
		{
			fprintf(fp, "-A INPUT -p udp --sport 67 --dport 68 -j %s\n", logaccept);
		}
		// Firewall between WAN and Local
		if (nvram_match("misc_http_x", "1"))
		{
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %s -j %s\n", lan_ip, nvram_safe_get("lan_port"), logaccept);
#ifdef RTCONFIG_HTTPS
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %s -j %s\n", lan_ip, nvram_safe_get("https_lanport"), logaccept);
#endif		
		}
		
		if (!nvram_match("enable_ftp", "0"))
		{	
			//fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %s -j %s\n", wan_ip, nvram_safe_get("usb_ftpport_x"), logaccept);
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport 21 -j %s\n", logaccept);	// oleg patch
		}

#ifdef RTCONFIG_WEBDAV
		if (!nvram_match("enable_webdav", "0"))
		{	
			//fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %s -j %s\n", wan_ip, nvram_safe_get("usb_ftpport_x"), logaccept);
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport 443 -j %s\n", logaccept);	// oleg patch
		}
#endif

		if (!nvram_match("misc_ping_x", "0"))	// qq
		{
			//fprintf(fp, "-A INPUT -p icmp -d %s -j %s\n", wan_ip, logaccept);
			fprintf(fp, "-A INPUT -p icmp -j %s\n", logaccept);	// oleg patch
		}

		if (!nvram_match("misc_lpr_x", "0"))
		{
/*
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %d -j %s\n", wan_ip, 515, logaccept);
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %d -j %s\n", wan_ip, 9100, logaccept);
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %d -j %s\n", wan_ip, 3838, logaccept);
*/
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 515, logaccept);	// oleg patch
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 9100, logaccept);	// oleg patch
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 3838, logaccept);	// oleg patch
		}

		//Add for pptp server
		if (nvram_match("pptpd_enable", "1")) {
			for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
				snprintf(prefix, sizeof(prefix), "wan%d_", unit);
				if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
					continue;

				wan_if = get_wan_ifname(unit);

				fprintf(fp, "-A INPUT -i %s -p tcp --dport %d -j %s\n", wan_if, 1723, logaccept);
			}

			fprintf(fp, "-A INPUT -p 47 -j %s\n",logaccept);
		}

		fprintf(fp, "-A INPUT -j %s\n", logdrop);
	}

#ifdef RTCONFIG_IPV6
	i = get_ipv6_service();
	switch (i) {
	case IPV6_ANYCAST_6TO4:
	case IPV6_6IN4:
		// Accept ICMP requests from the remote tunnel endpoint
		if (i == IPV6_ANYCAST_6TO4)
			sprintf(tmp, "192.88.99.%d", nvram_get_int("ipv6_relay"));
		else
			strlcpy(tmp, nvram_safe_get("ipv6_tun_v4end"), sizeof(tmp));
		if (*tmp && strcmp(tmp, "0.0.0.0") != 0)
			fprintf(fp, "-A INPUT -p icmp -s %s -j %s\n", tmp, "ACCEPT");
		fprintf(fp, "-A INPUT -p 41 -j %s\n", "ACCEPT");
		break;
	}
#endif

/* apps_dm DHT patch */
	if (nvram_match("apps_dl_share", "1"))
	{
		fprintf(fp, "-I INPUT -p udp --dport 6881 -j ACCEPT\n");	// DHT port
		// port range
		fprintf(fp, "-I INPUT -p udp --dport %s:%s -j ACCEPT\n", nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"));
		fprintf(fp, "-I INPUT -p tcp --dport %s:%s -j ACCEPT\n", nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"));
	}

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	fprintf(fp_ipv6,
		"-A FORWARD -m rt --rt-type 0 -j DROP\n");
#endif

// oleg patch ~
	/* Pass multicast */
	if (nvram_match("mr_enable_x", "1"))
	{
		fprintf(fp, "-A FORWARD -p udp -d 224.0.0.0/4 -j ACCEPT\n");
		if (strlen(macaccept)>0)
			fprintf(fp, "-A MACS -p udp -d 224.0.0.0/4 -j ACCEPT\n");
	}

	/* Clamp TCP MSS to PMTU of WAN interface before accepting RELATED packets */
	// TODO handle for multiple WAN
	if (!strcmp(wan_proto, "pptp") || !strcmp(wan_proto, "pppoe") || !strcmp(wan_proto, "l2tp"))
	{
		fprintf(fp, "-A FORWARD -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");
		if (strlen(macaccept)>0)
			fprintf(fp, "-A MACS -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");

#ifdef RTCONFIG_IPV6
		switch (get_ipv6_service()) {
		case IPV6_ANYCAST_6TO4:
		case IPV6_6IN4:
			fprintf(fp_ipv6, "-A FORWARD -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");
			if (strlen(macaccept)>0)
			fprintf(fp_ipv6, "-A MACS -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");
			break;
		}
#endif
	}

// ~ oleg patch
	fprintf(fp, "-A FORWARD -m state --state ESTABLISHED,RELATED -j %s\n", logaccept);
	if (strlen(macaccept)>0)
		fprintf(fp, "-A MACS -m state --state ESTABLISHED,RELATED -j %s\n", logaccept);

	for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
			continue;

		wan_if = get_wan_ifname(unit);
		wanx_if = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

// ~ oleg patch
		/* Filter out invalid WAN->WAN connections */
		fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", wan_if, lan_if, logdrop); 
		if (unit == WAN_UNIT_WANPORT1 && strcmp(wanx_if, wan_if))
			fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", wanx_if, lan_if, logdrop);
// oleg patch ~
	}

	/* Drop the wrong state, INVALID, packets */
	fprintf(fp, "-A FORWARD -m state --state INVALID -j %s\n", logdrop);
	if (strlen(macaccept)>0)
		fprintf(fp, "-A MACS -m state --state INVALID -j %s\n", logdrop);

	/* Accept the redirect, might be seen as INVALID, packets */
	fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);	
	if (strlen(macaccept)>0)
	{
		fprintf(fp, "-A MACS -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);
	}

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	{
		fprintf(fp_ipv6, "-A FORWARD -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);
		if (strlen(macaccept)>0)
		fprintf(fp_ipv6, "-A MACS -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);

		// Filter out invalid WAN->WAN connections
		if (*wan6face)
			fprintf(fp_ipv6, "-A FORWARD -o %s ! -i %s -j %s\n", wan6face, lan_if, logdrop);

		fprintf(fp_ipv6, "-A FORWARD -p ipv6-nonxt -m length --length 40 -j ACCEPT\n");

		// ICMPv6 rules
		for (i = 0; i < sizeof(allowed_icmpv6)/sizeof(int); ++i) {
			fprintf(fp_ipv6, "-A FORWARD -p ipv6-icmp --icmpv6-type %i -j %s\n", allowed_icmpv6[i], logaccept);
		}
	}
#endif

#ifdef RTCONFIG_IPV6
	// RFC-4890, sec. 4.4.1
	const int allowed_local_icmpv6[] =
		{ 130, 131, 132, 133, 134, 135, 136,
		  141, 142, 143,
		  148, 149, 151, 152, 153 };
	if (ipv6_enabled())
	{
		fprintf(fp_ipv6,
			"-A INPUT -m rt --rt-type 0 -j %s\n"
			/* "-A INPUT -m state --state INVALID -j DROP\n" */
			"-A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT\n",
			logaccept);

		fprintf(fp_ipv6, "-A INPUT -p ipv6-nonxt -m length --length 40 -j ACCEPT\n");

		fprintf(fp_ipv6,
			"-A INPUT -i %s -j ACCEPT\n" // anything coming from LAN
			"-A INPUT -i lo -j ACCEPT\n",
				lan_if);

		switch (get_ipv6_service()) {
		case IPV6_ANYCAST_6TO4:
		case IPV6_NATIVE_DHCP:
			// allow responses from the dhcpv6 server
			fprintf(fp_ipv6, "-A INPUT -p udp --dport 546 -j %s\n", logaccept);
			break;
		}

		// ICMPv6 rules
		for (n = 0; n < sizeof(allowed_icmpv6)/sizeof(int); n++) {
			fprintf(fp_ipv6, "-A INPUT -p ipv6-icmp --icmpv6-type %i -j %s\n", allowed_icmpv6[n], logaccept);
		}
		for (n = 0; n < sizeof(allowed_local_icmpv6)/sizeof(int); n++) {
			fprintf(fp_ipv6, "-A INPUT -p ipv6-icmp --icmpv6-type %i -j %s\n", allowed_local_icmpv6[n], logaccept);
		}

		// if logging
		if (nvram_match("macfilter_enable_x", "2"))
			fprintf(fp_ipv6, "-A INPUT -j %s\n", logdrop);

		// default policy: DROP

		fprintf(fp_ipv6, "-A OUTPUT -m rt --rt-type 0 -j %s\n", logdrop);
	}
#endif

	/* Clamp TCP MSS to PMTU of WAN interface */
/* oleg patch mark off
	if ( nvram_match("wan_proto", "pppoe"))
	{
		fprintf(fp, "-I FORWARD -p tcp --tcp-flags SYN,RST SYN -m tcpmss --mss %d: -j TCPMSS "
			  "--set-mss %d\n", atoi(nvram_safe_get("wan_pppoe_mtu"))-39, atoi(nvram_safe_get("wan_pppoe_mtu"))-40);
		
		if (strlen(macaccept)>0)
			fprintf(fp, "-A MACS -p tcp --tcp-flags SYN,RST SYN -m tcpmss --mss %d: -j TCPMSS "
			  "--set-mss %d\n", atoi(nvram_safe_get("wan_pppoe_mtu"))-39, atoi(nvram_safe_get("wan_pppoe_mtu"))-40);
	}
	if (nvram_match("wan_proto", "pptp"))
	{
		fprintf(fp, "-A FORWARD -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");
		if (strlen(macaccept)>0)
			fprintf(fp, "-A MACS -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");
 	}
*/

	//if (nvram_match("fw_enable_x", "1"))
	if ( nvram_match("fw_enable_x", "1") && nvram_match("misc_ping_x", "0") )	// ham 0902 //2008.09 magic
		for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
				continue;

			wan_if = get_wan_ifname(unit);

			fprintf(fp, "-A FORWARD -i %s -p icmp -j DROP\n", wan_if);
		}

	if (nvram_match("fw_enable_x", "1") && !nvram_match("fw_dos_x", "0"))	// oleg patch
	{
		for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
				continue;

			wan_if = get_wan_ifname(unit);

			// DoS attacks
			// sync-flood protection	
			fprintf(fp, "-A FORWARD -i %s -p tcp --syn -m limit --limit 1/s -j %s\n", wan_if, logaccept);
			// furtive port scanner
			fprintf(fp, "-A FORWARD -i %s -p tcp --tcp-flags SYN,ACK,FIN,RST RST -m limit --limit 1/s -j %s\n", wan_if, logaccept);
			// ping of death
			fprintf(fp, "-A FORWARD -i %s -p icmp --icmp-type echo-request -m limit --limit 1/s -j %s\n", wan_if, logaccept);
		}
	}

	// FILTER from LAN to WAN
	// Rules for MAC Filter and LAN to WAN Filter
	// Drop rules always before Accept
	if (nvram_match("macfilter_enable_x", "1"))
		strcpy(chain, "MACS");
	else strcpy(chain, "FORWARD");

	if (nvram_match("fw_lw_enable_x", "1"))
	{		
		char lanwan_timematch[128];
		char ptr[32], *icmplist;
		char *ftype, *dtype;
		char protoptr[16], flagptr[16];
		int apply;
		
		apply = timematch_conv(lanwan_timematch, "filter_lw_date_x", "filter_lw_time_x");
 
		if (nvram_match("filter_lw_default_x", "DROP"))
		{
			dtype = logdrop;
			ftype = logaccept;

		}
		else
		{
			dtype = logaccept;
			ftype = logdrop;
		}
		
		if(apply) {
			v4v6_ok = IPT_V4;

			// LAN/WAN filter		
			nv = nvp = strdup(nvram_safe_get("filter_lwlist"));

			if(nv) {
				while ((b = strsep(&nvp, "<")) != NULL) {
					if((vstrsep(b, ">", &srcip, &srcport, &dstip, &dstport, &proto) !=5 )) continue;
					(void)protoflag_conv(proto, protoptr, 0);
					(void)protoflag_conv(proto, flagptr, 1);
					g_buf_init(); // need to modified
					setting = filter_conv(protoptr, flagptr, srcip, srcport, dstip, dstport);
					if (srcip) v4v6_ok = ipt_addr_compact(srcip, v4v6_ok, (v4v6_ok == IPT_V4));
					if (dstip) v4v6_ok = ipt_addr_compact(dstip, v4v6_ok, (v4v6_ok == IPT_V4));
					for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
						snprintf(prefix, sizeof(prefix), "wan%d_", unit);
						if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
							continue;

						wan_if = get_wan_ifname(unit);
						if (v4v6_ok & IPT_V4)
						fprintf(fp, "-A %s %s -i %s -o %s %s -j %s\n", chain, lanwan_timematch, lan_if, wan_if, setting, ftype);
#ifdef RTCONFIG_IPV6
						if (ipv6_enabled() && (v4v6_ok & IPT_V6) && *wan6face)
						fprintf(fp_ipv6, "-A %s %s -i %s -o %s %s -j %s\n", chain, lanwan_timematch, lan_if, wan6face, setting, ftype);
#endif
					}
				}
				free(nv);
			}
		}

		// ICMP	
		foreach(ptr, nvram_safe_get("filter_lw_icmp_x"), icmplist)
		{
			for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
				snprintf(prefix, sizeof(prefix), "wan%d_", unit);
				if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
					continue;

				wan_if = get_wan_ifname(unit);

				fprintf(fp, "-A %s %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", chain, lanwan_timematch, lan_if, wan_if, ptr, ftype);
#ifdef RTCONFIG_IPV6
				if (ipv6_enabled() && *wan6face)
				fprintf(fp_ipv6, "-A %s %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", chain, lanwan_timematch, lan_if, wan6face, ptr, ftype);
#endif
			}
		}	

		// Default
		for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
				continue;

			wan_if = get_wan_ifname(unit);

			fprintf(fp, "-A %s -i %s -o %s -j %s\n", chain, lan_if, wan_if, dtype);
#ifdef RTCONFIG_IPV6
			if (ipv6_enabled() && *wan6face)
			fprintf(fp_ipv6, "-A %s -i %s -o %s -j %s\n", chain, lan_if, wan6face, dtype);
#endif
		}
	}
	else if (nvram_match("macfilter_enable_x", "1"))
	{
		for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
				continue;

			wan_if = get_wan_ifname(unit);

	 		fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", lan_if, wan_if, logdrop);
	 		fprintf(fp, "-A MACS -i %s -o %s -j %s\n", lan_if, wan_if, logaccept);
#ifdef RTCONFIG_IPV6
			if (ipv6_enabled() && *wan6face)
			{
	 			fprintf(fp_ipv6, "-A FORWARD -i %s -o %s -j %s\n", lan_if, wan6face, logdrop);
	 			fprintf(fp_ipv6, "-A MACS -i %s -o %s -j %s\n", lan_if, wan6face, logaccept);
			}
#endif
		}
	}

	// Block VPN traffic
	if (nvram_match("fw_pt_pptp", "0")){
		for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
				continue;

			wan_if = get_wan_ifname(unit);

			fprintf(fp, "-I %s -i %s -o %s -p tcp --dport %d -j %s\n", chain, lan_if, wan_if, 1723, "DROP");
		}
	}

	if (nvram_match("fw_pt_l2tp", "0")){
		for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
				continue;

			wan_if = get_wan_ifname(unit);

			fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 1701, "DROP");
		}
	}

	if (nvram_match("fw_pt_ipsec", "0")){
		for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
				continue;

			wan_if = get_wan_ifname(unit);

			fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 500, "DROP");
			fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 4500, "DROP");
		}
	}

	if (nvram_match("fw_pt_pptp", "0")){
		for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
				continue;

			wan_if = get_wan_ifname(unit);

			fprintf(fp, "-I %s -i %s -o %s -p 47 -j %s\n", chain, lan_if, wan_if, "DROP");
		}
	}

	if (nvram_match("fw_pt_ipsec", "0")){
		for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
				continue;

			wan_if = get_wan_ifname(unit);

			fprintf(fp, "-I %s -i %s -o %s -p 50 -j %s\n", chain, lan_if, wan_if, "DROP");
			fprintf(fp, "-I %s -i %s -o %s -p 51 -j %s\n", chain, lan_if, wan_if, "DROP");
		}
	}

	// Filter from WAN to LAN
	if (nvram_match("fw_wl_enable_x", "1"))
	{
		char wanlan_timematch[128];
		char ptr[32], *icmplist;
		char *dtype, *ftype;
		int apply;

		apply = timematch_conv(wanlan_timematch, "filter_wl_date_x", "filter_wl_time_x");

		if (nvram_match("filter_wl_default_x", "DROP"))
		{
			dtype = logdrop;
			ftype = logaccept;
		}
		else
		{
			dtype = logaccept;
			ftype = logdrop;
		}

		if(apply) {
			v4v6_ok = IPT_V4;

			// WAN/LAN filter		
			nv = nvp = strdup(nvram_safe_get("filter_wllist"));

			if(nv) {
				while ((b = strsep(&nvp, "<")) != NULL) {
					if ((vstrsep(b, ">", &proto, &flag, &srcip, &srcport, &dstip, &dstport) != 5)) continue;

					setting=filter_conv(proto, flag, srcip, srcport, dstip, dstport);
					if (srcip) v4v6_ok = ipt_addr_compact(srcip, v4v6_ok, (v4v6_ok == IPT_V4));
					if (dstip) v4v6_ok = ipt_addr_compact(dstip, v4v6_ok, (v4v6_ok == IPT_V4));
					if (v4v6_ok & IPT_V4)
					for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
						snprintf(prefix, sizeof(prefix), "wan%d_", unit);
						if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
							continue;

						wan_if = get_wan_ifname(unit);

			 			fprintf(fp, "-A FORWARD %s -i %s -o %s %s -j %s\n", wanlan_timematch, wan_if, lan_if, setting, ftype);
#ifdef RTCONFIG_IPV6
						if (ipv6_enabled() && (v4v6_ok & IPT_V6) && *wan6face)
						fprintf(fp_ipv6, "-A FORWARD %s -i %s -o %s %s -j %s\n", wanlan_timematch, wan6face, lan_if, setting, ftype);
#endif
			 		}
				}
				free(nv);
			}
		}

		// ICMP	
		foreach(ptr, nvram_safe_get("filter_wl_icmp_x"), icmplist)
		{
			for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
				snprintf(prefix, sizeof(prefix), "wan%d_", unit);
				if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
					continue;

				wan_if = get_wan_ifname(unit);

				fprintf(fp, "-A FORWARD %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", wanlan_timematch, wan_if, lan_if, ptr, ftype);
#ifdef RTCONFIG_IPV6
				if (ipv6_enabled() && (v4v6_ok & IPT_V6) && *wan6face)
				fprintf(fp_ipv6, "-A FORWARD %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", wanlan_timematch, wan6face, lan_if, ptr, ftype);
#endif
			}
		}	
	 	
		// thanks for Oleg
		// Default
		// fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", wan_if, lan_if, dtype);
	}

TRACE_PT("writing vts_enable_x\n");
	
	/* Write forward chain rules of NAT */
	//if ((fp1 = fopen("/tmp/nat_forward_rules", "r"))!=NULL)
	// oleg patch ~
	/* Enable Virtual Servers */
	// fprintf(fp, "-A FORWARD -m conntrack --ctstate DNAT -j %s\n", logaccept);	// disable for tmp 

	// add back vts forward rules
	if (is_nat_enabled() && nvram_match("vts_enable_x", "1"))
	{
		char *proto, *port, *lport, *dstip, *desc, *protono;
		char dstips[256], dstports[256];
		// WAN/LAN filter		
		nv = nvp = strdup(nvram_safe_get("vts_rulelist"));
		
		if(nv) {
		while ((b = strsep(&nvp, "<")) != NULL) {
			if ((vstrsep(b, ">", &desc, &port, &dstip, &lport, &proto, &protono) != 6)) continue;

			if (lport!=NULL && strlen(lport)!=0)
			{
				sprintf(dstips, "%s:%s", dstip, lport);
				sprintf(dstports, "%s", lport);
			}
			else
			{
				sprintf(dstips, "%s:%s", dstip, port);
				sprintf(dstports, "%s", port);
			}

			if (strcmp(proto, "TCP")==0 || strcmp(proto, "BOTH")==0)
			{
				fprintf(fp, "-A FORWARD -p tcp -m tcp -d %s --dport %s -j %s\n",  dstip, dstports, logaccept);  // add back for conntrack patch
			}

			if (strcmp(proto, "UDP")==0 || strcmp(proto, "BOTH")==0)
			{
				fprintf(fp, "-A FORWARD -p udp -m udp -d %s --dport %s -j %s\n", dstip, dstports, logaccept);	// add back for conntrack patch
			}

			if (strcmp(proto, "OTHER")==0)
			{
				fprintf(fp, "-A FORWARD -p %s -d %s -j %s\n", protono, dstip, logaccept);	// add back for conntrack patch
			}
		}
		free(nv);
		}
	}

TRACE_PT("write porttrigger\n");
	if (is_nat_enabled() && nvram_match("autofw_enable_x", "1"))
		write_porttrigger(fp, wan_if, 0);

	if (is_nat_enabled()) 
		write_upnp_filter(fp, wan_if);
	// ~ add back
#if 0
	if (is_nat_enabled() && !nvram_match("sp_battle_ips", "0"))
	// ~ oleg patch

	{
/* oleg patch mark off
		while (fgets(line, sizeof(line), fp1))
		{
			fprintf(fp, "%s", line);
		}

		fclose(fp1);
*/
		fprintf(fp, "-A FORWARD -p udp --dport %d -j %s\n", BASEPORT, logaccept);	// oleg patch
	}
#endif

TRACE_PT("write wl filter\n");
	if (nvram_match("fw_wl_enable_x", "1")) // Thanks for Oleg
	{
		for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
				continue;

			wan_if = get_wan_ifname(unit);

			// Default
			fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", wan_if, lan_if, nvram_match("filter_wl_default_x", "DROP") ? logdrop : logaccept);
		}
	}
	// logaccept chain
	fprintf(fp, "-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logaccept -j ACCEPT\n");

	// logdrop chain
	fprintf(fp,"-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP\" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logdrop -j DROP\n");

TRACE_PT("write_url filter\n");

#ifdef WEBSTRFILTER
/* url filter corss midnight patch start */
	if (valid_url_filter_time()) {
		if (!makeTimestr(timef)) {
			nv = nvp = strdup(nvram_safe_get("url_rulelist"));

			if(nv) {
			while ((b = strsep(&nvp, "<")) != NULL) {
TRACE_PT("filterstr %s\n", b);
				if(vstrsep(b, ">", &filterstr) != 1) continue;		
				if(strlen(filterstr)==0) continue;
TRACE_PT("filterstr %s %s\n", timef, filterstr);
				fprintf(fp,"-I FORWARD -p tcp %s -m webstr --url \"%s\" -j DROP\n", timef, filterstr); //2008.10 magic
			}
			free(nv);
			}
		}

		if (!makeTimestr2(timef2)) {
			nv = nvp = strdup(nvram_safe_get("url_rulelist"));

			if(nv) {
			while ((b = strsep(&nvp, "<")) != NULL) {
				if(vstrsep(b, ">", &filterstr) != 1) continue;		
				if (strlen(filterstr)==0) continue;
				fprintf(fp,"-I FORWARD -p tcp %s -m webstr --url \"%s\" -j DROP\n", timef2, filterstr); //2008.10 magic
			}
			free(nv);
			}
		}
	}
/* url filter corss midnight patch end */
#endif

#ifdef CONTENTFILTER
	if (valid_keyword_filter_time())
	{
		if (!makeTimestr_content(timef)) {
			nv = nvp = strdup(nvram_safe_get("keyword_rulelist"));
			if(nv) {
			while ((b = strsep(&nvp, "<")) != NULL) {
				if(vstrsep(b, ">", &filterstr) != 1) continue;		
				if (strcmp(filterstr, ""))
					fprintf(fp,"-I FORWARD -p tcp --sport 80 %s -m string --string \"%s\" --algo bm -j DROP\n", timef, filterstr);
			}
			free(nv);
			}
		}
		if (!makeTimestr2_content(timef2)) {
			nv = nvp = strdup(nvram_safe_get("keyword_rulelist"));
			if(nv) {
			while ((b = strsep(&nvp, "<")) != NULL) {
				if(vstrsep(b, ">", &filterstr) != 1) continue;		
				if (strcmp(filterstr, ""))
					fprintf(fp,"-I FORWARD -p tcp --sport 80 %s -m string --string \"%s\" --algo bm -j DROP\n", timef, filterstr);
			}
			free(nv);
			}
		}
	}
#endif

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	//system("iptables -F");
	xstart("iptables-restore","/tmp/filter_rules");

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	{
		fprintf(fp_ipv6, "COMMIT\n\n");
		fclose(fp_ipv6);
		xstart("ip6tables-restore","/tmp/filter_rules_ipv6");
	}
#endif
}
#endif

void
write_upnp_filter(FILE *fp, char *wan_if)
{
	// TODO: should we log upnp forwarded packets
	fprintf(fp, "-A FORWARD -i %s -j FUPNP\n", wan_if); 
}

void
write_porttrigger(FILE *fp, char *wan_if, int is_nat)
{
	char *nv, *nvp, *b;
	int i;
	char *out_proto, *in_proto, *out_port, *in_port, *desc;
	char out_protoptr[16], in_protoptr[16];
	int  out_start, out_end, in_start, in_end;
	int first = 1;

	if(is_nat) {
		fprintf(fp, "-A VSERVER -j TRIGGER --trigger-type dnat\n");
		return;
	}

	nvp = nv = strdup(nvram_safe_get("autofw_rulelist"));

	if (!nv) return;

	while ((b = strsep(&nvp, "<")) != NULL) {
		if ((vstrsep(b, ">", &desc, &out_port, &out_proto, &in_port, &in_proto) != 5)) continue;

		if(first) {
			fprintf(fp, ":triggers - [0:0]\n");
			fprintf(fp, "-A FORWARD -o %s -j triggers\n", wan_if);
			fprintf(fp, "-A FORWARD -i %s -j TRIGGER --trigger-type in\n", wan_if);
			first = 0;
		}
		(void)proto_conv(in_proto, in_protoptr);
		(void)proto_conv(out_proto, out_protoptr);


		fprintf(fp, "-A FORWARD -p %s -m %s --dport %s "
			"-j TRIGGER --trigger-type out --trigger-proto %s --trigger-match %s --trigger-relate %s\n",
			out_protoptr, out_protoptr, out_port,
			in_protoptr, out_port, in_port);
	}
	free(nv);
}

int
mangle_setting(char *wan_if, char *wan_ip, char *lan_if, char *lan_ip, char *logaccept, char *logdrop)
{
	if(nvram_match("qos_enable", "1")) {
		add_iQosRules(wan_if);
	}
	else {
		eval("iptables", "-t", "mangle", "-F");
#ifdef RTCONFIG_IPV6
		eval("ip6tables", "-t", "mangle", "-F");
#endif
	}

	/* mark connect to bypass CTF */		
	if(nvram_match("ctf_disable", "0")) {
		/* mark 80 port connection */
		if(nvram_match("url_enable_x", "1")) {
			eval("iptables", "-t", "mangle", "-A", "FORWARD", "-p", "tcp", "--dport", "80", "-j", "MARK", "--set-mark", "0x01");
		}
	}	
}

#ifdef RTCONFIG_USB_MODEM
int
mangle_setting2(char *lan_if, char *lan_ip, char *logaccept, char *logdrop)
{
	/*if(nvram_match("qos_enable", "1")) {
		add_iQosRules(wan_if);
	}
	else {//*/
		eval("iptables", "-t", "mangle", "-F");
#ifdef RTCONFIG_IPV6
		eval("ip6tables", "-t", "mangle", "-F");
#endif
	//}

	/* mark connect to bypass CTF */		
	if(nvram_match("ctf_disable", "0")) {
		/* mark 80 port connection */
		if(nvram_match("url_enable_x", "1")) {
			eval("iptables", "-t", "mangle", "-A", "FORWARD", "-p", "tcp", "--dport", "80", "-j", "MARK", "--set-mark", "0x01");
		}
	}	
}
#endif

// TODO: handle multiple wan
//int start_firewall(char *wan_if, char *wan_ip, char *lan_if, char *lan_ip)
int start_firewall(int wanunit, int lanunit)
{
	// TODO: handle multiple wan

	DIR *dir;
	struct dirent *file;
	FILE *fp;
	char name[NAME_MAX];
	//char logaccept[32], logdrop[32];
	//oleg patch ~
	char logaccept[32], logdrop[32];
	char *mcast_ifname;
	char wan_if[IFNAMSIZ+1], wan_ip[32], *lan_if[IFNAMSIZ+1], lan_ip[32];
	char wanx_if[IFNAMSIZ+1], wanx_ip[32], wan_proto[16];
	char prefix[]="wanXXXXXX_", tmp[100];

	if (!is_routing_enabled())
		return -1;

#ifdef RTCONFIG_EMF
	/* Force IGMPv2 due EMF limitations */
	if (nvram_get_int("emf_enable")) {
		f_write_string("/proc/sys/net/ipv4/conf/default/force_igmp_version", "2", 0, 0);
		f_write_string("/proc/sys/net/ipv4/conf/all/force_igmp_version", "2", 0, 0);
	}
#endif

	snprintf(prefix, sizeof(prefix), "wan%d_", wanunit);

	//(void)wan_ifname(wanunit, wan_if);
	strcpy(wan_if, get_wan_ifname(wanunit));
	strcpy(wan_proto, nvram_safe_get(strcat_r(prefix, "proto", tmp)));

	strcpy(wan_ip, nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)));
	strcpy(wanx_if, nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
	mcast_ifname = wanx_if;
	strcpy(wanx_ip, nvram_safe_get(strcat_r(prefix, "xipaddr", tmp)));

#ifdef RTCONFIG_IPV6
	strlcpy(wan6face, get_wan6face(), sizeof(wan6face));
#endif

	// handle one only
	strcpy(lan_if, nvram_safe_get("lan_ifname"));
	strcpy(lan_ip, nvram_safe_get("lan_ipaddr"));

	/* mcast needs rp filter to be turned off only for non default iface */
	if (!(nvram_match("mr_enable_x", "1") || nvram_invmatch("udpxy_enable_x", "0")) ||
	    strcmp(wan_if, mcast_ifname) == 0)
		mcast_ifname = NULL;
	// ~ oleg patch

	/* Block obviously spoofed IP addresses */
	if (!(dir = opendir("/proc/sys/net/ipv4/conf")))
		perror("/proc/sys/net/ipv4/conf");
	while (dir && (file = readdir(dir))) {
		if (strncmp(file->d_name, ".", NAME_MAX) != 0 &&
		    strncmp(file->d_name, "..", NAME_MAX) != 0) {
			sprintf(name, "/proc/sys/net/ipv4/conf/%s/rp_filter", file->d_name);
			if (!(fp = fopen(name, "r+"))) {
				perror(name);
				break;
			//}
			//fputc('1', fp);
			}
			fputc(mcast_ifname && strncmp(file->d_name, 	// oleg patch
				mcast_ifname, NAME_MAX) == 0 ? '0' : '1', fp);
			fclose(fp);	// oleg patch
		}
	}
	closedir(dir);

	/* Determine the log type */
	if (nvram_match("fw_log_x", "accept") || nvram_match("fw_log_x", "both"))
		strcpy(logaccept, "logaccept");
	else strcpy(logaccept, "ACCEPT");

	if (nvram_match("fw_log_x", "drop") || nvram_match("fw_log_x", "both"))
		strcpy(logdrop, "logdrop");
	else strcpy(logdrop, "DROP");

#ifdef RTCONFIG_IPV6
	modprobe("nf_conntrack_ipv6");
	modprobe("ip6t_REJECT");
	modprobe("ip6t_LOG");
	modprobe("xt_length");
#endif

	/* nat setting */
#ifdef RTCONFIG_USB_MODEM
	if(nvram_match("modem_mode", "2")){
		nat_setting2(lan_if, lan_ip, logaccept, logdrop);

#ifdef WEB_REDIRECT
		redirect_setting();
#endif

		filter_setting2(lan_if, lan_ip, logaccept, logdrop);

		mangle_setting2(lan_if, lan_ip, logaccept, logdrop);
	}
	else
#endif
	{
		if(wanunit != wan_primary_ifunit())
			return 0;

		nat_setting(wan_if, wan_ip, wanx_if, wanx_ip, lan_if, lan_ip, logaccept, logdrop);

#ifdef WEB_REDIRECT
		redirect_setting();
#endif

		filter_setting(wan_if, wan_ip, lan_if, lan_ip, logaccept, logdrop);

		mangle_setting(wan_if, wan_ip, lan_if, lan_ip, logaccept, logdrop);
	}

#ifdef RTCONFIG_IPV6
	if (!ipv6_enabled())
	{
		eval("ip6tables", "-F");
		eval("ip6tables", "-t", "mangle", "-F");
	}
#endif

	enable_ip_forward();

	/* Tweak NAT performance... */
/*
	if ((fp=fopen("/proc/sys/net/core/netdev_max_backlog", "w+")))
	{
		fputs("2048", fp);
		fclose(fp);
	}
	if ((fp=fopen("/proc/sys/net/core/somaxconn", "w+")))
	{
		fputs("1024", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_max_syn_backlog", "w+")))
	{
		fputs("1024", fp);
		fclose(fp);
	}
	if ((fp=fopen("/proc/sys/net/core/rmem_default", "w+")))
	{
		fputs("262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/core/rmem_max", "w+")))
	{
		fputs("262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/core/wmem_default", "w+")))
	{
		fputs("262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/core/wmem_max", "w+")))
	{
		fputs("262144", fp);
		fclose(fp);
	}
	if ((fp=fopen("/proc/sys/net/ipv4/tcp_rmem", "w+")))
	{
		fputs("8192 131072 262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_wmem", "w+")))
	{
		fputs("8192 131072 262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/neigh/default/gc_thresh1", "w+")))
	{
		fputs("1024", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/neigh/default/gc_thresh2", "w+")))
	{
		fputs("2048", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/neigh/default/gc_thresh3", "w+")))
	{
		fputs("4096", fp);
		fclose(fp);
	}
*/
	if ((fp=fopen("/proc/sys/net/ipv4/tcp_fin_timeout", "w+")))
	{
		fputs("40", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_intvl", "w+")))
	{
		fputs("30", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_probes", "w+")))
	{
		fputs("5", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_time", "w+")))
	{
		fputs("1800", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_retries2", "w+")))
	{
		fputs("5", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_syn_retries", "w+")))
	{
		fputs("3", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_synack_retries", "w+")))
	{
		fputs("3", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_tw_recycle", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_tw_reuse", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	/* Tweak DoS-related... */
	if ((fp=fopen("/proc/sys/net/ipv4/icmp_ignore_bogus_error_responses", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/icmp_echo_ignore_broadcasts", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_rfc1337", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_syncookies", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	setup_ftp_conntrack(nvram_get_int("vts_ftpport"));
	setup_pt_conntrack();

#if defined(RTCONFIG_APP_PREINSTALLED) || defined(RTCONFIG_APP_NETINSTALLED)
	if(strcmp(nvram_safe_get("apps_dev"), "") != 0)
		run_app_script("downloadmaster", "firewall-start");
#endif

#ifdef RTCONFIG_IPV6
	modprobe_r("xt_length");
	modprobe_r("ip6t_LOG");
	modprobe_r("ip6t_REJECT");
	modprobe_r("nf_conntrack_ipv6");
#endif

	return 0;
}


void enable_ip_forward(void)
{
	/*
		ip_forward - BOOLEAN
			0 - disabled (default)
			not 0 - enabled

			Forward Packets between interfaces.

			This variable is special, its change resets all configuration
			parameters to their default state (RFC1122 for hosts, RFC1812
			for routers)
	*/
	f_write_string("/proc/sys/net/ipv4/ip_forward", "1", 0, 0);

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) {
		f_write_string("/proc/sys/net/ipv6/conf/default/forwarding", "1", 0, 0);
		f_write_string("/proc/sys/net/ipv6/conf/all/forwarding", "1", 0, 0);
	}
#endif
}

