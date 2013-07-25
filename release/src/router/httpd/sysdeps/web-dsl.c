/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * ASUS Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 */

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <httpd.h>
#endif /* WEBS */
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <ralink.h>
#include <iwlib.h>
#include <stapriv.h>
#include <ethutils.h>
#include <semaphore_mfp.h>
#include <shared.h>
#include <sys/mman.h>
#ifndef O_BINARY
#define O_BINARY 	0
#endif
#include <image.h>
#ifndef MAP_FAILED
#define MAP_FAILED (-1)
#endif

#define wan_prefix(unit, prefix)	snprintf(prefix, sizeof(prefix), "wan%d_", unit)
//static char * rfctime(const time_t *timep);
//static char * reltime(unsigned int seconds);
void reltime(unsigned int seconds, char *buf);

#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <net/if_arp.h>

#include <dirent.h>

#define MAX_LINE_SIZE 512
// copy from web.c
static int dump_file(webs_t wp, char *filename)
{
	FILE *fp;
	char buf[MAX_LINE_SIZE];
	int ret=0;

	fp = fopen(filename, "r");
		
	if (fp==NULL) 
	{
		ret+=websWrite(wp, "");
		return (ret);
	}

	ret = 0;
		
	while (fgets(buf, MAX_LINE_SIZE, fp)!=NULL)
	{	 	
	    ret += websWrite(wp, buf);
	}		    				     		
	 
	fclose(fp);		
	
	return (ret);
}




/*
 * retreive and convert dsls values for specified dsl_unit 
 * Example: 
 * <% dsl_get_parameter(); %> for coping wan[n]_ to wan_
 */
int ej_dsl_get_parameter(int eid, webs_t wp, int argc, char_t **argv)
{
	int unit;

	unit = nvram_get_int("dsl_unit");

	// handle generate cases first
	(void)copy_index_to_unindex("dsl_", unit, -1);

	return (websWrite(wp,""));
}

int wanlink_hook_dsl(int eid, webs_t wp, int argc, char_t **argv){
// DSLTODO : dummy code
	char type[16], ip[16], netmask[16], gateway[16], statusstr[16];
	int status = 0, unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int wan_state = -1, wan_sbstate = -1, wan_auxstate = -1;
	char wan_proto[16];

	status = 1;
	strcpy(statusstr, "Connected");

	memset(type, 0, 16);
	strcpy(type, nvram_safe_get(nvram_safe_get("dsl_proto")));
	
	memset(ip, 0, 16);
	memset(netmask, 0, 16);
	memset(gateway, 0, 16);
	if(status == 0){
		strcpy(ip, "0.0.0.0");
		strcpy(netmask, "0.0.0.0");
		strcpy(gateway, "0.0.0.0");	
	}
	else{
		strcpy(ip, nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)));
		strcpy(netmask, nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
		strcpy(gateway, nvram_safe_get(strcat_r(prefix, "gateway", tmp)));	
	}
	
	websWrite(wp, "function wanlink_status() { return %d;}\n", status);
	websWrite(wp, "function wanlink_statusstr() { return '%s';}\n", statusstr);
	websWrite(wp, "function wanlink_type() { return '%s';}\n", type);
	websWrite(wp, "function wanlink_ipaddr() { return '%s';}\n", ip);
	websWrite(wp, "function wanlink_netmask() { return '%s';}\n", netmask);
	websWrite(wp, "function wanlink_gateway() { return '%s';}\n", gateway);
	websWrite(wp, "function wanlink_dns() { return '%s';}\n", nvram_safe_get(strcat_r(prefix, "dns", tmp)));
	websWrite(wp, "function wanlink_lease() { return %s;}\n", nvram_safe_get(strcat_r(prefix, "lease", tmp)));
	websWrite(wp, "function is_private_subnet() { return '%d';}\n", is_private_subnet(nvram_safe_get(strcat_r(prefix, "ipaddr", tmp))));
			

	websWrite(wp, "function wanlink_status() { return %d;}\n", status);
	websWrite(wp, "function wanlink_statusstr() { return '%s';}\n", statusstr);
	websWrite(wp, "function wanlink_type() { return '%s';}\n", type);
	websWrite(wp, "function wanlink_ipaddr() { return '%s';}\n", ip);
	websWrite(wp, "function wanlink_netmask() { return '%s';}\n", netmask);
	websWrite(wp, "function wanlink_gateway() { return '%s';}\n", gateway);
	websWrite(wp, "function wanlink_dns() { return '%s';}\n", nvram_safe_get(strcat_r(prefix, "dns", tmp)));
	websWrite(wp, "function wanlink_lease() { return %s;}\n", nvram_safe_get(strcat_r(prefix, "lease", tmp)));
	websWrite(wp, "function is_private_subnet() { return '%d';}\n", is_private_subnet(nvram_safe_get(strcat_r(prefix, "ipaddr", tmp))));
}

void
do_adsllog_cgi(char *path, FILE *stream)
{
	dump_file(stream, "/tmp/adsl/adsllog.log-1");
	dump_file(stream, "/tmp/adsl/adsllog.log");
	fputs("\r\n", stream); /* terminator */
	fputs("\r\n", stream); /* terminator */
}

int ej_get_isp_list(int eid, webs_t wp, int argc, char_t **argv){
	char *name;
	FILE* fpIsp;
	char bufIsp[512];

	if (ejArgs(argc, argv, "%s", &name) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	fpIsp = fopen(name,"r");
	if (fpIsp != NULL)
	{
	  // read out UTF-8 3 bytes header
	        fread(bufIsp,3,1,fpIsp);
		while(!feof(fpIsp))
		{
			char* ret_fgets;
			ret_fgets = fgets(bufIsp,sizeof(bufIsp),fpIsp);
			if (ret_fgets != NULL)
			{
				websWrite(wp, bufIsp);
				websWrite(wp, "\n");
			}
		}
		fclose(fpIsp);
	}
	return 0;
}

int ej_get_DSL_WAN_list(int eid, webs_t wp, int argc, char_t **argv){
        char buf[MAX_LINE_SIZE];
        char buf2[MAX_LINE_SIZE];
		char prefix[]="dslXXXXXX_", tmp[100];
        int unit;
        int j;        
        int firstItem;

		char *display_items[] = {"dsl_enable", "dsl_vpi", "dsl_vci","dsl_proto", "dsl_encap", 
			"dsl_svc_cat", "dsl_pcr","dsl_scr","dsl_mbs",NULL};

		for ( unit = 0; unit<8; unit++ ) {
			snprintf(prefix, sizeof(prefix), "dsl%d_", unit);
			firstItem = 1;
			websWrite(wp, "[");			
			for ( j = 0; display_items[j] != NULL; j++ ) {
                if(firstItem == 1) {
                        firstItem = 0;
				}
                else {
                        websWrite(wp, ", ");
				}
				strcpy(buf,nvram_safe_get(strcat_r(prefix, &display_items[j][4], tmp)));
				if (strcmp(buf,"")==0) {
					strcpy(buf2,"\"0\"");
				}
				else {
					sprintf(buf2,"\"%s\"",buf);
				}
                websWrite(wp, "%s", buf2);
	        }
	        if (unit != 7) {
				websWrite(wp, "], ");
			}
			else {
				websWrite(wp, "]");			
			}
		}

        return 0;
}

