all: modengine

modengine: build resourcefiles
	@(cd ./build && cmake .. && make all)
	@echo modengine output to ./build/modengine

resourcefiles: build   		# This should probably be in CMakeLists.txt but is doesn't seem to play nice
	@rm -rf ./build/res
	@cp -r ./res ./build/
build:
	@mkdir -p ./build
clean:
	@rm -r ./build
