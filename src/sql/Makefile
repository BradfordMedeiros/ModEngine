SOURCES := $(wildcard ./*.cpp)

all: plugin binary

plugin: plugin.cpp
	@g++ -fPIC -shared -I /usr/include/guile/2.2/ $(SOURCES) -std=c++17 -lstdc++fs -lguile-2.2  -o plugin.so

binary: plugin.cpp
	@g++ -D BINARY_MODE -ggdb -I /usr/include/guile/2.2/ $(SOURCES) -std=c++17 -lstdc++fs -lguile-2.2 -o plugin
clean: 
	rm -f plugin.so plugin.o plugin

