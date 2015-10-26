#!/bin/bash

set -eu

BASENAME='slave'
NUM_OF_SLAVES=10

for i in `seq 1 ${NUM_OF_SLAVES}`
do
    NAME=${BASENAME}-${i}
    CID=`sudo docker run -d --name "$NAME" -h "$NAME" lastg/rpclass-jenkins-slave`
    docker inspect ${CID} | python -c "import sys,json; print(json.load(sys.stdin)[0]['NetworkSettings']['IPAddress'])"

done


#sudo docker run --rm -i -t -v `pwd`:/src lastg/graph-test
