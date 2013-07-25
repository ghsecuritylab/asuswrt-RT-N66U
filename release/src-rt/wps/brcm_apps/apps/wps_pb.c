/*
 * WPS push button
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wps_pb.c 241376 2011-02-18 03:19:15Z stakita $
 */

#include <stdio.h>
#include <time.h>
#include <tutrace.h>
#include <wps_pb.h>
#include <wpscommon.h>
#include <bcmparams.h>
#include <wps_led.h>
#include <security_ipc.h>
#include <wps_wl.h>
#include <bcmutils.h>
#include <wlif_utils.h>
#include <shutils.h>
#include <wps.h>

#include <wps_enrapi.h>
#include <wps_sta.h>
#include <wps_enr_osl.h>
#include <wps_ui.h>
#include <wps_enrapi.h>

extern int find_pbc_ap(char *bssid, char *ssid, uint8 *wsec);

static time_t wps_pb_selecting_time = 0;
static PBC_STA_INFO pbc_info[PBC_OVERLAP_CNT];
static int wps_pb_state = WPS_PB_STATE_INIT;
static wps_hndl_t pb_hndl;
static int pb_last_led_id = -1;
static char pb_ifname[IFNAMSIZ] = {0};

/* for apsta mode concurrent wps */
#define WPS_FINDPBC_STATE_INIT		0
#define WPS_FINDPBC_STATE_SCANNING	1
#define WPS_FINDPBC_STATE_FINDINGAP	2

static unsigned long wps_enr_scan_state_time = 0;
static char wps_enr_scan_state = WPS_FINDPBC_STATE_INIT;
static int wps_virtual_pressed = WPS_NO_BTNPRESS;
static bool wps_pbc_apsta_upstream_pushed = FALSE;

/* PBC Overlapped detection */
int
wps_pb_check_pushtime(unsigned long time)
{
	int i;
	int PBC_sta = PBC_OVERLAP_CNT;
	for (i = 0; i < PBC_OVERLAP_CNT; i++) {
		/*
		 * 120 seconds is too sensitive, it have a chance that we receive 
		 * a last ProbeReq with WPS_DEVICEPWDID_PUSH_BTN after clearing
		 * this station.  So plus 2 seconds
		 */
		if ((time < pbc_info[i].last_time) ||
		    ((time - pbc_info[i].last_time) > (120 + 2))) {
			memset(&pbc_info[i], 0, sizeof(PBC_STA_INFO));
		}

		if (pbc_info[i].last_time == 0)
			PBC_sta--;
	}

	TUTRACE((TUTRACE_INFO, "There are %d sta in PBC mode!\n", PBC_sta));
	TUTRACE((TUTRACE_INFO, "sta1: %02x:%02x:%02x:%02x:%02x:%02x, LT:%d/CT:%d\n",
		pbc_info[0].mac[0], pbc_info[0].mac[1], pbc_info[0].mac[2], pbc_info[0].mac[3],
		pbc_info[0].mac[4], pbc_info[0].mac[5], pbc_info[0].last_time, time));
	TUTRACE((TUTRACE_INFO, "sta2: %02x:%02x:%02x:%02x:%02x:%02x, LT:%d/CT:%d\n",
		pbc_info[1].mac[0], pbc_info[1].mac[1], pbc_info[1].mac[2], pbc_info[1].mac[3],
		pbc_info[1].mac[4], pbc_info[1].mac[5], pbc_info[1].last_time, time));
	return PBC_sta;
}

void
wps_pb_update_pushtime(char *mac)
{
	int i;
	unsigned long now;

	(void) time((time_t*)&now);

	wps_pb_check_pushtime(now);

	for (i = 0; i < PBC_OVERLAP_CNT; i++) {
		if (memcmp(mac, pbc_info[i].mac, 6) == 0) {
			/* Confirmed in PF #3 */
			/* Do update new time, see test case 4.2.13 */
			pbc_info[i].last_time = now;
			return;
		}
	}

	if (pbc_info[0].last_time <= pbc_info[1].last_time)
		i = 0;
	else
		i = 1;

	memcpy(pbc_info[i].mac, mac, 6);
	pbc_info[i].last_time = now;

	return;
}

