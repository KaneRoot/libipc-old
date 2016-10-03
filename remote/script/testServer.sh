#!/bin/dash

REP=/tmp/ipc/
SERVICE="tcpd"

echo "listen 127.0.0.1 6000" > ${REP}${SERVICE}
