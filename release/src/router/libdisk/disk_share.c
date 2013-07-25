/*
 *  * This program is free software; you can redistribute it and/or
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>	// for mkdir()
#include <sys/types.h>	// for mkdir()
#include <unistd.h>	// for rmdir()
#include <bcmnvram.h>
#include <shared.h>

#include "usb_info.h"
#include "disk_initial.h"
#include "disk_share.h"

#include <semaphore_mfp.h>

// Success: return value is account number. Fail: return value is -1.
extern int get_account_list(int *acc_num, char ***account_list) {
	char *nvram, *nvram_value;
	char **tmp_account_list, **tmp_account;
	int len, i, j;
	
	*acc_num = atoi(nvram_safe_get("acc_num"));
	if (*acc_num <= 0)
		return 0;
	
	for (i = 0; i < *acc_num; ++i) {
		if (i < 10)
			len = strlen("M");
		else if (i < 100)
			len = strlen("MN");
		else
			len = strlen("MNO");
		len += strlen("acc_username");
		nvram = (char *)malloc(sizeof(char)*(len+1));
		if (nvram == NULL) {
			usb_dbg("Can't malloc \"nvram\".\n");
			return -1;
		}
		sprintf(nvram, "acc_username%d", i);
		nvram[len] = 0;
		
		nvram_value = nvram_safe_get(nvram);
		free(nvram);
		if (nvram_value == NULL || strlen(nvram_value) <= 0)
			return -1;
		
		tmp_account = (char **)malloc(sizeof(char *)*(i+1));
		if (tmp_account == NULL) {
			usb_dbg("Can't malloc \"tmp_account\".\n");
			return -1;
		}

		len = strlen(nvram_value);
		tmp_account[i] = (char *)malloc(sizeof(char)*(len+1));
		if (tmp_account[i] == NULL) {
			usb_dbg("Can't malloc \"tmp_account[%d]\".\n", i);
			free(tmp_account);
			return -1;
		}
		strcpy(tmp_account[i], nvram_value);
		tmp_account[i][len] = 0;

		if (i != 0) {
			for (j = 0; j < i; ++j)
				tmp_account[j] = tmp_account_list[j];

			free(tmp_account_list);
			tmp_account_list = tmp_account;
		}
		else
			tmp_account_list = tmp_account;
	}
	
	*account_list = tmp_account_list;
	
	return *acc_num;
}

extern int get_folder_list(const char *const mount_path, int *sh_num, char ***folder_list) {
	char **tmp_folder_list, target[16];
	int len, i;
	char *list_file, *list_info;
	char *follow_info, *follow_info_end, backup;
	
	// 1. get list file
	len = strlen(mount_path)+strlen("/.__folder_list.txt");
	list_file = (char *)malloc(sizeof(char)*(len+1));
	if (list_file == NULL) {
		usb_dbg("Can't malloc \"list_file\".\n");
		return -1;
	}
	sprintf(list_file, "%s/.__folder_list.txt", mount_path);
	list_file[len] = 0;
	
	// 2. read if the list file is existed
	if (!test_if_file(list_file)){
#if 1
		initial_folder_list(mount_path);
#else
		usb_dbg("No file: %s.\n", list_file);
		free(list_file);
		return -1;
#endif
	}
	
	list_info = read_whole_file(list_file);
	if (list_info == NULL) {
		usb_dbg("No content in %s.\n", list_file);
		free(list_file);
		return -1;
	}
	free(list_file);
	
	// 3. find sh_num
	follow_info = strstr(list_info, "sh_num=");
	if (follow_info == NULL) {
		usb_dbg("The content in %s is wrong.\n", list_file);
		free(list_info);
		return -1;
	}
	
	follow_info += strlen("sh_num=");
	follow_info_end = follow_info;
	while (*follow_info_end != 0 && *follow_info_end != '\n')
		++follow_info_end;
	if (*follow_info_end == 0) {
		usb_dbg("The content in %s is wrong.\n", list_file);
		free(list_info);
		return -1;
	}
	backup = *follow_info_end;
	*follow_info_end = 0;

	*sh_num = atoi(follow_info);
	*follow_info_end = backup;
	
	if (*sh_num <= 0){
		usb_dbg("There is no folder in %s.\n", mount_path);
		return 0;
	}
	
	// 4. get folder list from the folder list file
	tmp_folder_list = (char **)malloc(sizeof(char *)*((*sh_num)+1));
	if (tmp_folder_list == NULL) {
		usb_dbg("Can't malloc \"tmp_folder_list\".\n");
		free(list_info);
		return -1;
	}
	
	for (i = 0; i < *sh_num; ++i) {
		// 5. get folder name
		memset(target, 0, 16);
		sprintf(target, "\nsh_name%d=", i);
		follow_info = strstr(list_info, target);
		if (follow_info == NULL) {
			usb_dbg("The list content in %s is wrong.\n", mount_path);
			free(list_info);
			free_2_dimension_list(sh_num, &tmp_folder_list);
			return -1;
		}
		
		follow_info += strlen(target);
		follow_info_end = follow_info;
		while (*follow_info_end != 0 && *follow_info_end != '\n')
			++follow_info_end;
		if (*follow_info_end == 0) {
			usb_dbg("The list content in %s is wrong.\n", mount_path);
			free(list_info);
			free_2_dimension_list(sh_num, &tmp_folder_list);
			return -1;
		}
		backup = *follow_info_end;
		*follow_info_end = 0;
		
		len = strlen(follow_info);
		tmp_folder_list[i] = (char *)malloc(sizeof(char)*(len+1));
		if (tmp_folder_list == NULL) {
			usb_dbg("Can't malloc \"tmp_folder_list\".\n");
			*follow_info_end = backup;
			free(list_info);
			free_2_dimension_list(sh_num, &tmp_folder_list);
			return -1;
		}
		strcpy(tmp_folder_list[i], follow_info);
		tmp_folder_list[i][len] = 0;
		
		*follow_info_end = backup;
	}
	
	*folder_list = tmp_folder_list;
	
	return *sh_num;
}

extern int get_all_folder(const char *const mount_path, int *sh_num, char ***folder_list) {
	DIR *pool_to_open;
	struct dirent *dp;
	char *testdir;
	char **tmp_folder_list, **tmp_folder;
	int len, i;
	
	pool_to_open = opendir(mount_path);
	if (pool_to_open == NULL) {
		usb_dbg("Can't opendir \"%s\".\n", mount_path);
		return -1;
	}
	
	*sh_num = 0;
	while ((dp = readdir(pool_to_open)) != NULL) {
		//if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
		if (dp->d_name[0] == '.')
			continue;
		
		if (test_if_System_folder(dp->d_name) == 1)
			continue;
		
		len = strlen(mount_path)+strlen("/")+strlen(dp->d_name);
		testdir = (char *)malloc(sizeof(char)*(len+1));
		if (testdir == NULL) {
			closedir(pool_to_open);
			return -1;
		}
		sprintf(testdir, "%s/%s", mount_path, dp->d_name);
		testdir[len] = 0;
		if (!test_if_dir(testdir)) {
			free(testdir);
			continue;
		}
		free(testdir);
		
		tmp_folder = (char **)malloc(sizeof(char *)*(*sh_num+1));
		if (tmp_folder == NULL) {
			usb_dbg("Can't malloc \"tmp_folder\".\n");
			return -1;
		}
		
		len = strlen(dp->d_name);
		tmp_folder[*sh_num] = (char *)malloc(sizeof(char)*(len+1));
		if (tmp_folder[*sh_num] == NULL) {
			usb_dbg("Can't malloc \"tmp_folder[%d]\".\n", *sh_num);
			free(tmp_folder);
			return -1;
		}
		strcpy(tmp_folder[*sh_num], dp->d_name);
		if (*sh_num != 0) {
			for (i = 0; i < *sh_num; ++i)
				tmp_folder[i] = tmp_folder_list[i];

			free(tmp_folder_list);
			tmp_folder_list = tmp_folder;
		}
		else
			tmp_folder_list = tmp_folder;
		
		++(*sh_num);
	}
	closedir(pool_to_open);
	
	*folder_list = tmp_folder_list;
	
	return 0;
}

extern int get_var_file_name(const char *const account, const char *const path, char **file_name){
	int len;
	char *var_file;

	if(path == NULL)
		return -1;

	len = strlen(path)+strlen("/.___var.txt");
	if(account != NULL)
		len += strlen(account);
	*file_name = (char *)malloc(sizeof(char)*(len+1));
	if(*file_name == NULL)
		return -1;

	var_file = *file_name;
	if(account != NULL)
		sprintf(var_file, "%s/.__%s_var.txt", path, account);
	else
		sprintf(var_file, "%s/.___var.txt", path);
	var_file[len] = 0;

	return 0;
}

extern void free_2_dimension_list(int *num, char ***list) {
	int i;
	char **target = *list;
	
	if (*num <= 0 || target == NULL){
		*num = 0;
		return;
	}
	
	for (i = 0; i < *num; ++i)
		if (target[i] != NULL)
			free(target[i]);
	
	if (target != NULL)
		free(target);
	
	*num = 0;
}

extern int initial_folder_list(const char *const mount_path) {
	int sh_num;
	char **folder_list;
	FILE *fp;
	char *list_file;
	int result, len, i;
	
	if (mount_path == NULL || strlen(mount_path) <= 0) {
		usb_dbg("No input, mount_path\n");
		return -1;
	}
	
	// 2. get the list_file
	len = strlen(mount_path)+strlen("/.__folder_list.txt");
	list_file = (char *)malloc(sizeof(char)*(len+1));
	if (list_file == NULL) {
		usb_dbg("Can't malloc \"list_file\".\n");
		return -1;
	}
	sprintf(list_file, "%s/.__folder_list.txt", mount_path);
	list_file[len] = 0;
	
	// 3. get the folder number and folder_list
	result = get_all_folder(mount_path, &sh_num, &folder_list);
	if (result != 0) {
		usb_dbg("Can't get the folder list in \"%s\".\n", mount_path);
		free_2_dimension_list(&sh_num, &folder_list);
		free(list_file);
		return -1;
	}
	
	// 4. write the folder info
	fp = fopen(list_file, "w");
	if (fp == NULL) {
		usb_dbg("Can't create folder_list, \"%s\".\n", list_file);
		free_2_dimension_list(&sh_num, &folder_list);
		free(list_file);
		return -1;
	}
	free(list_file);
	
	fprintf(fp, "sh_num=%d\n", sh_num);
	for (i = 0; i < sh_num; ++i)
		fprintf(fp, "sh_name%d=%s\n", i, folder_list[i]);
	fclose(fp);
	
	free_2_dimension_list(&sh_num, &folder_list);
	return 0;
}

extern int initial_var_file(const char *const account, const char *const mount_path) {
	FILE *fp;
	char *var_file;
	int result, i;
	int sh_num;
	char **folder_list;
	int samba_right, ftp_right, dms_right;
#ifdef RTCONFIG_WEBDAV
	int webdav_right;
#endif
	
	if (mount_path == NULL || strlen(mount_path) <= 0) {
		usb_dbg("No input, mount_path\n");
		return -1;
	}
	
	// 1. get the folder number and folder_list
	//result = get_folder_list(mount_path, &sh_num, &folder_list);
	result = get_all_folder(mount_path, &sh_num, &folder_list);
	
	// 2. get the var file
	if(get_var_file_name(account, mount_path, &var_file)){
		usb_dbg("Can't malloc \"var_file\".\n");
		free_2_dimension_list(&sh_num, &folder_list);
		return -1;
	}
	
	// 3. get the default permission of all protocol.
	if(account == NULL // share mode.
			|| !strcmp(account, nvram_safe_get("http_username"))){
		samba_right = DEFAULT_SAMBA_RIGHT;
		ftp_right = DEFAULT_FTP_RIGHT;
		dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV
		webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
	}
	else{
		samba_right = 0;
		ftp_right = 0;
		dms_right = 0;
#ifdef RTCONFIG_WEBDAV
		webdav_right = 0;
#endif
	}
	
	// 4. write the default content in the var file
	if ((fp = fopen(var_file, "w")) == NULL) {
		usb_dbg("Can't create the var file, \"%s\".\n", var_file);
		free_2_dimension_list(&sh_num, &folder_list);
		free(var_file);
		return -1;
	}
	
	for (i = -1; i < sh_num; ++i) {
		fprintf(fp, "*");
		
		if(i != -1)
			fprintf(fp, "%s", folder_list[i]);
#ifdef RTCONFIG_WEBDAV
		fprintf(fp, "=%d%d%d%d\n", samba_right, ftp_right, dms_right, webdav_right);
#else
		fprintf(fp, "=%d%d%d\n", samba_right, ftp_right, dms_right);
#endif
	}
	
	fclose(fp);
	free_2_dimension_list(&sh_num, &folder_list);
	free(var_file);
	
	return 0;
}

extern int initial_all_var_file(const char *const mount_path) {
	char *command;
	int len, i, result;
	int acc_num;
	char **account_list;
	
	if (mount_path == NULL || strlen(mount_path) <= 0) {
		usb_dbg("No input, mount_path\n");
		return -1;
	}
	
	// 1. get the account number and account_list
	result = get_account_list(&acc_num, &account_list);
	
	// 2. delete all var files
	len = strlen("cd ")+strlen(mount_path)+strlen(" && rm -f .__* && cd");
	command = (char *)malloc(sizeof(char)*(len+1));
	if (command == NULL) {
		usb_dbg("Can't malloc \"command\".\n");
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}
	sprintf(command, "cd %s && rm -f .__* && cd", mount_path);
	command[len] = 0;
	result = system(command);
	free(command);
	if (result != 0)
		usb_dbg("Fail to delete all var files!\n");
	
	// 3. initial the var file
	result = initial_var_file(NULL, mount_path); // share mode.
	if (result != 0)
		usb_dbg("Can't initial the var file for the share mode.\n");
	
	for (i = 0; i < acc_num; ++i) {
		result = initial_var_file(account_list[i], mount_path);
		if (result != 0)
			usb_dbg("Can't initial the var file of \"%s\".\n", account_list[i]);
	}
	free_2_dimension_list(&acc_num, &account_list);
	
	// 4. initial the folder list
	result = initial_folder_list(mount_path);
	if (result != 0)
		usb_dbg("Can't initial the folder list.\n");
	
	return 0;
}

extern int test_of_var_files(const char *const mount_path) {
	create_if_no_var_files(mount_path);	// According to the old folder_list, add the new folder.
	initial_folder_list(mount_path);	// get the new folder_list.
	create_if_no_var_files(mount_path);	// According to the new folder_list, add the new var file.
	
	return 0;
}

extern int create_if_no_var_files(const char *const mount_path) {
	int acc_num;
	char **account_list;
	int result, i;
	char *var_file;
	
	// 1. get the account number and account_list
	result = get_account_list(&acc_num, &account_list);
	
	// 2. get the var_file for the share mode.
	if(get_var_file_name(NULL, mount_path, &var_file)){ // share mode.
		usb_dbg("Can't malloc \"var_file\".\n");
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}
	
	// 3. test if the var_file is existed
	if (!test_if_file(var_file)) {
		// 4.1. create the var_file when it's not existed
		result = initial_var_file(NULL, mount_path);
		if (result != 0)
			usb_dbg("Can't initial the var file for the share mode.\n");
	}
	else{
		// 4.2. add the new folder into the var file
		result = modify_if_exist_new_folder(NULL, mount_path);
		if (result != 0)
			usb_dbg("Can't check if there's new folder in %s.\n", mount_path);
	}
	free(var_file);
	
	// 5. get the var_file of all accounts.
	for (i = 0; i < acc_num; ++i) {
		if(get_var_file_name(account_list[i], mount_path, &var_file)){
			usb_dbg("Can't malloc \"var_file\".\n");
			free_2_dimension_list(&acc_num, &account_list);
			return -1;
		}
		
		if (!test_if_file(var_file)) {
			result = initial_var_file(account_list[i], mount_path);
			if (result != 0)
				usb_dbg("Can't initial the var file of \"%s\".\n", account_list[i]);
		}
		else{
			result = modify_if_exist_new_folder(account_list[i], mount_path);
			if (result != 0)
				usb_dbg("Can't check if there's new folder in %s.\n", mount_path);
		}
		free(var_file);
	}
	free_2_dimension_list(&acc_num, &account_list);
	
	return 0;
}

extern int modify_if_exist_new_folder(const char *const account, const char *const mount_path) {
	int sh_num;
	char **folder_list, *target;
	int result, i, len;
	char *var_file;
	FILE *fp;
	int samba_right, ftp_right, dms_right;
#ifdef RTCONFIG_WEBDAV
	int webdav_right;
#endif
	
	// 1. get the var file
	if(get_var_file_name(account, mount_path, &var_file)){
		usb_dbg("Can't malloc \"var_file\".\n");
		free_2_dimension_list(&sh_num, &folder_list);
		return -1;
	}
	
	// 2. get all folder in mount_path
	result = get_all_folder(mount_path, &sh_num, &folder_list);
	if (result != 0) {
		usb_dbg("Can't get the folder list in \"%s\".\n", mount_path);
		free_2_dimension_list(&sh_num, &folder_list);
		free(var_file);
		return -1;
	}
	
	for (i = 0; i < sh_num; ++i) {
		result = test_if_exist_share(mount_path, folder_list[i]);
		if(result)
			continue;
		
		// 3. get the target
		len = strlen("*")+strlen(folder_list[i])+strlen("=");
		target = (char *)malloc(sizeof(char)*(len+1));
		if (target == NULL) {
			usb_dbg("Can't allocate \"target\".\n");
			free_2_dimension_list(&sh_num, &folder_list);
			free(var_file);
			return -1;
		}
		sprintf(target, "*%s=", folder_list[i]);
		target[len] = 0;
		
		// 4. get the default permission of all protocol.
		if(account == NULL // share mode.
				|| !strcmp(account, nvram_safe_get("http_username"))){
			samba_right = DEFAULT_SAMBA_RIGHT;
			ftp_right = DEFAULT_FTP_RIGHT;
			dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV
			webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
		}
		else{
			samba_right = 0;
			ftp_right = 0;
			dms_right = 0;
#ifdef RTCONFIG_WEBDAV
			webdav_right = 0;
#endif
		}
		
		// 5. add the information of the new folder
		fp = fopen(var_file, "a+");
		if (fp == NULL) {
			usb_dbg("Can't write \"%s\".\n", var_file);
			free_2_dimension_list(&sh_num, &folder_list);
			free(var_file);
			free(target);
			return -1;
		}
		
#ifdef RTCONFIG_WEBDAV
		fprintf(fp, "%s%d%d%d%d\n", target, samba_right, ftp_right, dms_right, webdav_right);
#else
		fprintf(fp, "%s%d%d%d\n", target, samba_right, ftp_right, dms_right);
#endif
		free(target);
		fclose(fp);
	}
	free_2_dimension_list(&sh_num, &folder_list);
	free(var_file);
	
	return 0;
}

extern int get_permission(const char *const account,
						  const char *const mount_path,
						  const char *const folder,
						  const char *const protocol) {
	char *var_file, *var_info;
	char *target, *follow_info;
	int len, result;
	
	// 1. get the var file
	if(get_var_file_name(account, mount_path, &var_file)){
		usb_dbg("Can't malloc \"var_file\".\n");
		return -1;
	}
	
	// 2. get the content of the var_file of the account
	var_info = read_whole_file(var_file);
	if (var_info == NULL) {
		usb_dbg("get_permission: \"%s\" isn't existed or there's no content.\n", var_file);
		free(var_file);
		return -1;
	}
	free(var_file);
	
	// 3. get the target in the content
	if(folder == NULL)
		len = strlen("*=");
	else
		len = strlen("*")+strlen(folder)+strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (target == NULL) {
		usb_dbg("Can't allocate \"target\".\n");
		free(var_info);
		return -1;
	}
	if(folder == NULL)
		strcpy(target, "*=");
	else
		sprintf(target, "*%s=", folder);
	target[len] = 0;
	
	follow_info = upper_strstr(var_info, target);
	free(target);
	if (follow_info == NULL) {
		if(account == NULL)
			usb_dbg("No right about \"%s\" with the share mode.\n", (folder == NULL?"Pool":folder));
		else
			usb_dbg("No right about \"%s\" with \"%s\".\n", (folder == NULL?"Pool":folder), account);
		free(var_info);
		return -1;
	}
	
	follow_info += len;
	if (follow_info[3] != '\n') {
		if(account == NULL)
			usb_dbg("The var info is incorrect.\nPlease reset the var file of the share mode.\n");
		else
			usb_dbg("The var info is incorrect.\nPlease reset the var file of \"%s\".\n", account);
		free(var_info);
		return -1;
	}
	
	// 4. get the right of folder
	if (!strcmp(protocol, PROTOCOL_CIFS))
		result = follow_info[0]-'0';
	else if (!strcmp(protocol, PROTOCOL_FTP))
		result = follow_info[1]-'0';
	else if (!strcmp(protocol, PROTOCOL_MEDIASERVER))
		result = follow_info[2]-'0';
	else{
		usb_dbg("The protocol, \"%s\", is incorrect.\n", protocol);
		free(var_info);
		return -1;
	}
	free(var_info);
	
	if (result < 0 || result > 3) {
		if(account == NULL)
			usb_dbg("The var info is incorrect.\nPlease reset the var file of the share mode.\n");
		else
			usb_dbg("The var info is incorrect.\nPlease reset the var file of \"%s\".\n", account);
		return -1;
	}
	
	return result;
}

extern int set_permission(const char *const account,
						  const char *const mount_path,
						  const char *const folder,
						  const char *const protocol,
						  const int flag) {
	FILE *fp;
	char *var_file, *var_info;
	char *target, *follow_info;
	int len;
	
	if (flag < 0 || flag > 3) {
		usb_dbg("correct Rights is 0, 1, 2, 3.\n");
		return -1;
	}
	
	// 1. get the var file
	if(get_var_file_name(account, mount_path, &var_file)){
		usb_dbg("Can't malloc \"var_file\".\n");
		return -1;
	}
	
	// 2. get the content of the var_file of the account
	var_info = read_whole_file(var_file);
	if (var_info == NULL) {
		initial_var_file(account, mount_path);
		sleep(1);
		var_info = read_whole_file(var_file);
		if (var_info == NULL) {
			usb_dbg("set_permission: \"%s\" isn't existed or there's no content.\n", var_file);
			free(var_file);
			return -1;
		}
	}
	
	// 3. get the target in the content
	if(folder == NULL)
		len = strlen("*=");
	else
		len = strlen("*")+strlen(folder)+strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (target == NULL) {
		usb_dbg("Can't allocate \"target\".\n");
		free(var_file);
		free(var_info);
		
		return -1;
	}
	if(folder == NULL)
		strcpy(target, "*=");
	else
		sprintf(target, "*%s=", folder);
	target[len] = 0;
	
	// 4. judge if the target is in the var file.
	follow_info = upper_strstr(var_info, target);
	if (follow_info == NULL) {
		if(account == NULL)
			usb_dbg("No right about \"%s\" with the share mode.\n", (folder == NULL?"Pool":folder));
		else
			usb_dbg("No right about \"%s\" with \"%s\".\n", (folder == NULL?"Pool":folder), account);
		free(var_info);
		
		fp = fopen(var_file, "a+");
		if (fp == NULL) {
			usb_dbg("1. Can't rewrite the file, \"%s\".\n", var_file);
			free(var_file);
			return -1;
		}
		free(var_file);
		
		fprintf(fp, "%s", target);
		free(target);
		
		// 5.1 change the right of folder
#ifdef RTCONFIG_WEBDAV
		if (!strcmp(protocol, PROTOCOL_CIFS))
			fprintf(fp, "%d%d%d%d\n", flag, 0, DEFAULT_DMS_RIGHT, DEFAULT_WEBDAV_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_FTP))
			fprintf(fp, "%d%d%d%d\n", 0, flag, DEFAULT_DMS_RIGHT, DEFAULT_WEBDAV_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_MEDIASERVER))
			fprintf(fp, "%d%d%d%d\n", 0, 0, flag, DEFAULT_WEBDAV_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_WEBDAV))
			fprintf(fp, "%d%d%d%d\n", 0, 0, DEFAULT_DMS_RIGHT, flag);
#else
		if (!strcmp(protocol, PROTOCOL_CIFS))
			fprintf(fp, "%d%d%d\n", flag, 0, DEFAULT_DMS_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_FTP))
			fprintf(fp, "%d%d%d\n", 0, flag, DEFAULT_DMS_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_MEDIASERVER))
			fprintf(fp, "%d%d%d\n", 0, 0, flag);
#endif
		else{
			usb_dbg("The protocol, \"%s\", is incorrect.\n", protocol);
			
			fclose(fp);
			return -1;
		}
		
		fclose(fp);
		return 0;
	}
	free(target);
	
	follow_info += len;
	if (follow_info[3] != '\n') {
		if(account == NULL)
			usb_dbg("The var info is incorrect.\nPlease reset the var file of the share mode.\n");
		else
			usb_dbg("The var info is incorrect.\nPlease reset the var file of \"%s\".\n", account);
		free(var_file);
		free(var_info);
		return -1;
	}
	
	// 5.2. change the right of folder
	if(!strcmp(protocol, PROTOCOL_CIFS))
		follow_info += PROTOCOL_CIFS_BIT;
	else if(!strcmp(protocol, PROTOCOL_FTP))
		follow_info += PROTOCOL_FTP_BIT;
	else if(!strcmp(protocol, PROTOCOL_MEDIASERVER))
		follow_info += PROTOCOL_MEDIASERVER_BIT;
#ifdef RTCONFIG_WEBDAV
	else if(!strcmp(protocol, PROTOCOL_WEBDAV))
		follow_info += PROTOCOL_WEBDAV_BIT;
#endif
	else{
		usb_dbg("The protocol, \"%s\", is incorrect.\n", protocol);
		free(var_file);
		free(var_info);
		return -1;
	}
	
	if(follow_info[0] == '0'+flag){
		usb_dbg("The %s right of \"%s\" is the same.\n", protocol, folder);
		free(var_file);
		free(var_info);
		return 0;
	}
	
	follow_info[0] = '0'+flag;
	
	// 6. rewrite the var file.
	fp = fopen(var_file, "w");
	if (fp == NULL) {
		usb_dbg("2. Can't rewrite the file, \"%s\".\n", var_file);
		free(var_file);
		free(var_info);
		return -1;
	}
	fprintf(fp, "%s", var_info);
	fclose(fp);
	
	free(var_file);
	free(var_info);
	
	// 7. re-run ftp and samba
	/*result = system("/sbin/run_ftpsamba");
	if (result != 0) {
		usb_dbg("Fail to \"run_ftpsamba\"!\n");
		return -1;
	}//*/
	
	return 0;
}

