## Build

You have to provide or set environment variables:
* TBB_LIBRARY -- path to dir where 'lib' tbb dir resides
* TBB_INCLUDE_DIR -- path to directory with tbb include files

```bash
cd <project root>
mkdir build && cd build
cmake -DTBB_INCLUDE_DIR=/home/eg/thirdparty/tbb2017_20161128oss/include/ -DTBB_LIBRARY=/home/eg/thirdparty/tbb2017_20161128oss ../
make
```

## USE

```bash
./gea_1 --help
./gea_1 -b 0 -l 100
./gea_1 -b 234 -l 100 -f log.txt
```
