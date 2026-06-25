#!/bin/sh

# TODO: use temporary filename
./create_platform_cfg.sh > platform.cfg

# TODO: add option to pass define to cppcheck
./defines.sh > defines.txt

./cppcheck --platform=platform.cfg "$@"