void
wps_pb_clear_sta(char *mac)
{
	int i;

	for (i = 0; i < PBC_OVERLAP_CNT; i++) {
		if (memcmp(mac, pbc_info[i].mac, 6) == 0) {
			memset(&pbc_info[i], 0, sizeof(PBC_STA_INFO));
			return;
		}
	}

	return;
}

static int
wps_pb_find_next()
{
	char *value, *next;
	int next_id;
	int max_id = wps_hal_led_wl_max();
	char tmp[100];
	char ifname[IFNAMSIZ];
	char *wlnames;

	int target_id = -1;
	int target_instance = 0;
	int i, imax;

	imax = wps_get_ess_num();

refind:
	for (i = 0; i < imax; i++) {
		sprintf(tmp, "ess%d_led_id", i);
		value = wps_get_conf(tmp);
		if (value == NULL)
			continue;

		next_id = atoi(value);
		if ((next_id > pb_last_led_id) && (next_id <= max_id)) {
			if ((target_id == -1) || (next_id < target_id)) {
				/* Save the candidate */
				target_id = next_id;
				target_instance = i;
			}
		}
	}

	/* A candidate found ? */
	if (target_id == -1) {
		pb_last_led_id = -1;
		goto refind;
	}

	pb_last_led_id = target_id;

	/* Take the first wl interface */
	sprintf(tmp, "ess%d_wlnames", target_instance);

	wlnames = wps_safe_get_conf(tmp);
	foreach(ifname, wlnames, next) {
		strncpy(pb_ifname, ifname, sizeof(pb_ifname));
		break;
	}

	return target_id;
}

static int
wps_pb_is_virtual_pressed()
{
	if (wps_virtual_pressed == WPS_LONG_BTNPRESS) {
		wps_virtual_pressed = WPS_NO_BTNPRESS;
		return WPS_LONG_BTNPRESS;
	}

	return WPS_NO_BTNPRESS;
}

#ifdef BCMWPSAPSTA
static void
wps_pb_virtual_btn_pressed()
{
	wps_virtual_pressed = WPS_LONG_BTNPRESS;
}
#endif /* BCMWPSAPSTA */

