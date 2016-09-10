#!/bin/bash

for i in $(seq 1 10)
do
    echo "${i} 1 1 pub chan1" > /tmp/ipc/gen
    sleep 0.1
done
