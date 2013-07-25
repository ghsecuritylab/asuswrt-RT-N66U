/*
 * WPS ENROLL thread (Platform independent portion)
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wps_enr_core.c 241376 2011-02-18 03:19:15Z stakita $
 *
 */

#include <stdio.h>
#include <unistd.h>

#if defined(__ECOS)
#include <proto/ethernet.h>
#endif

#include <wpserror.h>
#include <portability.h>
#include <reg_prototlv.h>
#include <wps_wl.h>
#include <wps_enrapi.h>
#include <wps_sta.h>
#include <wps_enr.h>
#include <wps_ui.h>
#include <wps_enr_osl.h>
#include <wps_version.h>
#include <wps_staeapsm.h>
#include <wpscommon.h>
#include <bcmutils.h>
#include <shutils.h>
#include <wlif_utils.h>
#include <tutrace.h>

#if !defined(MOD_VERSION_STR)
#error "wps_version.h doesn't exist !"
#endif

static char def_pin[9] = "12345670\0";
static unsigned long start_time;
static char *pin = def_pin; /* set pin to default */
static uint8 bssid[6];
static char ssid[SIZE_SSID_LENGTH] = "broadcom\0";
static uint8 wsec = 1; /* by default, assume wep is ON */
static uint8 enroll_again = false;
static bool b_wps_version2 = false;
static uint8 empty_mac[SIZE_MAC_ADDR] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define WPS_ASSOC_STATE_INIT		0
#define WPS_ASSOC_STATE_SCANNING	1
#define WPS_ASSOC_STATE_FINDINGAP	2
#define WPS_ASSOC_STATE_ASSOCIATING	3
#define WPS_ASSOC_STATE_ASSOCIATED	4
static char assoc_state = WPS_ASSOC_STATE_INIT;
static unsigned long assoc_state_time;

static uint32 config_init();
static int display_aplist(wps_ap_list_info_t *aplist);
static int do_enrollement_again(void);
static int wpsenr_deinit(void *app_wksp);

int find_pbc_ap(char *bssid, char *ssid, uint8 *wsec);


/* do join network without waiting association */
static void
do_join(uint8 again)
{
	enroll_again = again;

	leave_network();
	WpsSleep(1);
	wpsenr_osl_proc_states(WPS_ASSOCIATING);
	join_network_with_bssid(ssid, wsec, bssid);
	assoc_state = WPS_ASSOC_STATE_ASSOCIATING;
	assoc_state_time = get_current_time();
}

/*
 * Name        : wpsenr_wksp_mainloop
 * Description : Main loop point for the WPS stack
 * Arguments   : wpsenr_param_t *param - argument set
 * Return type : int
 */
