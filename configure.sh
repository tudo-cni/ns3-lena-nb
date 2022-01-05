#!/bin/bash
#cd ../../ && ./waf clean && CXXFLAGS="-Wno-error" ./waf configure --disable-python --enable-examples --enable-module=lte 
#cd ../../ && ./waf clean && CXXFLAGS="-g -march=native" ./waf configure --disable-python --enable-examples --enable-module=lte


#cd ../../ && ./waf distclean && CXXFLAGS="-O3 -w" ./waf -d optimized configure --enable-static --enable-examples --enable-modules=lte
#cd ../../ && ./waf clean && ./waf distclean && 
#cd ../../ && ./waf clean && CXXFLAGS="-O3 -g" ./waf -d optimized configure --enable-examples --enable-modules=lte --disable-python

#cd ../../ && ./waf clean && CXXFLAGS="-Ofast" ./waf -d optimized configure --enable-examples --enable-modules=lte --disable-python --enable-static
cd ../../ && ./waf clean && CXXFLAGS="-g -O3 -march=native" ./waf -d optimized configure --enable-examples --enable-modules=lte --disable-python --enable-static #####
#cd ../../ && ./waf clean && CXX="clang++" CXXFLAGS="-Ofast" ./waf -d optimized configure --enable-examples --enable-modules=lte --disable-python --enable-static
