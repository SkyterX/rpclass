#!/bin/bash

sudo docker build -t 'lastg/graph-test' .
sudo docker build -t 'lastg/rpclass-jenkins-slave' - < jenkins-slave.docker
sudo docker build -t 'lastg/rpclass-teamcity-slave' - < teamcity-slave.docker

