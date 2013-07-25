/*
 * WPS upnp
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wps_upnp.c 241376 2011-02-18 03:19:15Z stakita $
 */

#include <osl.h>
#include <stdio.h>
#include <tutrace.h>
#include <time.h>
#include <wps_wl.h>
#include <wps_ui.h>
#include <shutils.h>
#include <wlif_utils.h>
#include <wps_ap.h>
#include <security_ipc.h>
#include <wps.h>
#include <sminfo.h>
#include <reg_proto.h>
#include <wps_upnp.h>
#include <wps_ie.h>
#include <wps_staeapsm.h>
#include <ap_upnp_sm.h>

static upnp_attached_if *upnpif_head = 0;
static unsigned long upnpssr_time = 0;

static void
wps_upnp_attach_wfa_dev(int if_instance)
{
	char buf[] = {"attach WFADevice.xml"};

	/* Send request to UPnP */
	TUTRACE((TUTRACE_ERR, "%s (%d)", buf, if_instance));

	wps_osl_upnp_ipc(if_instance, buf, NULL, NULL);
	return;
}

static void
wps_upnp_detach_wfa_dev(int if_instance)
{
	char buf[] = {"detach WFADevice.xml"};

	/* Send request to UPnP */
	TUTRACE((TUTRACE_ERR, "%s (%d)", buf, if_instance));

	wps_osl_upnp_ipc(if_instance, buf, NULL, NULL);
	return;
}

/*
 * Base64 encoding
 * We should think about put this part in bcmcrypto, because
 * httpd, bcmupnp and wps all need it.
 */
static const char cb64[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char cd64[] =
	"|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";
/* 
 * Base64 block encoding,
 * encode 3 8-bit binary bytes as 4 '6-bit' characters
 */
static void
wps_upnp_base64_encode_block(unsigned char in[3], unsigned char out[4], int len)
{
	switch (len) {
	case 3:
		out[0] = cb64[ in[0] >> 2 ];
		out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
		out[2] = cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ];
		out[3] = cb64[ in[2] & 0x3f ];
		break;
	case 2:
		out[0] = cb64[ in[0] >> 2 ];
		out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
		out[2] = cb64[ ((in[1] & 0x0f) << 2) ];
		out[3] = (unsigned char) '=';
		break;
	case 1:
		out[0] = cb64[ in[0] >> 2 ];
		out[1] = cb64[ ((in[0] & 0x03) << 4)  ];
		out[2] = (unsigned char) '=';
		out[3] = (unsigned char) '=';
		break;
	default:
		break;
		/* do nothing */
	}
}

/*
 * Base64 encode a stream adding padding and line breaks as per spec.
 * input	- stream to encode
 * inputlen	- length of the input stream
 * target	- stream encoded with null ended.
 *
 * Returns The length of the encoded stream.
 */
static int
wps_upnp_base64_encode(unsigned char *input, const int inputlen, unsigned char *target)
{
	unsigned char *out;
	unsigned char *in;

	out = target;
	in  = input;

	if (input == NULL || inputlen == 0) {
		*out = 0;
		return 0;
	}

	while ((in+3) <= (input+inputlen)) {
		wps_upnp_base64_encode_block(in, out, 3);
		in += 3;
		out += 4;
	}

	if ((input+inputlen) - in == 1) {
		wps_upnp_base64_encode_block(in, out, 1);
		out += 4;
	}
	else {
		if ((input+inputlen)-in == 2) {
			wps_upnp_base64_encode_block(in, out, 2);
			out += 4;
		}
	}

	*out = 0;
	return (int)(out - target);
}

