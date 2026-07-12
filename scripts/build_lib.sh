#!/usr/bin/env bash
# Build GoogologyLib into library files (static .a + shared .dll/.so).
#
# 把 GoogologyLib 编译成库文件：静态库 (libgoogology.a) 与
# 动态库 (libgoogology.dll + 导入库 libgoogology.dll.a，Linux 下为 .so)。
# 产物输出到 build/lib/；build/ 已被 .gitignore 忽略，故库文件不入库，
# 需要时用本脚本重新生成即可。
#
# Usage:
#   bash scripts/build_lib.sh            # build both static + shared
#   bash scripts/build_lib.sh static     # static archive only
#   bash scripts/build_lib.sh shared     # shared library only
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

CXX="${CXX:-g++}"
STD="-std=c++17"
OPT="${OPT:--O2}"
INC="-Iinclude"
OBJ_DIR="build/obj"
LIB_DIR="build/lib"
WHAT="${1:-all}"

# Detect shared-library extension per platform.
case "$(uname -s 2>/dev/null || echo Windows)" in
  MINGW*|MSYS*|CYGWIN*|Windows*) SO="dll"; IMPLIB="-Wl,--out-implib,$LIB_DIR/libgoogology.dll.a" ;;
  Darwin)                        SO="dylib"; IMPLIB="" ;;
  *)                             SO="so"; IMPLIB="" ;;
esac

# Library sources = every .cpp under src/ EXCEPT the demo entry point.
mapfile -t SOURCES < <(find src -name '*.cpp' ! -name 'main.cpp' | sort)

echo ">> compiling ${#SOURCES[@]} sources ($CXX $STD $OPT)"
mkdir -p "$OBJ_DIR" "$LIB_DIR"
OBJS=()
for f in "${SOURCES[@]}"; do
  o="$OBJ_DIR/$(basename "${f%.cpp}").o"
  "$CXX" $STD $OPT $INC -c "$f" -o "$o"
  OBJS+=("$o")
  echo "   OK $o"
done

if [ "$WHAT" = "all" ] || [ "$WHAT" = "static" ]; then
  ar rcs "$LIB_DIR/libgoogology.a" "${OBJS[@]}"
  echo ">> static : $LIB_DIR/libgoogology.a"
fi

if [ "$WHAT" = "all" ] || [ "$WHAT" = "shared" ]; then
  "$CXX" $STD $OPT $INC -shared "${OBJS[@]}" -o "$LIB_DIR/libgoogology.$SO" $IMPLIB
  echo ">> shared : $LIB_DIR/libgoogology.$SO"
fi

echo ">> done. link with:  -Iinclude -Lbuild/lib -lgoogology"
