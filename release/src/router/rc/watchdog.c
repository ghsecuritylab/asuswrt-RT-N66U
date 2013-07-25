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
#include <rc.h>
 
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <shutils.h>
#include <stdarg.h>
#ifdef RTCONFIG_RALINK
#include <ralink.h>
#endif

#include <wlioctl.h>
#include <syslog.h>
#include <bcmnvram.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#define BCM47XX_SOFTWARE_RESET  0x40		/* GPIO 6 */
#define RESET_WAIT		2		/* seconds */
#define RESET_WAIT_COUNT	RESET_WAIT * 10 /* 10 times a second */

#define TEST_PERIOD		100		/* second */
#define NORMAL_PERIOD		1		/* second */
#define URGENT_PERIOD		100 * 1000	/* microsecond */	
#define RUSHURGENT_PERIOD	50 * 1000	/* microsecond */

#ifdef BTN_SETUP
#define SETUP_WAIT		3		/* seconds */
#define SETUP_WAIT_COUNT	SETUP_WAIT * 10 /* 10 times a second */
#define SETUP_TIMEOUT		60 		/* seconds */
#define SETUP_TIMEOUT_COUNT	SETUP_TIMEOUT * 10 /* 60 times a second */
#endif // BTN_SETUP

static int nm_timer = 0;
static int httpd_timer = 0;
#if 0
static int cpu_timer = 0;
static int ddns_timer = 1;
static int media_timer = 0;
static int mem_timer = -1;
static int u2ec_timer = 0;
#endif
struct itimerval itv;
int watchdog_period = 0;
int watchdog_count = 0;
static int btn_pressed = 0;
static int btn_count = 0;
long sync_interval = 1;	// every 10 seconds a unit
int sync_flag = 0;
long timestamp_g = 0;
int stacheck_interval = -1;
#ifdef BTN_SETUP
int btn_pressed_setup = 0;
int btn_pressed_flag = 0;
int btn_count_setup = 0;
int btn_count_timeout = 0;
int wsc_timeout = 0;
int btn_count_setup_second = 0;
#endif

extern int g_wsc_configured;
extern int g_isEnrollee;

void 
sys_exit()
{
	printf("[watchdog] sys_exit");
	set_action(ACT_REBOOT);
	kill(1, SIGTERM);
}

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec  = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

int no_need_to_start_wps()
{
	if (nvram_match("wps_band", "0"))
	{
		if (	nvram_match("wl0_auth_mode_x", "shared") ||
			nvram_match("wl0_auth_mode_x", "wpa") ||
			nvram_match("wl0_auth_mode_x", "wpa2") ||
			nvram_match("wl0_auth_mode_x", "wpawpa2") ||
			nvram_match("wl0_auth_mode_x", "radius") /*||
			nvram_match("wl0_radio", "0")*/ ||
			!nvram_match("sw_mode", "1"))
		{
			return 1;
		}
	}
	else
	{
		if (	nvram_match("wl1_auth_mode_x", "shared") ||
			nvram_match("wl1_auth_mode_x", "wpa") ||
			nvram_match("wl1_auth_mode_x", "wpa2") ||
			nvram_match("wl1_auth_mode_x", "wpawpa2") ||
			nvram_match("wl1_auth_mode_x", "radius") /*||
			nvram_match("wl1_radio", "0")*/ ||
			!nvram_match("sw_mode", "1"))
		{
			return 1;
		}
	}

	return 0;
}
 