/* Perform wlan event notification */
void
wps_upnp_update_wlan_event(int if_instance, unsigned char *macaddr,
	char *databuf, int datalen, int init)
{
	char *base64buf;
	char *buf;
	char *ssr_ipaddr, ipaddr[sizeof("255.255.255.255")] = {"*"};

	/* Allocate buffer for base 64, and message */
	base64buf = (char *)malloc(datalen * 2);
	if (!base64buf)
		return;

	buf = (char *)malloc(datalen * 2 + 128);
	if (!buf) {
		free(base64buf);
		return;
	}

	/* Build base64 encoded event */
	buf[0] = WPS_WLAN_EVENT_TYPE_EAP_FRAME;
	sprintf(buf+1, "%02X:%02X:%02X:%02X:%02X:%02X",
	        macaddr[0], macaddr[1], macaddr[2], macaddr[3],
	        macaddr[4], macaddr[5]);
	memcpy(buf+18, databuf, datalen);

	wps_upnp_base64_encode(buf, datalen+18, base64buf);

	if (databuf && datalen > WPS_MSGTYPE_OFFSET) {
		switch (databuf[WPS_MSGTYPE_OFFSET]) {
		case WPS_ID_MESSAGE_M3:
		case WPS_ID_MESSAGE_M5:
		case WPS_ID_MESSAGE_M7:
			ssr_ipaddr = ap_upnp_sm_get_ssr_ipaddr();
			if (strlen(ssr_ipaddr) != 0)
				strcpy(ipaddr, ssr_ipaddr);
			break;

		default:
			break;
		}
	}

	/* Build IPC commands */
	sprintf(buf,
		"notify %s urn:schemas-wifialliance-org:service:WFAWLANConfig\n"
		"WLANEvent=%s\n",
		ipaddr,
		base64buf);

	/* Append APStatus and STAStatus if init */
	if (init) {
		strcat(buf, "APStatus=1\n");
		strcat(buf, "STAStatus=1\n");
	}

	strcat(buf, "\n");

	/* Perform IPC */
	wps_osl_upnp_ipc(if_instance, buf, NULL, NULL);

	/* Free buffer */
	free(base64buf);
	free(buf);
	return;
}

void
wps_upnp_update_init_wlan_event(int if_instance, char *mac, int init)
{
	uint8 message = WPS_ID_MESSAGE_ACK;
	uint8 version = WPS_VERSION;
	uint8 null_nonce[16] = {0};
	BufferObj *bufObj;

	bufObj = buffobj_new();
	if (!bufObj)
		return;

	/* Compose wps event */
	tlv_serialize(WPS_ID_VERSION, bufObj, &version,   WPS_ID_VERSION_S);
	tlv_serialize(WPS_ID_MSG_TYPE, bufObj, &message,   WPS_ID_MSG_TYPE_S);
	tlv_serialize(WPS_ID_ENROLLEE_NONCE, bufObj, null_nonce, SIZE_128_BITS);
	tlv_serialize(WPS_ID_REGISTRAR_NONCE, bufObj, null_nonce, SIZE_128_BITS);

	wps_upnp_update_wlan_event(if_instance, mac,
		bufObj->pBase, bufObj->m_dataLength, init);

	buffobj_del(bufObj);
	return;
}

/*
 * Functions called by wps monitor
 */
void
wps_upnp_device_uuid(unsigned char *uuid)
{
	int i;
	char *value;
	unsigned int wps_uuid[16];

	/* convert string to hex */
	value = wps_safe_get_conf("wps_uuid");
	sscanf(value, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		&wps_uuid[0], &wps_uuid[1], &wps_uuid[2], &wps_uuid[3],
		&wps_uuid[4], &wps_uuid[5], &wps_uuid[6], &wps_uuid[7],
		&wps_uuid[8], &wps_uuid[9], &wps_uuid[10], &wps_uuid[11],
		&wps_uuid[12], &wps_uuid[13], &wps_uuid[14], &wps_uuid[15]);

	for (i = 0; i < 16; i++)
		uuid[i] = (wps_uuid[i] & 0xff);

	return;
}

/* 
 * WPS upnp ssr related functions 
 */
static int
wps_upnp_get_ssr_ifname(int ess_id, char *client_ip, char *wps_ifname)
{
	unsigned char mac[6] = {0};
	unsigned char mac_list[128 * 6];
	int count;
	int i;
	char ifname[IFNAMSIZ];
	char *wlnames;
	char tmp[100];
	char *next;
	char *value;

	/* Get from arp table for ip to mac */
	if (wps_osl_arp_get(client_ip, mac) == -1)
		return -1;

	/* Only search for br0 and br1 */
	sprintf(tmp, "ess%d_wlnames", ess_id);
	wlnames = wps_safe_get_conf(tmp);

	foreach(ifname, wlnames, next) {
		/* query wl for authenticated sta list */
		count = wps_wl_get_maclist(ifname, mac_list, (sizeof(mac_list)/6));
		if (count == -1) {
			TUTRACE((TUTRACE_ERR, "GET authe_sta_list error!!!\n"));
			return -1;
		}

		for (i = 0; i < count; i++) {
			if (!memcmp(mac, &mac_list[i*6], 6)) {
				/* WPS ifname found */
				strcpy(wps_ifname, ifname);
				TUTRACE((TUTRACE_INFO, "wps_ifname = %s\n", wps_ifname));
				return 0;
			}
		}
	}

	/*
	 * mac address is not belong to any wireless client.
	 * Get first wps enabled interrface in the same bridge
	 */
	sprintf(tmp, "ess%d_wlnames", ess_id);
	wlnames = wps_safe_get_conf(tmp);

	/* find first wps available non-STA interface of target bridge */
	foreach(ifname, wlnames, next) {
		sprintf(tmp, "%s_wps_mode", ifname);
		value = wps_safe_get_conf(tmp);
		if (!strcmp(value, "enr_enabled"))
			continue;

		strcpy(wps_ifname, ifname);
		return 0;
	} /* foreach().... */

	return -1;
}

