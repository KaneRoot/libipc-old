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

    # we make the application pipes
    mkfifo ${REP}${pid}-1-1-in 2>/dev/null
    mkfifo ${REP}${pid}-1-1-out 2>/dev/null

    echo "connect 127.0.0.1 6000 ${pid} 1 1" > ${REP}${SERVICE}

    # the purpose is to send something in the pipe
    cat /dev/urandom | base64 | head -n 1 > ${REP}${pid}-1-1-out
    # echo "hello world" > ${REP}${pid}-1-out

    sleep 2
    # the the service will answer with our message
    echo "pid : ${pid}"
    cat ${REP}/${pid}-1-1-in

    echo "exit" > ${REP}${pid}-1-1-out

done
