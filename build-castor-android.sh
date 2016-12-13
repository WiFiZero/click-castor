#!/bin/sh

if [ "x$ARCH" = 'x' ] ; then
    echo "You shouldn't use build-castor-android.sh directly, use build-castor-android-[arch].sh instead"
    exit 1
fi

CLICK_DIR=`pwd`
SODIUM_SRC_DIR=${CLICK_DIR}/libsodium
INSTALL_DIR=${SODIUM_SRC_DIR}/libsodium-android-${ARCH}

git submodule update --init
if [ ! -d ${INSTALL_DIR} ]; then
	echo "Installing libsodium in ${INSTALL_DIR}"
	cd ${SODIUM_SRC_DIR}
	if [ ! -e configure ]; then
		./autogen.sh
	fi
	git apply ../patch-libsodium-maketoolchain-force.diff
	export ANDROID_NDK_HOME=${NDK_ROOT}
	dist-build/android-${ARCH}.sh
fi

echo "Building Click with Castor"
cd ${CLICK_DIR}
make distclean
export CPPFLAGS="-I${INSTALL_DIR}/include"
export CXXFLAGS="-static"
export LDFLAGS="-L${INSTALL_DIR}/lib"
export CLICK_OPTIONS="--enable-castor \
                      --enable-tools=no \
                      --disable-app --disable-aqm --disable-analysis --disable-test \
                      --disable-tcpudp --disable-icmp --disable-threads"
android/android-${ARCH}.sh
