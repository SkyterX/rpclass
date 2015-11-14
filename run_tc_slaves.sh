#!/bin/bash

set -eu

BASENAME='tc-slave'
NUM_OF_SLAVES=3

for i in `seq 1 ${NUM_OF_SLAVES}`
do
    NAME=${BASENAME}-${i}
    CID=`sudo docker run -d --name "$NAME" -h "$NAME" -v /var/data:/var/data lastg/rpclass-teamcity-slave`
    docker inspect ${CID} | python -c "import sys,json; print(json.load(sys.stdin)[0]['NetworkSettings']['IPAddress'])"

done

