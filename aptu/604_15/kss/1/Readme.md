Async chat with boost asio
===========================

## Dependencies:
###### Single command for dependencies installation for ubuntu: `sudo apt-get install curl autoconf libtool gcc g++ make wget tar`
###### All dependencies
- `curl`
- `autoconf`
- `libtool`
- `gcc`
- `g++`
- `make`
- `wget`
- `tar`

## Build:
1. `./download_and_build_dependencies.sh` - download and build boost and protobuf libs (it will take some time)
2. `export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PWD/protobuf/install_out/lib" && ./protobuf/install_out/bin/protoc -I=. --cpp_out=./src ./message.proto` - create message class from `message.proto`
3. `make` - build chat server executable

## Run:
1. `./chat_server $PORT$ $WORKERS_COUNT$`

