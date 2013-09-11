#!/bin/sh
# $1: filesystem type, $2: device path.


ret_dir=/tmp/fsck_ret

# $1: device path, $2: error code.
_set_fsck_code(){
	pool_name=`echo "$1" |awk '{FS="/"; print $3}'`

	if [ ! -d "$ret_dir" ]; then
		rm -rf $ret_dir
		mkdir $ret_dir
	fi

	rm -f $ret_dir/$pool_name.[0-2]
	touch "$ret_dir/$pool_name.$2"
}

# $1: device path.
_get_fsck_logfile(){
	pool_name=`echo "$1" |awk '{FS="/"; print $3}'`

	if [ ! -d "$ret_dir" ]; then
		rm -rf $ret_dir
		mkdir $ret_dir
	fi

	echo "$ret_dir/$pool_name.log"
}

if [ -z "$1" ] || [ -z "$2" ]; then
	echo "Usage: app_fsck.sh [filesystem type] [device's path]"
	exit 0
fi

autocheck_option=
autofix_option=
autofix=`nvram get apps_state_autofix`

log_file=`_get_fsck_logfile $2`
log_option="> $log_file 2>&1"

if [ "$autofix" == "1" ]; then
	if [ "$1" == "ntfs" ] || [ "$1" == "ufsd" ]; then
		autocheck_option="-a"
		autofix_option="-f"
	else
		autocheck_option=
		autofix_option=p
	fi
else
	if [ "$1" == "ntfs" ] || [ "$1" == "ufsd" ]; then
		autocheck_option="-a"
		autofix_option=
	else
		autocheck_option=n
		autofix_option=
	fi
fi

set -o pipefail
_set_fsck_code $2 2
if [ "$1" == "ntfs" ] || [ "$1" == "ufsd" ]; then
	c=0
	RET=1
	while [ ${c} -lt 4 -a ${RET} -ne 0 ] ; do
		c=$((${c} + 1))
		eval chkntfs $autocheck_option $autofix_option --verbose $2 $log_option
		RET=$?
		if [ ${RET} -ge 251 -a ${RET} -le 254 ] ; then
			break;
		fi
	done
else
	eval fsck.$1 -$autocheck_option$autofix_option -v $2 $log_option
	RET=$?
fi

if [ "${RET}" != "0" ]; then
	_set_fsck_code $2 1
else
	_set_fsck_code $2 0
fi
