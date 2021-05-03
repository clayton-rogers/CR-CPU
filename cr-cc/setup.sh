#!/bin/bash

pushd x64/Debug
export PATH=$PATH:$(pwd)
popd


pushd stdlib
export CRSTDLIBPATH=$(pwd)
popd

