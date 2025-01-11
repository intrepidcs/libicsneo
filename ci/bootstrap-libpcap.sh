#!/bin/sh

VERSION="1.10.5"
ROOT="$PWD/libpcap"
SOURCE="$ROOT/source"
BUILD="$ROOT/build"
INSTALL="$ROOT/install"

mkdir -p "$ROOT"
cd "$ROOT" || exit 1

curl -LO "https://www.tcpdump.org/release/libpcap-$VERSION.tar.xz" || exit 1
tar -xf "libpcap-$VERSION.tar.xz" || exit 1
mv "libpcap-$VERSION" "$SOURCE" || exit 1

cmake -D CMAKE_POSITION_INDEPENDENT_CODE=ON -D CMAKE_INSTALL_PREFIX="$INSTALL" -D BUILD_SHARED_LIBS=OFF -D BUILD_WITH_LIBNL=OFF -D DISABLE_DBUS=ON -D DISABLE_LINUX_USBMON=ON -D DISABLE_BLUETOOTH=ON -D DISABLE_NETMAP=ON -D DISABLE_DPDK=ON -D DISABLE_RDMA=ON -D DISABLE_DAG=ON -D DISABLE_SEPTEL=ON -D DISABLE_SNF=ON -D DISABLE_TC=ON -B "$BUILD" -S "$SOURCE" || exit 1
cmake --build "$BUILD" || exit 1
cmake --install "$BUILD" || exit 1