static uint32
wps_upnp_set_ssr(int ess_id, void *data, int len, char *ssr_client_ip)
{
	char wps_ifname[IFNAMSIZ] = {0};
	CTlvSsrIE ssrmsg;

	/* De-serialize the data to get the TLVs */
	BufferObj *bufObj = buffobj_new();
	if (!bufObj)
		return -1;

	buffobj_dserial(bufObj, (uint8 *)data, len);

	/* Version */
	tlv_dserialize(&ssrmsg.version, WPS_ID_VERSION, bufObj, 0, 0);
	/* scState */
	ssrmsg.scState.m_data = WPS_SCSTATE_CONFIGURED;
	/* Selected Registrar */
	tlv_dserialize(&ssrmsg.selReg, WPS_ID_SEL_REGISTRAR, bufObj, 0, 0);
	/* Device Password ID */
	tlv_dserialize(&ssrmsg.devPwdId, WPS_ID_DEVICE_PWD_ID, bufObj, 0, 0);
	/* Selected Registrar Config Methods */
	tlv_dserialize(&ssrmsg.selRegCfgMethods,
		WPS_ID_SEL_REG_CFG_METHODS, bufObj, 0, 0);

	buffobj_del(bufObj);

	TUTRACE((TUTRACE_INFO, "SSR request: reg = %x, id = %x, method = %x\n",
		ssrmsg.selReg.m_data, ssrmsg.devPwdId.m_data, ssrmsg.selRegCfgMethods.m_data));

	if (wps_upnp_get_ssr_ifname(ess_id, ssr_client_ip, wps_ifname) < 0) {
		TUTRACE((TUTRACE_ERR, "Can't find SSR interface unit, stop SSR setting!!\n"));
		return -1;
	}
	else
		TUTRACE((TUTRACE_INFO, "SSR interface unit = %s\n", wps_ifname));

	/* record upnpssr start time */
	upnpssr_time = time(0);
	wps_ie_set(wps_ifname, &ssrmsg);

	/* Save select registrar ipaddr */
	ap_upnp_sm_set_ssr_ipaddr(ssr_client_ip);

	return 0;
}

void
wps_upnp_clear_ssr_timer()
{
	upnpssr_time = 0;
}

void
wps_upnp_clear_ssr()
{
	char ipaddr[sizeof("255.255.255.255")] = {0};

	wps_upnp_clear_ssr_timer();
	ap_upnp_sm_set_ssr_ipaddr(ipaddr);
}

int
wps_upnp_ssr_expire()
{
	unsigned long now;
	wps_app_t *wps_app = get_wps_app();

	if (!wps_app->wksp && upnpssr_time) {
		now = time(0);
		if ((now < upnpssr_time) || ((now - upnpssr_time) > WPS_MAX_TIMEOUT))
			return 1;
	}
	return 0;
}

/*
 * Function with regard to process messages
 */
