#!/bin/sh
# Astyle settings used to format our source code
/usr/bin/astyle --indent=spaces=4 --indent-switches --brackets=break \
                --convert-tabs --keep-one-line-statements --keep-one-line-blocks \
                $*