void btn_check(void)
{
#ifdef BTN_SETUP
	if (btn_pressed_setup == BTNSETUP_NONE)
	{
#endif

	if (button_pressed(BTN_RESET))
	{
		TRACE_PT("button RESET pressed\n");

	/*--------------- Add BTN_RST MFG test ------------------------*/
		if (nvram_match("asus_mfg", "1"))
		{
			nvram_set("btn_rst", "1");
		}
		else
		{
			if (!btn_pressed)
			{
				btn_pressed = 1;
				btn_count = 0;
				alarmtimer(0, URGENT_PERIOD);
			}
			else
			{	/* Whenever it is pushed steady */
				if (++btn_count > RESET_WAIT_COUNT)
				{
					fprintf(stderr, "You can release RESET button now!\n");

					btn_pressed = 2;
				}
				if (btn_pressed == 2)
				{
				/* 0123456789 */
				/* 0011100111 */
					if ((btn_count % 10) < 1 || ((btn_count % 10) > 4 && (btn_count % 10) < 7))
						led_control(LED_POWER, LED_OFF);
					else
						led_control(LED_POWER, LED_ON);
				}
			}
		} // end BTN_RST MFG test
	}
	else
	{
		if (btn_pressed == 1)
		{
			btn_count = 0;
			btn_pressed = 0;
			led_control(LED_POWER, LED_ON);
			alarmtimer(NORMAL_PERIOD, 0);
		}
		else if (btn_pressed == 2)
		{
			led_control(LED_POWER, LED_OFF);
			alarmtimer(0, 0);
			eval("mtd-erase","-d","nvram");
			/* FIXME: all stop-wan, umount logic will not be called
			 * prevous sys_exit (kill(1, SIGTERM) was ok
			 * since nvram isn't valid stop_wan should just kill possible daemons,
			 * nothing else, maybe with flag */
			sync();
			reboot(RB_AUTOBOOT);
		}
	}

#ifdef BTN_SETUP
	}
	if (btn_pressed != 0) return;

	if (btn_pressed_setup < BTNSETUP_START)
	{
		if (button_pressed(BTN_WPS) && /*!no_need_to_start_wps(0)*/ !no_need_to_start_wps())
		{
			TRACE_PT("button WPS pressed\n");

			/* Add BTN_EZ MFG test */
			if (nvram_match("asus_mfg", "1"))
			{
				nvram_set("btn_ez", "1");
			}
			else
			{
				if (btn_pressed_setup == BTNSETUP_NONE)
				{
					btn_pressed_setup = BTNSETUP_DETECT;
					btn_count_setup = 0;
					alarmtimer(0, RUSHURGENT_PERIOD);
				}
				else
				{	/* Whenever it is pushed steady */
					if (++btn_count_setup > SETUP_WAIT_COUNT)
					{
						btn_pressed_setup = BTNSETUP_START;
						btn_count_setup = 0;
						btn_count_setup_second = 0;
#if 0
						start_wps_pbc(nvram_get_int("wps_band"));
#else
						start_wps_pbc(0);	// always 2.4G
#endif
						wsc_timeout = 120*20;
					}
				}
			} // end BTN_EZ MFG test
		} 
		else if (btn_pressed_setup == BTNSETUP_DETECT)
		{
			btn_pressed_setup = BTNSETUP_NONE;
			btn_count_setup = 0;
			led_control(LED_POWER, LED_ON);
			alarmtimer(NORMAL_PERIOD, 0);
		}
	}
	else 
	{
		if (button_pressed(BTN_WPS) && /*!no_need_to_start_wps(0)*/ !no_need_to_start_wps())
		{
			/* Whenever it is pushed steady, again... */
			if (++btn_count_setup_second > SETUP_WAIT_COUNT)
			{
				btn_pressed_setup = BTNSETUP_START;
				btn_count_setup_second = 0;
#if 0
				start_wps_pbc(nvram_get_int("wps_band"));
#else
				start_wps_pbc(0);	// always 2.4G
#endif
				wsc_timeout = 120*20;
			}
		}

		if (is_wps_stopped() || --wsc_timeout == 0)
		{
			wsc_timeout = 0;

			btn_pressed_setup = BTNSETUP_NONE;
			btn_count_setup = 0;

			led_control_normal();

			alarmtimer(NORMAL_PERIOD, 0);
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
#ifdef RTCONFIG_RALINK
			stop_wps_method(0);
			stop_wps_method(1);
#else
			stop_wps_method(nvram_get_int("wps_band"));
#endif
#endif
			return;
		}

		++btn_count_setup;
		btn_count_setup = (btn_count_setup % 20);

		/* 0123456789 */
		/* 1010101010 */
		if ((btn_count_setup % 2) == 0 && (btn_count_setup > 10))
			led_control(LED_WPS, LED_ON);
		else
			led_control(LED_WPS, LED_OFF);
	}
#endif
}