static int
wps_upnp_create_device_info(upnp_attached_if *upnp_if)
{
	BufferObj *outMsg;
	void *databuf = upnp_if->m1_buf;
	int *datalen = &upnp_if->m1_len;
	RegData regInfo;
	DevInfo devInfo;
	char tmp[32], prefix[] = "wlXXXXXXXXXX_";
	unsigned char wps_uuid[16] = {0};
	char *pc_data;


	/* Get prefix */
	snprintf(prefix, sizeof(prefix), "%s_", upnp_if->wl_name);

	/* create M1 */
	outMsg = buffobj_setbuf(databuf, *datalen);
	if (outMsg == NULL)
		return -1;

	memset(&regInfo, 0, sizeof(RegData));
	memset(&devInfo, 0, sizeof(DevInfo));

	regInfo.p_enrolleeInfo = &devInfo;
	regInfo.e_lastMsgSent = MNONE;
	regInfo.outMsg = buffobj_new();
	if (!regInfo.outMsg) {
		buffobj_del(outMsg);
		return -1;
	}

	/* Start to setup device info */
	wps_upnp_device_uuid(wps_uuid);
	memcpy(devInfo.uuid, wps_uuid, sizeof(devInfo.uuid));

	memcpy(devInfo.macAddr, upnp_if->mac, SIZE_6_BYTES);

	devInfo.authTypeFlags = (uint16) 0x27;
	devInfo.encrTypeFlags = (uint16) 0xf;
	devInfo.connTypeFlags = 1;

	pc_data = wps_get_conf("wps_config_method");
	devInfo.configMethods = pc_data ? strtoul(pc_data, NULL, 16) : 0x88;

	sprintf(tmp, "ess%d_wps_oob", upnp_if->ess_id);
	pc_data = wps_safe_get_conf(tmp);
	if (!strcmp(pc_data, "disabled") || wps_ui_is_pending())
		devInfo.scState = WPS_SCSTATE_CONFIGURED;
	else
		devInfo.scState = WPS_SCSTATE_UNCONFIGURED;

	strncpy(devInfo.manufacturer, wps_safe_get_conf("wps_mfstring"),
		sizeof(devInfo.manufacturer));

	strncpy(devInfo.modelName, wps_safe_get_conf("wps_modelname"),
		sizeof(devInfo.modelName));

	strncpy(devInfo.modelNumber, wps_safe_get_conf("wps_modelnum"),
		sizeof(devInfo.modelNumber));

	strncpy(devInfo.serialNumber, wps_safe_get_conf("boardnum"),
		sizeof(devInfo.serialNumber));

	strncpy(devInfo.deviceName, wps_safe_get_conf("wps_device_name"),
		sizeof(devInfo.deviceName));

	devInfo.primDeviceCategory = 6;
	devInfo.primDeviceOui = 0x0050F204;
	devInfo.primDeviceSubCategory = 1;

	sprintf(tmp, "ess%d_band", upnp_if->ess_id);
	pc_data = wps_safe_get_conf(tmp);
	devInfo.rfBand = atoi(pc_data);

	devInfo.osVersion = 0x80000000;

	devInfo.assocState = WPS_ASSOC_NOT_ASSOCIATED;
	devInfo.configError = 0; /* No error */
	devInfo.devPwdId = WPS_DEVICEPWDID_DEFAULT;

	reg_proto_create_m1(&regInfo, outMsg);

	memcpy(upnp_if->enr_nonce, regInfo.enrolleeNonce, SIZE_128_BITS);
	BN_bn2bin(regInfo.DHSecret->priv_key, upnp_if->private_key);

	*datalen = outMsg->m_dataLength;
	buffobj_del(outMsg);
	buffobj_del(regInfo.outMsg);

	if (regInfo.DHSecret)
		DH_free(regInfo.DHSecret);

	return 0;
}

int
wps_upnp_send_msg(int if_instance, char *buf, int len, int type)
{
	UPNP_WPS_CMD cmd;

	if (!buf || !len) {
		TUTRACE((TUTRACE_ERR, "Invalid Parameters\n"));
		return WPS_ERR_INVALID_PARAMETERS;
	}

	/* Construct msg */
	memset(&cmd, 0, UPNP_WPS_CMD_SIZE);

	cmd.type = type;
	cmd.length = len;

	/* Send out */
	return wps_osl_upnp_send(if_instance, &cmd, buf);
}

char *
wps_upnp_parse_msg(char *upnpmsg, int upnpmsg_len, int *len, int *type, char *addr)
{
	UPNP_WPS_CMD *cmd = (UPNP_WPS_CMD *)upnpmsg;

	if (addr)
		strcpy(addr, cmd->dst_addr);

	*len = cmd->length;
	*type = cmd->type;
	return cmd->data;
}

