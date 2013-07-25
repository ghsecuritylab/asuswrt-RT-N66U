/*
 * ppp scripts
 *
 * Copyright (C) 2009, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: ppp.c,v 1.19 2009/12/02 20:07:37 Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <rc.h>

/*
* parse ifname to retrieve unit #
*/
#if 0
int
ppp_ifunit(char *ifname)
{
	if (strncmp(ifname, "ppp", 3))
		return -1;
	if (!isdigit(ifname[3]))
		return -1;
	return atoi(&ifname[3]);
}
#else
int ppp_ifunit(char *ifname){
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		if(nvram_match(strcat_r(prefix, "pppoe_ifname", tmp), ifname))
			return unit;
	}

	return -1;
}
#endif

/*
 * Called when link comes up
 */
int
ipup_main(int argc, char **argv)
{
	FILE *fp;
	char *wan_ifname = safe_getenv("IFNAME");
	char *value;
	char buf[256];
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	_dprintf("%s():: %s\n", __FUNCTION__, argv[0]);

	char *pppd_pid = safe_getenv("PPPD_PID");
	char *DEVICE = safe_getenv("DEVICE");
	char *PPPLOGNAME = safe_getenv("PPPLOGNAME");
	char *LINKNAME = safe_getenv("LINKNAME");
	char *BUNDLE = safe_getenv("BUNDLE");
	char *MACREMOTE = safe_getenv("MACREMOTE");
	char *PEERNAME = safe_getenv("PEERNAME");

	_dprintf("%s: argc=%d.\n", __FUNCTION__, argc);
	int i;
	for(i = 1; i < argc; ++i)
		_dprintf("%s: argv[%d]=%s.\n", __FUNCTION__, i, argv[i]);
	_dprintf("%s: pppd_pid=%s.\n", __FUNCTION__, pppd_pid);
	_dprintf("%s: DEVICE=%s.\n", __FUNCTION__, DEVICE);
	_dprintf("%s: PPPLOGNAME=%s.\n", __FUNCTION__, PPPLOGNAME);
	_dprintf("%s: LINKNAME=%s.\n", __FUNCTION__, LINKNAME);
	_dprintf("%s: BUNDLE=%s.\n", __FUNCTION__, BUNDLE);
	_dprintf("%s: MACREMOTE=%s.\n", __FUNCTION__, MACREMOTE);
	_dprintf("%s: PEERNAME=%s.\n", __FUNCTION__, PEERNAME);

#ifdef RTCONFIG_USB_MODEM
	if(!nvram_match("modem_mode", "0")){
		if(isSerialNode(DEVICE) || isACMNode(DEVICE))
			unit = WAN_UNIT_USBPORT;
		else
			unit = WAN_UNIT_WANPORT1;
	}
	else
#endif
		unit = wan_primary_ifunit();

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

#ifdef RTCONFIG_USB_MODEM
	// wanX_ifname is used for device for USB Modem
	if(isSerialNode(DEVICE) || isACMNode(DEVICE))
		nvram_set(strcat_r(prefix, "ifname", tmp), DEVICE);
#endif
	nvram_set(strcat_r(prefix, "pppoe_ifname", tmp), wan_ifname);

	/* Touch connection file */
	if (!(fp = fopen(strcat_r("/tmp/ppp/link.", wan_ifname, tmp), "a"))) {
		perror(tmp);
		return errno;
	}
	fclose(fp);

	if ((value = getenv("IPLOCAL"))) {
		if (nvram_invmatch(strcat_r(prefix, "ipaddr", tmp), value))
			ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
		_ifconfig(wan_ifname, IFUP, value, "255.255.255.255", getenv("IPREMOTE"));
		nvram_set(strcat_r(prefix, "ipaddr", tmp), value);
		nvram_set(strcat_r(prefix, "netmask", tmp), "255.255.255.255");
	}

	if ((value = getenv("IPREMOTE")))
		nvram_set(strcat_r(prefix, "gateway", tmp), value);

	strcpy(buf, "");
	if (getenv("DNS1"))
		sprintf(buf, "%s", getenv("DNS1"));
	if (getenv("DNS2"))
		sprintf(buf + strlen(buf), "%s%s", strlen(buf) ? " " : "", getenv("DNS2"));

	if (nvram_match(strcat_r(prefix, "dnsenable_x", tmp), "1"))
		nvram_set(strcat_r(prefix, "dns", tmp), buf);

	wan_up(wan_ifname);
	_dprintf("ipup_main:: done\n");
	return 0;
}

/*
 * Called when link goes down
 */
int
ipdown_main(int argc, char **argv)
{
	char *wan_ifname = safe_getenv("IFNAME");
	char *DEVICE = safe_getenv("DEVICE");
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	_dprintf("%s():: %s\n", __FUNCTION__, argv[0]);

#ifdef RTCONFIG_USB_MODEM
	if(!nvram_match("modem_mode", "0")){
		if(isSerialNode(DEVICE) || isACMNode(DEVICE))
			unit = WAN_UNIT_USBPORT;
		else
			unit = WAN_UNIT_WANPORT1;
	}
	else
#endif
		unit = wan_primary_ifunit();

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	wan_down(wan_ifname);

_dprintf("%s(2):: prefix=%s.\n", __FUNCTION__, prefix);
	// override wan_state to get real reason
	update_wan_state(prefix, WAN_STATE_STOPPED, pppstatus());
_dprintf("%s(3):: prefix=%s.\n", __FUNCTION__, prefix);

	unlink(strcat_r("/tmp/ppp/link.", wan_ifname, tmp));

	preset_wan_routes(wan_ifname);

	_dprintf("ipdown_main:: done\n");
	return 0;
}

#ifdef RTCONFIG_IPV6
int ip6up_main(int argc, char **argv)
{
	return 0;
}

int ip6down_main(int argc, char **argv)
{
	return 0;
}
#endif  // IPV6

/*
 * Called when link goes up with auth 
 */
int
authup_main(int argc, char **argv)
{
	char *wan_ifname = safe_getenv("IFNAME");
	char *DEVICE = safe_getenv("DEVICE");
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	_dprintf("%s():: %s\n", __FUNCTION__, argv[0]);

#ifdef RTCONFIG_USB_MODEM
	if(!nvram_match("modem_mode", "0")){
		if(isSerialNode(DEVICE) || isACMNode(DEVICE))
			unit = WAN_UNIT_USBPORT;
		else
			unit = WAN_UNIT_WANPORT1;
	}
	else
#endif
		unit = wan_primary_ifunit();

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	_dprintf("authup_main:: done\n");
	return 0;
}

/*
 * Called when link goes down with auth fail
 */
int
authdown_main(int argc, char **argv)
{
	char *wan_ifname = safe_getenv("IFNAME");
	char *DEVICE = safe_getenv("DEVICE");
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	_dprintf("%s():: %s\n", __FUNCTION__, argv[0]);

#ifdef RTCONFIG_USB_MODEM
	if(!nvram_match("modem_mode", "0")){
		if(isSerialNode(DEVICE) || isACMNode(DEVICE))
			unit = WAN_UNIT_USBPORT;
		else
			unit = WAN_UNIT_WANPORT1;
	}
	else
#endif
		unit = wan_primary_ifunit();

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	update_wan_state(prefix, WAN_STATE_STOPPED, pppstatus());

	_dprintf("authdown_main:: done\n");
	return 0;
}
