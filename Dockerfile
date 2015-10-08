FROM library/ubuntu:15.04

RUN apt-get update &&\
 apt-get install -yy --force-yes build-essential libboost-dev cmake

ENV CODE_DIR '/src'
ENV BUILD_DIR '/build'

WORKDIR $CODE_DIR
VOLUME ["$CODE_DIR", "${BUILD_DIR}"]

CMD mkdir -p ${BUILD_DIR} &&\
  cd ${BUILD_DIR} &&\
  cmake ${CODE_DIR} &&\
  make &&\
  cd bin && ./graph-test
