#!/bin/dash

REP=/tmp/ipc/
SERVICE="tcpd"

# pid index version
echo "exit" > ${REP}${SERVICE}

