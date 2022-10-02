#!/bin/bash

set -euxo pipefail

sudo apt install \
  gcc \
  g++ \
  clang-12 \
  cmake \
  pkg-config \
  libunwind-dev \
  llvm-12-dev \
  libclang-12-dev \
  libclang-cpp12-dev \
  libncurses-dev \
  libboost-system-dev \
  libboost-filesystem-dev \
  libctemplate-dev \
  libdw-dev \
  doxygen \
  graphviz

git submodule init
git submodule update
cmake -B build
pushd build
make -j`nproc` all
timeout 10s ./tests/testKernel -s

