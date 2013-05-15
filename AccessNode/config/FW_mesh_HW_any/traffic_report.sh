#!/bin/sh
# If run without parameters, it adds a delta line to /tmp/trafic_report.txt
# $ ./traffic_report.sh    =>  1127380269 T 292 309
#
# If run with 2 parameters, it adds them to wan_trafic_report.txt
# $ ./traffic_report.sh P orange_ro => 1127380276 P orange_ro
# $ ./traffic_report.sh O 82.76.12.100:20500 => 1127380310 O 82.76.12.100:20500
#
# If /tmp/trafic_report.txt is bigger than 7K, send it and delete it

REPORT_FILE=$NIVIS_TMP/traffic_report.txt
REPORT_PREV=$NIVIS_TMP/traffic_report.prev
REPORT_TEMP=$NIVIS_TMP/traffic_report.tmp
PROXY_FILE=$NIVIS_TMP/traffic_report.proxy
PROVIDER_FILE=$NIVIS_TMP/traffic_report.provider
TIMESTAMP_FILE=$NIVIS_TMP/traffic_report.timestamp

# send file if bigger than 7K or if not reported in the last hour
file_stat=`[ -f  $REPORT_FILE ] && ls -l $REPORT_FILE`
file_size=`echo $file_stat | cut -f 5 -d' '`
NOW=`date +%s`
[ ! -s $REPORT_TEMP ] && echo $NOW > $REPORT_TEMP
LAST=`cat $REPORT_TEMP`
echo "<T>$NOW</T>" > $TIMESTAMP_FILE

if let $(( NOW-LAST>3600 || ${file_size:=0}>7000 )); then
	mv $REPORT_FILE $REPORT_FILE.sending
	echo $NOW > $REPORT_TEMP
	[ -s $REPORT_FILE.sending ] && 
		(echo "<root>`cat $REPORT_FILE.sending`</root>" > $REPORT_FILE.sending2
		raise_an_ev -e 36 -c 3 -v $REPORT_FILE.sending2
		rm $REPORT_FILE.sending
		) &
fi

# P or O type records:
if [ $# -ge 2 ]; then
	if [ $1 = "O" ]; then
		IP=`echo $2 | cut -f 1 -d':'`
		P=`echo $2 | cut -f 2 -d':'`
		echo "<IP>$IP</IP><P>$P</P>" > $PROXY_FILE
	fi
	if [ $1 = "P" ]; then
		echo "<Pr>$2</Pr>" > $PROVIDER_FILE
	fi
	if [ -s $PROVIDER_FILE ] && [ -s $PROXY_FILE ]; then
		R="<R>`cat $TIMESTAMP_FILE $PROVIDER_FILE $PROXY_FILE`</R>"
		echo $R >> $REPORT_FILE
	fi
	exit
fi

# T type record:
#(only if we have the P and O)
[ ! -s $PROVIDER_FILE ] && exit
[ ! -s $PROXY_FILE ] && exit
PPPSTATS=`grep ppp /proc/net/dev`
ETHSTATS=`grep eth /proc/net/dev`
STATS=${PPPSTATS:-$ETHSTATS}
STATS=`echo $STATS | cut -f 2 -d':'`
DEV=ppp
[ -z "$PPPSTATS" ] && DEV=eth

IN=`echo $STATS | cut -f 1 -d' '`
OUT=`echo $STATS | cut -f 9 -d' '`
#echo "Crt: $IN $OUT"

OLD_STATS=`grep $DEV $REPORT_PREV | cut -f 2 -d':'`
OLD_IN=`echo $OLD_STATS | cut -f 1 -d' '`
OLD_OUT=`echo $OLD_STATS | cut -f 9 -d' '`
#echo "Prev: $OLD_IN $OLD_OUT"

DELTA_IN=$((IN-OLD_IN))
DELTA_OUT=$((OUT-OLD_OUT))
[ $DELTA_IN -lt 0 ] && DELTA_IN=$IN
[ $DELTA_OUT -lt 0 ] && DELTA_OUT=$OUT
DELTA_IN="<I>$DELTA_IN</I>"
DELTA_OUT="<O>$DELTA_OUT</O>"
#echo Delta: $DELTA_IN $DELTA_OUT

echo "$PPPSTATS $ETHSTATS" > $REPORT_PREV
R="<R>`cat $TIMESTAMP_FILE $PROVIDER_FILE $PROXY_FILE` $DELTA_IN $DELTA_OUT</R>"
echo $R >> $REPORT_FILE