extern int add_account(const char *const account, const char *const password) {
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	int acc_num;
	char **account_list;
	int result, i;
	char nvram[16], nvram_value[128];
	
	if (account == NULL || strlen(account) <= 0) {
		usb_dbg("No input, \"account\".\n");
		return -1;
	}
	if (password == NULL || strlen(password) <= 0) {
		usb_dbg("No input, \"password\".\n");
		return -1;
	}
	
	// 1. check if can create the account
	result = get_account_list(&acc_num, &account_list);
	if (result < 0) {
		usb_dbg("Can't get the account list\n");
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}
	if (acc_num >= MAX_ACCOUNT_NUM) {
		usb_dbg("Too many accounts are created.\n");
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}
	
	for (i = 0; i < acc_num; ++i)
		if (!strcmp(account, account_list[i])) {
			usb_dbg("\"%s\" is already created.\n", account);
			free_2_dimension_list(&acc_num, &account_list);
			return -1;
		}
	
	// 2. create nvram value about the new account
	memset(nvram_value, 0, 128);
	sprintf(nvram_value, "%d", acc_num+1);
	nvram_set("acc_num", nvram_value);
	
	memset(nvram, 0, 16);
	sprintf(nvram, "acc_username%d", acc_num);
	nvram_set(nvram, account);
	
	memset(nvram, 0, 16);
	sprintf(nvram, "acc_password%d", acc_num);
	nvram_set(nvram, password);
	
	spinlock_lock(SPINLOCK_NVRAMCommit);
	nvram_commit();
	spinlock_unlock(SPINLOCK_NVRAMCommit);

	free_2_dimension_list(&acc_num, &account_list);
	
	// 3. find every pool
	disk_list = read_disk_data();
	if (disk_list == NULL) {
		usb_dbg("Couldn't read the disk list out.\n");
		return -1;
	}
	
	for (follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next) {			
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (follow_partition->mount_point == NULL)
				continue;
			
			// 4. initial the var file of the account
			result = initial_var_file(account, follow_partition->mount_point);
			if (result != 0)
				usb_dbg("Can't initial the var file of \"%s\".\n", account);
		}
	}
	free_disk_data(&disk_list);
	
	// 6. re-run samba
	/*result = system("/sbin/run_samba");
	if (result != 0) {
		usb_dbg("Fail to \"run_samba\"!\n");
		return -1;
	}//*/
	
	return 0;
}

