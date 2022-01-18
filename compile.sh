#!/bin/bash
cd ../../ && NS_LOG="UdpEchoClientApplication=level_info|time" ./waf --run lena-nb-5G-scenario 2> test.log
#cd ../../ && NS_LOG="LteEnbRrc=level_info|time" ./waf --run lena-simple-epc 2> test.log
