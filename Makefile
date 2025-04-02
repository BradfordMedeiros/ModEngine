all: modengine
release: modengine_release

assimp:
	@git submodule update --init --recursive

buildtest: build
	@echo "test project to evaluating (to evaluate dependencies)"
	@(cd ./build && cmake -DBUILDTEST=True -DCMAKE_BUILD_TYPE=Debug .. && make all)	

modengine: build
	@echo "making modengine debug"
	@(cd ./build && cmake -DCMAKE_BUILD_TYPE=Debug -DADDITIONAL_SRC=$(additional_src) .. && make -j12 all)
	@echo modengine output to ./build/modengine

modengine_release: build
	@echo "making modengine release"
	@(cd ./build && cmake -DCMAKE_BUILD_TYPE=Release -DADDITIONAL_SRC=$(additional_src) .. && make -j12 all)
	@echo modengine release output to ./build/modengine          

build:
	@mkdir -p ./build
	@mkdir -p ./build/screenshots
clean:
	@rm -r ./build