extern int del_account(const char *const account) {
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	int acc_num, target;
	char **account_list;
	int result, i;
	char nvram[16], nvram_value[128];
	char *var_file;
	
	if (account == NULL || strlen(account) <= 0) {
		usb_dbg("No input, \"account\".\n");
		return -1;
	}
	
	// 1. check if can create the account
	result = get_account_list(&acc_num, &account_list);
	if (result < 0) {
		usb_dbg("Can't get the account list\n");
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}
	if (acc_num <= 0) {
		usb_dbg("It's too few account to delete.\n");
		return -1;
	}
	
	result = 0;
	for (i = 0; i < acc_num; ++i)
		if (!strcmp(account_list[i], account)) {
			result = 1;
			target = i;
			break;
		}
	if (result == 0) {
		usb_dbg("There's no \"%s\" in the account list.\n", account);
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}
	
	// 2. delete the nvram value about the deleted account
	--acc_num;
	memset(nvram_value, 0, 128);
	sprintf(nvram_value, "%d", acc_num);
	nvram_set("acc_num", nvram_value);
	
	for (i = target; i < acc_num; ++i) {
		memset(nvram, 0, 16);
		sprintf(nvram, "acc_username%d", i);
		nvram_set(nvram, account_list[i+1]);
		
		memset(nvram_value, 0, 128);
		sprintf(nvram_value, "acc_password%d", i+1);
		
		memset(nvram, 0, 16);
		sprintf(nvram, "acc_password%d", i);
		nvram_set(nvram, nvram_safe_get(nvram_value));
	}
	
	// 3. change to the share mode when no account
	if (acc_num <= 0) {
		nvram_set("st_samba_mode", "1");
		//nvram_set("st_samba_mode", "3");
		nvram_set("st_ftp_mode", "1");
#ifdef RTCONFIG_WEBDAV
		nvram_set("st_webdav_mode", "1");
#endif
	}
	
	spinlock_lock(SPINLOCK_NVRAMCommit);
	nvram_commit();
	spinlock_unlock(SPINLOCK_NVRAMCommit);

	free_2_dimension_list(&acc_num, &account_list);
	
	// 4. find every pool
	disk_list = read_disk_data();
	if (disk_list == NULL) {
		usb_dbg("Couldn't read the disk list out.\n");
		return -1;
	}
	
	for (follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next) {			
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (follow_partition->mount_point == NULL)
				continue;
			
			// 5. delete the var file of the deleted account
			if(get_var_file_name(account, follow_partition->mount_point, &var_file)){
				usb_dbg("Can't malloc \"var_file\".\n");
				free_disk_data(&disk_list);
				return -1;
			}
			
			unlink(var_file);
			
			free(var_file);
		}
	}
	free_disk_data(&disk_list);
	
	// 6. re-run ftp and samba
	/*result = system("/sbin/run_ftpsamba");
	if (result != 0) {
		usb_dbg("Fail to \"run_ftpsamba\"!\n");
		return -1;
	}//*/
	
	return 0;
}

