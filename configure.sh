#!/bin/bash
cd ../../ && ./waf clean && ./waf configure --disable-python --enable-examples --enable-module=lte
#cd ../../ && ./waf configure --disable-python --enable-examples --enable-module=lte


#cd ../../ && ./waf distclean && CXXFLAGS="-O3 -w" ./waf -d optimized configure --enable-static --enable-examples --enable-modules=lte
#cd ../../ && ./waf clean && ./waf distclean && 
#cd ../../ && ./waf clean && CXXFLAGS="-O3 -g" ./waf -d optimized configure --enable-examples --enable-modules=lte --disable-python

#cd ../../ && ./waf clean && CXXFLAGS="-O3 -w -g" ./waf -d optimized configure --enable-examples --enable-modules=lte --disable-python