static int
wps_pb_retrieve_event(int unit, char *wps_ifname)
{
	int press;
	int event = WPSBTN_EVTI_NULL;
	int imax = wps_get_ess_num();
	bool wps_pbc_apsta_enabled = FALSE;

	if (strcmp(wps_safe_get_conf("wps_pbc_apsta"), "enabled") == 0)
		wps_pbc_apsta_enabled = TRUE;

	/*
	 * According to WPS Usability and Protocol Best Practices spec
	 * section 3.1 WPS Push Button Enrollee Addition sub-section 2.2.
	 * Skip using LAN led to indicate which ESS is going
	 * to run WPS when WPS ESS only have one
	 */
	press = wps_hal_btn_pressed();
	if (wps_pbc_apsta_enabled == TRUE && press == WPS_NO_BTNPRESS)
		press = wps_pb_is_virtual_pressed();

	if ((imax == 1 || wps_pbc_apsta_enabled == TRUE) &&
	    (press == WPS_LONG_BTNPRESS || press == WPS_SHORT_BTNPRESS)) {
		wps_pb_find_next();
		wps_pb_state = WPS_PB_STATE_CONFIRM;

		if (wps_pbc_apsta_enabled == FALSE)
			strcpy(wps_ifname, pb_ifname);
		else {
			/*
			 * For apsta mode (wps_pbc_apsta enabled),
			 * wps_monitor decide which interface to run WPS when HW PBC pressed
			 */
			if (wps_pbc_apsta_upstream_pushed == TRUE)
				strcpy(wps_ifname, wps_safe_get_conf("wps_pbc_sta_ifname"));
			else
				strcpy(wps_ifname, wps_safe_get_conf("wps_pbc_ap_ifname"));

			TUTRACE((TUTRACE_INFO, "wps_pbc_apsta mode enabled, wps_ifname = %s \n",
				wps_ifname));
		}
		return WPSBTN_EVTI_PUSH;
	}

	switch (press) {
	case WPS_LONG_BTNPRESS:
		TUTRACE((TUTRACE_INFO, "Detected a wps LONG button-push\n"));

		if (pb_last_led_id == -1)
			wps_pb_find_next();

		/* clear LAN all leds first */
		if (wps_pb_state == WPS_PB_STATE_INIT)
			wps_led_wl_select_on();

		wps_led_wl_confirmed(pb_last_led_id);

		wps_pb_state = WPS_PB_STATE_CONFIRM;
		strcpy(wps_ifname, pb_ifname);

		event = WPSBTN_EVTI_PUSH;
		break;

	/* selecting */
	case WPS_SHORT_BTNPRESS:
		TUTRACE((TUTRACE_INFO, "Detected a wps SHORT button-push, "
			"wps_pb_state = %d\n", wps_pb_state));

		if (wps_pb_state == WPS_PB_STATE_INIT ||
			wps_pb_state == WPS_PB_STATE_SELECTING) {

			/* start bssid selecting, find next enabled bssid */
			/*
			 * NOTE: currently only support wireless unit 0
			 */
			wps_pb_find_next();

			/* clear LAN all leds when first time enter */
			if (wps_pb_state == WPS_PB_STATE_INIT)
				wps_led_wl_select_on();

			wps_led_wl_selecting(pb_last_led_id);

			/* get selecting time */
			wps_pb_state = WPS_PB_STATE_SELECTING;
			wps_pb_selecting_time = time(0);
		}
		break;

	default:
		break;
	}

	return event;
}

#ifdef BCMWPSAPSTA
static bool
wps_pbc_apsta_HW_pressed()
{
	int pbc = WPS_UI_PBC_NONE;
	char *val;

	val = wps_ui_get_env("wps_pbc_method");
	if (val)
		pbc = atoi(val);

	/* Retrieve ENV */
	if (pbc ==  WPS_UI_PBC_HW)
		return TRUE;
	else
		return FALSE;
}

static void
wps_pbc_apsta_scan(int session_opened)
{
	int find_pbc;
	unsigned long now;
	uint8 bssid[6];
	char ssid[SIZE_SSID_LENGTH] = "";
	uint8 wsec = 1;

	if (strcmp(wps_safe_get_conf("wps_pbc_apsta"), "enabled") == 0 &&
	    wps_pbc_apsta_HW_pressed() == TRUE && session_opened == 0) {

		now = get_current_time();

		/* continue to find pbc ap or check associating status */
		wps_osl_set_ifname(wps_safe_get_conf("wps_pbc_sta_ifname"));
		switch (wps_enr_scan_state) {
		case WPS_FINDPBC_STATE_INIT:
			do_wps_scan();
			wps_enr_scan_state = WPS_FINDPBC_STATE_SCANNING;
			break;

		case WPS_FINDPBC_STATE_SCANNING:
			/* keep checking scan results for 10 second and issue scan again */
			if ((now - wps_enr_scan_state_time) > 10) {
				TUTRACE((TUTRACE_INFO,  "%s: do_wps_scan\n", __FUNCTION__));
				do_wps_scan();
				wps_enr_scan_state_time = get_current_time();
			}
			else if (get_wps_scan_results() != NULL) {
				/* got scan results, check to find pbc ap state */
				wps_enr_scan_state = WPS_FINDPBC_STATE_FINDINGAP;
			}
			break;

		case WPS_FINDPBC_STATE_FINDINGAP:
			/* find pbc ap */
			find_pbc = find_pbc_ap((char *)bssid, (char *)ssid, &wsec);
			if (find_pbc == PBC_OVERLAP) {
				TUTRACE((TUTRACE_INFO,  "%s: PBC_OVERLAP\n", __FUNCTION__));
				wps_enr_scan_state = WPS_FINDPBC_STATE_INIT;
				break;
			}
			if (find_pbc == PBC_FOUND_OK) {
				TUTRACE((TUTRACE_INFO,  "%s: PBC_FOUND_OK\n", __FUNCTION__));

				/* 1. Switch wps_pbc_apsta_role to WPS_PBC_APSTA_ENR */
				wps_pbc_apsta_upstream_pushed = TRUE;

				/* 2. Generate a virtual PBC pressed event */
				wps_pb_virtual_btn_pressed();

				wps_enr_scan_state = WPS_FINDPBC_STATE_INIT;
				wps_enr_scan_state_time = get_current_time();
			}
			else {
				wps_enr_scan_state = WPS_FINDPBC_STATE_SCANNING;
			}
			break;
		}
	}
}
#endif /* BCMWPSAPSTA */

