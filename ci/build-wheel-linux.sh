#!/bin/sh

export CIBW_BEFORE_ALL="sh ci/bootstrap-cibuildwheel.sh && sh ci/bootstrap-libpcap.sh"
export CIBW_BUILD="*manylinux*" # no musl
export CIBW_ENVIRONMENT="LD_LIBRARY_PATH=/project/fc/libicsbl-src/lib64 PCAP_ROOT=/project/libpcap/install"
export SKBUILD_CMAKE_DEFINE="LIBICSNEO_ENABLE_BOOTLOADER=ON;ICSPB_BOOTSTRAP_DIR=/project/icspb;FETCHCONTENT_BASE_DIR=/project/fc"

sh ci/build-wheel-posix.sh
