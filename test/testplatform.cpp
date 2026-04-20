/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2026 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "platform.h"
#include "fixture.h"
#include "standards.h"

#include <string>

#include "xml.h"


class TestPlatform : public TestFixture {
public:
    TestPlatform() : TestFixture("TestPlatform") {}

private:
    void run() override {
        TEST_CASE(empty);
        TEST_CASE(valid_config_win32a);
        TEST_CASE(valid_config_unix64);
        TEST_CASE(valid_config_win32w);
        TEST_CASE(valid_config_unix32);
        TEST_CASE(valid_config_win64);
        // TODO: test native and unspecified
        TEST_CASE(valid_config_file_1);
        TEST_CASE(valid_config_file_2);
        TEST_CASE(valid_config_file_4);
        TEST_CASE(invalid_config_file_1);
        TEST_CASE(invalid_config_file_2);
        TEST_CASE(empty_elements);
        TEST_CASE(default_platform);
        TEST_CASE(limitsDefines);
        TEST_CASE(charMinMax);
        TEST_CASE(no_root_node);
        TEST_CASE(wrong_root_node);
    }

    class PlatformTest final : public Platform
    {
        friend class TestPlatform;
    };

    static bool readPlatform(PlatformTest& platform, const char* xmldata) {
        tinyxml2::XMLDocument doc;
        return (doc.Parse(xmldata) == tinyxml2::XML_SUCCESS) && platform.loadFromXmlDocument(&doc);
    }

    void empty() const {
        // An empty platform file does not change values, only the type.
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n<platform/>";
        PlatformTest platform;
        // TODO: this should fail - platform files need to be complete
        TODO_ASSERT(!readPlatform(platform, xmldata));
    }

    void valid_config_win32a() const {
        // Verify if native Win32A platform is loaded correctly
        Platform platform;
        PLATFORM(platform, Platform::Type::Win32A);
        ASSERT_EQUALS(Platform::Type::Win32A, platform.type);
        ASSERT(platform.isWindows());
        ASSERT_EQUALS(1, platform.sizeof_bool);
        ASSERT_EQUALS(2, platform.sizeof_short);
        ASSERT_EQUALS(4, platform.sizeof_int);
        ASSERT_EQUALS(4, platform.sizeof_long);
        ASSERT_EQUALS(8, platform.sizeof_long_long);
        ASSERT_EQUALS(4, platform.sizeof_float);
        ASSERT_EQUALS(8, platform.sizeof_double);
        ASSERT_EQUALS(8, platform.sizeof_long_double);
        ASSERT_EQUALS(2, platform.sizeof_wchar_t);
        ASSERT_EQUALS(4, platform.sizeof_size_t);
        ASSERT_EQUALS(4, platform.sizeof_pointer);
        ASSERT_EQUALS('s', platform.defaultSign);
        ASSERT_EQUALS(8, platform.char_bit);
        ASSERT_EQUALS(16, platform.short_bit);
        ASSERT_EQUALS(32, platform.int_bit);
        ASSERT_EQUALS(32, platform.long_bit);
        ASSERT_EQUALS(64, platform.long_long_bit);
    }

    void valid_config_unix64() const {
        // Verify if native Unix64 platform is loaded correctly
        Platform platform;
        PLATFORM(platform, Platform::Type::Unix64);
        ASSERT_EQUALS(Platform::Type::Unix64, platform.type);
        ASSERT(!platform.isWindows());
        ASSERT_EQUALS(1, platform.sizeof_bool);
        ASSERT_EQUALS(2, platform.sizeof_short);
        ASSERT_EQUALS(4, platform.sizeof_int);
        ASSERT_EQUALS(8, platform.sizeof_long);
        ASSERT_EQUALS(8, platform.sizeof_long_long);
        ASSERT_EQUALS(4, platform.sizeof_float);
        ASSERT_EQUALS(8, platform.sizeof_double);
        ASSERT_EQUALS(16, platform.sizeof_long_double);
        ASSERT_EQUALS(4, platform.sizeof_wchar_t);
        ASSERT_EQUALS(8, platform.sizeof_size_t);
        ASSERT_EQUALS(8, platform.sizeof_pointer);
        ASSERT_EQUALS('s', platform.defaultSign);
        ASSERT_EQUALS(8, platform.char_bit);
        ASSERT_EQUALS(16, platform.short_bit);
        ASSERT_EQUALS(32, platform.int_bit);
        ASSERT_EQUALS(64, platform.long_bit);
        ASSERT_EQUALS(64, platform.long_long_bit);
        ASSERT_EQUALS(32, platform.float_bit);
        ASSERT_EQUALS(64, platform.double_bit);
        ASSERT_EQUALS(128, platform.long_double_bit);
    }

