#!/bin/sh

cmake -GNinja -Bbuild -DCMAKE_BUILD_TYPE=Release -DLIBICSNEO_BUILD_EXAMPLES=ON \
	-DLIBICSNEO_BUILD_UNIT_TESTS=ON -DLIBICSNEO_ENABLE_TCP=OFF || exit 1

cmake --build build || exit 1

exit 0
