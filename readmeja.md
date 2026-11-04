# Cppcheck

| Linux ビルド状態 | Windows ビルド状態 | Coverity Scan Build 状態 |
|:--:|:--:|:--:|
| [![Linux ビルド状態](https://img.shields.io/travis/cppcheck-opensource/cppcheck/master.svg?label=Linux%20build)](https://travis-ci.org/cppcheck-opensource/cppcheck) | [![Windows ビルド状態](https://img.shields.io/appveyor/ci/cppcheck-opensource/cppcheck/master.svg?label=Windows%20build)](https://ci.appveyor.com/project/cppcheck-opensource/cppcheck/branch/master) | [![Coverity Scan Build 状態](https://img.shields.io/coverity/scan/512.svg)](https://scan.coverity.com/projects/512) |

## 名前について

このプログラムは元々、"C++check"という名前でしたが後に"Cppcheck"に変更されました。

このような名前ですが、Cppcheckは CとC++の両方に対して設計されています。

## マニュアル

マニュアルは[オンライン上に](https://cppcheck.sourceforge.io/manual.pdf)あります。

## ビルド

C++11に対応したコンパイラが利用できます。部分的にC++11にサポートしたコンパイラも利用できるかもしれません。もし、あなたのコンパイラがVisual Studio 2013や GCC 4.8で利用できるC++11機能がサポートされているなら、そのコンパイラが利用できます。

GUIも利用する場合、Qtライブラリが必要です。

コマンドラインツールをビルドする場合、[PCRE](http://www.pcre.org/)はオプションです。これはルールを作成するために利用します。

コンパイル上の選択肢がいくつかあります。
* cmake - クロスプラットフォームのビルドツール
* Windows: Visual Studio (VS 2013 またはそれ以上)
* Windows: Qt Creator + mingw
* gnu make
* g++ 4.8 (またはそれ以上)
* clang++

### cmake

cmakeでCppcheckをコンパイルする例

```shell
mkdir build
cd build
cmake ..
cmake --build .
```

C++標準を指定する必要がある場合次のオプションを指定します。
-DCMAKE_CXX_STANDARD=11

CppcheckのGUIが必要な場合次のフラグを指定します。
-DBUILD_GUI=ON

pcreが必要になりますが、正規表現のルールサポートが必要な場合次のフラグを指定します。
-DHAVE_RULES=ON

### Visual Studio

cppcheck.slnファイルが利用できます。このファイルは、Visual Studio 2019向けです。しかし、このプラットフォームツールセットはこれより新しいバージョンまたは古いバージョン向けに変更できます。このソルーションには、プラットフォームターゲットとしてx86とx64があります。

ルールをコンパイルするためには、"Release-PCRE" または "Debug-PCRE" 設定を選択してください。pcre.lib (または pcre64.lib x64ビルド向け) と pcre.h を /externals にコピーしてください。Visual Studio のための PCRE の最新バージョンは [vcpkg](https://github.com/microsoft/vcpkg) から取得できます。

### Qt Creator + MinGW

コマンドラインツールをビルドするには、PCRE.dllが必要です。これは以下のURLからダウンロードできます。:
http://software-download.name/pcre-library-windows/

### GNU make

単純で最適化しないビルド(依存関係なし):

```shell
make
```

推奨するリリースビルド方法:

```shell
make MATCHCOMPILER=yes FILESDIR=/usr/share/cppcheck HAVE_RULES=yes CXXOPTS="-O2 -DNDEBUG"
```

フラグ:

1. `MATCHCOMPILER=yes`
cppcheckの最適化にPythonを使用します。Token::Match パターンはコンパイル時にlC++コードに変換されます。

2. `FILESDIR=/usr/share/cppcheck`
cppcheckの設定ファイル(addon や cfg や platform)を置くディレクトリを指定します。

3. `HAVE_RULES=yes`
ルール機能の有効化 (ルール機能には PCRE が必要です)設定です。

4. `CXXOPTS="-O2 -DNDEBUG"`
ほとんどのコンパイラの最適化オプション、cppcheckの内部デバッグコードの無効化、基本的なコンパイラ警告の有効化

### g++ (エキスパート向け)

依存関係なく Cppcheckをビルドしたい場合、次のコマンドを利用できます。

```shell
g++ -o cppcheck -std=c++11 -Iexternals -Iexternals/simplecpp -Iexternals/tinyxml2 -Ilib cli/*.cpp lib/*.cpp externals/simplecpp/simplecpp.cpp externals/tinyxml2/*.cpp
```

`--rule` や `--rule-file` を利用する場合、依存ライブラリが必要です。

```shell
g++ -o cppcheck -std=c++11 -lpcre -DHAVE_RULES -Iexternals -Iexternals/simplecpp -Iexternals/tinyxml2 -Ilib cli/*.cpp lib/*.cpp externals/simplecpp/simplecpp.cpp externals/tinyxml2/*.cpp
```

### MinGW

```shell
mingw32-make
```

### その他のコンパイラ/IDE

1. 空のプロジェクトファイル /makefileの作成
2. cppcheck cli それに lib ディレクトリに含まれる全てのcppファイルをそのプロジェクトファイルまたはmakefileに加えます。
3. externalsフォルダの全てのcppファイルをプロジェクトファイル / makefileに追加します。
4. ビルド

### Linux で Win32 コマンドラインバージョンをクロスコンパイル

```shell
sudo apt-get install mingw32
make CXX=i586-mingw32msvc-g++ LDFLAGS="-lshlwapi" RDYNAMIC=""
mv cppcheck cppcheck.exe
```

## Webページ

https://cppcheck.sourceforge.io/
