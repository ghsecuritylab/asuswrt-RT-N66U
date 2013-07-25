#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <bcmnvram.h>
#include <bcmdevs.h>
#include "shared.h"

int get_model(void)
{
	if(nvram_match("productid","RT-N66U")) return MODEL_RTN66U;
	else if(nvram_match("productid","RT-N53")) return MODEL_RTN53;
	else if(nvram_match("productid","RT-N16")) return MODEL_RTN16;
	else if(nvram_match("productid","RT-N15U")) return MODEL_RTN15U;
	else if(nvram_match("productid", "RT-N12")) return MODEL_RTN12;
	else if(nvram_match("productid", "RT-N10U")) return MODEL_RTN10U;
#ifdef RTCONFIG_RALINK
	else if(nvram_match("productid", "EA-N66")) return MODEL_EAN66;
	else if(nvram_match("productid", "RT-N56U")) return MODEL_RTN56U;
	else if(nvram_match("productid", "DSL-N55U")) return MODEL_RTN55U;
	else if(nvram_match("productid", "RT-N13U")) return MODEL_RTN13U;
#endif
	else return MODEL_UNKNOWN;
}
