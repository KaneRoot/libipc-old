#!/bin/dash

REP=/tmp/ipc/
SERVICE="pongd"
NB=10

# CLEAN UP !
if [ $# -ne 0 ] && [ "$1" = clean ]
then
    echo "clean rep ${REP}"
    rm ${REP}/${SERVICE} 2>/dev/null
    rm ${REP}/*-in 2>/dev/null
    rm ${REP}/*-out 2>/dev/null

    exit 0
fi

if [ $# -ne 0 ]
then
    NB=$1
fi

for pid in `seq 1 ${NB}`
do
    # we make the application pipes
    mkfifo ${REP}${pid}-1-5-in 2>/dev/null
    mkfifo ${REP}${pid}-1-5-out 2>/dev/null

    # pid index version
    echo "${pid} 1 5" > ${REP}${SERVICE}

    # the purpose is to send something in the pipe
    cat /dev/urandom | base64 | head -n 1 > ${REP}${pid}-1-5-out
    # echo "hello world" > ${REP}${pid}-1-5-out

    # the the service will answer with our message
    echo "pid : ${pid}"
    cat ${REP}${pid}-1-5-in
done

    echo "clean rep"
    rm ${REP}/*-in
    rm ${REP}/*-out