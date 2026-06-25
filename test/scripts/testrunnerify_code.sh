#!/bin/bash

sed s@\"@\\\\\"@g | sed s@^@\"@ $1 | sed s@\$@\\\\n\"@ | sed 's@\t@    @g'
