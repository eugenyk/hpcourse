OS := $(shell uname)
OPTIONS:= 

ifeq ($(OS),Darwin)
	OPTIONS += -framework OpenCL
else
	OPTIONS += -l OpenCL
endif

main: main.c
	gcc main.c -o main -O2 /usr/lib/x86_64-linux-gnu/libOpenCL.so.1

clean:
	rm -rf main
