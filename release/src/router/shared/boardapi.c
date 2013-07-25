#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bcmnvram.h>

#include "utils.h"
#include "shutils.h"
#include "shared.h"

#define GPIO_ACTIVE_LOW 0x1000

int btn_rst_gpio = 0x0ff;
int btn_wps_gpio = 0xff;
int led_pwr_gpio = 0xff;
int led_usb_gpio = 0xff;
int wan_port = 0xff;
int fan_gpio = 0xff;
int have_fan_gpio = 0xff;

int init_gpio(void)
{

	// TODO: system dependent initialization
	
	return 0;
}


int button_pressed(int which)
{
	int use_gpio;
	int gpio_value;

	switch(which) {
		case BTN_RESET:
			use_gpio = nvram_get_int("btn_rst_gpio");
			break;
		case BTN_WPS:
			use_gpio = nvram_get_int("btn_wps_gpio");
			break;
		case BTN_FAN:
			use_gpio = nvram_get_int("fan_gpio");
			break;
		case BTN_HAVE_FAN:
			use_gpio = nvram_get_int("have_fan_gpio");
                        break;
		default:
			use_gpio = 0xff;
			break;
	}

	if((use_gpio&0xff)!=0x0ff) 
	{
		gpio_value = get_gpio(use_gpio&0xff);

		//_dprintf("use_gpio: %x gpio_value: %x\n", use_gpio, gpio_value);

		if((use_gpio&GPIO_ACTIVE_LOW)==0) // active high case
			return gpio_value==0 ? 0 : 1;
		else 
			return gpio_value==0 ? 1 : 0;
	}
	else return 0;
}


int led_control(int which, int mode)
{
	int use_gpio;
//	int gpio_value;
	int enable, disable;

	switch(which) {
		case LED_POWER:
			use_gpio = nvram_get_int("led_pwr_gpio");
			break;
		case LED_USB:
			use_gpio = nvram_get_int("led_usb_gpio");
			break;
		case LED_WPS:	
			use_gpio = nvram_get_int("led_wps_gpio");
			break;
		case LED_LAN:
			use_gpio = nvram_get_int("led_lan_gpio");
			break;
		case LED_WAN:
			use_gpio = nvram_get_int("led_wan_gpio");
			break;
		case FAN:
			use_gpio = nvram_get_int("fan_gpio");
			break;
                case HAVE_FAN:
                        use_gpio = nvram_get_int("have_fan_gpio");
                        break;
		default:
			use_gpio = 0xff;
			break;
	}

	if((use_gpio&0xff) != 0xff)
	{
		enable = (use_gpio&GPIO_ACTIVE_LOW)==0 ? 1 : 0;
		disable = (use_gpio&GPIO_ACTIVE_LOW)==0 ? 0: 1;

		switch(mode) {
			case LED_ON:
			case FAN_ON:
			case HAVE_FAN_ON:
				set_gpio((use_gpio&0xff), enable);
				break;
			case LED_OFF:	
			case FAN_OFF:
			case HAVE_FAN_OFF:
				set_gpio((use_gpio&0xff), disable);
				break;
		}
	}
	return 0;
}

int wanport_status(void)
{
//TODO: turn it into a general way?
#ifdef RTCONFIG_RALINK
	return rtl8367m_wanPort_phyStatus();
#else
	char word[100], *next;
	int mask;

	mask = 0;

	foreach(word, nvram_safe_get("wanports"), next) {
		mask |= (0x0001<<atoi(word));
	}
#ifdef RTCONFIG_WIRELESSWAN
	// to do for report wireless connection status
	if(is_wirelesswan_enabled())
		return 1;
#endif
	return get_phy_status(mask);
#endif
}

int wanport_speed(void)
{
	char word[100], *next;
	int mask;

	mask = 0;

	foreach(word, nvram_safe_get("wanports"), next) {
		mask |= (0x0003<<(atoi(word)*2));
	}

#ifdef RTCONFIG_WIRELESSWAN
	if(is_wirelesswan_enabled())
		return 0x01;
#endif
	return get_phy_speed(mask);
}

int wanport_ctrl(int ctrl)
{
#ifdef RTCONFIG_RALINK
	// TODO? no one use it.
	return 1;
#else
	char word[100], *next;
	int mask;

	mask = 0;

	foreach(word, nvram_safe_get("wanports"), next) {
		mask |= (0x0001<<atoi(word));
	}
#ifdef RTCONFIG_WIRELESSWAN
	// TODO for enable/disable wireless radio
	if(is_wirelesswan_enabled())
		return 0;
#endif
	return set_phy_ctrl(mask, ctrl);
#endif
}

int lanport_status(void)
{
// turn into a general way?
#ifdef RTCONFIG_RALINK
	return rtl8367m_lanPorts_phyStatus();
#else
	char word[100], *next;
	int mask;

	mask = 0;

	foreach(word, nvram_safe_get("lanports"), next) {
		mask |= (0x0001<<atoi(word));
	}
	return get_phy_status(mask);
#endif
}

int lanport_speed(void)
{
#ifdef RTCONFIG_RALINK
	return 1;
#else
	char word[100], *next;
	int mask;

	mask = 0;

	foreach(word, nvram_safe_get("lanports"), next) {
		mask |= (0x0003<<(atoi(word)*2));
	}
	return get_phy_speed(mask);
#endif
}

int lanport_ctrl(int ctrl)
{
	// no general way for ralink platform, so individual api for each kind of switch are used
#ifdef RTCONFIG_RALINK
	if(ctrl) rtl8367m_LanPort_linkUp();
	else rtl8367m_LanPort_linkDown();
	return 1;
#else
	char word[100], *next;
	int mask;

	mask = 0;

	foreach(word, nvram_safe_get("lanports"), next) {
		mask |= (0x0001<<atoi(word));
	}
	return set_phy_ctrl(mask, ctrl);
#endif
}

