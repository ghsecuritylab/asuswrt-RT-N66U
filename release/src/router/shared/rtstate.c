#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <bcmdevs.h>
#include <shutils.h>
#include <rtstate.h>
#include <shared.h>

/* keyword for rc_support 	*/
/* ipv6 mssid update parental 	*/

void add_rc_support(char *feature)
{
	char features[128];

	strcpy(features, nvram_safe_get("rc_support"));

	if(strlen(features)==0) nvram_set("rc_support", feature);
	else {
		sprintf(features, "%s %s", features, feature);
		nvram_set("rc_support", features);
	}
}

int get_wan_state(int unit)
{
	char tmp[100], prefix[]="wanXXXXXX_";

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	return nvram_get_int(strcat_r(prefix, "state_t", tmp));
}

// get wan_unit from device ifname or hw device ifname
#if 0
int get_wan_unit(char *ifname)
{
	char word[256], tmp[100], *next;
	char prefix[32]="wanXXXXXX_";
	int unit, found = 0;

	unit = WAN_UNIT_WANPORT1;

	foreach (word, nvram_safe_get("wan_ifnames"), next) {
		if(strncmp(ifname, "ppp", 3)==0) {
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(strcmp(nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp)), ifname)==0) {
				found = 1;
			}	
		}
		else if(strcmp(ifname, word)==0) {
			found = 1;
		}
		if(found) break;
		unit ++;
	}

	if(!found) unit = WAN_UNIT_WANPORT1;
	return unit;
}
#else
int get_wan_unit(char *ifname)
{
	char tmp[100], prefix[32]="wanXXXXXX_";
	int unit;

	if(ifname == NULL)
		return -1;

	if(!strncmp(ifname, "tty", 3) || !strncmp(ifname, "/dev/tty", 8))
		return WAN_UNIT_USBPORT;

	for(unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; ++unit){
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		if(!strncmp(ifname, "ppp", 3)){
			if(nvram_match(strcat_r(prefix, "pppoe_ifname", tmp), ifname))
				return unit;
		}
		else if(nvram_match(strcat_r(prefix, "ifname", tmp), ifname))
			return unit;
	}

	return -1;
}
#endif

// Get physical wan ifname of working connection
char *get_wanx_ifname(int unit)
{
	char *wan_ifname;
	char tmp[100], prefix[]="wanXXXXXXXXXX_";
	
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	wan_ifname=nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	return wan_ifname;
}

// Get wan ifname of working connection
char *get_wan_ifname(int unit)
{
	char *wan_proto, *wan_ifname;
	char tmp[100], prefix[]="wanXXXXXXXXXX_";
	
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));

	//_dprintf("wan_proto: %s\n", wan_proto);

#ifdef RTCONFIG_USB_MODEM
	if(unit == WAN_UNIT_USBPORT)
		wan_ifname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
	else
#endif
	if(strcmp(wan_proto, "pppoe") == 0 ||
		strcmp(wan_proto, "pptp") == 0 ||
		strcmp(wan_proto, "l2tp") == 0)
		wan_ifname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
	else wan_ifname=nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	//_dprintf("wan_ifname: %s\n", wan_ifname);

	return wan_ifname;
}

#ifdef RTCONFIG_IPV6

char *get_wan6_ifname(int unit)
{
	char *wan_proto, *wan_ifname;
	char tmp[100], prefix[]="wanXXXXXXXXXX_";
	
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));


	if(strcmp(nvram_safe_get("ipv6_ifdev"), "eth")==0) {
		wan_ifname=nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		return wan_ifname;
	}
	//_dprintf("wan_proto: %s\n", wan_proto);

#ifdef RTCONFIG_USB_MODEM
	if(unit == WAN_UNIT_USBPORT)
		wan_ifname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
	else
#endif
	if(strcmp(wan_proto, "pppoe") == 0 ||
		strcmp(wan_proto, "pptp") == 0 ||
		strcmp(wan_proto, "l2tp") == 0)
		wan_ifname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
	else wan_ifname=nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	//_dprintf("wan_ifname: %s\n", wan_ifname);

	return wan_ifname;
}

