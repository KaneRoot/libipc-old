#!/bin/dash

REP=/tmp/ipc/
SERVICE="tcpd"

# pid index version
echo "connect 127.0.0.1 6000 111111 1 1" | nc -U ${REP}${SERVICE}
sleep 1
echo "hello frero" | nc -U ${REP}111111-1-1


