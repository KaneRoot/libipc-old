#!/bin/dash

REP=/tmp/ipc/
SERVICE="pingpong"
NB=3

# CLEAN UP !
if [ $# -ne 0 ] && [ "$1" = clean ]
then
    echo "clean rep ${REP}"
    rm ${REP}/${SERVICE}
    rm ${REP}/*-in
    rm ${REP}/*-out

    exit 0
fi

if [ $# -ne 0 ]
then
    NB=$1
fi

for pid in `seq 1 ${NB}`
do
    # we make the application pipes
    mkfifo ${REP}/${pid}-1-in 2>/dev/null
    mkfifo ${REP}/${pid}-1-out 2>/dev/null

    # pid index version
    echo "${pid} 1 1" > ${REP}/${SERVICE}

    # the purpose is to send something in the pipe
    cat /dev/urandom | base64 | head -n 1 > ${REP}/${pid}-1-out

    # the the service will answer with our message
    cat ${REP}/${pid}-1-in
done
