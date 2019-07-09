all: modengine

modengine: build
	@(cd ./build && cmake .. && make all)
	@echo modengine output to ./build/modengine
build:
	@mkdir -p ./build
clean:
	@rm -r ./build
