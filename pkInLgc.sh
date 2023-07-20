#!/bin/bash
echo "installation de base "
sudo apt update sudo apt && sudo apt upgrade 
sudo apt install nfs-kernel-server
sudo install openssh-server
sudo apt install ifconfig
sudo apt install python3
sudo install scons


echo installation g++
sudo apt install g++

echo installation gdb
sudo apt install gdb

echo installation protobuf
echo "***********************************"
sudo apt install -y protobuf-compiler

echo "************************************************************************************"
sudo apt install libcrypto++8 libcrypto
sudo apt install libcrypto++8 libcrypto++-dev libcrypto++8-dbg
sudo apt install libcrypto++8 libcrypto++-dev libcrypto++dbg
sudo apt install libcrypto++8 libcrypto++-dev


echo "configuration nfs in etc/fstab"