#!/bin/bash

REP=/tmp/ipc
SERVICE=gen
#SERVICE=pubsub
NB=10

for i in $(seq 1 ${NB})
do
    mkfifo ${REP}/${i}-1-1-in
    mkfifo ${REP}/${i}-1-1-out

    echo "${i} 1 1 both chan1" > ${REP}/${SERVICE}
    sleep 0.1
done
