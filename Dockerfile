FROM ubuntu:22.04

MAINTAINER leleliu008@gmail.com

ADD ndk-pkg /usr/bin/

RUN apt-get -y update && \
    apt-get -y install build-essential pkg-config curl libssl-dev zlib1g-dev && \
    apt-get clean && \
    rm -fr /var/lib/apt/lists/* /tmp/* /var/tmp/*

ENTRYPOINT [ "/bin/bash", "-l" ]
