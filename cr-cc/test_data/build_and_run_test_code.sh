#!/bin/bash -e


sim_dirs=(
	#"echo"  # missing devices in sim
	"flash"
	#"map_test"  # compiler bug, duplicate symbol
	"mpmp_cats"
	#"mpmp_marching_band" # takes too long???
	#"mpmp_no_squares" # takes too long???
	#"mpmp_triangle" # takes too long???
	#"os_link"  # sim does not use os yet
	"sieve_of_eratosthenese"
	"universal_makefile"
	)

build_only=(
	"echo"
	"pong"
	"os_link"
	"mpmp_marching_band"
	"mpmp_no_squares"
	"mpmp_triangle"
	)

for i in "${sim_dirs[@]}"
do
	echo "===== Now running: " $i
	pushd $i > /dev/null
	make clean
	make sim
	popd > /dev/null
done

for i in "${build_only[@]}"
do
	echo "===== Now building: " $i
	pushd $i > /dev/null
	make clean
	make
	popd > /dev/null
done
