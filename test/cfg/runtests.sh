#!/bin/bash
set -e # abort on error
#set -x # be verbose

# TODO: do not bail out on first error - always run all tests and return error instead

function exit_if_strict {
    if [ -n "${STRICT}" ] && [ "${STRICT}" -eq 1 ]; then
      exit 1
    fi
}

echo "Checking for pkg-config..."
if pkg-config --version; then
  HAS_PKG_CONFIG=1
  echo "pkg-config found."
else
  HAS_PKG_CONFIG=0
  echo "pkg-config is not available, skipping all syntax checks."
  exit_if_strict
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/
CPPCHECK="$DIR"../../cppcheck
CFG="$DIR"../../cfg/

# TODO: remove missingInclude disabling when it no longer is implied by --enable=information
# Cppcheck options
# need to suppress unmatchedSuppression in case valueFlowBailout is not reported
CPPCHECK_OPT=(
    "--check-library"
    "--platform=unix64"
    "--enable=style,information"
    "--inconclusive"
    "--force"
    "--check-level=exhaustive"
    "--error-exitcode=-1"
    "--inline-suppr"
    "--template=\"{file}:{line}:{severity}:{id}:{message}\""
    "--debug-warnings"
    "--suppress=checkersReport")

# Compiler settings
CXX=g++
CXX_OPT=("-fsyntax-only" "-w" "-std=c++2a")
CC=gcc
CC_OPT=("-fsyntax-only" "-w" "-std=c11")

IWYU_OPTS=("-Xiwyu" "--no_fwd_decls" "-Xiwyu" "--update_comments")

function get_pkg_config_cflags {
    # TODO: get rid of the error enabling/disabling?
    set +e
    PKGCONFIG=$(pkg-config --cflags "$@")
    PKGCONFIG_RETURNCODE=$?
    set -e
    if [ $PKGCONFIG_RETURNCODE -ne 0 ]; then
        PKGCONFIG=
    else
        # make sure the config is not empty when no flags were found - happens with e.g. libssl and sqlite3
        if [ -z "$PKGCONFIG" ]; then
            PKGCONFIG=" "
        fi
    fi
    echo "$PKGCONFIG"
}

function iwyu_run {
    # TODO: convert -I includes provided by pkg-config to -isystem so IWYU does not produce warnings for system headers
    ${IWYU} "${IWYU_OPTS[@]}" "${IWYU_CLANG_INC}" "$@"
}

function cc_syntax {
    if [ -z "$IWYU" ]; then
        ${CC} "${CC_OPT[@]}" "$@"
    else
        iwyu_run "${CC_OPT[@]}" "$@"
    fi
}

function cxx_syntax {
    if [ -z "$IWYU" ]; then
        ${CXX} "${CXX_OPT[@]}" "$@"
    else
        iwyu_run "${CXX_OPT[@]}" "$@"
    fi
}

function cppcheck_run {
    if [ -z "$IWYU" ]; then
        "${CPPCHECK}" "${CPPCHECK_OPT[@]}" "$@"
    fi
}

# posix.c
function posix_fn {
    echo "POSIX assumed to be present, checking syntax with ${CC} now."
    cc_syntax ${DIR}posix.c
}

# gnu.c
function gnu_fn {
    echo "GNU assumed to be present, checking syntax with ${CC} now."
    cc_syntax ${DIR}gnu.c
}

# qt.cpp
function qt_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        # TODO: check syntax with Qt5 and Qt6?
        QTCONFIG=$(get_pkg_config_cflags Qt6Core Qt6Test Qt6Gui)
        if [ -z "$QTCONFIG" ]; then
          QTCONFIG=$(get_pkg_config_cflags Qt5Core Qt5Test Qt5Gui)
          if [ -n "$QTCONFIG" ]; then
            QTBUILDCONFIG=$(pkg-config --variable=qt_config Qt5Core Qt5Test Qt5Gui)
            [[ $QTBUILDCONFIG =~ (^|[[:space:]])reduce_relocations($|[[:space:]]) ]] && QTCONFIG="${QTCONFIG} -fPIC"
          fi
        else
          QTBUILDCONFIG=$(pkg-config --variable=qt_config Qt6Core Qt6Test Qt6Gui)
          QTCONFIG="${QTCONFIG} -fPIC"
        fi
        if [ -n "$QTCONFIG" ]; then
            # TODO: get rid of the error enabling/disabling?
            set +e
            echo -e "#include <QString>" | ${CXX} "${CXX_OPT[@]}" ${QTCONFIG} -x c++ -
            QTCHECK_RETURNCODE=$?
            set -e
            if [ $QTCHECK_RETURNCODE -ne 0 ]; then
                echo "Qt not completely present or not working, skipping syntax check with ${CXX}."
                exit_if_strict
            else
                echo "Qt found and working, checking syntax with ${CXX} now."
                cxx_syntax ${QTCONFIG} ${DIR}qt.cpp
            fi
        else
            echo "Qt not present, skipping syntax check with ${CXX}."
            exit_if_strict
        fi
    fi
}

