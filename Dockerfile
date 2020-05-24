#FROM alpine
FROM debian:buster-slim

WORKDIR /root

#RUN apk update && \
#    apk add bash mesa-dev libgcc musl-dev gcc g++
RUN apt-get update && \
    apt-get -y install g++ libgl1-mesa-dev

ADD . /root/

