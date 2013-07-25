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
 * $Id: wps_pb.h 241376 2011-02-18 03:19:15Z stakita $
 */

#ifndef __WPS_PB_H__
#define __WPS_PB_H__

#include <wps.h>

#define PBC_OVERLAP_CNT			2
#define WPS_PB_SELECTING_MAX_TIMEOUT	10	/* second */

typedef struct {
	unsigned char	mac[6];
	unsigned int	last_time;
} PBC_STA_INFO;

enum {
	WPS_PB_STATE_INIT = 0,
	WPS_PB_STATE_CONFIRM,
	WPS_PB_STATE_SELECTING
} WPS_PB_STATE_T;

int wps_pb_check_pushtime(unsigned long time);
void wps_pb_update_pushtime(char *mac);
void wps_pb_clear_sta(char *mac);
wps_hndl_t *wps_pb_check(char *buf, int *buflen);
int wps_pb_init();
int wps_pb_deinit();
void wps_pb_reset();
void wps_pb_timeout(int session_opened);
int wps_pb_state_reset();

#endif	/* __WPS_PB_H__ */
