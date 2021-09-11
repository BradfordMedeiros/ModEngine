all: modengine
release: modengine_release

assimp:
	@git submodule update --init --recursive

modengine: build resourcefiles
	@(cd ./build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make all)
	@echo modengine output to ./build/modengine

modengine_release: build resourcefiles
	@(cd ./build && cmake .. -DCMAKE_BUILD_TYPE=Release  && make all)
	@echo modengine release output to ./build/modengine          

resourcefiles: build #assimp  		# This should probably be in CMakeLists.txt but is doesn't seem to play nice
	@rm -rf ./build/res
	@cp -r ./res ./build/
build:
	@mkdir -p ./build
	@mkdir -p ./build/screenshots
clean:
	@rm -r ./build
