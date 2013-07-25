#!/bin/sh


wget_timeout=`nvram get apps_wget_timeout`
#wget_options="-nv -t 2 -T $wget_timeout --dns-timeout=120"
wget_options="-q -t 2 -T $wget_timeout"

nvram set apps_state_update=0 # INITIALIZING
#nvram set apps_state_error=0
APPS_PATH=/opt
CONF_FILE=$APPS_PATH/etc/ipkg.conf
TEMP_FILE=/tmp/ipkg.server.list

SERVER_LIST_FILES="Packages.gz Packages.zip"
SERVER_LIST_FILE=
TEMP_LIST_FILE=/tmp/Packages.gz
LIST_DIR=$APPS_PATH/lib/ipkg/lists

if [ ! -f "$CONF_FILE" ]; then
	echo "No conf file of ipkg!"
	exit 1
fi

link_internet=`nvram get link_internet`
if [ "$link_internet" != "1" ]; then
	exit 1
fi

grep -n '^src.*' $CONF_FILE |sort -r |awk '{print $2 " " $3}' > $TEMP_FILE
row_num=`wc -l < $TEMP_FILE`
if [ -z "$row_num" ]; then
	row_num=0
fi


nvram set apps_state_update=1 # UPDATING
i=0
while [ $i -lt $row_num ]; do
	i=$(($i+1))
	list_name=`sed -n $i'p' $TEMP_FILE |awk '{print $1}'`
	server_name=`sed -n $i'p' $TEMP_FILE |awk '{print $2}'`

	if [ "$list_name" == "optware.asus" ]; then
		server_name=http://dlcdnet.asus.com/pub/ASUS/LiveUpdate/Release/Wireless
	fi

	SERVER_LIST_FILE=
	for list in $SERVER_LIST_FILES; do
		echo "wget --spider $wget_options $server_name/$list"
		wget --spider $wget_options $server_name/$list
		if [ "$?" == "0" ]; then
			SERVER_LIST_FILE=$list
			break
		fi
	done

	if [ -z "$SERVER_LIST_FILE" ]; then
		continue
	fi

	echo "wget $wget_options -O $TEMP_LIST_FILE $server_name/$SERVER_LIST_FILE"
	wget $wget_options -O $TEMP_LIST_FILE $server_name/$SERVER_LIST_FILE
	if [ "$?" != "0" ]; then
		continue
	fi

	gunzip -c $TEMP_LIST_FILE > $LIST_DIR/$list_name

	rm -f $TEMP_LIST_FILE
done
rm -f $TEMP_FILE


nvram set apps_state_update=2 # FINISHED
