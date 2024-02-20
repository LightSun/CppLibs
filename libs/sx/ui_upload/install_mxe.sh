
sudo apt-get install autoconf automake autopoint bash bison bzip2 flex gettext
sudo apt-get install git g++ gperf intltool libffi-dev libgdk-pixbuf2.0-dev
sudo apt-get install libtool lzip libltdl-dev libssl-dev libxml-parser-perl
sudo apt-get install openssl p7zip-full patch perl pkg-config python ruby scons
sudo apt-get install sed unzip wget xz-utils g++-multilib libc6-dev-i386 libtool-bin

git clone https://github.com/mxe/mxe.git
make MXE_TARGETS='x86_64-w64-mingw32.static i686-w64-mingw32.static' qt5

