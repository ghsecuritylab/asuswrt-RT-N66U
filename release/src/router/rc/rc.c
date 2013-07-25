/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/

#include "rc.h"

#ifdef DEBUG_RCTEST
// used for various testing
static int rctest_main(int argc, char *argv[])
{
	int on;

	if (argc < 3) {
		_dprintf("test what?\n");
	}
	else if (strcmp(argv[1], "rc_service")==0) {
		notify_rc(argv[2]);
	}
	else if(strcmp(argv[1], "get_phy_status")==0) {
		int mask;
		mask = atoi(argv[2]);
		TRACE_PT("debug for phy_status %x\n", get_phy_status(mask));
	}
	else if(strcmp(argv[1], "get_phy_speed")==0) {
		int mask;
		mask = atoi(argv[2]);
		TRACE_PT("debug for phy_speed %x\n", get_phy_speed(mask));
	}
	else if(strcmp(argv[1], "set_phy_ctrl")==0) {
		int mask, ctrl;
		mask = atoi(argv[2]);
		ctrl = atoi(argv[3]);
		TRACE_PT("debug for phy_speed %x\n", set_phy_ctrl(mask, ctrl));
	}
	else if(strcmp(argv[1], "handle_notifications")==0) {
		handle_notifications();
	}
	else if(strcmp(argv[1], "check_action")==0) {
		_dprintf("check: %d\n", check_action());
	}
	else {
		on = atoi(argv[2]);
		_dprintf("%s %d\n", argv[1], on);

		if (strcmp(argv[1], "vlan") == 0)
		{	
			if(on) start_vlan();
			else stop_vlan();
		}
		else if (strcmp(argv[1], "lan") == 0) {
			if(on) start_lan();
			else stop_lan();
		}
		else if (strcmp(argv[1], "wl") == 0) {
			if(on) start_wl();
		}
		else if (strcmp(argv[1], "wan") == 0) {
			if(on) start_wan();
			else stop_wan();
		}
		else if (strcmp(argv[1], "firewall") == 0) {
			//if(on) start_firewall();
			//else stop_firewall();
		}
		else if (strcmp(argv[1], "watchdog") == 0) {
			if(on) start_watchdog();
			else stop_watchdog();
		}
#ifdef RTCONFIG_FANCTRL
		else if (strcmp(argv[1], "phy_tempsense") == 0) {
			if(on) start_phy_tempsense();
			else stop_phy_tempsense();
		}
#endif
		else if (strcmp(argv[1], "qos") == 0) {//qos test
			if(on){
#ifdef RTCONFIG_RALINK
				if (is_hwnat_loaded())
				{
					modprobe_r("hw_nat");
					sleep(1);
					system("echo 0 > /proc/sys/net/ipv4/conf/default/force_igmp_version");
					system("echo 0 > /proc/sys/net/ipv4/conf/all/force_igmp_version");
				}
#endif
				add_iQosRules(get_wan_ifname(0));
				start_iQos();
			}
			else 
			{
#ifdef RTCONFIG_RALINK
				if (nvram_get_int("hwnat") &&
					!((nvram_get_int("fw_pt_l2tp") || nvram_get_int("fw_pt_ipsec") || nvram_get_int("wl0_mrate_x") || nvram_get_int("wl1_mrate_x"))) &&
					!is_hwnat_loaded())
				{
					system("echo 2 > /proc/sys/net/ipv4/conf/default/force_igmp_version");
					system("echo 2 > /proc/sys/net/ipv4/conf/all/force_igmp_version");
					sleep(1);
					modprobe("hw_nat");
				}
#endif
				del_iQosRules();
				stop_iQos();
			}
		}
		else if (strcmp(argv[1], "gpiow") == 0) {
			if(argc>=4) set_gpio(atoi(argv[2]), atoi(argv[3]));
		}
		else if (strcmp(argv[1], "gpior") == 0) {
			_dprintf("%d\n", get_gpio(atoi(argv[2])));
		}
		else if (strcmp(argv[1], "init_switch") == 0) {
			init_switch(on);
		}
		else if (strcmp(argv[1], "set_action") == 0) {
			set_action(on);
		}	
		else {
			printf("what?\n");
		}
	}
	return 0;
}
#endif


