#!/bin/bash

source test_lib.sh


test_e2e_1(){
	file_path="results/e2e_result1.txt"

	read -r -a expected_stats <<< $(grep -A 2 "EXPECTED ALLOCATOR STATS" $file_path | tail -1)
	read -r -a actual_stats <<< $(grep -A 2 "ALLOCATOR STATS" $file_path | tail -1)

	compare_line expected_stats[@] actual_stats[@]
}
