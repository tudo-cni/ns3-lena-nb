#!/bin/bash
cd ../../ && ./waf distclean && ./waf configure --disable-python --enable-examples --enable-module=lte


#cd ../../ && ./waf distclean && CXXFLAGS="-O3 -w" ./waf -d optimized configure --enable-static --enable-examples --enable-modules=lte
#cd ../../ && ./waf clean && ./waf distclean && 
#cd ../../ && CXXFLAGS="-w" ./waf -d optimized configure --enable-examples --enable-modules=lte

#cd ../../ && ./waf clean && CXXFLAGS="-O3 -w" ./waf -d optimized configure --enable-examples --enable-modules=lte
