#!/bin/bash
#cd ../../ && ./waf configure --disable-python --enable-examples --enable-module=lte


cd ../../ && ./waf distclean && CXXFLAGS="-O3 -w" ./waf -d optimized configure --enable-static --enable-examples --enable-modules=lte