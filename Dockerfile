FROM debian:buster-slim

WORKDIR /root

RUN apt-get update && \
    apt-get -y install g++ libgl1-mesa-dev

ADD . /root/