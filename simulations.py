#!/usr/bin/python3
import subprocess
from time import sleep


configure_command = "cd ../../ && ./waf clean && CXXFLAGS=\"-O3 -w\" ./waf -d optimized configure --enable-examples --enable-modules=lte --disable-python"

simulation_command = "cd ../../ && ./waf --run \"lena-nb-connection-setup --scenario 0\""

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
simmulations_run = [(1,0,0), (0,1,0),
                    (2,0,0), (0,2,0),
                    (5,0,0), (0,5,0),
                    (10,0,0), (0,10,0),
                    (20,0,0), (0,20,0),
                    (50,0,0), (0,50,0),
                    (100,0,0), (0,100,0),
                    (  1,1,0),
                    (  2,1,0),
                    (  5,1,0),
                    ( 10,1,0),
                    ( 20,1,0),
                    ( 50,1,0),
                    (100,1,0),
                    (  1,2,0),
                    (  2,2,0),
                    (  5,2,0),
                    ( 10,2,0),
                    ( 20,2,0),
                    ( 50,2,0),
                    (100,2,0),
                    (  1,5,0),
                    (  2,5,0),
                    (  5,5,0),
                    ( 10,5,0),
                    ( 20,5,0),
                    ( 50,5,0),
                    (100,5,0),
                    (  1,10,0),
                    (  2,10,0),
                    (  5,10,0),
                    ( 10,10,0),
                    ( 20,10,0),
                    ( 50,10,0),
                    (100,10,0),

                     ]
#simTime = 3600000
simTime = 300
#                    (500,500,500), (500,500,500), (500,500,500),
                    
#subprocess.run(configure_command, shell=True, check=True)
for seed in range(1,10):
    for params in simmulations_run:
        if simTime != 0 :
            print("yep")
            cmd = simulation_command[:-1] + f" --numUesCe0={params[0]} --numUesCe1={params[1]} --numUesCe2={params[2]} --simTime={simTime} --randomSeed={seed}\""
        else:
            cmd = simulation_command[:-1] + f" --numUesCe0={params[0]} --numUesCe1={params[1]} --numUesCe2={params[2]} --randomSeed={seed}\""
        subprocess.run(cmd, shell=True, check=True)
        sleep(5)
