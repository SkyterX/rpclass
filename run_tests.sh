#!/bin/bash

GRAPHS_FOLDER="/var/data/"

for tst in build/bin/*-test
do
    echo Running $tst
    $tst --gtest_output=xml:${tst}_details.xml $GRAPHS_FOLDER
done