// "new_account" can be the same with "account"!
extern int mod_account(const char *const account, const char *const new_account, const char *const new_password) {
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	int acc_num;
	char **account_list;
	int result, i;
	char nvram[16];
	int account_order = -1;
	char *var_file, *new_var_file;
	
	if (account == NULL || strlen(account) <= 0) {
		usb_dbg("No input, \"account\".\n");
		
		return -1;
	}
	if ((new_account == NULL || strlen(new_account) <= 0) &&
			(new_password == NULL || strlen(new_password) <= 0)) {
		usb_dbg("No input, \"new_account\" or \"new_password\".\n");
		
		return -1;
	}
	
	// 1. check if can modify the account
	result = get_account_list(&acc_num, &account_list);
	if (result < 0) {
		usb_dbg("Can't get the account list!\n");
		free_2_dimension_list(&acc_num, &account_list);
		
		return -1;
	}
	
	for (i = 0; i < acc_num; ++i) {
		if (new_account != NULL && strcmp(new_account, account) && strlen(new_account) > 0) {
			if (!strcmp(account_list[i], new_account)) {
				usb_dbg("\"%s\" is already created.\n", new_account);
				free_2_dimension_list(&acc_num, &account_list);
				
				return -1;
			}
		}
		
		if (!strcmp(account, account_list[i]))
			account_order = i;
	}
	free_2_dimension_list(&acc_num, &account_list);
	
	if (account_order == -1) {
		usb_dbg("1. \"%s\" is not in the existed accounts.\n", account);
		
		return -1;
	}
	
	// 2. modify nvram value about the new account
	if (new_account != NULL && strcmp(new_account, account) && strlen(new_account) > 0) {
		memset(nvram, 0, 16);
		sprintf(nvram, "acc_username%d", account_order);
		
		nvram_set(nvram, new_account);
	}
	
	if (new_password != NULL && strlen(new_password) > 0) {
		memset(nvram, 0, 16);
		sprintf(nvram, "acc_password%d", account_order);
		
		nvram_set(nvram, new_password);
	}
	
	spinlock_lock(SPINLOCK_NVRAMCommit);
	nvram_commit();
	spinlock_unlock(SPINLOCK_NVRAMCommit);
	
	// 3. find every pool
	if (new_account == NULL || strlen(new_account) <= 0/* ||
			(new_account != NULL && !strcmp(new_account, account))*/)
		return 0;

	if ((new_account != NULL && !strcmp(new_account, account)))
		goto rerun;

	disk_list = read_disk_data();
	if (disk_list == NULL) {
		usb_dbg("Couldn't read the disk list out.\n");
		return -1;
	}
	
	for (follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next) {			
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (follow_partition->mount_point == NULL)
				continue;
			
			// 4. get the var_file and new_var_file
			if(get_var_file_name(account, follow_partition->mount_point, &var_file)){
				usb_dbg("Can't malloc \"var_file\".\n");
				free_disk_data(&disk_list);
				return -1;
			}
			
			if(get_var_file_name(new_account, follow_partition->mount_point, &new_var_file)){
				usb_dbg("Can't malloc \"new_var_file\".\n");
				free_disk_data(&disk_list);
				return -1;
			}
			
			// 5. rename the var file
			rename(var_file, new_var_file);
			
			free(var_file);
			free(new_var_file);
		}
	}
