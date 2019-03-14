#!/bin/bash
#Program
# test-1652195 start stop restart

PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin
export PATH

prog="test-1652195"
Test_Bin="/usr/sbin"
RETVAL=0

start()
{
	echo "Starting $prog..."
	cd /usr/sbin
	./test-1652195
	RETVAL=$?
	return $RETVAL
}
stop()
{
	echo "Stopping $prog..."
	pkill test-1652195
	RETVAL=$?
	return $RETVAL
}
restart()
{
	echo "Restarting $prog..."
	stop
	start
}

case "$1" in
	start)
		start
		;;
	stop)
		stop
		;;
	restart)
		restart;;
	*)
		echo $"Usage:$0{start|stop|restart}"
		RETVAL=1
esac
exit $RETVAL