# bsd.c
function bsd_fn {
    # TODO: add syntax check
    true
}

# std.c
function std_c_fn {
    echo "C standard library assumed to be present, checking syntax with ${CC} now."
    cc_syntax "${DIR}"std.c
}

# std.cpp
function std_cpp_fn {
    echo "C++ standard library assumed to be present, checking syntax with ${CXX} now."
    cxx_syntax "${DIR}"std.cpp
}

# windows.cpp
function windows_fn {
    # TODO: Syntax check via g++ does not work because it can not find a valid windows.h
    #cxx_syntax ${DIR}windows.cpp
    true
}

# mfc.cpp
function mfc_fn {
    # TODO: Add syntax check
    true
}

# wxwidgets.cpp
function wxwidgets_fn {
    # TODO: get rid of the error enabling/disabling?
    set +e
    WXCONFIG=$(wx-config --cxxflags)
    WXCONFIG_RETURNCODE=$?
    set -e
    if [ $WXCONFIG_RETURNCODE -ne 0 ]; then
        echo "wx-config does not work, skipping syntax check for wxWidgets tests."
        exit_if_strict
    else
        # TODO: get rid of the error enabling/disabling?
        set +e
        echo -e "#include <wx/filefn.h>\n#include <wx/app.h>\n#include <wx/artprov.h>\n#include <wx/version.h>\n#if wxVERSION_NUMBER<2950\n#error \"Old version\"\n#endif" | ${CXX} "${CXX_OPT[@]}"  ${WXCONFIG} -x c++ -
        WXCHECK_RETURNCODE=$?
        set -e
        if [ $WXCHECK_RETURNCODE -ne 0 ]; then
            echo "wxWidgets not completely present (with GUI classes) or not working, skipping syntax check with ${CXX}."
            exit_if_strict
        else
            echo "wxWidgets found, checking syntax with ${CXX} now."
            cxx_syntax ${WXCONFIG} -Wno-deprecated-declarations "${DIR}"wxwidgets.cpp
        fi
    fi
}

# gtk.c
function gtk_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        GTKCONFIG=$(get_pkg_config_cflags gtk+-3.0)
        if [ -z "$GTKCONFIG" ]; then
            GTKCONFIG=$(get_pkg_config_cflags gtk+-2.0)
        fi
        if [ -n "$GTKCONFIG" ]; then
            # TODO: get rid of the error enabling/disabling?
            set +e
            echo -e "#include <gtk/gtk.h>" | ${CC} "${CC_OPT[@]}" ${GTKCONFIG} -x c -
            GTKCHECK_RETURNCODE=$?
            set -e
            if [ $GTKCHECK_RETURNCODE -ne 0 ]; then
                echo "GTK+ not completely present or not working, skipping syntax check with ${CC}."
                exit_if_strict
            else
                echo "GTK+ found and working, checking syntax with ${CC} now."
                cc_syntax ${GTKCONFIG} "${DIR}"gtk.c
            fi
        else
            echo "GTK+ not present, skipping syntax check with ${CC}."
            exit_if_strict
        fi
    fi
}

# boost.cpp
function boost_fn {
    # TODO: get rid of the error enabling/disabling?
    set +e
    echo -e "#include <boost/config.hpp>" | ${CXX} "${CXX_OPT[@]}" -x c++ -
    BOOSTCHECK_RETURNCODE=$?
    set -e
    if [ ${BOOSTCHECK_RETURNCODE} -ne 0 ]; then
        echo "Boost not completely present or not working, skipping syntax check with ${CXX}."
        exit_if_strict
    else
        echo "Boost found and working, checking syntax with ${CXX} now."
        cxx_syntax "${DIR}"boost.cpp
    fi
}