    void valid_config_win32w() const {
        // Verify if native Win32W platform is loaded correctly
        Platform platform;
        PLATFORM(platform, Platform::Type::Win32W);
        ASSERT_EQUALS(Platform::Type::Win32W, platform.type);
        ASSERT(platform.isWindows());
        ASSERT_EQUALS(1, platform.sizeof_bool);
        ASSERT_EQUALS(2, platform.sizeof_short);
        ASSERT_EQUALS(4, platform.sizeof_int);
        ASSERT_EQUALS(4, platform.sizeof_long);
        ASSERT_EQUALS(8, platform.sizeof_long_long);
        ASSERT_EQUALS(4, platform.sizeof_float);
        ASSERT_EQUALS(8, platform.sizeof_double);
        ASSERT_EQUALS(8, platform.sizeof_long_double);
        ASSERT_EQUALS(2, platform.sizeof_wchar_t);
        ASSERT_EQUALS(4, platform.sizeof_size_t);
        ASSERT_EQUALS(4, platform.sizeof_pointer);
        ASSERT_EQUALS('s', platform.defaultSign);
        ASSERT_EQUALS(8, platform.char_bit);
        ASSERT_EQUALS(16, platform.short_bit);
        ASSERT_EQUALS(32, platform.int_bit);
        ASSERT_EQUALS(32, platform.long_bit);
        ASSERT_EQUALS(64, platform.long_long_bit);
        ASSERT_EQUALS(32, platform.float_bit);
        ASSERT_EQUALS(64, platform.double_bit);
        ASSERT_EQUALS(64, platform.long_double_bit);
    }

    void valid_config_unix32() const {
        // Verify if native Unix32 platform is loaded correctly
        Platform platform;
        PLATFORM(platform, Platform::Type::Unix32);
        ASSERT_EQUALS(Platform::Type::Unix32, platform.type);
        ASSERT(!platform.isWindows());
        ASSERT_EQUALS(1, platform.sizeof_bool);
        ASSERT_EQUALS(2, platform.sizeof_short);
        ASSERT_EQUALS(4, platform.sizeof_int);
        ASSERT_EQUALS(4, platform.sizeof_long);
        ASSERT_EQUALS(8, platform.sizeof_long_long);
        ASSERT_EQUALS(4, platform.sizeof_float);
        ASSERT_EQUALS(8, platform.sizeof_double);
        ASSERT_EQUALS(12, platform.sizeof_long_double);
        ASSERT_EQUALS(4, platform.sizeof_wchar_t);
        ASSERT_EQUALS(4, platform.sizeof_size_t);
        ASSERT_EQUALS(4, platform.sizeof_pointer);
        ASSERT_EQUALS('s', platform.defaultSign);
        ASSERT_EQUALS(8, platform.char_bit);
        ASSERT_EQUALS(16, platform.short_bit);
        ASSERT_EQUALS(32, platform.int_bit);
        ASSERT_EQUALS(32, platform.long_bit);
        ASSERT_EQUALS(64, platform.long_long_bit);
        ASSERT_EQUALS(32, platform.float_bit);
        ASSERT_EQUALS(64, platform.double_bit);
        ASSERT_EQUALS(96, platform.long_double_bit);
    }

    void valid_config_win64() const {
        // Verify if native Win64 platform is loaded correctly
        Platform platform;
        PLATFORM(platform, Platform::Type::Win64);
        ASSERT_EQUALS(Platform::Type::Win64, platform.type);
        ASSERT(platform.isWindows());
        ASSERT_EQUALS(1, platform.sizeof_bool);
        ASSERT_EQUALS(2, platform.sizeof_short);
        ASSERT_EQUALS(4, platform.sizeof_int);
        ASSERT_EQUALS(4, platform.sizeof_long);
        ASSERT_EQUALS(8, platform.sizeof_long_long);
        ASSERT_EQUALS(4, platform.sizeof_float);
        ASSERT_EQUALS(8, platform.sizeof_double);
        ASSERT_EQUALS(8, platform.sizeof_long_double);
        ASSERT_EQUALS(2, platform.sizeof_wchar_t);
        ASSERT_EQUALS(8, platform.sizeof_size_t);
        ASSERT_EQUALS(8, platform.sizeof_pointer);
        ASSERT_EQUALS('s', platform.defaultSign);
        ASSERT_EQUALS(8, platform.char_bit);
        ASSERT_EQUALS(16, platform.short_bit);
        ASSERT_EQUALS(32, platform.int_bit);
        ASSERT_EQUALS(32, platform.long_bit);
        ASSERT_EQUALS(64, platform.long_long_bit);
        ASSERT_EQUALS(32, platform.float_bit);
        ASSERT_EQUALS(64, platform.double_bit);
        ASSERT_EQUALS(64, platform.long_double_bit);
    }

