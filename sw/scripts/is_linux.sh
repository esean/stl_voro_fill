#!/bin/bash

is_linux() {
	set|grep OSTYPE=|grep -q linux
}

is_linux
# returns true(0) if linux

# use like,
# is_linux.sh && echo "I am linux"
# is_linux.sh || echo "I am not linux"
