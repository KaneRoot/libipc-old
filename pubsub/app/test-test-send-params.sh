#!/bin/bash

# start pubsub-test-send alone, with some parameters
# then test it with this script

REP=/tmp/ipc/
PID=10
INDEX=1
VERSION=1
ACTION="pub"
CHAN="chan1"

if [ $# != 0 ] ; then
    PID=$1
    shift
fi

if [ $# != 0 ] ; then
    INDEX=$1
    shift
fi

if [ $# != 0 ] ; then
    ACTION=$1
    shift
fi

if [ $# != 0 ] ; then
    CHAN=$1
    shift
fi

echo "there should be a line in $REP/pubsub"
cat $REP/pubsub

echo ""
echo "there should be something to read in $REP/${PID}-${INDEX}-out"
cat $REP/${PID}-${INDEX}-out | xxd
