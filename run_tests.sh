#!/bin/bash

GRAPHS_FOLDER="/var/data/"

for tst in build/bin/*-{unit,other}
do
    if [[ -e "$tst" ]];
    then
        echo Running $tst
        "$tst" --gtest_output=xml:${tst}_details.xml $GRAPHS_FOLDER
    fi
done
