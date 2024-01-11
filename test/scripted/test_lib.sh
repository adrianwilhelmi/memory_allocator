#!/bin/bash

tests_ok=1

compare_line(){
	local line1=("${!1}")
	local line2=("${!2}")
	
	local length=${#line1[@]}
	
	for (( i=0; i<length; i++ )); do
		if [ "${line1[i]}" != "${line2[i]}" ]; then
			echo "TEST FAILURE: $2."
			echo "expected : ${line1[i]}"
			echo "got: ${line2[i]}"
			tests_ok=0
		fi
	done
}

finalize_tests(){
	if [ $tests_ok -eq 1 ]; then
		echo "scripted tests ok"
	fi
}