rerun:
	// 6. re-run ftp and samba
	/*result = system("/sbin/run_ftpsamba");
	if (result != 0) {
		usb_dbg("Fail to \"run_ftpsamba\"!\n");
		return -1;
	}//*/
	
	return 0;
}

extern int test_if_exist_account(const char *const account) {
	int acc_num;
	char **account_list;
	int result, i;
	
	if(account == NULL)
		return 1;
	
	result = get_account_list(&acc_num, &account_list);
	if (result < 0) {
		usb_dbg("Can't get the account list.\n");
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}
	
	result = 0;
	for (i = 0; i < acc_num; ++i)
		if (!strcmp(account, account_list[i])) {
			result = 1;
			break;
		}
	free_2_dimension_list(&acc_num, &account_list);
	
	return result;
}

extern int add_folder(const char *const account, const char *const mount_path, const char *const folder) {
	int result, i, len;
	int acc_num;
	char **account_list;
	char *var_file, *var_info;
	char *target;
	FILE *fp;
	int samba_right, ftp_right, dms_right;
#ifdef RTCONFIG_WEBDAV
	int webdav_right;
#endif
	char *full_path;
	
	if (mount_path == NULL || strlen(mount_path) <= 0) {
		usb_dbg("No input, \"mount_path\".\n");
		
		return -1;
	}
	if (folder == NULL || strlen(folder) <= 0) {
		usb_dbg("No input, \"folder\".\n");
		
		return -1;
	}
	
	// 1. test if creatting the folder
	result = test_if_exist_share(mount_path, folder);
	if (result != 0) {
		usb_dbg("\"%s\" is already created in %s.\n", folder, mount_path);
		
		return -1;
	}
	
	// 2. create the folder
	len = strlen(mount_path)+strlen("/")+strlen(folder);
	full_path = (char *)malloc(sizeof(char)*(len+1));
	if (full_path == NULL) {
		usb_dbg("Can't malloc \"full_path\".\n");
		
		return -1;
	}
	sprintf(full_path, "%s/%s", mount_path, folder);
	full_path[len] = 0;
	
	umask(0000);
	result = mkdir(full_path, 0777);
	free(full_path);
	if (result != 0) {
		usb_dbg("To create \"%s\" is failed!\n", folder);
		
		return -1;
	}
	
	// 3. add folder's right to every var file
	result = get_account_list(&acc_num, &account_list);
	if (result < 0) {
		usb_dbg("Can't get the account list\n");
		free_2_dimension_list(&acc_num, &account_list);
		
		return -1;
	}
	
	len = strlen("*")+strlen(folder)+strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (target == NULL) {
		usb_dbg("Can't allocate \"target\".\n");
		free_2_dimension_list(&acc_num, &account_list);
		
		return -1;
	}
	sprintf(target, "*%s=", folder);
	target[len] = 0;
	
	for (i = -1; i < acc_num; ++i) {
		// 4. get the var file
		if(i == -1) // share mode.
			result = get_var_file_name(NULL, mount_path, &var_file);
		else
			result = get_var_file_name(account_list[i], mount_path, &var_file);
		if(result){
			usb_dbg("Can't malloc \"var_file\".\n");
			free_2_dimension_list(&acc_num, &account_list);
			free(target);
			return -1;
		}
		
		// 5. check if the created target is exist in the var file
		var_info = read_whole_file(var_file);
		if (var_info == NULL) {
			usb_dbg("add_folder: \"%s\" isn't existed or there's no content.\n", var_file);
		}
		else if (upper_strstr(var_info, target) != NULL) {
			free(var_file);
			free(var_info);
			
			continue;
		}
		else
			free(var_info);
		
		// 6. add the folder's info in the var file
		fp = fopen(var_file, "a+");
		if (fp == NULL) {
			usb_dbg("Can't write \"%s\".\n", var_file);
			free_2_dimension_list(&acc_num, &account_list);
			free(target);
			free(var_file);
			
			return -1;
		}
		
		// 7. get the default permission of all protocol.
		if(i == -1 // share mode.
				|| !strcmp(account_list[i], nvram_safe_get("http_username"))
				){
			samba_right = DEFAULT_SAMBA_RIGHT;
			ftp_right = DEFAULT_FTP_RIGHT;
			dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV
			webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
		}
		else if(account != NULL && !strcmp(account_list[i], account)){
			samba_right = DEFAULT_SAMBA_RIGHT;
			ftp_right = DEFAULT_FTP_RIGHT;
			dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV
			webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
		}
		else{
			samba_right = 0;
			ftp_right = 0;
			dms_right = 0;
#ifdef RTCONFIG_WEBDAV
			webdav_right = 0;
#endif
		}
		
#ifdef RTCONFIG_WEBDAV
		fprintf(fp, "%s%d%d%d%d\n", target, samba_right, ftp_right, dms_right, webdav_right);
#else
		fprintf(fp, "%s%d%d%d\n", target, samba_right, ftp_right, dms_right);
#endif
		fclose(fp);
		free(var_file);
	}
	free_2_dimension_list(&acc_num, &account_list);
	
	// 8. add the folder's info in the folder list
	initial_folder_list(mount_path);
	
	free(target);
	
	return 0;
}