static int hotplug_main(int argc, char *argv[])
{
	if (argc >= 2) {
		if (strcmp(argv[1], "net") == 0) {
			hotplug_net();
		}
#ifdef RTCONFIG_USB
		// !!TB - USB Support
		else if (strcmp(argv[1], "usb") == 0) {
			hotplug_usb();
		}
#ifdef LINUX26
		else if (strcmp(argv[1], "block") == 0) {
			hotplug_usb();
		}
#endif
#endif
	}
	return 0;
}

static int rc_main(int argc, char *argv[])
{
	if (argc < 2) return 0;
	if (strcmp(argv[1], "start") == 0) return kill(1, SIGUSR2);
	if (strcmp(argv[1], "stop") == 0) return kill(1, SIGINT);
	if (strcmp(argv[1], "restart") == 0) return kill(1, SIGHUP);
	return 0;
}



typedef struct {
	const char *name;
	int (*main)(int argc, char *argv[]);
} applets_t;

static const applets_t applets[] = {
	{ "init",			init_main				},
	{ "console",			console_main			},
#ifdef DEBUG_RCTEST
	{ "rc",				rctest_main			},
#endif
	{ "ip-up",			ipup_main				},
	{ "ip-down",			ipdown_main				},
#ifdef RTCONFIG_IPV6
	{ "ipv6-up",			ip6up_main				},
	{ "ipv6-down",			ip6down_main				},
#endif
	{ "auth-up",			authup_main				},
	{ "auth-down",			authdown_main				},
#ifdef RTCONFIG_EAPOL
	{ "wpa_cli",			wpacli_main			},
#endif
	{ "hotplug",			hotplug_main			},
	{ "mtd-write",			mtd_write_main			},
	{ "mtd-erase",			mtd_unlock_erase_main		},
	{ "mtd-unlock",			mtd_unlock_erase_main		},
	{ "watchdog",			watchdog_main			},
#ifdef RTCONFIG_FANCTRL
	{ "phy_tempsense",		phy_tempsense_main		},
#endif
#ifdef RTCONFIG_USB
	{ "usbled",			usbled_main			},
#endif
	{ "ddns_updated", 		ddns_updated_main		},
	{ "radio",			radio_main			},
	{ "ots",			ots_main			},
	{ "udhcpc",			udhcpc_wan			},
	{ "udhcpc_lan",			udhcpc_lan			},
#ifdef RTCONFIG_IPV6
	{ "dhcp6c-state",		dhcp6c_state_main		},
#endif
	{ "halt",			reboothalt_main			},
	{ "reboot",			reboothalt_main			},
	{ "ntp", 			ntp_main			},
#ifdef RTCONFIG_RALINK
	{ "8367m",			config8367m			},
#endif
	{ "wanduck",                    wanduck_main                    },
	{ "tcpcheck",                   tcpcheck_main                   },
	{ "autodet", 			autodet_main			},
#ifdef RTCONFIG_CIFS
	{ "mount-cifs",			mount_cifs_main			},
#endif
#ifdef RTCONFIG_USB
	{ "ejusb",                   ejusb_main                   },
#endif
	{NULL, NULL}
};

