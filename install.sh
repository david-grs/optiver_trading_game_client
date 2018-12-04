#!/bin/bash 

if [ "$#" -ne 2 ]; then
  echo "usage: $0 username remote_address"
  exit 1
fi

if [ "$1" = "YOUR_USERNAME" ]; then
  echo "username should be different than YOUR_USERNAME"
  exit 1
fi

local_eth0_addr=$(ip addr show eth0 | grep "inet\b" | awk '{print $2}' | cut -d/ -f1)
sed -i s/LOCAL_ADDRESS/$local_eth0_addr/g src/autotrader.h

username=$1
sed -i s/USERNAME/$username/ src/execution_client.cc

remote_addr=$2
sed -i s/REMOTE_ADDRESS/$remote_addr/g src/autotrader.h

sudo apt update
sudo apt -y install g++ gdb cmake libboost-all-dev

mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j2 && git commit -am "configuration" && echo -e "*****************************\n      SETUP COMPLETE\n*****************************\nLOCAL_ADDRESS = $local_eth0_addr"

