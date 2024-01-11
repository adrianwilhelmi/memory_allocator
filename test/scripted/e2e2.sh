#!/bin/bash

source test_lib.sh

test_e2e_2(){
	file_path="results/e2e_result2.txt"

	read -r -a expected_messages <<< $(grep "expected" $file_path | awk '{$1= ""; print $0}')
	read -r -a actual_messages <<< $(grep "actual" $file_path | awk '{$1= ""; print $0}')

	compare_line expected_messages[@] actual_messages[@]
}
