/*
 * WPS environment variables
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wps_ui.h 241376 2011-02-18 03:19:15Z stakita $
 */

#ifndef __WPS_UI_H__
#define __WPS_UI_H__

#include <wps.h>

int wps_ui_is_pending();
void wps_ui_clear_pending();
int wps_ui_pending_expire();
int wps_ui_init();
void wps_ui_deinit();
char *wps_ui_get_env(char *name);
void wps_ui_set_env(char *name, char *value);
void wps_ui_reset_env();
int wps_ui_process_msg(char *buf, int buflen);

#ifdef WPS_ADDCLIENT_WWTP
int wps_ui_is_SET_cmd(char *buf, int buflen);
void wps_ui_close_addclient_window();
#endif

#endif	/* __WPS_UI_H__ */