#define DAYSTART (0)
#define DAYEND (60*60*23 + 60*59 + 59) // 86399

int timecheck_item(char *activeDate, char *activeTime)
{
	int current, active, activeTimeStart, activeTimeEnd;
	time_t now;
	struct tm *tm;

	setenv("TZ", nvram_safe_get("time_zone_x"), 1);

	time(&now);
	tm = localtime(&now);
	current = tm->tm_hour * 60 + tm->tm_min;
	active = 0;

	activeTimeStart = ((activeTime[0]-'0')*10 + (activeTime[1]-'0'))*60 + (activeTime[2]-'0')*10 + (activeTime[3]-'0');
	activeTimeEnd = ((activeTime[4]-'0')*10 + (activeTime[5]-'0'))*60 + (activeTime[6]-'0')*10 + (activeTime[7]-'0');

	if (activeDate[tm->tm_wday] == '1')
	{
		if (activeTimeEnd < activeTimeStart)
		{
			if ((current >= activeTimeStart && current <= DAYEND) ||
			   (current >= DAYSTART && current <= activeTimeEnd))
			{
				active = 1;
			}
			else
			{
				active = 0;
			}
		}
		else
		{
			if (current >= activeTimeStart && current <= activeTimeEnd)
			{
				active = 1;
			}
			else
			{
				active = 0;
			}
		}
	}

//	fprintf(stderr, "[watchdog] time check: %2d:%2d, active: %d\n", tm->tm_hour, tm->tm_min, active);

	return active;
}

int svcStatus[8] = { -1, -1, -1, -1, -1, -1, -1, -1};

/* Check for time-reated service 	*/
/* 1. Wireless Radio			*/
/* 2. Guess SSID expire time 		*/
//int timecheck(int argc, char *argv[])
void timecheck(void)
{
	int activeNow;
	char *svcDate, *svcTime;
	char prefix[]="wlXXXXXX_", tmp[100];
	char word[256], *next;
	int unit, item;
	char *lan_ifname;
	char wl_vifs[256], nv[40];
	int expire, need_commit = 0;

	item = 0;
	// radio on/off
	unit = 0;
	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		svcDate = nvram_safe_get(strcat_r(prefix, "radio_date_x", tmp));
		svcTime = nvram_safe_get(strcat_r(prefix, "radio_time_x", tmp));

		activeNow = timecheck_item(svcDate, svcTime);
		snprintf(tmp, sizeof(tmp), "%d", unit);

		if(svcStatus[item]!=activeNow) {
			svcStatus[item] = activeNow;
			if(activeNow) eval("radio", "on", tmp);
			else eval("radio", "off", tmp);
		}
		item++;
		unit++;
	}

	// guest ssid expire check
	if (strlen(nvram_safe_get("wl0_vifs")) || strlen(nvram_safe_get("wl1_vifs")))
	{
		lan_ifname = nvram_safe_get("lan_ifname");
		sprintf(wl_vifs, "%s %s", nvram_safe_get("wl0_vifs"), nvram_safe_get("wl1_vifs"));

		foreach (word, wl_vifs, next) {
			snprintf(nv, sizeof(nv) - 1, "%s_expire_tmp", word);
			expire = atoi(nvram_safe_get(nv));
			
			if (expire)
			{
				if (expire <= 30)
				{
					nvram_set(nv, "0");
					snprintf(nv, sizeof(nv) - 1, "%s_bss_enabled", word);
					nvram_set(nv, "0");
					if (!need_commit) need_commit = 1;
	 				eval("wlconf", word, "down");
	                        	ifconfig(word, 0, NULL, NULL);
	                        	eval("brctl", "delif", lan_ifname, word);
				}
				else
				{
					expire -= 30;
					sprintf(tmp, "%d", expire);
					nvram_set(nv, tmp);
				}
			}
		}

		if (need_commit)
		{
			need_commit = 0;
			nvram_commit();
		}
	}

	return 0;
}

