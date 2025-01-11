#!/bin/sh

VERSION="1.0.27"
ROOT="$PWD/libusb"
SOURCE="$ROOT/source"
BUILD="$ROOT/build"
INSTALL="$ROOT/install"

mkdir -p "$ROOT"
cd "$ROOT" || exit 1

curl -LO "https://github.com/libusb/libusb/releases/download/v$VERSION/libusb-$VERSION.tar.bz2" || exit 1
tar -xf "libusb-$VERSION.tar.bz2" || exit 1
mv "libusb-$VERSION" "$SOURCE" || exit 1

mkdir "$BUILD" || exit 1
cd "$BUILD" || exit 1
"$SOURCE/configure" --prefix="$INSTALL" --disable-shared --disable-udev --disable-eventfd --disable-timerfd --with-pic || exit 1
make || exit 1
make install || exit 1

