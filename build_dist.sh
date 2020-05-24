#!/bin/bash

ctime -begin stella_linux.ctm

docker build -t stella_build -f Dockerfile .

CONTAINER_NAME="stella_build-$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1)"
docker run --name $CONTAINER_NAME stella_build /root/build.sh

CONTAINER_ID=$(docker ps -aqf "name=$CONTAINER_NAME")

rm -f build/stella
docker cp ${CONTAINER_ID}:/root/build/stella build/stella

docker container rm ${CONTAINER_ID}

ctime -end stella_linux.ctm