int
wps_upnp_process_msg(char *upnpmsg, int upnpmsg_len, wps_hndl_t *hndl)
{
	int ret = WPS_CONT;
	unsigned int now = time(0);
	char *data;
	int data_len, type;
	char peer_addr[sizeof("255.255.255.255")] = {0};
	upnp_attached_if *upnp_if;
	wps_app_t *wps_app = get_wps_app();

	upnp_if = upnpif_head;
	while (upnp_if) {
		if (hndl == &upnp_if->upnp_hndl)
			break;
		upnp_if = upnp_if->next;
	}
	if (upnp_if == NULL) {
		TUTRACE((TUTRACE_ERR, "upnp interface instance not exist!!\n"));
		return ret;
	}

	/* forward upnp message if wps_ap exists */
	if (wps_app->wksp) {
		if (upnp_if->ess_id != ((wpsap_wksp_t *)wps_app->wksp)->ess_id) {
			TUTRACE((TUTRACE_ERR, "Expect UPnP from ESS %d, rather than ESS %d\n",
				((wpsap_wksp_t *)wps_app->wksp)->ess_id,
				upnp_if->ess_id));
			return ret;
		}

		return (*wps_app->process)(wps_app->wksp, upnpmsg, upnpmsg_len,
			TRANSPORT_TYPE_UPNP_DEV);
	}

	if (!(data = wps_upnp_parse_msg(upnpmsg, upnpmsg_len, &data_len,
		&type, peer_addr))) {
		TUTRACE((TUTRACE_ERR, "Received data without valid message "
			"from upnp\n"));
		return ret;
	}

	switch (type) {
	case UPNP_WPS_TYPE_SSR:
		TUTRACE((TUTRACE_INFO, "receive SSR request\n"));
		wps_upnp_set_ssr(upnp_if->ess_id, data, data_len, peer_addr);
		break;

	case UPNP_WPS_TYPE_GDIR:
		TUTRACE((TUTRACE_INFO, "receive GDIR request\n"));

		/*
		  * Create M1 message.
		  * Note: we don't maintain per client nounce and private key.  Within 5 seconds,
		  *          the nounce, private key and M1 will keep unchanged.
		  */
		if (upnp_if->m1_buf == NULL || (now - upnp_if->m1_built_time) >= 5) {
			/* Allocate nonce and private key buffer */
			if (upnp_if->enr_nonce == NULL) {
				upnp_if->enr_nonce = (char *)malloc(SIZE_128_BITS);
				if (upnp_if->enr_nonce == NULL)
					break;
			}

			if (upnp_if->private_key == NULL) {
				upnp_if->private_key = (char *)malloc(SIZE_PUB_KEY);
				if (upnp_if->private_key == NULL)
					break;
			}


			if (upnp_if->m1_buf == NULL) {
				upnp_if->m1_buf = (char *)malloc(WPS_EAPD_READ_MAX_LEN);
				if (upnp_if->m1_buf == NULL)
					break;
			}

			upnp_if->m1_len = WPS_EAPD_READ_MAX_LEN;
			memset(upnp_if->m1_buf, 0, WPS_EAPD_READ_MAX_LEN);

			/* generate device info */
			if (wps_upnp_create_device_info(upnp_if) != 0) {
				free(upnp_if->m1_buf);
				upnp_if->m1_buf = 0;
				break;
			}

			upnp_if->m1_built_time = now;
		}

		/* send out device info */
		wps_upnp_send_msg(upnp_if->instance,
			upnp_if->m1_buf, upnp_if->m1_len, UPNP_WPS_TYPE_GDIR);
		break;

	case UPNP_WPS_TYPE_PMR:
		TUTRACE((TUTRACE_INFO, "receive PMR request\n"));

		/* Weak-up wps_ap to process external registrar configuring */
		TUTRACE((TUTRACE_INFO, "wps_ap is unavaliabled!! Wake-up wps_ap now !!\n"));

		/* Open ap session */
		ret = wpsap_open_session(wps_app, WPSM_UNCONFAP, upnp_if->mac, NULL,
			upnp_if->wl_name, upnp_if->enr_nonce, upnp_if->private_key);

		if (ret == WPS_CONT) {
			TUTRACE((TUTRACE_INFO, "wps_ap is ready. !!send PMR request to "
				"wps_ap. data len = %d\n", upnpmsg_len));

			ret = (*wps_app->process)(wps_app->wksp, upnpmsg, upnpmsg_len,
				TRANSPORT_TYPE_UPNP_DEV);
		}
		else {
			TUTRACE((TUTRACE_INFO, "wps_ap opening Fail !!"));
		}

		free(upnp_if->enr_nonce);
		upnp_if->enr_nonce = NULL;
		free(upnp_if->private_key);
		upnp_if->private_key = NULL;
		free(upnp_if->m1_buf);
		upnp_if->m1_buf = NULL;

		break;
	default:
		break;
	}

	return ret;
}