# sqlite3.c
function sqlite3_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        SQLITE3CONFIG=$(get_pkg_config_cflags sqlite3)
        if [ -n "$SQLITE3CONFIG" ]; then
            # TODO: get rid of the error enabling/disabling?
            set +e
            echo -e "#include <sqlite3.h>" | ${CC} "${CC_OPT[@]}" ${SQLITE3CONFIG} -x c -
            SQLITE3CHECK_RETURNCODE=$?
            set -e
            if [ $SQLITE3CHECK_RETURNCODE -ne 0 ]; then
                echo "SQLite3 not completely present or not working, skipping syntax check with ${CC}."
                exit_if_strict
            else
                echo "SQLite3 found and working, checking syntax with ${CC} now."
                cc_syntax ${SQLITE3CONFIG} "${DIR}"sqlite3.c
            fi
        else
            echo "SQLite3 not present, skipping syntax check with ${CC}."
            exit_if_strict
        fi
    fi
}

# openmp.c
function openmp_fn {
    # TODO: omp.h not found with IWYU
    # MacOS compiler has no OpenMP by default
    if ! command -v sw_vers; then
      echo "OpenMP assumed to be present, checking syntax with ${CC} now."
      cc_syntax -fopenmp ${DIR}openmp.c
    fi
}

# python.c
function python_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        PYTHON3CONFIG=$(get_pkg_config_cflags python3)
        if [ -n "$PYTHON3CONFIG" ]; then
            # TODO: get rid of the error enabling/disabling?
            set +e
            echo -e "#include <Python.h>" | ${CC} "${CC_OPT[@]}" ${PYTHON3CONFIG} -x c -
            PYTHON3CONFIG_RETURNCODE=$?
            set -e
            if [ $PYTHON3CONFIG_RETURNCODE -ne 0 ]; then
                echo "Python 3 not completely present or not working, skipping syntax check with ${CC}."
                exit_if_strict
            else
                echo "Python 3 found and working, checking syntax with ${CC} now."
                cc_syntax ${PYTHON3CONFIG} "${DIR}"python.c
            fi
        else
            echo "Python 3 not present, skipping syntax check with ${CC}."
            exit_if_strict
        fi
    fi
}

# lua.c
function lua_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        LUACONFIG=$(get_pkg_config_cflags lua)
        if [ -z "$LUACONFIG" ]; then
            LUACONFIG=$(get_pkg_config_cflags lua-5.3)
        fi
        if [ -n "$LUACONFIG" ]; then
            # TODO: get rid of the error enabling/disabling?
            set +e
            echo -e "#include <lua.h>" | ${CC} "${CC_OPT[@]}" ${LUACONFIG} -x c -
            LUACONFIG_RETURNCODE=$?
            set -e
            if [ $LUACONFIG_RETURNCODE -ne 0 ]; then
                echo "Lua not completely present or not working, skipping syntax check with ${CC}."
                exit_if_strict
            else
                echo "Lua found and working, checking syntax with ${CC} now."
                cc_syntax ${LUACONFIG} "${DIR}"lua.c
            fi
        else
            echo "Lua not present, skipping syntax check with ${CC}."
            exit_if_strict
        fi
    fi
}

# libcurl.c
function libcurl_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        LIBCURLCONFIG=$(get_pkg_config_cflags libcurl)
        if [ -n "$LIBCURLCONFIG" ]; then
            # TODO: get rid of the error enabling/disabling?
            set +e
            echo -e "#include <curl/curl.h>" | ${CC} "${CC_OPT[@]}" ${LIBCURLCONFIG} -x c -
            LIBCURLCONFIG_RETURNCODE=$?
            set -e
            if [ $LIBCURLCONFIG_RETURNCODE -ne 0 ]; then
                echo "libcurl not completely present or not working, skipping syntax check with ${CC}."
                exit_if_strict
            else
                echo "libcurl found and working, checking syntax with ${CC} now."
                cc_syntax ${LIBCURLCONFIG} "${DIR}"libcurl.c
            fi
        else
            echo "libcurl not present, skipping syntax check with ${CC}."
            exit_if_strict
        fi
    fi
}

