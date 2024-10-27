#!/bin/sh

args="-d"
case "$1" in
	start)
		echo "Starting aesdsocket server"
		start-stop-daemon -S -n aesdsocket -a /usr/bin/aesdsocket -- $args
		;;
	stop)
		echo "Stopping aesdsocket server"
		pkill -SIGTERM aesdsocket
		start-stop-daemon -K -n aesdsocket
		;;
	*)
		echo "Usage: $0 {start|stop}"
	exit 1
esac

exit 0
