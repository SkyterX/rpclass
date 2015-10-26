#!/bin/bash

sudo docker build -t 'lastg/graph-test' .
sudo docker build -t 'lastg/rpclass-jenkins-slave' - < jenkins-slave.docker
