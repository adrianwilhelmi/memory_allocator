#!/bin/bash

for file in *.sh; do
	if [ "$file" != "test_scripted.sh" ]; then
		[ -f "$file" ] && source "$file"
	fi
done

test_e2e_1
test_e2e_2

finalize_tests