#if 0
int high_cpu_usage_count = 0;

void
cpu_usage_monitor()
{
	cpu_timer = (cpu_timer + 1) % 2;
	if (cpu_timer) return;

	unsigned int usage = cpu_main(0, NULL, 0);

	if (usage >= 95)
		high_cpu_usage_count++;
	else
		high_cpu_usage_count = 0;

	if (high_cpu_usage_count >= 5)
		sys_exit();
}
#endif

static void catch_sig(int sig)
{
	if (sig == SIGUSR1)
	{
		fprintf(stderr, "[watchdog] Handle WPS LED for WPS Start\n");

		alarmtimer(NORMAL_PERIOD, 0);

		btn_pressed_setup = BTNSETUP_START;
		btn_count_setup = 0;
		btn_count_setup_second = 0;
		wsc_timeout = 120*20;
		alarmtimer(0, RUSHURGENT_PERIOD);
	}
	else if (sig == SIGUSR2)
	{
		fprintf(stderr, "[watchdog] Handle WPS LED for WPS Stop\n");

		btn_pressed_setup = BTNSETUP_NONE;
		btn_count_setup = 0;
		btn_count_setup_second = 0;
		wsc_timeout = 1;
		alarmtimer(NORMAL_PERIOD, 0);

		led_control_normal();
	}
#ifdef RTCONFIG_RALINK
	else if (sig == SIGTTIN)
	{
		wsc_user_commit();
	}
#endif
}

int block_dm_count = 0;
void dm_block_chk()
{
	if(nvram_match("dm_block", "1"))
		++block_dm_count;
	else
		block_dm_count = 0;
	
	if(block_dm_count > 20)	// 200 seconds
	{	
		block_dm_count = 0;
		nvram_set("dm_block", "0");
		printf("[watchdog] unblock dm execution\n");	// tmp test
	}
}

/* wathchdog is runned in NORMAL_PERIOD, 1 seconds
 * check in each NORMAL_PERIOD
 *	1. button
 *
 * check in each NORAML_PERIOD*10
 *
 *      1. time-dependent service
 */
void watchdog(int sig)
{
	/* handle button */
	btn_check();

	/* if timer is set to less than 1 sec, then bypass the following */
	if (itv.it_value.tv_sec == 0) return;

	if (!nvram_match("asus_mfg", "0")) return;

	watchdog_period = (watchdog_period + 1) % 30;

	if (watchdog_period) return;

#ifdef BTN_SETUP
	if (btn_pressed_setup >= BTNSETUP_START) return;
#endif

	/* check for time-related services */
	timecheck();
#if 0
	cpu_usage_monitor();
#endif
	return;
}

void led_control_normal(void)
{
	// the behaviro in normal when wps led != power led
	// wps led = on, power led = on
	
	led_control(LED_WPS, LED_ON);
	// in case LED_WPS != LED_POWER
	led_control(LED_POWER, LED_ON);
}

int 
watchdog_main(int argc, char *argv[])
{
	FILE *fp;

	/* write pid */
	if ((fp = fopen("/var/run/watchdog.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

#ifdef RTCONFIG_RALINK
	// TODO: handle sysdep part
	doSystem("iwpriv %s set WatchdogPid=%d", WIF_5G, getpid());
	doSystem("iwpriv %s set WatchdogPid=%d", WIF_2G, getpid());
#endif

	/* set the signal handler */
	signal(SIGUSR1, catch_sig);
	signal(SIGUSR2, catch_sig);
	signal(SIGTSTP, catch_sig);
	signal(SIGALRM, watchdog);
	signal(SIGTTIN, catch_sig);

	if (!pids("ots"))
		start_ots();

	setenv("TZ", nvram_safe_get("time_zone_x"), 1);

	_dprintf("TZ watchdog\n");
	/* set timer */
	alarmtimer(NORMAL_PERIOD, 0);


	led_control_normal();

	/* Most of time it goes to sleep */
	while (1)
	{	
		pause();
	}

	return 0;
}
