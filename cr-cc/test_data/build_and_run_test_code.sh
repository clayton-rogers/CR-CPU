#!/bin/bash -e


sim_dirs=(
	#"echo"  # missing devices in sim
	#"flash"  # buildin sleeps take too long
	#"map_test"  # can't load multiple bin into sim
	"mpmp_cats"
	#"mpmp_marching_band" # takes too long???
	#"mpmp_no_squares" # takes too long???
	#"mpmp_triangle" # takes too long???
	#"os_link"  # sim does not use os yet
	"sieve_of_eratosthenese"
	"universal_makefile"
	"spi_test"
	)

build_only=(
	"flash"
	"echo"
	"pong"
	"os_link"
	"map_test"
	"mpmp_marching_band"
	"mpmp_no_squares"
	"mpmp_triangle"
	"fake_os"
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