extern int del_folder(const char *const mount_path, const char *const folder) {
	int result, i, len;
	int acc_num;
	char **account_list;
	char *var_file, *var_info;
	char *follow_info, backup;
	char *target;
	FILE *fp;
	char *full_path;
	
	if (mount_path == NULL || strlen(mount_path) <= 0) {
		usb_dbg("No input, \"mount_path\".\n");
		return -1;
	}
	if (folder == NULL || strlen(folder) <= 0) {
		usb_dbg("No input, \"folder\".\n");
		return -1;
	}
	
	// 1. test if deleting the folder
	len = strlen(mount_path)+strlen("/")+strlen(folder);
	full_path = (char *)malloc(sizeof(char)*(len+1));
	if (full_path == NULL) {
		usb_dbg("Can't malloc \"full_path\".\n");
		
		return -1;
	}
	sprintf(full_path, "%s/%s", mount_path, folder);
	full_path[len] = 0;
	
	result = test_if_exist_share(mount_path, folder);
	if (result == 0) {
		result = test_if_dir(full_path);
		
		if (result != 1) {
			usb_dbg("\"%s\" isn't already existed in %s.\n", folder, mount_path);
			free(full_path);
			
			return -1;
		}
	}
	
	// 2. delete the folder
	result = rmdir(full_path);
	free(full_path);
	if (result != 0) {
		usb_dbg("To delete \"%s\" is failed!\n", folder);
		
		return -1;
	}
	
	// 3. get the target which is deleted in every var file
	len = strlen("*")+strlen(folder)+strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (target == NULL) {
		usb_dbg("Can't allocate \"target\".\n");
		
		return -1;
	}
	sprintf(target, "*%s=", folder);
	target[len] = 0;
	
	// 4. del folder's right to every var file
	result = get_account_list(&acc_num, &account_list);
	if (result < 0) {
		usb_dbg("Can't get the account list\n");
		free_2_dimension_list(&acc_num, &account_list);
		free(target);
		return -1;
	}
	
	for (i = -1; i < acc_num; ++i) {
		// 5. get the var file
		if(i == -1) // share mode.
			result = get_var_file_name(NULL, mount_path, &var_file);
		else
			result = get_var_file_name(account_list[i], mount_path, &var_file);
		if(result){
			usb_dbg("Can't malloc \"var_file\".\n");
			free_2_dimension_list(&acc_num, &account_list);
			free(target);
			return -1;
		}
		
		// 6. delete the content about the folder
		var_info = read_whole_file(var_file);
		if (var_info == NULL) {
			usb_dbg("del_folder: \"%s\" isn't existed or there's no content.\n", var_file);
			free(var_file);
			continue;
		}
		
		follow_info = upper_strstr(var_info, target);
		if (follow_info == NULL) {
			if(i == -1)
				usb_dbg("No right about \"%s\" of the share mode.\n", folder);
			else
				usb_dbg("No right about \"%s\" with \"%s\".\n", folder, account_list[i]);
			free(var_file);
			free(var_info);
			continue;
		}
		backup = *follow_info;
		*follow_info = 0;
		
		fp = fopen(var_file, "w");
		if (fp == NULL) {
			usb_dbg("Can't write \"%s\".\n", var_file);
			*follow_info = backup;
			free(var_file);
			free(var_info);
			continue;
		}
		fprintf(fp, "%s", var_info);
		
		*follow_info = backup;
		while (*follow_info != 0 && *follow_info != '\n')
			++follow_info;
		if (*follow_info != 0 && *(follow_info+1) != 0) {
			++follow_info;
			fprintf(fp, "%s", follow_info);
		}
		fclose(fp);
		
		free(var_file);
		free(var_info);
	}
	free_2_dimension_list(&acc_num, &account_list);
	
	// 8. modify the folder's info in the folder list
	initial_folder_list(mount_path);
	
	free_2_dimension_list(&acc_num, &account_list);
	free(target);
	
	return 0;
}

