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

LIBVPX_SO := $(shell ldconfig -p | grep -E '\blibvpx\.so\b' | awk '{print $$NF}' | head -n1)

build: ./lib/ffmpeg_n4.4.1
	@mkdir -p ./build
	@mkdir -p ./build/screenshots
	@mkdir -p ./build/runtime_libs
	@cp -L ./lib/ffmpeg_n4.4.1/build/lib/libavcodec.so.58 ./build/runtime_libs
	@cp -L ./lib/ffmpeg_n4.4.1/build/lib/libavformat.so.58 ./build/runtime_libs
	@cp -L ./lib/ffmpeg_n4.4.1/build/lib/libavutil.so.56 ./build/runtime_libs
	@cp -L ./lib/ffmpeg_n4.4.1/build/lib/libswresample.so.3 ./build/runtime_libs
	@cp -L ./lib/ffmpeg_n4.4.1/build/lib/libswscale.so.5 ./build/runtime_libs
	@cp -L $(LIBVPX_SO) ./build/runtime_libs/

clean:
	@rm -r ./build

