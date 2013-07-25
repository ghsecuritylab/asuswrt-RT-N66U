/*
 * WPS ENROLL header file
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wps_enr.h 241376 2011-02-18 03:19:15Z stakita $
 */

#ifndef __WPS_ENR_H__
#define __WPS_ENR_H__

#define WPS_ENR_EAP_DATA_MAX_LENGTH			2048
#define WPS_ENR_EAP_READ_DATA_TIMEOUT		1

/* OS dependent APIs */
void wpsenr_osl_proc_states(int state);
int wpsenr_osl_set_wsec(void *credential);
int wpsenr_osl_clear_wsec(void);
int wpsenr_osl_restore_wsec(void);

/* Common interface to enr wksp */
int wpsenr_open_session(void *wps_app, char*ifname);

#endif /* __WPS_ENR_H__ */