extern int mod_folder(const char *const mount_path, const char *const folder, const char *const new_folder) {
	int result, i, len;
	int acc_num;
	char **account_list;
	char *var_file, *var_info;
	char *target, *new_target;
	FILE *fp;
	char *follow_info, backup;
	char *full_path, *new_full_path;
	
	if (mount_path == NULL || strlen(mount_path) <= 0) {
		usb_dbg("No input, \"mount_path\".\n");
		return -1;
	}
	if (folder == NULL || strlen(folder) <= 0) {
		usb_dbg("No input, \"folder\".\n");
		return -1;
	}
	if (new_folder == NULL || strlen(new_folder) <= 0) {
		usb_dbg("No input, \"new_folder\".\n");
		return -1;
	}
	
	// 1. test if modifying the folder
	len = strlen(mount_path)+strlen("/")+strlen(folder);
	full_path = (char *)malloc(sizeof(char)*(len+1));
	if (full_path == NULL) {
		usb_dbg("Can't malloc \"full_path\".\n");
		
		return -1;
	}
	sprintf(full_path, "%s/%s", mount_path, folder);
	full_path[len] = 0;
	
	len = strlen(mount_path)+strlen("/")+strlen(new_folder);
	new_full_path = (char *)malloc(sizeof(char)*(len+1));
	if (new_full_path == NULL) {
		usb_dbg("Can't malloc \"new_full_path\".\n");
		
		return -1;
	}
	sprintf(new_full_path, "%s/%s", mount_path, new_folder);
	new_full_path[len] = 0;
	
	result = test_if_exist_share(mount_path, folder);
	if (result == 0) {
		result = test_if_dir(full_path);
		
		if (result != 1) {
			usb_dbg("\"%s\" isn't already existed in %s.\n", folder, mount_path);
			free(full_path);
			free(new_full_path);
			
			return -1;
		}
		
		// the folder is existed but not in .__folder_list.txt
		add_folder(NULL, mount_path, folder);
	}
	
	// 2. modify the folder
	result = rename(full_path, new_full_path);
	free(full_path);
	free(new_full_path);
	if (result != 0) {
		usb_dbg("To delete \"%s\" is failed!\n", folder);
		
		return -1;
	}
	
	// 3. add folder's right to every var file
	result = get_account_list(&acc_num, &account_list);
	if (result < 0) {
		usb_dbg("Can't get the account list\n");
		free_2_dimension_list(&acc_num, &account_list);
		
		return -1;
	}
	
	len = strlen("*")+strlen(folder)+strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (target == NULL) {
		usb_dbg("Can't allocate \"target\".\n");
		free_2_dimension_list(&acc_num, &account_list);
		
		return -1;
	}
	sprintf(target, "*%s=", folder);
	target[len] = 0;
	
	len = strlen("*")+strlen(new_folder)+strlen("=");
	new_target = (char *)malloc(sizeof(char)*(len+1));
	if (new_target == NULL) {
		usb_dbg("Can't allocate \"new_target\".\n");
		free_2_dimension_list(&acc_num, &account_list);
		free(target);
		
		return -1;
	}
	sprintf(new_target, "*%s=", new_folder);
	new_target[len] = 0;
	
	for (i = -1; i < acc_num; ++i) {
		// 5. get the var file
		if(i == -1) // share mode.
			result = get_var_file_name(NULL, mount_path, &var_file);
		else
			result = get_var_file_name(account_list[i], mount_path, &var_file);
		if(result){
			usb_dbg("Can't malloc \"var_file\".\n");
			free_2_dimension_list(&acc_num, &account_list);
			free(target);
			free(new_target);
			return -1;
		}
		
		// 6. check if the created target is exist in the var file
		var_info = read_whole_file(var_file);
		if (var_info == NULL) {
			usb_dbg("mod_folder: \"%s\" isn't existed or there's no content.\n", var_file);
			free_2_dimension_list(&acc_num, &account_list);
			free(target);
			free(new_target);
			free(var_file);
			
			return -1;
		}
		
		if ((follow_info = upper_strstr(var_info, target)) == NULL) {
			usb_dbg("1. No \"%s\" in \"%s\"..\n", folder, var_file);
			free_2_dimension_list(&acc_num, &account_list);
			free(target);
			free(new_target);
			free(var_file);
			free(var_info);
			
			return -1;
		}
		
		// 7. modify the folder's info in the var file
		fp = fopen(var_file, "w");
		if (fp == NULL) {
			usb_dbg("Can't write \"%s\".\n", var_file);
			free_2_dimension_list(&acc_num, &account_list);
			free(target);
			free(new_target);
			free(var_file);
			free(var_info);
			
			return -1;
		}
		
		// write the info before target
		backup = *follow_info;
		*follow_info = 0;
		fprintf(fp, "%s", var_info);
		*follow_info = backup;
		
		// write the info before new_target
		fprintf(fp, "%s", new_target);
		
		// write the info after target
		follow_info += strlen(target);
		fprintf(fp, "%s", follow_info);
		
		fclose(fp);
		
		free(var_file);
		free(var_info);
	}
	
	// 8. modify the folder's info in the folder list
	initial_folder_list(mount_path);
	
	free_2_dimension_list(&acc_num, &account_list);
	free(target);
	free(new_target);
	
	return 0;
}