# cairo.c
function cairo_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        CAIROCONFIG=$(get_pkg_config_cflags cairo)
        if [ -n "$CAIROCONFIG" ]; then
            # TODO: get rid of the error enabling/disabling?
            set +e
            echo -e "#include <cairo.h>" | ${CC} "${CC_OPT[@]}" ${CAIROCONFIG} -x c -
            CAIROCONFIG_RETURNCODE=$?
            set -e
            if [ $CAIROCONFIG_RETURNCODE -ne 0 ]; then
                echo "cairo not completely present or not working, skipping syntax check with ${CC}."
                exit_if_strict
            else
                echo "cairo found and working, checking syntax with ${CC} now."
                cc_syntax ${CAIROCONFIG} "${DIR}"cairo.c
            fi
        else
            echo "cairo not present, skipping syntax check with ${CC}."
            exit_if_strict
        fi
    fi
}

# googletest.cpp
function googletest_fn {
    # TODO: add syntax check
    true
}

# kde.cpp
function kde_fn {
    # TODO: get rid of the error enabling/disabling?
    set +e
    KDECONFIG=$(kde4-config --path include)
    KDECONFIG_RETURNCODE=$?
    set -e
    if [ $KDECONFIG_RETURNCODE -ne 0 ]; then
        echo "kde4-config does not work, skipping syntax check."
        exit_if_strict
    else
        KDEQTCONFIG=$(get_pkg_config_cflags QtCore)
        if [ -n "$KDEQTCONFIG" ]; then
            echo "Suitable Qt not present, Qt is necessary for KDE. Skipping syntax check."
            exit_if_strict
        else
            # TODO: get rid of the error enabling/disabling?
            set +e
            echo -e "#include <KDE/KGlobal>\n" | ${CXX} "${CXX_OPT[@]}" -isystem${KDECONFIG} ${KDEQTCONFIG} -x c++ -
            KDECHECK_RETURNCODE=$?
            set -e
            if [ $KDECHECK_RETURNCODE -ne 0 ]; then
                echo "KDE headers not completely present or not working, skipping syntax check with ${CXX}."
                exit_if_strict
            else
                echo "KDE found, checking syntax with ${CXX} now."
                cxx_syntax -isystem${KDECONFIG} ${KDEQTCONFIG} "${DIR}"kde.cpp
            fi
        fi
    fi
}

# libsigc++.cpp
function libsigcpp_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        LIBSIGCPPCONFIG=$(get_pkg_config_cflags sigc++-2.0)
        if [ -n "$LIBSIGCPPCONFIG" ]; then
            # TODO: get rid of the error enabling/disabling?
            set +e
            echo -e "#include <sigc++/sigc++.h>\n" | ${CXX} "${CXX_OPT[@]}" ${LIBSIGCPPCONFIG} -x c++ -
            LIBSIGCPPCONFIG_RETURNCODE=$?
            set -e
            if [ $LIBSIGCPPCONFIG_RETURNCODE -ne 0 ]; then
                echo "libsigc++ not completely present or not working, skipping syntax check with ${CXX}."
                exit_if_strict
            else
                echo "libsigc++ found and working, checking syntax with ${CXX} now."
                cxx_syntax ${LIBSIGCPPCONFIG} "${DIR}"libsigc++.cpp
            fi
        else
            echo "libsigc++ not present, skipping syntax check with ${CXX}."
            exit_if_strict
        fi
    fi
}

# openssl.c
function openssl_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        OPENSSLCONFIG=$(get_pkg_config_cflags libssl)
        if [ -n "$OPENSSLCONFIG" ]; then
            # TODO: get rid of the error enabling/disabling?
            set +e
            echo -e "#include <openssl/ssl.h>" | ${CC} "${CC_OPT[@]}" ${OPENSSLCONFIG} -x c -
            OPENSSLCONFIG_RETURNCODE=$?
            set -e
            if [ $OPENSSLCONFIG_RETURNCODE -ne 0 ]; then
                echo "OpenSSL not completely present or not working, skipping syntax check with ${CC}."
                exit_if_strict
            else
                echo "OpenSSL found and working, checking syntax with ${CC} now."
                cc_syntax ${OPENSSLCONFIG} "${DIR}"openssl.c
            fi
        else
            echo "OpenSSL not present, skipping syntax check with ${CC}."
            exit_if_strict
        fi
    fi
}

