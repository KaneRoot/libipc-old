#!/bin/dash

REP=/tmp/ipc/
SERVICE="tcpd"
NB=10

if [ $# -ne 0 ]
then
    NB=$1
fi

for pid in `seq 1 ${NB}`
do
    ./tcpdtest.bin

done