#endif

// OR all lan port status
int get_lanports_status()
{
	return lanport_status();
}

// OR all wan port status
int get_wanports_status()
{
	return wanport_status();
}

int get_usb_modem_state(){
	if(!strcmp(nvram_safe_get("modem_running"), "1"))
		return 1;
	else
		return 0;
}

int set_usb_modem_state(const int flag){
	if(flag != 1 && flag != 0)
		return 0;

	if(flag){
		nvram_set("modem_running", "1");
		return 1;
	}
	else{
		nvram_set("modem_running", "0");
		return 0;
	}
}

int
set_wan_primary_ifunit(const int unit)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int i;

	if (unit < WAN_UNIT_WANPORT1 || unit >= WAN_UNIT_MAX)
		return -1;

	nvram_set_int("wan_primary", unit);
	for (i = WAN_UNIT_WANPORT1; i < WAN_UNIT_MAX; i++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", i);
		nvram_set_int(strcat_r(prefix, "primary", tmp), (i == unit) ? 1 : 0);
	}

	return 0;
}

int
wan_primary_ifunit(void)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int unit;

	/* TODO: Why not just nvram_get_int("wan_primary")? */
	for (unit = WAN_UNIT_WANPORT1; unit < WAN_UNIT_MAX; unit ++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if (nvram_match(strcat_r(prefix, "primary", tmp), "1"))
			return unit;
	}

	return 0;
}

void
set_invoke_later(int flag)
{
	nvram_set_int("invoke_later", nvram_get_int("invoke_later")|flag);
}

int
get_invoke_later()
{
	return(nvram_get_int("invoke_later"));
}

#ifdef RTCONFIG_USB

char ehci_string[32];
char ohci_string[32];

char *get_usb_ehci_port(int port)
{
	char word[100], *next;
	int i=0;

	strcpy(ehci_string, "xxxxxxxx");

	foreach(word, nvram_safe_get("ehci_ports"), next) {
		if(i==port) {
			strcpy(ehci_string, word);
		}		
		i++;
	}
	return ehci_string;
}

char *get_usb_ohci_port(int port)
{
	char word[100], *next;
	int i=0;

	strcpy(ohci_string, "xxxxxxxx");

	foreach(word, nvram_safe_get("ohci_ports"), next) {
		if(i==port) {
			strcpy(ohci_string, word);
		}		
		i++;
	}
	return ohci_string;
}

int get_usb_port_number(const char *usb_port){
	char word[100], *next;
	int port_num, i;

	port_num = 0;
	i = 0;
	foreach(word, nvram_safe_get("ehci_ports"), next){
		++i;
		if(!strcmp(usb_port, word)){
			port_num = i;
			break;
		}
	}

	i = 0;
	if(port_num == 0){
		foreach(word, nvram_safe_get("ohci_ports"), next){
			++i;
			if(!strcmp(usb_port, word)){
				port_num = i;
				break;
			}
		}
	}

	return port_num;
}
#endif

void add_wanscap_support(char *feature)
{
	char features[128];

	strcpy(features, nvram_safe_get("wans_cap"));

	if(strlen(features)==0) nvram_set("wans_cap", feature);
	else {
		sprintf(features, "%s %s", features, feature);
		nvram_set("wans_cap", features);
	}
}

int get_wans_dualwan(void) 
{
	int caps=0;
	char wancaps = nvram_safe_get("wans_dualwan");

	if(strstr(wancaps, "dsl")) caps |= WANSCAP_DSL;
	else if(strstr(wancaps, "wan")) caps |= WANSCAP_WAN;
	else if(strstr(wancaps, "lan")) caps |= WANSCAP_LAN;
	else if(strstr(wancaps, "2g")) caps |= WANSCAP_2G;
	else if(strstr(wancaps, "5g")) caps |= WANSCAP_5G;

	return caps;
}

