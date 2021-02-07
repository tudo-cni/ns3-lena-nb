#!/bin/bash
cd ../../ && NS_LOG="LteUeRrc=level_all|time" ./waf --run lena-simple-epc 2> test.log
