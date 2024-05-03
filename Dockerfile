# syntax=docker/dockerfile:1

FROM ubuntu:24.04
WORKDIR /nimlib
COPY . .
RUN mkdir "build_d"
RUN apt-get update && apt-get install -y build-essential git cmake python3
WORKDIR /opt
RUN git clone -b 3.3.0 https://github.com/randombit/botan.git
WORKDIR /opt/botan
RUN python3 configure.py
RUN make && make install
WORKDIR /nimlib/build_d
RUN cmake ..
RUN make
