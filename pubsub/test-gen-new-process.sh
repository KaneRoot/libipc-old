#!/bin/bash

REP=/tmp/ipc
SERVICE=gen
#SERVICE=pubsub
NB=10

if [ $# -ge 1 ] ; then
    SERVICE=$1
    shift
fi

if [ $# -ge 1 ] ; then
    NB=$1
    shift
fi

for i in $(seq 1 ${NB})
do
    mkfifo ${REP}/${i}-1-1-in
    mkfifo ${REP}/${i}-1-1-out

    echo "${i} 1 1 both chan1" > ${REP}/${SERVICE}
    sleep 0.1
done
