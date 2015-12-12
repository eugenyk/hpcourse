#!/bin/bash
set -e
#protobuf
rm -rf protobuf
git clone https://github.com/google/protobuf.git
cd protobuf
git checkout v2.6.1 -b v2.6.1
./autogen.sh
mkdir -p install_out && ./configure --prefix=$PWD/install_out
make
make check
make install
cd ..

#boost
rm -rf boost
wget http://sourceforge.net/projects/boost/files/boost/1.59.0/boost_1_59_0.tar.bz2
tar -xf boost_1_59_0.tar.bz2
rm -f boost_1_59_0.tar.bz2
mv boost_1_59_0 boost
cd boost
./bootstrap.sh --with-libraries=system,thread,date_time,regex,serialization --prefix=$PWD/install_out toolset=gcc cflags="-Ofast -mtune=native"
./b2 install
cd ..
