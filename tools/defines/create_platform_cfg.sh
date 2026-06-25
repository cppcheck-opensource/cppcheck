#!/bin/sh

if [ -n "$1" ]; then
  compiler_cmd=$1
else
  compiler_cmd="gcc"
fi

compiler_defs=$($compiler_cmd -dM -E - < /dev/null)

char_bit=$(echo "$compiler_defs" | grep __CHAR_BIT__ | cut -d' ' -f3)
# only set by compiler if -funsigned-char is specified
char_unsigned=$(echo "$compiler_defs" | grep __CHAR_UNSIGNED__ | cut -d' ' -f3)
if [ -n "$char_unsigned" ] && [ "$char_unsigned" -eq 1 ]; then
  default_sign="unsigned"
else
  default_sign="signed"
fi
# TODO
size_of_bool=
size_of_short=$(echo "$compiler_defs" | grep __SIZEOF_SHORT__ | cut -d' ' -f3)
size_of_int=$(echo "$compiler_defs" | grep __SIZEOF_INT__ | cut -d' ' -f3)
size_of_long=$(echo "$compiler_defs" | grep __SIZEOF_LONG__ | cut -d' ' -f3)
size_of_long_long=$(echo "$compiler_defs" | grep __SIZEOF_LONG_LONG__ | cut -d' ' -f3)
size_of_float=$(echo "$compiler_defs" | grep __SIZEOF_FLOAT__ | cut -d' ' -f3)
size_of_double=$(echo "$compiler_defs" | grep __SIZEOF_DOUBLE__ | cut -d' ' -f3)
size_of_long_double=$(echo "$compiler_defs" | grep __SIZEOF_LONG_DOUBLE__ | cut -d' ' -f3)
size_of_pointer=$(echo "$compiler_defs" | grep __SIZEOF_POINTER__ | cut -d' ' -f3)
size_of_size_t=$(echo "$compiler_defs" | grep __SIZEOF_SIZE_T__ | cut -d' ' -f3)
size_of_wchar_t=$(echo "$compiler_defs" | grep __SIZEOF_WCHAR_T__ | cut -d' ' -f3)

echo "<?xml version=\"1.0\"?>
<platform>
  <char_bit>$char_bit</char_bit>
  <default-sign>$default_sign</default-sign>
  <sizeof>
    <bool>$size_of_bool</bool>
    <short>$size_of_short</short>
    <int>$size_of_int</int>
    <long>$size_of_long</long>
    <long-long>$size_of_long_long</long-long>
    <float>$size_of_float</float>
    <double>$size_of_double</double>
    <long-double>$size_of_long_double</long-double>
    <pointer>$size_of_pointer</pointer>
    <size_t>$size_of_size_t</size_t>
    <wchar_t>$size_of_wchar_t</wchar_t>
  </sizeof>
</platform>"
