#!/bin/bash

for tst in build/bin/*-test
do
    echo Running $tst
    $tst --gtest_output=xml:${tst}_details.xml
done
