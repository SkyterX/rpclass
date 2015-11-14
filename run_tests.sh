#!/bin/bash

GRAPHS_FOLDER="/var/data/"
BASE_DIR=${1:-build}
echo  $BASE_DIR

for tst in ${BASE_DIR}/bin/*-{unit,other}
do
    if [[ -e "$tst" ]];
    then
        echo Running $tst
        "$tst" --gtest_output=xml:${tst}_details.xml $GRAPHS_FOLDER
    fi
done
