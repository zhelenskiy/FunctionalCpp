cmake_minimum_required(VERSION 3.14)
project(Functional)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-g -Wa,-mbig-obj -O1 -ftemplate-backtrace-limit=0")
add_executable(Functional main.cpp LazySeq.hpp LazySeq.h OrderedLazySeq.hpp OrderedLazySeq.h main.h ReversedLazySeq.hpp ReversedLazySeq.h LazySeqLib.h Lambdas.h SmartFunction.h SliceHolder.h)