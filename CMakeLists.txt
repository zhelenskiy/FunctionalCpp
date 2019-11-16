cmake_minimum_required(VERSION 3.14)
project(Functional)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wa,-mbig-obj -O0")
add_executable(Functional main.cpp LazySeq.hpp LazySeq.h OrderedLazySeq.hpp OrderedLazySeq.h main.h ReversedLazySeq.hpp ReversedLazySeq.h LazySeqLib.h Lambdas.h SimpleCopyFunction.h)