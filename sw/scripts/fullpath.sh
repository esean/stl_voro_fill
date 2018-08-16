#!/bin/bash

die() { echo "ERROR:$@:$0 !"; exit 1; }
[ -z "$1" ] && die "USAGE:$0 [filename] - converts [filename] to full-path filename"

FN="$(cd "$(dirname "$1")"; pwd)/$(basename "$1")"
echo "$FN"