static int
wpsenr_init(char *ifname)
{
	int pbc = WPS_UI_PBC_SW;
	char start_ok = false;
	char user_ssid = false;
	char user_bssid = false;
	char pbc_requested = false;
	char user_pin = false;
	char scan = false;
	char enr_auto = false;
	char *val;
	wps_ap_list_info_t *wpsaplist;
	char *bssid_ptr = NULL;

	char op[6] = {0};
	unsigned char *env_ssid = NULL;
	unsigned char *env_sec = NULL;
	unsigned char *env_bssid = NULL;
	unsigned char *env_pin = NULL;
#ifdef __CONFIG_WFI__
	char *ui_env_pin = NULL;
#endif /* __CONFIG_WFI__ */


	printf("*********************************************\n");
	printf("WPS - Enrollee App Broacom Corp.\n");
	printf("Version: %s\n", MOD_VERSION_STR);
	printf("*********************************************\n");

	/* we need to specify the if name before anything else */
	if (!ifname) {
		TUTRACE((TUTRACE_INFO, "no ifname exist!! return\n"));
		return 0;
	}

	/* WSC 2.0,  support WPS V2 or not */
	if (strcmp(wps_safe_get_conf("wps_version2"), "enabled") == 0)
		b_wps_version2 = true;

	wps_set_ifname(ifname);
	wps_osl_set_ifname(ifname);

	/* reset assoc_state in INIT state */
	assoc_state = WPS_ASSOC_STATE_INIT;

	/* reset enroll_again */
	enroll_again = false;

	/* Check whether scan needed */
	val = wps_ui_get_env("wps_enr_scan");
	if (val)
		scan = atoi(val);

	/* if scan requested : display and exit */
	if (scan) {
		/* do scan and wait the scan results */
		do_wps_scan();
		while (get_wps_scan_results() == NULL)
			WpsSleep(1);

		/* use scan result to create ap list */
		wpsaplist = create_aplist();
		if (wpsaplist) {
			display_aplist(wpsaplist);
			wps_get_aplist(wpsaplist, wpsaplist);

			printf("WPS Enabled AP list :\n");
			display_aplist(wpsaplist);
		}
		goto exit;
	}

	val = wps_ui_get_env("wps_pbc_method");
	if (val)
		pbc = atoi(val);

	/* Retrieve ENV */
	if (pbc ==  WPS_UI_PBC_HW) {
		strcat(op, "pb");
	}
	else {
		env_sec = wps_ui_get_env("wps_enr_wsec");
		env_ssid = wps_ui_get_env("wps_enr_ssid");
		env_bssid = wps_ui_get_env("wps_enr_bssid");

		/* SW PBC */
		if (atoi(wps_ui_get_env("wps_method")) == WPS_UI_METHOD_PBC) {
			strcat(op, "pb");
		}
		else { /* PIN */
			strcat(op, "pin");
			env_pin = wps_get_conf("wps_device_pin");
#ifdef __CONFIG_WFI__
			/* For WiFi-Invite session PIN */
			ui_env_pin = wps_ui_get_env("wps_device_pin");
			if (ui_env_pin[0] != '\0')
				env_pin = ui_env_pin;
#endif /* __CONFIG_WFI__ */
		}
	}

	/* Check whether automatically enroll */
	val = wps_ui_get_env("wps_enr_auto");
	if (val)
		enr_auto = atoi(val);

	TUTRACE((TUTRACE_INFO, "pbc = %s%s, wpsenr param: ifname = %s, op = %s, sec = %s, "
		"ssid = %s, bssid = %s, pin = %s\n",
		(pbc == 1? "HW_PBC": "SW_PBC"),
		(enr_auto == 1? " Auto": ""),
		ifname,
		op,
		(env_sec? (char *)env_sec : "NULL"),
		(env_ssid? (char *)env_ssid : "NULL"),
		(env_bssid? (char *)env_bssid : "NULL"),
		(env_pin? (char *)env_pin : "NULL")));

	if (env_ssid) {
		strcpy(ssid, env_ssid);
		user_ssid = true;
	}

	if (env_bssid) {
		/* 
		 * WARNING : this "bssid" is used only to create an 802.1X socket.
		 *
		 * Normally, it should be the bssid of the AP we will associate to.
		 *
		 * Setting this manually means that we might be proceeding to
		 * eapol exchange with a different AP than the one we are associated to,
		 * which might work ... or not.
		 *
		 * When implementing an application, one might want to enforce association
		 * with the AP with that particular BSSID. In case of multiple AP
		 * on the ESS, this might not be stable with roaming enabled.
		 */
		ether_atoe(env_bssid, bssid);
		user_bssid = true;
		bssid_ptr = bssid;
	}
	if (!strcmp(op, "pin")) {
		val = env_pin;
		pin = val;
		user_pin = true;
	}
	else if (!strcmp(op, "pb")) {
		pin = NULL;
		user_pin = true;
		pbc_requested = true;
	}
	if (env_sec) {
		wsec = atoi(env_sec);
	}

	/* 
	 * setup device configuration for WPS
	 * needs to be done before eventual scan for PBC.
	 */
	if (config_init() != WPS_SUCCESS) {
		printf("Config initial failed, exit.\n");
		goto exit;
	}

	/* if ssid specified, use it */
	if (user_ssid && !enr_auto) {
		if (pbc_requested) {
			pin = NULL;
		}
		else if (!pin) {
			pin = def_pin;
			printf("\n\nStation Pin not specified, use default Pin %s\n\n", def_pin);
		}
		start_ok = true;
	}
	else if (pbc_requested) {
		/* handle enr_auto here... */
		wpsenr_osl_proc_states(WPS_FIND_PBC_AP);

		/* add wps ie to probe  */
		add_wps_ie(NULL, 0, TRUE, b_wps_version2);
		do_wps_scan();
		assoc_state_time = get_current_time();
		assoc_state = WPS_ASSOC_STATE_SCANNING;

		start_ok = false;
	}
	else {
		printf("\n\nPlease specify ssid or use pbc method\n\n");
		goto exit;
	}

	/* start WPS two minutes period at Finding a PBC AP or Associating with AP */
	start_time = get_current_time();

	if (start_ok) {
		/* clear current security setting */
		wpsenr_osl_clear_wsec();

		/*
		 * join. If user_bssid is specified, it might not
		 * match the actual associated AP.
		 * An implementation might want to make sure
		 * it associates to the same bssid.
		 * There might be problems with roaming.
		 */
		do_join(false);
		return WPS_SUCCESS;
	}
	else if (assoc_state == WPS_ASSOC_STATE_SCANNING) {
		return WPS_SUCCESS;
	}

	/* restore security settings if initiation failed */
	wpsenr_osl_restore_wsec();
	wpsenr_deinit(NULL);

exit: /* No cleanup */
	return WPS_ERR_SYSTEM;
}

