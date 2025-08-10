FROM ubuntu:24.04

LABEL org.opencontainers.image.authors=leleliu008@gmail.com

ADD ndk-pkg /usr/bin/

RUN apt-get -y update && \
    apt-get -y install curl && \
    apt-get clean && \
    rm -fr /var/lib/apt/lists/* /tmp/* /var/tmp/*

CMD [ "/bin/sh" ]
