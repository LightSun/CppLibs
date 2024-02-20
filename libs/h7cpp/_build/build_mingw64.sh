
export OPEN_SSL_DIR=/home/heaven7/heaven7/env/win64/mingw-build/openssl
export LIB_HV_DIR=/home/heaven7/heaven7/env/win64/mingw-build/libhv-1.2.6
export UPLOAD_UI_DIR=/home/heaven7/heaven7/env/win64/mingw-build/ui-upload
cmake .. \
	-DBUILD_MINGW64=ON \
	-DCMAKE_BUILD_TYPE=MinSizeRel 
#make -j8