static int
wpsenr_start()
{
	char *val;
	char user_bssid = false;
	uint32 len;
	uint band_num, active_band;
	int pbc = WPS_UI_PBC_SW;
	unsigned char *env_bssid = NULL;
	uint8 assoc_bssid[6];


	val = wps_ui_get_env("wps_pbc_method");
	if (val)
		pbc = atoi(val);

	/* Retrieve user specified bssid ENV */
	if (pbc !=  WPS_UI_PBC_HW) {
		env_bssid = wps_ui_get_env("wps_enr_bssid");
	}

	/* update specific RF band */
	wps_get_bands(&band_num, &active_band);
	if (active_band == WLC_BAND_5G)
		active_band = WPS_RFBAND_50GHZ;
	else if (active_band == WLC_BAND_2G)
		active_band = WPS_RFBAND_24GHZ;
	else
		active_band = WPS_RFBAND_24GHZ;
	wps_update_RFBand((uint8)active_band);


	/*
	 * We always retrieve bssid from associated AP. If driver support join network
	 * with bssid, the retrieved bssid should be same as user specified. Otherwise
	 * there might be problems with roaming.
	 */

	/* If user_bssid not defined, use associated AP's */
	if (wps_get_bssid(assoc_bssid)) {
		printf("Can not get [%s] BSSID, Quit....\n", ssid);
		goto err_exit;
	}

	/* check associated bssid consistent with user specified */
	if (user_bssid && memcmp(assoc_bssid, bssid, 6) != 0) {
		/* warning to user */
		printf("User specified BSSID %02x:%02x:%02x:%02x:%02x:%02x, but "
			"connect to %02x:%02x:%02x:%02x:%02x:%02x\n",
			bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],
			assoc_bssid[0], assoc_bssid[1], assoc_bssid[2], assoc_bssid[3],
			assoc_bssid[4], assoc_bssid[5]);
		memcpy(bssid, assoc_bssid, 6);
	}

	/* setup raw 802.1X socket with "bssid" destination  */
	if (wps_osl_init(bssid) != WPS_SUCCESS) {
		printf("Initializing 802.1x raw socket failed. \n");
		goto err_exit;
	}

	printf("Start enrollment for BSSID: %02x:%02x:%02x:%02x:%02x:%02x\n", bssid[0],
		bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

	wpsenr_osl_proc_states(WPS_ASSOCIATED); /* START ENROLLING */
	if (wps_start_enrollment(pin, start_time) != WPS_SUCCESS) {
		TUTRACE((TUTRACE_ERR, "Start enrollment failed!\n"));
		goto err_exit;
	}

	/* 
	 * start the process by sending the eapol start . Created from the
	 * Enrollee SM Initialize.
	 */
	len = wps_get_msg_to_send(&val, start_time);
	if (val) {
		send_eapol_packet(val, len);
		printf("Send EAPOL-Start\n");
	}
	else {
		/* this means the system is not initialized */
		return WPS_ERR_NOT_INITIALIZED;
	}

	return WPS_SUCCESS;

err_exit: /* Do cleanup */
	wpsenr_osl_restore_wsec();
	wpsenr_deinit(NULL);

	return WPS_ERR_SYSTEM;
}

