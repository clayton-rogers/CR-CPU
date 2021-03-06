#!/bin/bash -e

# The OS image is stored in git and needs to be updated before building the
# FPGA, thus we check each time the stdlib is built to make sure the os
# is up to date.

fpga_os="../../fpga/top.hex"
os="os.hex"

if cmp -s $fpga_os $os ; then
	echo "OS VERIFIED"
else
	echo "*** OS DIFFERENT ***"
	exit 1;
fi
