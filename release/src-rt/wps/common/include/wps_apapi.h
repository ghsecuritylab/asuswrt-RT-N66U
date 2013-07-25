/*
 * WPS Registratar API
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wps_apapi.h 241376 2011-02-18 03:19:15Z stakita $
 */

#ifndef _WPS_AP_API_H_
#define _WPS_AP_API_H_

#include <typedefs.h>
#include <info.h>


typedef enum {
	SEARCH_ONLY,
	SEARCH_ENTER
} sta_lookup_mode_t;

/* ap eap state machine states */
typedef enum {
	INIT = 0,
	EAPOL_START_SENT,
	EAP_START_SEND,
	PROCESSING_PROTOCOL,
	REG_SUCCESSFUL,
	REG_FAILED,
	EAP_TIMEOUT,
	EAP_FAILURED
} ap_eap_state_t;

typedef enum {
	WPS_WL_AKM_NONE = 0,
	WPS_WL_AKM_PSK,
	WPS_WL_AKM_PSK2,
	WPS_WL_AKM_BOTH
} WPS_WL_AKM_E;

typedef enum {
	WPS_PROC_STATE_INIT = 0,
	WPS_PROC_STATE_PROCESSING,
	WPS_PROC_STATE_SUCCESS,
	WPS_STATUS_MSGERR,
	WPS_STATUS_TIMEOUT
} WPS_PROC_STATE_E;

#define EAP_MAX_RESEND_COUNT 5

int wps_get_mode(void *mc_dev);
int wps_processMsg(void *mc_dev, void *inBuffer, uint32 in_len, void *outBuffer, uint32 *out_len,
	TRANSPORT_TYPE m_transportType);
uint32 wps_start_ap_registration(void *mc_dev, EMode e_currMode, char *devPwd, uint16 devPwdId,
	uint8 *authorizedMacs, uint32 authorizedMacs_len);
unsigned char * wps_get_mac_income(void *mc_dev);
unsigned char *wps_get_mac(void *mc_dev);
uint32 wps_genPin(char *devPwd, int devPwd_len);
uint8 wps_get_version2(void *mc_dev);

#endif /* _WPS_AP_API_H_ */