    void valid_config_file_1() const {
        // Valid platform configuration with all possible values specified.
        // Similar to the avr8 platform file.
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<platform>\n"
                                   "  <windows>false</windows>\n"
                                   "  <char_bit>8</char_bit>\n"
                                   "  <default-sign>unsigned</default-sign>\n"
                                   "  <sizeof>\n"
                                   "    <bool>1</bool>\n"
                                   "    <short>2</short>\n"
                                   "    <int>2</int>\n"
                                   "    <long>4</long>\n"
                                   "    <long-long>8</long-long>\n"
                                   "    <float>4</float>\n"
                                   "    <double>4</double>\n"
                                   "    <long-double>4</long-double>\n"
                                   "    <pointer>2</pointer>\n"
                                   "    <size_t>2</size_t>\n"
                                   "    <wchar_t>2</wchar_t>\n"
                                   "  </sizeof>\n"
                                   " </platform>";
        PlatformTest platform;
        ASSERT(readPlatform(platform, xmldata));
        ASSERT_EQUALS(Platform::Type::File, platform.type);
        ASSERT(!platform.isWindows());
        ASSERT_EQUALS(8, platform.char_bit);
        ASSERT_EQUALS('u', platform.defaultSign);
        ASSERT_EQUALS(1, platform.sizeof_bool);
        ASSERT_EQUALS(2, platform.sizeof_short);
        ASSERT_EQUALS(2, platform.sizeof_int);
        ASSERT_EQUALS(4, platform.sizeof_long);
        ASSERT_EQUALS(8, platform.sizeof_long_long);
        ASSERT_EQUALS(4, platform.sizeof_float);
        ASSERT_EQUALS(4, platform.sizeof_double);
        ASSERT_EQUALS(4, platform.sizeof_long_double);
        ASSERT_EQUALS(2, platform.sizeof_pointer);
        ASSERT_EQUALS(2, platform.sizeof_size_t);
        ASSERT_EQUALS(2, platform.sizeof_wchar_t);
        ASSERT_EQUALS(16, platform.short_bit);
        ASSERT_EQUALS(16, platform.int_bit);
        ASSERT_EQUALS(32, platform.long_bit);
        ASSERT_EQUALS(64, platform.long_long_bit);
    }

    void valid_config_file_2() const {
        // Valid platform configuration with all possible values specified and
        // char_bit > 8.
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<platform>\n"
                                   "  <windows>true</windows>\n"
                                   "  <char_bit>20</char_bit>\n"
                                   "  <default-sign>signed</default-sign>\n"
                                   "  <sizeof>\n"
                                   "    <bool>1</bool>\n"
                                   "    <short>2</short>\n"
                                   "    <int>3</int>\n"
                                   "    <long>4</long>\n"
                                   "    <long-long>5</long-long>\n"
                                   "    <float>6</float>\n"
                                   "    <double>7</double>\n"
                                   "    <long-double>8</long-double>\n"
                                   "    <pointer>9</pointer>\n"
                                   "    <size_t>10</size_t>\n"
                                   "    <wchar_t>11</wchar_t>\n"
                                   "  </sizeof>\n"
                                   " </platform>";
        PlatformTest platform;
        ASSERT(readPlatform(platform, xmldata));
        ASSERT_EQUALS(Platform::Type::File, platform.type);
        ASSERT(platform.isWindows());
        ASSERT_EQUALS(20, platform.char_bit);
        ASSERT_EQUALS('s', platform.defaultSign);
        ASSERT_EQUALS(1, platform.sizeof_bool);
        ASSERT_EQUALS(2, platform.sizeof_short);
        ASSERT_EQUALS(3, platform.sizeof_int);
        ASSERT_EQUALS(4, platform.sizeof_long);
        ASSERT_EQUALS(5, platform.sizeof_long_long);
        ASSERT_EQUALS(6, platform.sizeof_float);
        ASSERT_EQUALS(7, platform.sizeof_double);
        ASSERT_EQUALS(8, platform.sizeof_long_double);
        ASSERT_EQUALS(9, platform.sizeof_pointer);
        ASSERT_EQUALS(10, platform.sizeof_size_t);
        ASSERT_EQUALS(11, platform.sizeof_wchar_t);
        ASSERT_EQUALS(40, platform.short_bit);
        ASSERT_EQUALS(60, platform.int_bit);
        ASSERT_EQUALS(80, platform.long_bit);
        ASSERT_EQUALS(100, platform.long_long_bit);
    }