extern int test_if_exist_share(const char *const mount_path, const char *const folder) {
	int sh_num;
	char **folder_list;
	int result, i;
	
	result = get_folder_list(mount_path, &sh_num, &folder_list);
	if (result < 0) {
		usb_dbg("Can't read the folder list in %s.\n", mount_path);
		free_2_dimension_list(&sh_num, &folder_list);
		return 0;
	}
	
	result = 0;
	for (i = 0; i < sh_num; ++i)
		if (!upper_strcmp(folder, folder_list[i])) {
			result = 1;
			break;
		}
	free_2_dimension_list(&sh_num, &folder_list);
	
	return result;
}

// for FTP: root dir is POOL_MOUNT_ROOT.
extern int how_many_layer(const char *basedir, char **mount_path, char **share) {
	char *follow_info, *follow_info_end;
	int layer = 0, len = 0;
	
	*mount_path = NULL;
	*share = NULL;
	
	if (!strcmp(basedir, "/"))
		return layer;
	
	len = strlen(basedir);
	if (len > 1)
		layer = 1;
	
	//if (basedir[len-1] == '/')
	//	--layer;
	
	follow_info = (char *)basedir;
	while (*follow_info != 0 && (follow_info = index(follow_info+1, '/')) != NULL)
		++layer;
	
	if (layer >= MOUNT_LAYER) {
		follow_info = (char *)(basedir+strlen(POOL_MOUNT_ROOT));
		follow_info = index(follow_info+1, '/');
		
		if(mount_path != NULL){
			if (follow_info == NULL)
				len = strlen(basedir);
			else
				len = strlen(basedir)-strlen(follow_info);
			*mount_path = (char *)malloc(sizeof(char)*(len+1));
			if (*mount_path == NULL)
				return -1;
			strncpy(*mount_path, basedir, len);
			(*mount_path)[len] = 0;
		}
	}
	
	if (layer >= SHARE_LAYER && share != NULL) {
		++follow_info;
		follow_info_end = index(follow_info, '/');
		if (follow_info_end == NULL)
			len = strlen(follow_info);
		else
			len = strlen(follow_info)-strlen(follow_info_end);
		*share = (char *)malloc(sizeof(char)*(len+1));
		if (*share == NULL)
			return -1;
		strncpy(*share, follow_info, len);
		(*share)[len] = 0;
	}
	
	return layer;
}
