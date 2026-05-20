#!/usr/bin/env bash

set -euo pipefail

basedir="$(dirname "${BASH_SOURCE[0]}")"

function main {
	local typename
	local outfile
	if [[ ! "$1" =~ ^[0-9]+$ ]]; then
		echo "Not a number: $1" >&2
		exit 1
	fi
	typename="uint$1_t"
	outfile="$basedir/test_$typename.c"
	cat > "$outfile" <<-EOF
	#include "acutest.h"
	#include <stdint.h>
	#include <string.h>

	extern size_t countr_zero_$typename($typename);
	extern size_t countr_zero_${typename}_nonzero($typename);

	void test_contr_zero(void) {
		TEST_CHECK(countr_zero_$typename(0) == sizeof($typename) * 8);
		for (int i = 0; i < sizeof($typename) * 8; ++i) {
			size_t ctz = countr_zero_$typename(($typename)1 << i);
			TEST_CHECK_(ctz == i,
				"countr_zero_$typename(1 << %d): expected %d but got %zu", i, i, ctz);
		}
	}

	void test_contr_zero_nonzero(void) {
		for (int i = 0; i < sizeof($typename) * 8; ++i) {
			size_t ctz = countr_zero_${typename}_nonzero(($typename)1 << i);
			TEST_CHECK_(ctz == i,
				"countr_zero_${typename}_nonzero(1 << %d): expected %d but got %zu", i, i, ctz);
		}
	}

	TEST_LIST = {
			{"contr_zero_$typename", test_contr_zero},
			{"contr_zero_${typename}_nonzero", test_contr_zero_nonzero},
			{},
	};
	EOF
	clang-format -i "$outfile"
}

if ! type clang-format >/dev/null; then
	echo 'The clang-format command is required.'
	exit 1
fi
main "$@"