void
wps_pb_timeout(int session_opened)
{
	time_t curr_time;

	/* check timeout when in WPS_PB_STATE_SELECTING */
	if (wps_pb_state == WPS_PB_STATE_SELECTING) {
		curr_time = time(0);
		if (curr_time > (wps_pb_selecting_time+WPS_PB_SELECTING_MAX_TIMEOUT)) {
			/* Reset pb state to init because of timed-out */
			wps_pb_state_reset();
		}
	}

#ifdef BCMWPSAPSTA
	/* for apsta mode concurrent wps */
	wps_pbc_apsta_scan(session_opened);
#endif /* BCMWPSAPSTA */
}

int
wps_pb_state_reset()
{
	/* reset wps_pb_state to INIT */
	pb_last_led_id = -1;

	wps_pb_state = WPS_PB_STATE_INIT;

	/* Back to NORMAL */
	wps_hal_led_wl_select_off();

	return 0;
}


wps_hndl_t *
wps_pb_check(char *buf, int *buflen)
{
	char wps_ifname[IFNAMSIZ] = {0};

	/* note: push button currently only support wireless unit 0 */
	/* user can use PBC to tigger wps_enr start */
	if (WPSBTN_EVTI_PUSH == wps_pb_retrieve_event(0, wps_ifname)) {
		int uilen = 0;

		uilen += sprintf(buf + uilen, "SET ");

		TUTRACE((TUTRACE_INFO, "wps monitor: Button pressed!!\n"));

		wps_close_session();

		uilen += sprintf(buf + uilen, "wps_config_command=%d ", WPS_UI_CMD_START);
		uilen += sprintf(buf + uilen, "wps_ifname=%s ", wps_ifname);
		uilen += sprintf(buf + uilen, "wps_method=%d ", WPS_UI_METHOD_PBC);
		uilen += sprintf(buf + uilen, "wps_pbc_method=%d ", WPS_UI_PBC_HW);

		wps_setProcessStates(WPS_ASSOCIATED);

		/* for wps_enr application, start it right now */
		if (wps_is_wps_enr(wps_ifname)) {
			uilen += sprintf(buf + uilen, "wps_action=%d ", WPS_UI_ACT_ENROLL);
		}
		/* for wps_ap application */
		else {
			uilen += sprintf(buf + uilen, "wps_action=%d ", WPS_UI_ACT_ADDENROLLEE);
		}

		uilen += sprintf(buf + uilen, "wps_sta_pin=00000000 ");

		*buflen = uilen;
		return &pb_hndl;
	}

	return NULL;
}

void
wps_pb_reset()
{
	wps_pbc_apsta_upstream_pushed = FALSE;
}

int
wps_pb_init()
{
	memset(pbc_info, 0, sizeof(pbc_info));

	memset(&pb_hndl, 0, sizeof(pb_hndl));
	pb_hndl.type = WPS_RECEIVE_PKT_PB;
	pb_hndl.handle = -1;

	return (wps_hal_btn_init());
}

int
wps_pb_deinit()
{
	wps_pb_state_reset();

	wps_hal_btn_cleanup();

	return 0;
}
