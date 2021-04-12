#!/usr/bin/python3
import subprocess
from time import sleep


configure_command = "cd ../../ && ./waf clean && CXXFLAGS=\"-O3 -w\" ./waf -d optimized configure --enable-examples --enable-modules=lte --disable-python"

simulation_command = "cd ../../ && ./waf --run \"lena-simple-epc --scenario 0\""

simmulation_runs = [(1,0,0), (0,1,0), (0,0,1),
                    (2,0,0), (0,2,0), (0,0,2),
                    (5,0,0), (0,5,0), (0,0,5),
                    (10,0,0), (0,10,0), (0,0,10),
                    (20,0,0), (0,20,0), (0,0,20),
                    (50,0,0), (0,50,0), (0,0,50),
                    (100,0,0), (0,100,0), (0,0,100),
                    (200,0,0), (0,200,0), (0,0,200),
                    (500,0,0), (0,500,0), (0,0,500),
#                    (1000,0,0), (0,1000,0), (0,0,1000),
                    (1,1,1), #(1,1,1), (1,1,1),
                    (2,2,2), #(2,2,2), (2,2,2),
                    (5,5,5), #(5,5,5), (5,5,5),
                    (10,10,10), #(10,10,10), (10,10,10),
                    (20,20,20), #(20,20,20), (20,20,20),
                    (50,50,50), #(50,50,50), (50,50,50),
                    (100,100,100), #(100,100,100), (100,100,100),
                    (200,200,200), #(200,200,200), (200,200,200),
                    ]
#simTime = 3600000
simTime = 360
#                    (500,500,500), (500,500,500), (500,500,500),
                    
#subprocess.run(configure_command, shell=True, check=True)
for seed in range(1,10):
    for params in simmulation_runs:
        if simTime != 0 :
            print("yep")
            cmd = simulation_command[:-1] + f" --numUesCe0={params[0]} --numUesCe1={params[1]} --numUesCe2={params[2]} --simTime={simTime} --randomSeed={seed}\""
        else:
            cmd = simulation_command[:-1] + f" --numUesCe0={params[0]} --numUesCe1={params[1]} --numUesCe2={params[2]} --randomSeed={seed}\""
        subprocess.run(cmd, shell=True, check=True)
        sleep(5)
