#!/bin/sh

if [ $# -lt 2 ]
then 
	echo "$0 start|stop|reload server_id"
	exit
fi

server_id=$2

function start_proc()
{
	ProcessName=$1
	
	$ProcessName $3 $server_id & > /dev/null 2>&1
	sleep $2
	ProcessInfo=$(ps afx| grep $ProcessName| grep -v grep | grep -v Script | grep -v log)
	if [ -z "$ProcessInfo" ]
	then
		echo "start $ProcessName failed."
		
	fi
	
	echo "start $ProcessName success." 
}

function stop_proc()
{
	ProcessName=$1
	echo "stop $ProcessName ..."

	PIDS=`ps -ef | grep $ProcessName |egrep "\b$server_id\b" | grep -v grep | awk '{print $2}'`
	echo "Notify process ($PIDS) to stop service..."
	for PID in $PIDS
	do
		kill -USR2 $PID
		sleep 1
		kill -9 $PID
	done
	

	ProcessInfo=$(ps afx| grep $ProcessName| grep -v grep | grep -v Script| grep -v log)
	if [ -n "$ProcessInfo" ]
	then
		echo "stop $ProcessName failed."
		return
	fi
	echo "done"
}

function kick_proc()
{
	ProcessName=$1
	echo "kick $ProcessName ..."

	PIDS=`ps -ef | grep $ProcessName |grep $server_id | grep -v grep | awk '{print $2}'`
	echo "Notify process ($PIDS) to kick service..."
	for PID in $PIDS
	do
		kill -USR2 $PID
		sleep 1
	done
	

	ProcessInfo=$(ps afx| grep $ProcessName| grep -v grep | grep -v Script| grep -v log)
	if [ -n "$ProcessInfo" ]
	then
		echo "kick $ProcessName."
		return
	fi
	echo "done"
}

if [ $1 == "start" ]
then
	ulimit -c unlimited
	ulimit -n 65535
	ulimit -s 20480
	
	start_proc ./InnerServer 2
	start_proc ./OuterServer 2
	#valgrind --tool=memcheck --leak-check=full  --trace-children=yes --show-reachable=yes --error-limit=no --log-file=./Roledump ./RoleServer/RoleServer &
	#sleep 30
	#echo "start RoleServer success." 
	#valgrind --tool=memcheck --leak-check=full  --trace-children=yes --show-reachable=yes --error-limit=no --log-file=./Gamedump ./GameServer/GameServer 1 &
	#sleep 2
	#echo "start GameServer success." 
	start_proc ./RoleServer 70
	start_proc ./GameServer 5 1
	start_proc ./ChatServer 2
	start_proc ./TaskServer 2
	start_proc ./GateServer 2 1
	start_proc ./GateServer 2 2
	start_proc ./GateServer 2 3
	start_proc ./GateServer 2 4
	start_proc ./GateServer 2 5
	start_proc ./GateServer 2 6
	start_proc ./GateServer 2 7
	start_proc ./GateServer 2 8
	start_proc ./GateServer 2 9
	start_proc ./LoginServer 2 
	start_proc ./LoginGate 2 
	
	echo "done."
	
elif [ $1 == "stop" ]
then
  stop_proc LoginGate
	stop_proc LoginServer
	
	kick_proc GateServer
	sleep 30
	
	stop_proc GateServer
	stop_proc TaskServer
	stop_proc ChatServer
	stop_proc GameServer
	stop_proc RoleServer
	stop_proc InnerServer
	stop_proc OuterServer
	
elif [ $1 == "reload" ]
then
	echo "reload $ProcessName ..."
	if [ -z "$ProcessInfo" ]
	then
		echo "$ProcessName is up now ,please stop first."
		exit
	fi

	PIDS=`ps -ef | grep LotusSvrd | grep -v grep | awk '{print $2}'`
	echo "Notify process ($PIDS) to refresh config..."
	for PID in $PIDS
	do
		kill -s USR1 $PID
	done

	echo "done"
elif [ $1 == "check" ]
then
	for server in LoginGate LoginServer "GateServer 1" "GateServer 2" "GateServer 3" "GateServer 4" "GateServer 5" "GateServer 6" "GateServer 7" "GateServer 8" "GateServer 9" TaskServer ChatServer GameServer RoleServer InnerServer OuterServer
	do
		pid=`ps aux | grep "$server" | egrep "\b$server_id\b" | grep -v grep | awk '{print $2}'`
		#echo "$server:$pid"
		if [ -z $pid ];then
			echo "$server status: down"
		else
			echo "$server status: alive"
		fi
	done
elif [ $1 == "init" ]
then
	echo "initializing... "
	echo 1805306368 > /proc/sys/kernel/shmmax
	mkdir -p ../key
	chmod go+w ../key -R
	echo "done"
else
	echo "$0 start|stop|reload|check|init"
fi