int main(int argc, char **argv)
{
	char *base;
	int f;

	/*
		Make sure std* are valid since several functions attempt to close these
		handles. If nvram_*() runs first, nvram=0, nvram gets closed. - zzz
	*/
	if ((f = open("/dev/null", O_RDWR)) < 3) {
		dup(f);
		dup(f);
	}
	else {
		close(f);
	}

	base = strrchr(argv[0], '/');
	base = base ? base + 1 : argv[0];

#if 0
	if (strcmp(base, "rc") == 0) {
		if (argc < 2) return 1;
		if (strcmp(argv[1], "start") == 0) return kill(1, SIGUSR2);
		if (strcmp(argv[1], "stop") == 0) return kill(1, SIGINT);
		if (strcmp(argv[1], "restart") == 0) return kill(1, SIGHUP);
		++argv;
		--argc;
		base = argv[0];
	}
#endif

#if defined(DEBUG_NOISY)
	if (nvram_match("debug_logrc", "1")) {
		int i;

		cprintf("[rc %d] ", getpid());
		for (i = 0; i < argc; ++i) {
			cprintf("%s ", argv[i]);
		}
		cprintf("\n");

	}
#endif

#if defined(DEBUG_NOISY)
	if (nvram_match("debug_ovrc", "1")) {
		char tmp[256];
		char *a[32];

		realpath(argv[0], tmp);
		if ((strncmp(tmp, "/tmp/", 5) != 0) && (argc < 32)) {
			sprintf(tmp, "%s%s", "/tmp/", base);
			if (f_exists(tmp)) {
				cprintf("[rc] override: %s\n", tmp);
				memcpy(a, argv, argc * sizeof(a[0]));
				a[argc] = 0;
				a[0] = tmp;
				execvp(tmp, a);
				exit(0);
			}
		}
	}
#endif

	const applets_t *a;
	for (a = applets; a->name; ++a) {
		if (strcmp(base, a->name) == 0) {
			openlog(base, LOG_PID, LOG_USER);
			return a->main(argc, argv);
		}
	}


	if(!strcmp(base, "restart_wireless")){
		printf("restart wireless...\n");
		restart_wireless();
		return 0;
	}
#ifdef RTCONFIG_USB
	else if(!strcmp(base, "get_apps_name")){
		if(argc != 2){
			printf("Usage: get_apps_name [File name]\n");
			return 0;
		}

		return get_apps_name(argv[1]);
	}
	else if(!strcmp(base, "asus_sd")){
		if(argc != 3){
			printf("Usage: asus_sd [device_name] [action]\n");
			return 0;
		}

		return asus_sd(argv[1], argv[2]);
	}
	else if(!strcmp(base, "asus_lp")){
		if(argc != 3){
			printf("Usage: asus_lp [device_name] [action]\n");
			return 0;
		}

		return asus_lp(argv[1], argv[2]);
	}
	else if(!strcmp(base, "asus_sg")){
		if(argc != 3){
			printf("Usage: asus_sg [device_name] [action]\n");
			return 0;
		}

		return asus_sg(argv[1], argv[2]);
	}
	else if(!strcmp(base, "asus_sr")){
		if(argc != 3){
			printf("Usage: asus_sr [device_name] [action]\n");
			return 0;
		}

		return asus_sr(argv[1], argv[2]);
	}
	else if(!strcmp(base, "asus_tty")){
		if(argc != 3){
			printf("Usage: asus_tty [device_name] [action]\n");
			return 0;
		}

		return asus_tty(argv[1], argv[2]);
	}
	else if(!strcmp(base, "asus_usb_interface")){
		if(argc != 3){
			printf("Usage: asus_usb_interface [device_name] [action]\n");
			return 0;
		}

		return asus_usb_interface(argv[1], argv[2]);
	}
#endif
	else if(!strcmp(base, "run_app_script")){
		if(argc != 3){
			printf("Usage: run_app_script [Package name | allpkg] [APP action]\n");
			return 0;
		}

		if(!strcmp(argv[1], "allpkg"))
			return run_app_script(NULL, argv[2]);
		else
			return run_app_script(argv[1], argv[2]);
	}
	else if(!strcmp(base, "ATE")) {
		if( argc == 2 || argc == 3 || argc == 4) {
			asus_ate_command(argv[1], argv[2], argv[3]);
		}
		else
			printf("ATE_ERROR\n");
                return 0;
	}
	else if(!strcmp(base, "run_telnetd")) {
		run_telnetd();
		return 0;
	}
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
	else if(!strcmp(base, "run_pptpd")) {
		start_pptpd();
		return 0;
	}
#endif
#ifdef RTCONFIG_PARENTALCTRL
	else if(!strcmp(base, "pc")) {
		pc_main(argc, argv);
		return 0;
	}
#endif
#ifdef RTCONFIG_WIRELESSREPEATER
	else if (!strcmp(base, "wlcscan")) {
		return wlcscan_main();
	}
	else if (!strcmp(base, "wlcconnect")) {
		return wlcconnect_main();
	}
	else if (!strcmp(base, "setup_dnsmq")) {
		if(argc != 2)
			return 0;

		return setup_dnsmq(atoi(argv[1]));
	}
#endif
	else if (!strcmp(base, "add_multi_routes")) {
		return add_multi_routes();
	}
	printf("Unknown applet: %s\n", base);
	return 0;
}
