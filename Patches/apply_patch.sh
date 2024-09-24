#!/bin/bash

P_OPT=
[ -n "$MINGW_PREFIX" ] && P_OPT=--binary
P_DIR="$(dirname "$0")"

patch $P_OPT -p1 < "$P_DIR"/0001-*.patch || exit
patch $P_OPT -p1 < "$P_DIR"/0002-*.patch || exit
patch $P_OPT -p1 < "$P_DIR"/0003-*.patch || exit
patch $P_OPT -p1 < "$P_DIR"/0004-*.patch || exit
patch $P_OPT -p1 < "$P_DIR"/0005-*.patch || exit
patch $P_OPT -p1 < "$P_DIR"/0006-*.patch || exit
patch $P_OPT -p1 < "$P_DIR"/0007-*.patch || exit
patch $P_OPT -p1 < "$P_DIR"/0008-*.patch || exit
patch $P_OPT -p1 < "$P_DIR"/0009-*.patch || exit
patch $P_OPT -p1 < "$P_DIR"/0010-*.patch || exit
