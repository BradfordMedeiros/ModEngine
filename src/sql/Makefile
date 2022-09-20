SOURCES := $(wildcard ./*.cpp)

all: binary

binary: plugin.cpp
	@g++ -D BINARY_MODE -ggdb -I /usr/include/guile/2.2/ $(SOURCES) -std=c++17 -lstdc++fs -o plugin
clean: 
	rm -f plugin.so plugin.o plugin

