#!/bin/bash 

if [ "$#" -ne 3 ]; then
  echo "usage: $0 username remote_address id"
  exit 1
fi

if [ "$1" = "YOUR_USERNAME" ]; then
  echo "username should be different than YOUR_USERNAME"
  exit 1
fi

local_eth0_addr=$(ip addr show eth0 | grep "inet\b" | awk '{print $2}' | cut -d/ -f1)
sed -i s/LOCAL_ADDRESS/$local_eth0_addr/g src/autotrader.h

username=$1
sed -i "s/std::string Username = \"USERNAME\"/std::string Username = \"$username\"/" src/execution_client.cc

remote_addr=$2
sed -i s/REMOTE_ADDRESS/$remote_addr/g src/autotrader.h

local_info_port=`expr 7000 + $3`
sed -i s/LOCAL_INFO_PORT/$local_info_port/g src/autotrader.h

local_exec_port=`expr 8000 + $3`
sed -i s/LOCAL_EXEC_PORT/$local_exec_port/g src/autotrader.h

sudo apt update
sudo apt -y install g++ gdb cmake libboost-all-dev

mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j2 && git commit -am "configuration" && echo -e "*****************************\n      SETUP COMPLETE\n*****************************\nLOCAL_ADDRESS = $local_eth0_addr"

