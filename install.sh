#!/bin/bash -x

sudo apt update
sudo apt -y install g++ gdb cmake libboost-all-dev

local_eth0_addr=$(ip addr show eth0 | grep "inet\b" | awk '{print $2}' | cut -d/ -f1)
sed -i s/LOCAL_ADDRESS/$local_eth0_addr/g src/autotrader.h

mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j2