static int
wpsenr_deinit(void *app_wksp)
{
	wps_cleanup();
	return 0;
}

static int
wpsenr_process(void *app_wksp, char *buf, int len, int msgtype)
{
	uint32 buf_len = len;
	char *sendBuf;
	int last_recv_msg;
	char msg_type;
	unsigned long now = get_current_time();
	uint32 retVal;
	int state;
	char *assoc_state_name[] = {"init", "scanning", "findingap", "associating", "associated"};

	/* associate state checking */
	if (assoc_state != WPS_ASSOC_STATE_ASSOCIATED) {
		printf("In assoc state %s, ignore this packet\n",
			assoc_state_name[(int)assoc_state]);
		return WPS_CONT; /* ingore it */
	}

	/* eapol packet validation */
	retVal = wpsenr_eapol_validate(buf, &buf_len);
	if (retVal == WPS_SUCCESS) {
		/* Show receive message */
		msg_type = wps_get_msg_type(buf, buf_len);
		printf("Receive EAP-Request%s\n", wps_get_msg_string((int)msg_type));

		/* process ap message */
		retVal = wps_process_ap_msg(buf, buf_len);

		/* check return code to do more things */
		if (retVal == WPS_SEND_MSG_CONT ||
			retVal == WPS_SEND_MSG_SUCCESS ||
			retVal == WPS_SEND_MSG_ERROR) {
			len = wps_get_eapol_msg_to_send(&sendBuf, now);
			if (sendBuf) {
				msg_type = wps_get_msg_type(sendBuf, len);

				send_eapol_packet(sendBuf, len);
				printf("Send EAP-Response%s\n",
					wps_get_msg_string((int)msg_type));
			}

			/* over-write retVal */
			if (retVal == WPS_SEND_MSG_SUCCESS)
				retVal = WPS_SUCCESS;
			else if (retVal == WPS_SEND_MSG_ERROR)
				retVal = REG_FAILURE;
			else
				retVal = WPS_CONT;
		}
		else if (retVal == EAP_FAILURE) {
			/* we received an eap failure from registrar */
			/*
			 * check if this is coming AFTER the protocol passed the M2
			 * mark or is the end of the discovery after M2D.
			 */
			last_recv_msg = wps_get_recv_msg_id();
			printf("Received eap failure, last recv msg EAP-Request%s\n",
				wps_get_msg_string(last_recv_msg));
			if (last_recv_msg > WPS_ID_MESSAGE_M2D) {
				retVal = REG_FAILURE;
			}
			else {
				/* do join again */
				do_join(true);
				retVal = WPS_CONT;
			}
		}
		/* special case, without doing wps_eap_create_pkt */
		else if (retVal == WPS_SEND_MSG_IDRESP) {
			len = wps_get_msg_to_send(&sendBuf, now);
			if (sendBuf) {
				send_eapol_packet(sendBuf, len);
				printf("Send EAP-Response / Identity\n");
			}
			retVal = WPS_CONT;
		}
		/* Re-transmit last sent message, because we receive a re-transmit packet */
		else if (retVal == WPS_SEND_RET_MSG_CONT) {
			len = wps_get_retrans_msg_to_send(&sendBuf, now, &msg_type);
			if (sendBuf) {
				state = wps_get_eap_state();

				if (state == EAPOL_START_SENT)
					printf("Re-Send EAPOL-Start\n");
				else if (state == EAP_IDENTITY_SENT)
					printf("Re-Send EAP-Response / Identity\n");
				else
					printf("Re-Send EAP-Response%s\n",
						wps_get_msg_string((int)msg_type));

				send_eapol_packet(sendBuf, len);
			}
		}
		else if (retVal == WPS_SEND_FRAG_CONT ||
			retVal == WPS_SEND_FRAG_ACK_CONT) {
			len = wps_get_frag_msg_to_send(&sendBuf, now);
			if (sendBuf) {
				if (retVal == WPS_SEND_FRAG_CONT)
					printf("Send EAP-Response(FRAG)\n");
				else
					printf("Send EAP-Response(FRAG_ACK)\n");

				send_eapol_packet(sendBuf, len);
			}
		}

		/*
		 * exits with either success, failure or indication that
		 *  the registrar has not started its end of the protocol yet.
		 */
		if (retVal == WPS_SUCCESS) {
			char keystr[65];
			int len = 0;
			char ssid[SIZE_SSID_LENGTH];
			WpsEnrCred *credential = (WpsEnrCred *)malloc(sizeof(WpsEnrCred));

			if (credential == NULL) {
				printf("memory allocate failed!!\n");
				wpsenr_osl_proc_states(WPS_MSG_ERR); /* FAILED */
				retVal = WPS_RESULT_FAILURE;
			}

			printf("WPS Protocol SUCCEEDED !!\n");

			/* get credentials */
			wps_get_ssid(ssid, &len);
			wps_get_credentials(credential, ssid, len);
			printf("SSID = %s\n", credential->ssid);
			printf("Key Mgmt type is %s\n", credential->keyMgmt);
			strncpy(keystr, credential->nwKey, credential->nwKeyLen);
			keystr[credential->nwKeyLen] = 0;
			printf("WPA-PSK key : %s\n", keystr);
			if (credential->encrType == ENCRYPT_NONE) {
				printf("Encryption : NONE\n");
			}
			else {
				if (credential->encrType & ENCRYPT_WEP)
					printf("Encryption :  WEP\n");
				if (credential->encrType & ENCRYPT_TKIP)
					printf("Encryption :  TKIP\n");
				if (credential->encrType & ENCRYPT_AES)
					printf("Encryption :  AES\n");
			}

			wpsenr_osl_proc_states(WPS_OK); /* SUCCEEDED */

			if (wpsenr_osl_set_wsec(credential))
				retVal  = WPS_RESULT_SUCCESS_RESTART;
			else
				retVal  = WPS_RESULT_SUCCESS;

			/* free memory */
			free(credential);
		}
		else if (retVal != WPS_CONT) {
			printf("WPS Protocol FAILED. CODE = %x\n", retVal);
			wpsenr_osl_proc_states(WPS_MSG_ERR); /* FAILED */
			retVal = WPS_RESULT_FAILURE;
		}

		/* Failure, restore original wireless settings */
		if (retVal != WPS_RESULT_SUCCESS_RESTART &&
			retVal != WPS_RESULT_SUCCESS &&
			retVal != WPS_CONT) {
			wpsenr_osl_restore_wsec();
		}
	}

	return retVal;
}

