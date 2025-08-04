installed glfw and glad manually.

glfw from:
https://www.glfw.org/download.html
which should be same as https://github.com/glfw/glfw (forked to github).

glad from:
https://glad.dav1d.de/
v 4.6
core profile
default extensions

glm from:
https://github.com/g-truc/glm

stb from:
https://github.com/nothings/stb/blob/master/stb_image.h 

freetype from:
https://download.savannah.gnu.org/releases/freetype/

guile:
sudo apt-get install guile-2.2-dev

openal install from native package manager, should embed in cmake?
sudo apt-get install libalut-dev
sudo apt-get install libopenal-dev

sudo apt-get install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libswresample-dev


for vorbis file format support
sudo apt install libvorbis-dev

libswresample-dev
===============
dev tools:
cmake 
standard gnu stuff (make, bash utils etc)


# webm...might get rid of this, probably should move into submodule too i guess make?
git clone https://github.com/webmproject/libwebm.git
cd libwebm
cd libwebm/build
cmake ..
make
make install



### Building ffmpeg
Vendored in the dependencies, this is how i built it. 
headers: build/include  , .so files : build/lib


ffmpeg 
./configure \
  --prefix="$PWD/build" \
  --disable-static \
  --enable-shared \
  --disable-everything \
  --enable-avformat \
  --enable-avcodec \
  --enable-avutil \
  --enable-protocol=file \
  --enable-decoder=vp8 \
  --enable-decoder=vp9 \
  --enable-decoder=libvpx_vp8 \
  --enable-decoder=libvpx_vp9 \
  --enable-decoder=opus \
  --enable-decoder=vorbis \
  --enable-demuxer=matroska \
  --enable-parser=vp8 \
  --enable-parser=vp9 \
  --enable-libvpx \
  --enable-libopus \
  --enable-libvorbis \
  --disable-network \
  --disable-doc \
  --disable-debug \
  --disable-autodetect

make -j$(nproc)
make install


for curl
sudo apt install libcurl4-openssl-dev
