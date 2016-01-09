path_to_compiler=g++

mkdir -p obj
objs=
for source in *.cpp
do
	$path_to_compiler -Wall -std=c++11 -fexceptions -O2 -c "$source" -o "obj/${source%.*}.o" 
	objs="$objs obj/${source%.*}.o"
done
mkdir -p bin
$path_to_compiler -o "bin/sda_2_cpp" $objs -s -lpthread