static int
wpsenr_check_timeout(void *app_wksp)
{
	int state, find_pbc;
	int last_sent_msg;
	uint8 msg_type;
	char *sendBuf;
	uint32 sendBufLen;
	uint32 retVal = WPS_CONT;
	unsigned long now = get_current_time();
	uint8 assoc_bssid[6];


	if (now > start_time + 120) {
		printf("Overall protocol timeout \n");
		switch (assoc_state) {
		case WPS_ASSOC_STATE_SCANNING:
		case WPS_ASSOC_STATE_FINDINGAP:
			rem_wps_ie(NULL, 0, VNDR_IE_PRBREQ_FLAG);
			if (b_wps_version2)
				rem_wps_ie(NULL, 0, VNDR_IE_ASSOCREQ_FLAG);
			break;

		default:
			break;
		}

		wpsenr_osl_proc_states(WPS_TIMEOUT);
		return REG_FAILURE;
	}

	/* continue to find pbc ap or check associating status */
	switch (assoc_state) {
	case WPS_ASSOC_STATE_SCANNING:
		/* keep checking scan results for 10 second and issue scan again */
		if ((now - assoc_state_time) > 10 /* WPS_SCAN_MAX_WAIT_SEC */) {
			do_wps_scan();
			assoc_state_time = get_current_time();
		}
		else if (get_wps_scan_results() != NULL) {
			/* got scan results, check to find pbc ap state */
			assoc_state = WPS_ASSOC_STATE_FINDINGAP;
		}
		/* no need to do wps_eap_check_timer  */
		return WPS_CONT;

	case WPS_ASSOC_STATE_FINDINGAP:
		/* find pbc ap */
		find_pbc = find_pbc_ap((char *)bssid, (char *)ssid, &wsec);
		if (find_pbc == PBC_OVERLAP || find_pbc == PBC_TIMEOUT) {
			/* PBC_TIMEOUT should not happen anymore */
			rem_wps_ie(NULL, 0, VNDR_IE_PRBREQ_FLAG);
			if (b_wps_version2)
				rem_wps_ie(NULL, 0, VNDR_IE_ASSOCREQ_FLAG);

			if (find_pbc == PBC_OVERLAP)
				wpsenr_osl_proc_states(WPS_PBCOVERLAP);
			else
				wpsenr_osl_proc_states(WPS_TIMEOUT);

			return REG_FAILURE;
		}

		if (find_pbc == PBC_FOUND_OK) {
			do_join(FALSE);
			rem_wps_ie(NULL, 0, VNDR_IE_PRBREQ_FLAG);
			if (b_wps_version2)
				rem_wps_ie(NULL, 0, VNDR_IE_ASSOCREQ_FLAG);
		}
		else {
			/* do scan again */
			do_wps_scan();
			assoc_state = WPS_ASSOC_STATE_SCANNING;
			assoc_state_time = get_current_time();
		}
		/* no need to do wps_eap_check_timer  */
		return WPS_CONT;

	case WPS_ASSOC_STATE_ASSOCIATING:
		/* keep checking bssid status for 10 second and issue join again */
		if ((now - assoc_state_time) > 10 /* WPS_ENR_SCAN_JOIN_MAX_WAIT_SEC */) {
			join_network_with_bssid(ssid, wsec, bssid);
			assoc_state_time = get_current_time();
		}
		else if (wps_get_bssid(assoc_bssid) == 0) {
			/* associated with AP */
			assoc_state = WPS_ASSOC_STATE_ASSOCIATED;

			/* start wps */
			if (enroll_again) {
				if (do_enrollement_again() != 0)
					return REG_FAILURE;
			}
			else if (wpsenr_start() != WPS_SUCCESS)
				return WPS_ERR_OPEN_SESSION;
		}
		/* no need to do wps_eap_check_timer  */
		return WPS_CONT;

	default:
		break;
	}

	/* check eap receive timer. It might be time to re-transmit */
	/*
	 * Do we need this API ? We could just count how many times
	 * we re-transmit right here.
	 */
	retVal = wps_eap_check_timer(now);
	if (retVal == WPS_SEND_RET_MSG_CONT) {
		sendBufLen = wps_get_retrans_msg_to_send(&sendBuf, now, &msg_type);
		if (sendBuf) {
			state = wps_get_eap_state();

			if (state == EAPOL_START_SENT)
				printf("Re-Send EAPOL-Start\n");
			else if (state == EAP_IDENTITY_SENT)
				printf("Re-Send EAP-Response / Identity\n");
			else
				printf("Re-Send EAP-Response%s\n",
					wps_get_msg_string((int)msg_type));

			send_eapol_packet(sendBuf, sendBufLen);
		}
		retVal = WPS_CONT;
	}
	/* re-transmission count exceeded, give up */
	else if (retVal == EAP_TIMEOUT) {
		char last_recv_msg = wps_get_recv_msg_id();

		retVal = WPS_CONT;
		if (last_recv_msg == WPS_ID_MESSAGE_M2D) {
			printf("M2D Wait timeout, again.\n");

			/* do join again */
			do_join(true);
		}
		else if (last_recv_msg > WPS_ID_MESSAGE_M2D) {
			last_sent_msg = wps_get_sent_msg_id();
			printf("Timeout, give up. Last recv/sent msg "
				"[EAP-Response%s/EAP-Request%s]\n",
				wps_get_msg_string(last_recv_msg),
				wps_get_msg_string(last_sent_msg));
			retVal = REG_FAILURE;
		}
		else {
			printf("Re-transmission count exceeded, again\n");

			/* do join again */
			do_join(true);
		}

		/* Failure, restore original wireless settings */
		if (retVal != WPS_CONT)
			wpsenr_osl_restore_wsec();
	}

	return retVal;
}

