/*
 * Registrar protocol TLV
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: reg_prototlv.h 247315 2011-03-18 12:26:00Z kenlo $
 */

#ifndef _WPS_REG_PROTO_TLV_
#define _WPS_REG_PROTO_TLV_

#ifdef __cplusplus
extern "C" {
#endif

#include <wpstypes.h>

#include <slist.h>

#include <wpstlvbase.h>


/* Data Element Definitions */
#define WPS_ID_AP_CHANNEL         0x1001
#define WPS_ID_ASSOC_STATE        0x1002
#define WPS_ID_AUTH_TYPE          0x1003
#define WPS_ID_AUTH_TYPE_FLAGS    0x1004
#define WPS_ID_AUTHENTICATOR      0x1005
#define WPS_ID_CONFIG_METHODS     0x1008
#define WPS_ID_CONFIG_ERROR       0x1009
#define WPS_ID_CONF_URL4          0x100A
#define WPS_ID_CONF_URL6          0x100B
#define WPS_ID_CONN_TYPE          0x100C
#define WPS_ID_CONN_TYPE_FLAGS    0x100D
#define WPS_ID_CREDENTIAL         0x100E
#define WPS_ID_DEVICE_NAME        0x1011
#define WPS_ID_DEVICE_PWD_ID      0x1012
#define WPS_ID_E_HASH1            0x1014
#define WPS_ID_E_HASH2            0x1015
#define WPS_ID_E_SNONCE1          0x1016
#define WPS_ID_E_SNONCE2          0x1017
#define WPS_ID_ENCR_SETTINGS      0x1018
#define WPS_ID_ENCR_TYPE          0x100F
#define WPS_ID_ENCR_TYPE_FLAGS    0x1010
#define WPS_ID_ENROLLEE_NONCE     0x101A
#define WPS_ID_FEATURE_ID         0x101B
#define WPS_ID_IDENTITY           0x101C
#define WPS_ID_IDENTITY_PROOF     0x101D
#define WPS_ID_KEY_WRAP_AUTH      0x101E
#define WPS_ID_KEY_IDENTIFIER     0x101F
#define WPS_ID_MAC_ADDR           0x1020
#define WPS_ID_MANUFACTURER       0x1021
#define WPS_ID_MSG_TYPE           0x1022
#define WPS_ID_MODEL_NAME         0x1023
#define WPS_ID_MODEL_NUMBER       0x1024
#define WPS_ID_NW_INDEX           0x1026
#define WPS_ID_NW_KEY             0x1027
#define WPS_ID_NW_KEY_INDEX       0x1028
#define WPS_ID_NEW_DEVICE_NAME    0x1029
#define WPS_ID_NEW_PWD            0x102A
#define WPS_ID_OOB_DEV_PWD        0x102C
#define WPS_ID_OS_VERSION         0x102D
#define WPS_ID_POWER_LEVEL        0x102F
#define WPS_ID_PSK_CURRENT        0x1030
#define WPS_ID_PSK_MAX            0x1031
#define WPS_ID_PUBLIC_KEY         0x1032
#define WPS_ID_RADIO_ENABLED      0x1033
#define WPS_ID_REBOOT             0x1034
#define WPS_ID_REGISTRAR_CURRENT  0x1035
#define WPS_ID_REGISTRAR_ESTBLSHD 0x1036
#define WPS_ID_REGISTRAR_LIST     0x1037
#define WPS_ID_REGISTRAR_MAX      0x1038
#define WPS_ID_REGISTRAR_NONCE    0x1039
#define WPS_ID_REQ_TYPE           0x103A
#define WPS_ID_RESP_TYPE          0x103B
#define WPS_ID_RF_BAND            0x103C
#define WPS_ID_R_HASH1            0x103D
#define WPS_ID_R_HASH2            0x103E
#define WPS_ID_R_SNONCE1          0x103F
#define WPS_ID_R_SNONCE2          0x1040
#define WPS_ID_SEL_REGISTRAR      0x1041
#define WPS_ID_SERIAL_NUM         0x1042
#define WPS_ID_SC_STATE           0x1044
#define WPS_ID_SSID               0x1045
#define WPS_ID_TOT_NETWORKS       0x1046
#define WPS_ID_UUID_E             0x1047
#define WPS_ID_UUID_R             0x1048
#define WPS_ID_VENDOR_EXT         0x1049
#define WPS_ID_VERSION            0x104A
#define WPS_ID_X509_CERT_REQ      0x104B
#define WPS_ID_X509_CERT          0x104C
#define WPS_ID_EAP_IDENTITY       0x104D
#define WPS_ID_MSG_COUNTER        0x104E
#define WPS_ID_PUBKEY_HASH        0x104F
#define WPS_ID_REKEY_KEY          0x1050
#define WPS_ID_KEY_LIFETIME       0x1051
#define WPS_ID_PERM_CFG_METHODS   0x1052
#define WPS_ID_SEL_REG_CFG_METHODS 0x1053
#define WPS_ID_PRIM_DEV_TYPE      0x1054
#define WPS_ID_SEC_DEV_TYPE_LIST  0x1055
#define WPS_ID_PORTABLE_DEVICE    0x1056
#define WPS_ID_AP_SETUP_LOCKED    0x1057
#define WPS_ID_APP_LIST           0x1058
#define WPS_ID_EAP_TYPE           0x1059
#define WPS_ID_INIT_VECTOR        0x1060
#define WPS_ID_KEY_PROVIDED_AUTO  0x1061
#define WPS_ID_8021X_ENABLED      0x1062
#define WPS_ID_WEP_TRANSMIT_KEY   0x1064
#define WPS_ID_REQ_DEV_TYPE       0x106A

/* WSC 2.0, WFA Vendor Extension Subelements */
#define WFA_VENDOR_EXT_ID                 "\x00\x37\x2A"
#define WPS_WFA_SUBID_VERSION2            0x00
#define WPS_WFA_SUBID_AUTHORIZED_MACS     0x01
#define WPS_WFA_SUBID_NW_KEY_SHAREABLE    0x02
#define WPS_WFA_SUBID_REQ_TO_ENROLL       0x03
#define WPS_WFA_SUBID_SETTINGS_DELAY_TIME 0x04

/* sizes of the fixed size elements */
#define WPS_ID_AP_CHANNEL_S       2
#define WPS_ID_ASSOC_STATE_S      2
#define WPS_ID_AUTH_TYPE_S        2
#define WPS_ID_AUTH_TYPE_FLAGS_S  2
#define WPS_ID_AUTHENTICATOR_S    8
#define WPS_ID_CONFIG_METHODS_S   2
#define WPS_ID_CONFIG_ERROR_S     2
#define WPS_ID_CONN_TYPE_S          1
#define WPS_ID_CONN_TYPE_FLAGS_S    1
#define WPS_ID_DEVICE_PWD_ID_S      2
#define WPS_ID_ENCR_TYPE_S          2
#define WPS_ID_ENCR_TYPE_FLAGS_S    2
#define WPS_ID_FEATURE_ID_S         4
#define WPS_ID_MAC_ADDR_S           6
#define WPS_ID_MSG_TYPE_S           1
#define WPS_ID_SC_STATE_S           1
#define WPS_ID_RF_BAND_S            1
#define WPS_ID_OS_VERSION_S         4
#define WPS_ID_VERSION_S            1
#define WPS_ID_SEL_REGISTRAR_S      1
#define WPS_ID_SEL_REG_CFG_METHODS_S 2
#define WPS_ID_REQ_TYPE_S           1
#define WPS_ID_RESP_TYPE_S          1
#define WPS_ID_AP_SETUP_LOCKED_S    1

/* WSC 2.0, WFA Vendor Extension Subelements */
#define WPS_WFA_SUBID_VERSION2_S            1
#define WPS_WFA_SUBID_NW_KEY_SHAREABLE_S    1
#define WPS_WFA_SUBID_REQ_TO_ENROLL_S       1
#define WPS_WFA_SUBID_SETTINGS_DELAY_TIME_S 1

/* Association states */
#define WPS_ASSOC_NOT_ASSOCIATED  0
#define WPS_ASSOC_CONN_SUCCESS    1
#define WPS_ASSOC_CONFIG_FAIL     2
#define WPS_ASSOC_ASSOC_FAIL      3
#define WPS_ASSOC_IP_FAIL         4

/* Authentication types */
#define WPS_AUTHTYPE_OPEN        0x0001
#define WPS_AUTHTYPE_WPAPSK      0x0002	/* Deprecated in WSC 2.0 */
#define WPS_AUTHTYPE_SHARED      0x0004	/* Deprecated in WSC 2.0 */
#define WPS_AUTHTYPE_WPA         0x0008	/* Deprecated in WSC 2.0 */
#define WPS_AUTHTYPE_WPA2        0x0010
#define WPS_AUTHTYPE_WPA2PSK     0x0020

/* Config methods */
#define WPS_CONFMET_USBA            0x0001	/* Deprecated in WSC 2.0 */
#define WPS_CONFMET_ETHERNET        0x0002	/* Deprecated in WSC 2.0 */
#define WPS_CONFMET_LABEL           0x0004
#define WPS_CONFMET_DISPLAY         0x0008
#define WPS_CONFMET_EXT_NFC_TOK     0x0010
#define WPS_CONFMET_INT_NFC_TOK     0x0020
#define WPS_CONFMET_NFC_INTF        0x0040
#define WPS_CONFMET_PBC             0x0080
#define WPS_CONFMET_KEYPAD          0x0100
/* WSC 2.0 */
#define WPS_CONFMET_VIRT_PBC        0x0280
#define WPS_CONFMET_PHY_PBC         0x0480
#define WPS_CONFMET_VIRT_DISPLAY    0x2008
#define WPS_CONFMET_PHY_DISPLAY     0x4008

/* WPS error messages */
#define WPS_ERROR_NO_ERROR                0
#define WPS_ERROR_OOB_INT_READ_ERR        1
#define WPS_ERROR_DECRYPT_CRC_FAIL        2
#define WPS_ERROR_CHAN24_NOT_SUPP         3
#define WPS_ERROR_CHAN50_NOT_SUPP         4
#define WPS_ERROR_SIGNAL_WEAK             5	/* Deprecated in WSC 2.0 */
#define WPS_ERROR_NW_AUTH_FAIL            6	/* Deprecated in WSC 2.0 */
#define WPS_ERROR_NW_ASSOC_FAIL           7	/* Deprecated in WSC 2.0 */
#define WPS_ERROR_NO_DHCP_RESP            8	/* Deprecated in WSC 2.0 */
#define WPS_ERROR_FAILED_DHCP_CONF        9	/* Deprecated in WSC 2.0 */
#define WPS_ERROR_IP_ADDR_CONFLICT        10	/* Deprecated in WSC 2.0 */
#define WPS_ERROR_FAIL_CONN_REGISTRAR     11
#define WPS_ERROR_MULTI_PBC_DETECTED      12
#define WPS_ERROR_ROGUE_SUSPECTED         13
#define WPS_ERROR_DEVICE_BUSY             14
#define WPS_ERROR_SETUP_LOCKED            15
#define WPS_ERROR_MSG_TIMEOUT             16	/* Deprecated in WSC 2.0 */
#define WPS_ERROR_REG_SESSION_TIMEOUT     17	/* Deprecated in WSC 2.0 */
#define WPS_ERROR_DEV_PWD_AUTH_FAIL       18

/* Connection types */
#define WPS_CONNTYPE_ESS    0x01
#define WPS_CONNTYPE_IBSS   0x02

/* Device password ID */
#define WPS_DEVICEPWDID_DEFAULT          0x0000
#define WPS_DEVICEPWDID_USER_SPEC        0x0001
#define WPS_DEVICEPWDID_MACHINE_SPEC     0x0002
#define WPS_DEVICEPWDID_REKEY            0x0003
#define WPS_DEVICEPWDID_PUSH_BTN         0x0004
#define WPS_DEVICEPWDID_REG_SPEC         0x0005

/* Encryption type */
#define WPS_ENCRTYPE_NONE    0x0001
#define WPS_ENCRTYPE_WEP     0x0002	/* Deprecated in WSC 2.0 */
#define WPS_ENCRTYPE_TKIP    0x0004	/* Deprecated in version 2.0. TKIP can only
					  * be advertised on the AP when Mixed Mode
					  * is enabled (Encryption Type is 0x000c).
					  */
#define WPS_ENCRTYPE_AES     0x0008


/* WPS Message Types */
#define WPS_ID_BEACON            0x01
#define WPS_ID_PROBE_REQ         0x02
#define WPS_ID_PROBE_RESP        0x03
#define WPS_ID_MESSAGE_M1        0x04
#define WPS_ID_MESSAGE_M2        0x05
#define WPS_ID_MESSAGE_M2D       0x06
#define WPS_ID_MESSAGE_M3        0x07
#define WPS_ID_MESSAGE_M4        0x08
#define WPS_ID_MESSAGE_M5        0x09
#define WPS_ID_MESSAGE_M6        0x0A
#define WPS_ID_MESSAGE_M7        0x0B
#define WPS_ID_MESSAGE_M8        0x0C
#define WPS_ID_MESSAGE_ACK       0x0D
#define WPS_ID_MESSAGE_NACK      0x0E
#define WPS_ID_MESSAGE_DONE      0x0F

/* WSP private ID for local use */
#define WPS_PRIVATE_ID_IDENTITY		(WPS_ID_MESSAGE_DONE + 1)
#define WPS_PRIVATE_ID_WPS_START	(WPS_ID_MESSAGE_DONE + 2)
#define WPS_PRIVATE_ID_FAILURE		(WPS_ID_MESSAGE_DONE + 3)
#define WPS_PRIVATE_ID_FRAG		(WPS_ID_MESSAGE_DONE + 4)
#define WPS_PRIVATE_ID_FRAG_ACK		(WPS_ID_MESSAGE_DONE + 5)
#define WPS_PRIVATE_ID_EAPOL_START	(WPS_ID_MESSAGE_DONE + 6)


/* Device Type categories for primary and secondary device types */
#define WPS_DEVICE_TYPE_CAT_COMPUTER        1
#define WPS_DEVICE_TYPE_CAT_INPUT_DEVICE    2
#define WPS_DEVICE_TYPE_CAT_PRINTER         3
#define WPS_DEVICE_TYPE_CAT_CAMERA          4
#define WPS_DEVICE_TYPE_CAT_STORAGE         5
#define WPS_DEVICE_TYPE_CAT_NW_INFRA        6
#define WPS_DEVICE_TYPE_CAT_DISPLAYS        7
#define WPS_DEVICE_TYPE_CAT_MM_DEVICES      8
#define WPS_DEVICE_TYPE_CAT_GAME_DEVICES    9
#define WPS_DEVICE_TYPE_CAT_TELEPHONE       10
#define WPS_DEVICE_TYPE_CAT_AUDIO_DEVICES   11	/* WSC 2.0 */

/* Device Type sub categories for primary and secondary device types */
#define WPS_DEVICE_TYPE_SUB_CAT_COMP_PC         1
#define WPS_DEVICE_TYPE_SUB_CAT_COMP_SERVER     2
#define WPS_DEVICE_TYPE_SUB_CAT_COMP_MEDIA_CTR  3
#define WPS_DEVICE_TYPE_SUB_CAT_COMP_UM_PC      4	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_COMP_NOTEBOOK   5	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_COMP_DESKTOP    6	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_COMP_MID        7	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_COMP_NETBOOK    8	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_INP_Keyboard    1	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_INP_MOUSE       2	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_INP_JOYSTICK    3	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_INP_TRACKBALL   4	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_INP_GAM_CTRL    5	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_INP_REMOTE      6	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_INP_TOUCHSCREEN 7	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_INP_BIO_READER  8	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_INP_BAR_READER  9	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_PRTR_PRINTER    1
#define WPS_DEVICE_TYPE_SUB_CAT_PRTR_SCANNER    2
#define WPS_DEVICE_TYPE_SUB_CAT_PRTR_FAX        3	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_PRTR_COPIER     4	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_PRTR_ALLINONE   5	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_CAM_DGTL_STILL  1
#define WPS_DEVICE_TYPE_SUB_CAT_CAM_VIDEO_CAM   2	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_CAM_WEB_CAM     3	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_CAM_SECU_CAM    4	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_STOR_NAS        1
#define WPS_DEVICE_TYPE_SUB_CAT_NW_AP           1
#define WPS_DEVICE_TYPE_SUB_CAT_NW_ROUTER       2
#define WPS_DEVICE_TYPE_SUB_CAT_NW_SWITCH       3
#define WPS_DEVICE_TYPE_SUB_CAT_NW_Gateway      4	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_NW_BRIDGE       5	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_DISP_TV         1
#define WPS_DEVICE_TYPE_SUB_CAT_DISP_PIC_FRAME  2
#define WPS_DEVICE_TYPE_SUB_CAT_DISP_PROJECTOR  3
#define WPS_DEVICE_TYPE_SUB_CAT_DISP_MONITOR    4	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_MM_DAR          1
#define WPS_DEVICE_TYPE_SUB_CAT_MM_PVR          2
#define WPS_DEVICE_TYPE_SUB_CAT_MM_MCX          3
#define WPS_DEVICE_TYPE_SUB_CAT_MM_STB          4	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_MM_MS_ME        5	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_MM_PVP          6	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_GAM_XBOX        1
#define WPS_DEVICE_TYPE_SUB_CAT_GAM_XBOX_360    2
#define WPS_DEVICE_TYPE_SUB_CAT_GAM_PS          3
#define WPS_DEVICE_TYPE_SUB_CAT_GAM_GC          4	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_GAM_PGD         5	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_PHONE_WM        1
#define WPS_DEVICE_TYPE_SUB_CAT_PHONE_PSM       2	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_PHONE_PDM       3	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_PHONE_SSM       4	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_PHONE_SDM       5	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_AUDIO_TUNER     1	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_AUDIO_SPEAKERS  2	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_AUDIO_PMP       3	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_AUDIO_HEADSET   4	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_AUDIO_HPHONE    5	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_AUDIO_MPHONE    6	/* WSC 2.0 */
#define WPS_DEVICE_TYPE_SUB_CAT_AUDIO_HTS       7	/* WSC 2.0 */


/* Device request/response type */
#define WPS_MSGTYPE_ENROLLEE_INFO_ONLY    0x00
#define WPS_MSGTYPE_ENROLLEE_OPEN_8021X   0x01
#define WPS_MSGTYPE_REGISTRAR             0x02
#define WPS_MSGTYPE_AP_WLAN_MGR           0x03

/* RF Band */
#define WPS_RFBAND_24GHZ    0x01
#define WPS_RFBAND_50GHZ    0x02

/* Simple Config state */
#define WPS_SCSTATE_UNCONFIGURED    0x01
#define WPS_SCSTATE_CONFIGURED      0x02

typedef TlvObj_uint8
	CTlvRadioEnabled,
	CTlvReboot,
	CTlvRegistrarEstablished,
	CTlvSelRegistrar,
	CTlvPortableDevice,
	CTlvAPSetupLocked,
	CTlvKeyProvidedAuto,
	CTlv8021XEnabled;

typedef TlvObj_uint8
	CTlvConnType,
	CTlvConnTypeFlags,
	CTlvMsgType,
	CTlvNwIndex,
	CTlvNwKeyIndex,
	CTlvPowerLevel,
	CTlvPskCurrent,
	CTlvPskMax,
	CTlvRegistrarCurrent,
	CTlvRegistrarMax,
	CTlvReqType,
	CTlvRespType,
	CTlvRfBand,
	CTlvScState,
	CTlvTotNetworks,
	CTlvVersion,
	CTlvWEPTransmitKey;

typedef TlvObj_uint16
	CTlvApChannel,
	CTlvAssocState,
	CTlvAuthType,
	CTlvAuthTypeFlags,
	CTlvConfigMethods,
	CTlvConfigError,
	CTlvDevicePwdId,
	CTlvEncrType,
	CTlvEncrTypeFlags,
	CTlvPermittedCfgMethods,
	CTlvSelRegCfgMethods;

typedef TlvObj_uint32
	CTlvFeatureId,
	CTlvOsVersion,
	CTlvKeyLifetime;

typedef TlvObj_ptr
	CTlvConfUrl4, /* <= 64B */
	CTlvConfUrl6, /* <= 76B */
	CTlvDeviceName, /* <= 32B */
	CTlvIdentity, /* <= 80B */
	CTlvIdentityProof,
	CTlvManufacturer, /* <= 64B */
	CTlvModelName, /* <= 32B */
	CTlvModelNumber, /* <= 32B */
	CTlvNwKey, /* <= 64B */
	CTlvNewDeviceName, /* <= 32B */
	CTlvNewPwd, /* <= 64B */
	CTlvSerialNum, /* <= 32B */
	CTlvAppList; /* <= 512B */

typedef TlvObj_ptru
	CTlvMacAddr, /* =6B */
	CTlvAuthenticator, /* =8B */
	CTlvKeyWrapAuth, /* =8B */
	CTlvNonce, /* =16B */
	CTlvEnrolleeNonce, /* =16B */
	CTlvKeyIdentifier, /* =16B */
	CTlvRegistrarNonce, /* =16B */
	CTlvUuid, /* =16B */
	CTlvPubKeyHash, /* =16B */
	CTlvHash, /* =32B */
	CTlvInitVector, /* =32B */
	CTlvSsid, /* =32B */
	CTlvRekeyKey, /* =32B */
	CTlvPublicKey, /* =192B */
	CTlvEapId, /* TBD */
	CTlvX509CertReq,
	CTlvX509Cert,
	CTlvEapType; /* <=8B */

/* WSC 2.0, Subelements in WFA Vendor Extension */
typedef SubTlvObj_uint8
	CSubTlvVersion2,
	CSubTlvReqToEnr,
	CSubTlvSettingsDelayTime,
	CSubTlvNwKeyShareable;

typedef SubTlvObj_ptru
	CSubTlvUuid, /* =16B */
	CSubTlvAuthorizedMACs; /* <=30B */

typedef struct {
	tlvbase_s base;
	uint8 *iv; /* =16B */
	uint8 *ip_encryptedData;
	/* Internal variables, not part of actual TLV */
	uint16 encrDataLength;
} CTlvEncrSettings;

typedef struct {
	tlvbase_s base;
	uint8 *publicKeyHash; /* =20B */
	uint16 pwdId;
	uint8 *ip_devPwd; /* <= 32B */
	/* Internal variables, not part of TLV */
	uint16 devPwdLength;
} CTlvOobDevPwd;

typedef struct {
	tlvbase_s base;
	uint8 *registrarUUID; /* =16B */
	char *cp_deviceName;
} CTlvRegList; /* MAX LEN 512B ? */

typedef struct {
	tlvbase_s base;
	uint8 *vendorId;  /* 3 */
	uint8 *vendorData; /* <=1021B, <=246B for 802.11 Mgmt frames */
	/* Internal variables, not part of TLV */
	uint16 dataLength;
} CTlvVendorExt;

typedef struct {
	tlvbase_s base;

	/* Required attributes */
	CTlvNwIndex nwIndex;
	CTlvSsid ssid;
	CTlvAuthType authType;
	CTlvEncrType encrType;
	CTlvNwKeyIndex nwKeyIndex;
	CTlvNwKey nwKey;
	CTlvMacAddr macAddr;

	/* Optional attributes */
	CTlvEapType eapType;
	CTlvEapId eapIdentity; /* TBD: Define this */
	CTlvKeyLifetime keyLifetime; /* TBD: Define this */
	CTlvVendorExt vendorExt;   /* TBD: Ignore for now */
	CTlvRekeyKey rekeyKey;
	CTlvX509Cert x509Cert;
	CTlvKeyProvidedAuto keyProvidedAuto;
	CTlv8021XEnabled oneXEnabled;
	CTlvWEPTransmitKey WEPKeyIndex;

	/* WSC 2.0 */
	CSubTlvNwKeyShareable nwKeyShareable;
} CTlvCredential;

typedef struct {
	CTlvVersion version;
	CTlvScState scState;
	CTlvSelRegistrar selReg;
	CTlvDevicePwdId devPwdId;
	CTlvSelRegCfgMethods selRegCfgMethods;

	/* WSC 2.0 */
	CTlvUuid uuid_R;

	CTlvVendorExt vendorExt;
	CSubTlvVersion2 version2;
	CSubTlvAuthorizedMACs authorizedMacs;
} CTlvSsrIE;

/* Primary device type */
typedef struct {
	tlvbase_s base;
	uint16 categoryId;
	uint32 oui;
	uint16 subCategoryId;
} CTlvPrimDeviceType;

/* Requested device type */
typedef struct {
	tlvbase_s base;
	uint16 categoryId;
	uint32 oui;
	uint16 subCategoryId;
} CTlvReqDeviceType;

typedef struct {
	tlvbase_s base;
	LPLIST devList;
} CTlvSecDevType;

#define tlv_dserialize(a, b, c, d, f) tlv_dserialize_imp(a, b, c, d, f, __FUNCTION__, __LINE__)
#define tlv_delete(a, b) tlv_delete_imp(a, b, __FUNCTION__, __LINE__)
#define tlv_allocate(a, b, c) tlv_allocate_imp(a, b, c, __FUNCTION__, __LINE__)
#define tlv_find_dserialize(a, b, c, d, f) \
	tlv_find_dserialize_imp(a, b, c, d, f, __FUNCTION__, __LINE__)

#define subtlv_dserialize(a, b, c, d, f) \
	subtlv_dserialize_imp(a, b, c, d, f, __FUNCTION__, __LINE__)
#define subtlv_delete(a, b) subtlv_delete_imp(a, b, __FUNCTION__, __LINE__)
#define subtlv_allocate(a, b, c) subtlv_allocate_imp(a, b, c, __FUNCTION__, __LINE__)
#define subtlv_find_dserialize(a, b, c, d, f) \
	subtlv_find_dserialize_imp(a, b, c, d, f, __FUNCTION__, __LINE__)
#define tlv_find_vendorExtParse(a, b, c) tlv_find_vendorExtParse_imp(a, b, c)

#ifdef WFA_WPS_20_TESTBED
#define tlv_find_primDeviceTypeParse(a, b) tlv_find_primDeviceTypeParse_imp(a, b)
int tlv_find_primDeviceTypeParse_imp(CTlvPrimDeviceType *tlc, BufferObj *theBuf);
#endif /* WFA_WPS_20_TESTBED */

tlvbase_s * tlv_new(uint16 theType);
int tlv_allocate_imp(void *v, uint16 theType, uint16 len, const char *func, int line);
int tlv_dserialize_imp(void *b, uint16 theType, BufferObj * theBuf, uint16 dataSize,
	bool allocate, const char *func, int line);
int tlv_find_dserialize_imp(void *b, uint16 theType, BufferObj * theBuf, uint16 dataSize,
	bool allocate, const char *func, int line);
void tlv_serialize(uint16 theType, BufferObj * theBuf, void *data, uint16 len);
int tlv_write(void *v, BufferObj* theBuf);
int tlv_set(void *v, uint16 theType, void *val, uint16 len);
void tlv_delete_imp(void *b, bool content_only, const char *func, int line);
int tlv_encrSettingsParse(CTlvEncrSettings *tlv, BufferObj *theBuf);
void tlv_encrSettingsWrite(CTlvEncrSettings *tlv, BufferObj *theBuf);
void tlv_oobDevPwdParse(CTlvOobDevPwd *tlv, BufferObj *theBuf);
void tlv_oobDevPwdWrite(CTlvOobDevPwd *tlv, BufferObj *theBuf);
void tlv_credentialInit(CTlvCredential * tlv);
void tlv_credentialDelete(CTlvCredential *b, bool content_only);
void tlv_credentialParseAKey(CTlvCredential *tlv,  BufferObj *theBuf, bool allocate);
void tlv_credentialParse(CTlvCredential *tlv, BufferObj *theBuf, bool allocate);
void tlv_credentialWrite(CTlvCredential *tlv, BufferObj *theBuf);
int tlv_primDeviceTypeParse(CTlvPrimDeviceType *tlc, BufferObj *theBuf);
void tlv_primDeviceTypeWrite(CTlvPrimDeviceType *tlc, BufferObj *theBuf);
void tlv_reqDeviceTypeWrite(CTlvReqDeviceType *tlc, BufferObj *theBuf);

int tlv_vendorExt_set(CTlvVendorExt *v, uint8 *id, uint8 *data, uint16 datalen);
int tlv_vendorExtParse(CTlvVendorExt *tlv, BufferObj *theBuf);
void tlv_vendorExtWrite(CTlvVendorExt *tlv, BufferObj *theBuf);


subtlvbase_s * subtlv_new(uint8 theId);
int subtlv_allocate_imp(void *v, uint8 theId, uint8 len, const char *func, int line);
int subtlv_dserialize_imp(void *b, uint8 theId, BufferObj *theBuf, uint8 dataSize,
	bool allocate, const char *func, int line);
int subtlv_find_dserialize_imp(void *b, uint8 theId, BufferObj *theBuf, uint8 dataSize,
	bool allocate, const char *func, int line);
int subtlv_find_dserialize_imp(void *b, uint8 theId, BufferObj *theBuf, uint8 dataSize,
	bool allocate, const char *func, int line);
void subtlv_serialize(uint8 theId, BufferObj * theBuf, void *data, uint8 m_len);
int subtlv_write(void *v, BufferObj* theBuf);
int subtlv_set(void *v, uint8 theId, void *val, uint8 len);
void subtlv_delete_imp(void *b, bool content_only, const char *func, int line);
int tlv_find_vendorExtParse_imp(void *v, BufferObj *theBuf, uint8 *vendorId);

#define TLV_UINT8 1
#define TLV_UINT16 2
#define TLV_UINT32 3
#define TLV_CHAR_PTR 4
#define TLV_UINT8_PTR 5

/* special. tlv we need to identify when deleting */
#define TLV_CREDENTIAL 6

int tlv_gettype(uint16 theType);
int subtlv_getid(uint8 theId);


#ifdef __cplusplus
}
#endif

#endif /* _WPS_REG_PROTO_TLV_ */