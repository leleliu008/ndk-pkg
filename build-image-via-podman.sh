#!/bin/sh

podman build --platform linux/amd64 -t fpliu/ndk-pkg -f Dockerfile