    void invalid_config_file_2() const {
        // Invalid platform configuration without any usable information.
        // Similar like an empty file.
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<platform>\n"
                                   "  <windows1>true</windows1>\n"
                                   "  <char_bit1>8</char_bit1>\n"
                                   "  <default-sign1>unsigned</default-sign1>\n"
                                   "  <sizeof1>\n"
                                   "    <bool1>1</bool1>\n"
                                   "    <short1>2</short1>\n"
                                   "    <int1>3</int1>\n"
                                   "    <long1>4</long1>\n"
                                   "    <long-long1>5</long-long1>\n"
                                   "    <float1>6</float1>\n"
                                   "    <double1>7</double1>\n"
                                   "    <long-double1>8</long-double1>\n"
                                   "    <pointer1>9</pointer1>\n"
                                   "    <size_t1>10</size_t1>\n"
                                   "    <wchar_t1>11</wchar_t1>\n"
                                   "  </sizeof1>\n"
                                   " </platform>";
        PlatformTest platform;
        // TODO: needs to fail - files need to be complete
        TODO_ASSERT(!readPlatform(platform, xmldata));
    }

    void valid_config_file_4() const {
        // Valid platform configuration with all possible values specified and
        // set to 0.
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<platform>\n"
                                   "  <windows>true</windows>\n"
                                   "  <char_bit>0</char_bit>\n"
                                   "  <default-sign>z</default-sign>\n"
                                   "  <sizeof>\n"
                                   "    <bool>0</bool>\n"
                                   "    <short>0</short>\n"
                                   "    <int>0</int>\n"
                                   "    <long>0</long>\n"
                                   "    <long-long>0</long-long>\n"
                                   "    <float>0</float>\n"
                                   "    <double>0</double>\n"
                                   "    <long-double>0</long-double>\n"
                                   "    <pointer>0</pointer>\n"
                                   "    <size_t>0</size_t>\n"
                                   "    <wchar_t>0</wchar_t>\n"
                                   "  </sizeof>\n"
                                   " </platform>";
        PlatformTest platform;
        ASSERT(readPlatform(platform, xmldata));
        ASSERT_EQUALS(Platform::Type::File, platform.type);
        ASSERT(platform.isWindows());
        ASSERT_EQUALS(0, platform.char_bit);
        ASSERT_EQUALS('z', platform.defaultSign);
        ASSERT_EQUALS(0, platform.sizeof_bool);
        ASSERT_EQUALS(0, platform.sizeof_short);
        ASSERT_EQUALS(0, platform.sizeof_int);
        ASSERT_EQUALS(0, platform.sizeof_long);
        ASSERT_EQUALS(0, platform.sizeof_long_long);
        ASSERT_EQUALS(0, platform.sizeof_float);
        ASSERT_EQUALS(0, platform.sizeof_double);
        ASSERT_EQUALS(0, platform.sizeof_long_double);
        ASSERT_EQUALS(0, platform.sizeof_pointer);
        ASSERT_EQUALS(0, platform.sizeof_size_t);
        ASSERT_EQUALS(0, platform.sizeof_wchar_t);
        ASSERT_EQUALS(0, platform.short_bit);
        ASSERT_EQUALS(0, platform.int_bit);
        ASSERT_EQUALS(0, platform.long_bit);
        ASSERT_EQUALS(0, platform.long_long_bit);
    }

