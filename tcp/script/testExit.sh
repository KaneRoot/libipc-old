#!/bin/dash

REP=/tmp/ipc/
SERVICE="tcpd"

# pid index version
echo "exit" | nc -U ${REP}${SERVICE}