/* 
 * WPS upnp init/deinit functions 
 */
static int
wps_upnp_if_init(char *ifname, char *mac, int ess_id, int instance)
{
	upnp_attached_if *upnp_if;

	/* create upnp_if entry to retrieve msg from upnp */
	if ((upnp_if = calloc(sizeof(*upnp_if), 1)) == NULL) {
		TUTRACE((TUTRACE_ERR, "Allocate upnpif fail\n"));
		return -1;
	}

	upnp_if->ess_id = ess_id;
	upnp_if->instance = instance;

	/* Save wl interface informaction */
	memcpy(upnp_if->mac, mac, SIZE_6_BYTES);
	strcpy(upnp_if->wl_name, ifname);

	upnp_if->upnp_hndl.type = WPS_RECEIVE_PKT_UPNP;
	upnp_if->upnp_hndl.handle = wps_osl_upnp_handle_init(instance);
	if (upnp_if->upnp_hndl.handle == -1) {
		free(upnp_if);
		return -1;
	}

	wps_hndl_add(&upnp_if->upnp_hndl);

	upnp_if->next = upnpif_head;
	upnpif_head = upnp_if;

	return 0;
}

static void
wps_upnp_if_deinit(upnp_attached_if *upnp_if)
{
	/* Clean up allocated data if any */
	if (upnp_if->m1_buf)
		free(upnp_if->m1_buf);
	if (upnp_if->enr_nonce)
		free(upnp_if->enr_nonce);
	if (upnp_if->private_key)
		free(upnp_if->private_key);

	/* Detach WFA device */
	wps_upnp_detach_wfa_dev(upnp_if->instance);

	/* Delete handle */
	wps_osl_upnp_handle_deinit(upnp_if->upnp_hndl.handle);

	wps_hndl_del(&upnp_if->upnp_hndl);

	/* Free it */
	free(upnp_if);
	return;
}

void
wps_upnp_init()
{
	int i, imax, instance;
	char ifname[80], tmp[100];
	char prefix[] = "wlXXXXXXXXXX_";
	char wl_mac[SIZE_MAC_ADDR];
	char *wl_hwaddr, *wps_mode;
	char *wlnames, *next, *value;

	imax = wps_get_ess_num();
	for (i = 0; i < imax; i++) {
		sprintf(tmp, "ess%d_wlnames", i);
		wlnames = wps_get_conf(tmp);
		if (wlnames == NULL)
			continue;

		/* Get ESS UPnP instance */
		sprintf(tmp, "ess%d_upnp_instance", i);
		value = wps_get_conf(tmp);
		if (value == NULL)
			continue;

		instance = atoi(value);

		/* Find first AP mode interface for wps_upnp_if_init */
		foreach(ifname, wlnames, next) {
			/* Get configured ethernet MAC address */
			snprintf(prefix, sizeof(prefix), "%s_", ifname);

			/* Get wireless interface mac address */
			wl_hwaddr = wps_get_conf(strcat_r(prefix, "hwaddr", tmp));
			if (wl_hwaddr == NULL)
				continue;

			/* Ignore STA interface */
			wps_mode = wps_safe_get_conf(strcat_r(prefix, "wps_mode", tmp));
			if (strcmp(wps_mode, "enr_enabled") == 0)
				continue;

			ether_atoe(wl_hwaddr, wl_mac);

			/* Attach the WFA device */
			wps_upnp_attach_wfa_dev(instance);

			/* Send initial notify wlan event */
			wps_upnp_update_init_wlan_event(instance, wl_mac, 0);

			wps_upnp_if_init(ifname, wl_mac, i, instance);
			break; /* break foreach */
		} /* foreach().... */
	} /* For loop */

	return;
}

void
wps_upnp_deinit()
{
	upnp_attached_if *next;

	while (upnpif_head) {
		next = upnpif_head->next;

		wps_upnp_if_deinit(upnpif_head);

		upnpif_head = next;
	}
}