/*
 * find an AP with PBC active or timeout.
 * Returns SSID and BSSID.
 * Note : when we join the SSID, the bssid of the AP might be different
 * than this bssid, in case of multiple AP in the ESS ...
 * Don't know what to do in that case if roaming is enabled ...
 */
int
find_pbc_ap(char * bssid, char *ssid, uint8 *wsec)
{
	int pbc_ret = PBC_NOT_FOUND;
	wps_ap_list_info_t *wpsaplist;

	/* caller has to add wps ie and rem wps ie by itself */
	wpsaplist = create_aplist();
	if (wpsaplist) {
		wps_get_aplist(wpsaplist, wpsaplist);
		display_aplist(wpsaplist);
		/* pbc timeout handled by caller */
		pbc_ret = wps_get_pbc_ap(wpsaplist, bssid, ssid,
			wsec, get_current_time(), true);
	}

	return pbc_ret;
}

/* 
 * Fill up the device info and pass it to WPS.
 * This will need to be tailored to specific platforms (read from a file,
 * nvram ...)
 */

static uint32
config_init()
{
	DevInfo info;
	unsigned char mac[6];
	char *value;
	char uuid[16] = {0x22, 0x21, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0xa, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

	/* fill in device specific info. The way this information is stored is app specific */
	/* Would be good to document all of these ...  */

	memset((char *)(&info), 0, sizeof(info));
	info.version = WPS_VERSION;

	/* MAC addr */
	wps_osl_get_mac(mac);
	memcpy(info.macAddr, mac, 6);

	memcpy(info.uuid, uuid, 16);
	strcpy(info.deviceName, "Broadcom Client");
	info.primDeviceCategory = 1;
	info.primDeviceOui = 0x0050F204;
	info.primDeviceSubCategory = 1;
	strcpy(info.manufacturer, "Broadcom");
	strcpy(info.modelName, "WPS Wireless Client");
	strcpy(info.modelNumber, "1234");
	strcpy(info.serialNumber, "5678");

	if (b_wps_version2) {
		info.configMethods = (WPS_CONFMET_VIRT_PBC | WPS_CONFMET_PHY_PBC |
			WPS_CONFMET_VIRT_DISPLAY);
	} else {
		info.configMethods = WPS_CONFMET_PBC | WPS_CONFMET_DISPLAY;
	}

	/* WSC 2.0, WPS-PSK and SHARED are deprecated.
	 * When both the Registrar and the Enrollee are using protocol version 2.0
	 * or newer, this variable can use the value 0x0022 to indicate mixed mode
	 * operation (both WPA-Personal and WPA2-Personal enabled)
	 * NOTE: BCMWPA2 compile option MUST enabled
	 */
	if (b_wps_version2) {
		info.authTypeFlags = (uint16)(WPS_AUTHTYPE_OPEN | WPS_AUTHTYPE_WPAPSK |
			WPS_AUTHTYPE_WPA2PSK);
	} else {
		info.authTypeFlags = (uint16)(WPS_AUTHTYPE_OPEN | WPS_AUTHTYPE_WPAPSK |
			WPS_AUTHTYPE_SHARED | WPS_AUTHTYPE_WPA2PSK);
	}

	/* ENCR_TYPE_FLAGS */
	/*
	 * WSC 2.0, deprecated WEP. TKIP can only be advertised on the AP when
	 * Mixed Mode is enabled (Encryption Type is 0x000c)
	 */
	if (b_wps_version2) {
		info.encrTypeFlags = (uint16)(WPS_ENCRTYPE_NONE | WPS_ENCRTYPE_TKIP |
			WPS_ENCRTYPE_AES);
	} else {
		info.encrTypeFlags = (uint16)(WPS_ENCRTYPE_NONE | WPS_ENCRTYPE_WEP |
			WPS_ENCRTYPE_TKIP | WPS_ENCRTYPE_AES);
	}

	info.connTypeFlags = WPS_CONNTYPE_ESS;
	info.rfBand = WPS_RFBAND_24GHZ;
	info.osVersion = 0x80000000;
	info.featureId = 0x80000000;

	/* WSC 2.0 */
	if (b_wps_version2) {
		value = wps_get_conf("wps_version2_num");
		info.version2 = (uint8)(strtoul(value, NULL, 16));
		info.settingsDelayTime = WPS_SETTING_DELAY_TIME_LINUX;
		info.b_reqToEnroll = TRUE;
		info.b_nwKeyShareable = FALSE;
	}

	return (wps_enr_config_init(&info));
}

static int
do_enrollement_again(void)
{
	char *data;
	int len;
	unsigned long current_time = get_current_time();
	uint32 ret;

	/* do enrollement again */
	ret = wps_start_enrollment(pin, current_time);
	if (ret != MC_ERR_STACK_ALREADY_STARTED && ret != WPS_SUCCESS) {
		TUTRACE((TUTRACE_ERR, "Start enrollment failed!\n"));
		return -1;
	}

	/* 
	 * start the process by sending the eapol start . Created from the
	 * Enrollee SM Initialize.
	 */
	len = wps_get_msg_to_send(&data, current_time);
	if (data) {
		send_eapol_packet(data, len);
		printf("Send EAPOL-Start\n");
	}
	else {
		/* this means the system is not initialized */
		printf("Can not get EAPOL-Start to send, Quit...\n");
		return -1;
	}

	return 0;

}

int
display_aplist(wps_ap_list_info_t *ap)
{
	int i = 0, j;
	uint8 *mac;
	char eastr[ETHER_ADDR_STR_LEN];

	if (!ap)
		return 0;

	printf("-------------------------------------\n");
	while (ap->used == TRUE) {
		printf(" %d :  ", i);
		printf("SSID:%s  ", ap->ssid);
		printf("BSSID:%s  ", ether_etoa(ap->BSSID, eastr));
		printf("Channel:%d  ", ap->channel);
		if (ap->wep)
			printf("WEP");
		if (b_wps_version2 && ap->version2 >= WPS_VERSION2) {
			printf("V2(0x%02X)  ", ap->version2);

			mac = ap->authorizedMACs;
			printf("AuthroizedMACs:");
			for (j = 0; j < 5; j++) {
				if (memcmp(mac, empty_mac, SIZE_MAC_ADDR) == 0)
					break;

				printf(" %02x:%02x:%02x:%02x:%02x:%02x",
					mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				mac += SIZE_MAC_ADDR;
			}
		}

		printf("\n");
		ap++;
		i++;
	}

	printf("-------------------------------------\n");

	return 0;
}

int
set_mac_address(char *mac_string, char *mac_bin)
{
	int i = 0;
	char *endptr, *nptr;
	long val;
	char mac_addr[20] = {0x0};
	sprintf(mac_addr, "%s", mac_string);

	nptr = mac_addr;
	do {
		val = strtol(nptr, &endptr, 16);
		if (val > 255) {
			printf("invalid MAC address val\n");
			return -1;
		}
		if (endptr == nptr)
		{
			/* no more digits. */
			if (i != 6) {
				printf("MAC address too short\n");
				return -1;
			}
			return 0;
		}
		if (i >= 6) {
			printf("MAC address too long\n");
			return -1;
		}
		mac_bin[i++] = val;
		nptr = endptr+1;
	} while (nptr[0]);

	if (i != 6) {
		printf("invalid MAC address len\n");
		return -1;
	}

	return 0;
}

int
wpsenr_open_session(void *wps_app, char* ifname)
{
	TUTRACE((TUTRACE_INFO, "Start Enroll\n"));

	/* update mode */
	((wps_app_t *)wps_app)->wksp = wpsenr_init(ifname);
	if (((wps_app_t *)wps_app)->wksp != WPS_SUCCESS) {
		((wps_app_t *)wps_app)->wksp = 0; /* means open session failed */
		return WPS_ERR_OPEN_SESSION;
	}

	((wps_app_t *)wps_app)->mode = WPSM_ENROLL;
	((wps_app_t *)wps_app)->close = (int (*)(int))wpsenr_deinit;
	((wps_app_t *)wps_app)->process = (int (*)(int, char *, int, int))wpsenr_process;
	((wps_app_t *)wps_app)->check_timeout = (int (*)(int))wpsenr_check_timeout;

	return WPS_CONT;
}