    void invalid_config_file_1() const {
        // Invalid XML file: mismatching elements "boolt" vs "bool".
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<platform>\n"
                                   "  <windows>false</windows>\n"
                                   "  <char_bit>8</char_bit>\n"
                                   "  <default-sign>unsigned</default-sign>\n"
                                   "  <sizeof>\n"
                                   "    <boolt>1</bool>\n"
                                   "    <short>2</short>\n"
                                   "    <int>2</int>\n"
                                   "    <long>4</long>\n"
                                   "    <long-long>8</long-long>\n"
                                   "    <float>4</float>\n"
                                   "    <double>4</double>\n"
                                   "    <long-double>4</long-double>\n"
                                   "    <pointer>2</pointer>\n"
                                   "    <size_t>2</size_t>\n"
                                   "    <wchar_t>2</wchar_t>\n"
                                   "  </sizeof>\n"
                                   " </platform>";
        PlatformTest platform;
        ASSERT(!readPlatform(platform, xmldata));
    }

    void empty_elements() const {
        // Valid platform configuration without any usable information.
        // Similar like an empty file.
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<platform>\n"
                                   "  <windows></windows>\n"
                                   "  <char_bit></char_bit>\n"
                                   "  <default-sign></default-sign>\n"
                                   "  <sizeof>\n"
                                   "    <bool></bool>\n"
                                   "    <short></short>\n"
                                   "    <int></int>\n"
                                   "    <long></long>\n"
                                   "    <long-long></long-long>\n"
                                   "    <float></float>\n"
                                   "    <double></double>\n"
                                   "    <long-double></long-double>\n"
                                   "    <pointer></pointer>\n"
                                   "    <size_t></size_t>\n"
                                   "    <wchar_t></wchar_t>\n"
                                   "  </sizeof>\n"
                                   " </platform>";
        PlatformTest platform;
        ASSERT(!readPlatform(platform, xmldata));
    }

    void default_platform() const {
        Platform platform;
        ASSERT_EQUALS(Platform::Type::Native, platform.type);
    }

    void limitsDefines() const {
        Platform platform;
        ASSERT_EQUALS(true, platform.set(Platform::Unix64));
        const std::string defs = "CHAR_BIT=8;SCHAR_MIN=-128;SCHAR_MAX=127;UCHAR_MAX=255;CHAR_MIN=0;CHAR_MAX=127;SHRT_MIN=-32768;SHRT_MAX=32767;USHRT_MAX=65535;INT_MIN=(-2147483647 - 1);INT_MAX=2147483647;UINT_MAX=4294967295;LONG_MIN=(-9223372036854775807L - 1L);LONG_MAX=9223372036854775807L;ULONG_MAX=18446744073709551615UL";
        const std::string defs_c99 = "CHAR_BIT=8;SCHAR_MIN=-128;SCHAR_MAX=127;UCHAR_MAX=255;CHAR_MIN=0;CHAR_MAX=127;SHRT_MIN=-32768;SHRT_MAX=32767;USHRT_MAX=65535;INT_MIN=(-2147483647 - 1);INT_MAX=2147483647;UINT_MAX=4294967295;LONG_MIN=(-9223372036854775807L - 1L);LONG_MAX=9223372036854775807L;ULONG_MAX=18446744073709551615UL;LLONG_MIN=(-9223372036854775807LL - 1LL);LLONG_MAX=9223372036854775807LL;ULLONG_MAX=18446744073709551615ULL";
        ASSERT_EQUALS(defs, platform.getLimitsDefines(Standards::cstd_t::C89));
        ASSERT_EQUALS(defs_c99, platform.getLimitsDefines(Standards::cstd_t::C99));
        ASSERT_EQUALS(defs_c99, platform.getLimitsDefines(Standards::cstd_t::CLatest));
        ASSERT_EQUALS(defs, platform.getLimitsDefines(Standards::cppstd_t::CPP03));
        ASSERT_EQUALS(defs_c99, platform.getLimitsDefines(Standards::cppstd_t::CPP11));
        ASSERT_EQUALS(defs_c99, platform.getLimitsDefines(Standards::cppstd_t::CPPLatest));
    }

    void charMinMax() const {
        Platform platform;
        ASSERT_EQUALS(255, platform.unsignedCharMax());
        ASSERT_EQUALS(127, platform.signedCharMax());
        ASSERT_EQUALS(-128, platform.signedCharMin());
    }

    void no_root_node() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>";
        PlatformTest platform;
        ASSERT(!readPlatform(platform, xmldata));
    }

    void wrong_root_node() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<platforms/>";
        PlatformTest platform;
        ASSERT(!readPlatform(platform, xmldata));
    }
};

REGISTER_TEST(TestPlatform)