# opencv2.cpp
function opencv2_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        OPENCVCONFIG=$(get_pkg_config_cflags opencv)
        if [ -n "$OPENCVCONFIG" ]; then
            # TODO: get rid of the error enabling/disabling?
            set +e
            echo -e "#include <opencv2/opencv.hpp>\n" | ${CXX} "${CXX_OPT[@]}"  ${OPENCVCONFIG} -x c++ -
            OPENCVCONFIG_RETURNCODE=$?
            set -e
            if [ $OPENCVCONFIG_RETURNCODE -ne 0 ]; then
                echo "OpenCV not completely present or not working, skipping syntax check with ${CXX}."
                exit_if_strict
            else
                echo "OpenCV found and working, checking syntax with ${CXX} now."
                cxx_syntax ${OPENCVCONFIG} "${DIR}"opencv2.cpp
            fi
        else
            echo "OpenCV not present, skipping syntax check with ${CXX}."
            exit_if_strict
        fi
    fi
}

# cppunit.cpp
function cppunit_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        if ! pkg-config cppunit; then
            echo "cppunit not found, skipping syntax check for cppunit"
            exit_if_strict
        else
            echo "cppunit found, checking syntax with ${CXX} now."
            cxx_syntax -Wno-deprecated-declarations "${DIR}"cppunit.cpp
        fi
    fi
}

# emscripten.cpp
function emscripten_fn {
    # TODO: Syntax check via g++ does not work because it can not find a valid emscripten.h
    # cxx_syntax ${DIR}emscripten.cpp
    true
}

# selinux.c
function selinux_fn {
    # TODO: add syntax check
    true
}

function check_file {
    f=$(basename "$1")
    lib="${f%%.*}"
    case $f in
        boost.cpp)
            boost_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        bsd.c)
            bsd_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        cairo.c)
            cairo_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        cppunit.cpp)
            cppunit_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        emscripten.cpp)
            emscripten_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        gnu.c)
            gnu_fn
            # TODO: posix needs to specified first or it has a different mmap() config
            # TODO: get rid of posix dependency
            cppcheck_run --library=posix,"$lib" "${DIR}"gnu.c
            ;;
        googletest.cpp)
            googletest_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        gtk.c)
            gtk_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        kde.cpp)
            # TODO: "kde-4config" is no longer commonly available in recent distros
            #kde_fn
           cppcheck_run --library="$lib" --library=qt "${DIR}""$f"
            ;;
        libcurl.c)
            libcurl_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        libsigc++.cpp)
            libsigcpp_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        lua.c)
            lua_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        mfc.cpp)
            mfc_fn
            cppcheck_run --platform=win64 --library="$lib" "${DIR}""$f"
            ;;
        opencv2.cpp)
            # TODO: "opencv.pc" is not commonly available in distros
            #opencv2_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        openmp.c)
            openmp_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        openssl.c)
            openssl_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        posix.c)
            posix_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        python.c)
            python_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        qt.cpp)
            qt_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        selinux.c)
            selinux_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        sqlite3.c)
            sqlite3_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        std.c)
            std_c_fn
            cppcheck_run "${DIR}""$f"
            ;;
        std.cpp)
            std_cpp_fn
            cppcheck_run "${DIR}""$f"
            ;;
        windows.cpp)
            windows_fn
            cppcheck_run --platform=win32A --library="$lib" "${DIR}""$f"
            cppcheck_run --platform=win32W --library="$lib" "${DIR}""$f"
            cppcheck_run --platform=win64  --library="$lib" "${DIR}""$f"
            ;;
        wxwidgets.cpp)
            wxwidgets_fn
            cppcheck_run --library="$lib" "${DIR}""$f"
            ;;
        *)
          echo "Unhandled file $f"
          exit_if_strict
    esac
}

function check_files
{
for f in "$@"
do
    check_file "$f"
done
}

# Check the syntax of the defines in the configuration files
function check_defines_syntax
{
    if ! xmlstarlet --version; then
        echo "xmlstarlet needed to extract defines, skipping defines check."
        exit_if_strict
    else
        for configfile in "${CFG}"*.cfg; do
            echo "Checking defines in $configfile"
            # Disable debugging output temporarily since there could be many defines
            set +x
            # XMLStarlet returns 1 if no elements were found which is no problem here
            EXTRACTED_DEFINES=$(xmlstarlet sel -t -m '//define' -c . -n <"$configfile" || true)
            EXTRACTED_DEFINES=$(echo "$EXTRACTED_DEFINES" | sed 's/<define name="/#define /g' | sed 's/" value="/ /g' | sed 's/"\/>//g')
            echo "$EXTRACTED_DEFINES" | gcc -fsyntax-only -xc -Werror -
        done
    fi
}

if [ $# -eq 0  ]
then
    check_files "${DIR}"*.{c,cpp}
    check_defines_syntax
else
    check_files "$@"
fi

echo SUCCESS
