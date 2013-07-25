/*
 * WPS device infomation
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wps_devinfo.h 241376 2011-02-18 03:19:15Z stakita $
 */

#ifndef __WPS_DEVICE_INFO_H__
#define __WPS_DEVICE_INFO_H__


/* data structure to hold Enrollee and Registrar information */
typedef struct {
    uint8   version;
    uint8   uuid[SIZE_16_BYTES];
    uint8   macAddr[SIZE_6_BYTES];
    char    deviceName[SIZE_32_BYTES];
    uint16  primDeviceCategory;
    uint32  primDeviceOui;
    uint16  primDeviceSubCategory;
    uint16  authTypeFlags;
    uint16  encrTypeFlags;
    uint8   connTypeFlags;
    uint16  configMethods;
    uint8   scState;
    bool    selRegistrar;
    char    manufacturer[SIZE_64_BYTES];
    char    modelName[SIZE_32_BYTES];
    char    modelNumber[SIZE_32_BYTES];
    char    serialNumber[SIZE_32_BYTES];
    uint8   rfBand;
    uint32  osVersion;
    uint32  featureId;
    uint16  assocState;
    uint16  devPwdId;
    uint16  configError;
    bool    b_ap;
    char    ssid[SIZE_SSID_LENGTH];
    char    keyMgmt[SIZE_20_BYTES];
    uint16  auth;
    uint16  wep;
    uint16  crypto;
    uint16  reqDeviceCategory;
    uint32  reqDeviceOui;
    uint16  reqDeviceSubCategory;
    uint8   version2;
    uint8   settingsDelayTime;
    bool    b_reqToEnroll;
    bool    b_nwKeyShareable;
#ifdef WFA_WPS_20_TESTBED
    char    dummy_ssid[SIZE_SSID_LENGTH];
    bool    b_zpadding;
    bool    b_mca;
    int     nattr_len;
    char    nattr_tlv[SIZE_128_BYTES];
#endif /* WFA_WPS_20_TESTBED */
    int     authorizedMacs_len;
    char    authorizedMacs[SIZE_MAC_ADDR * SIZE_AUTHORIZEDMACS_NUM];
} DevInfo;


#endif /* __WPS_DEVICE_INFO_H__ */
