#!/usr/bin/python3
import os 
import subprocess
from time import sleep
from threading import Thread
import queue 
import time


configure_command = "cd ../../ && ./waf clean && CXXFLAGS=\"-O3 -w\" ./waf -d optimized configure --enable-examples --enable-modules=lte --disable-python"

simulation_command = "cd ../../ && ./waf --run \"lena-nb-csv-based-scenario"

callgrind_command = "cd ../../ && ./waf --command-template\"valgrind --tool=callgrind  \%\s\" --run \"lena-nb-udp-data-transfer"


class SimulationParameters:
    def __init__(self, simulation, simTime, randomSeed, path):
        self.simulation = simulation # .cc file to execute
        self.simTime = simTime # in MilliSeconds
        self.randomSeed = randomSeed
        self.path = path


    def generateExecutableCall(self):
        #--numUesCe0={params[0]} --numUesCe1={params[1]} --numUesCe2={params[2]} --simTime={simTime} --randomSeed={seed}
        call = self.simulation 
        call += f" --simTime={self.simTime}"
        call += f" --randomSeed={self.randomSeed}"
        call += f" --scenario=1"
        call += f" --path={self.path}"
        return call

class TaskQueue(queue.Queue):

    def __init__(self, num_workers=1):
        queue.Queue.__init__(self)
        self.num_workers = num_workers

    def add_task(self, task, *args, **kwargs):
        args = args or ()
        kwargs = kwargs or {}
        self.put((task, args, kwargs))

    def start_workers(self):
        for i in range(self.num_workers):
            print("Bla")
            t = Thread(target=self.worker, args=(i,))
            t.daemon = True
            t.start()

    def worker(self, id):
        while True:
            sleep(5) 
            simulationParameters = self.get()
            print(simulationParameters)
            cmd = simulationParameters[0].generateExecutableCall()
            cmd += f" --worker={id}\""
            try:
                result = subprocess.run(cmd, shell=True, check=True)
                if result.returncode != 0:
                    print("BLALALALALA")
                    self.put(simulationParameters)
            except:
                # Task failed, maybe because of missing resources
                # put simulation back on stac
                print("BLALALALALA")
                self.put(simulationParameters)

            self.task_done()

start_time = time.time()
simTime =300 
simu_queue = TaskQueue(20)
seed =10 
to_simulate = "../../scenarios"
for i in range(1,seed):
    for filename in os.listdir(to_simulate):
        print(filename)
        filepath=to_simulate[6:]+"/"+filename
        filepath.replace("/","\/")
        filepath = "\'"+filepath+"\'"
        print(filepath)
        simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=simulation_command, randomSeed=i,path=to_simulate[6:]+"/"+filename))

    #simu_queue.add_task(SimulationParameters(numUesCe0=10,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=20,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=30,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=40,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=50,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=60,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=70,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=80,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=90,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=100,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=200,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=300,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=400,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=500,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=600,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=700,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=800,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=900,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=1000,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))

    #simu_queue.add_task(SimulationParameters(numUesCe1=10,  numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=20,  numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=30,  numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=40,  numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=50,  numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=60,  numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=70,  numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=80,  numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=90,  numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=100, numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=200, numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=300, numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=400, numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=500, numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=600, numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=700, numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=800, numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=900, numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe1=1000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))

    #simu_queue.add_task(SimulationParameters(numUesCe0=10, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=20, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=30, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=40, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=50, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=60, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=70, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=80, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=90, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=100, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=200, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=300, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=400, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=500, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=600, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=700, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=800, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=900, numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=1000,numUesCe1=1,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))

    #simu_queue.add_task(SimulationParameters(numUesCe0=10, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=20, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=30, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=40, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=50, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=60, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=70, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=80, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=90, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=100, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=200, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=300, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=400, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=500, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=600, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=700, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=800, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=900, numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
    #simu_queue.add_task(SimulationParameters(numUesCe0=1000,numUesCe1=5,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))
simu_queue.start_workers()

simu_queue.join()
print("--- %s seconds ---" % (time.time() - start_time))
#simmulation_runs = [(1,0,0), (0,1,0), (0,0,1),
#                    (2,0,0), (0,2,0), (0,0,2),
#                    (5,0,0), (0,5,0), (0,0,5),
#                    (10,0,0), (0,10,0), (0,0,10),
#                    (20,0,0), (0,20,0), (0,0,20),
#                    (50,0,0), (0,50,0), (0,0,50),
#                    (100,0,0), (0,100,0), (0,0,100),
#                    (200,0,0), (0,200,0), (0,0,200),
#                    (500,0,0), (0,500,0), (0,0,500),
##                    (1000,0,0), (0,1000,0), (0,0,1000),
#                    (1,1,1), #(1,1,1), (1,1,1),
#                    (2,2,2), #(2,2,2), (2,2,2),
#                    (5,5,5), #(5,5,5), (5,5,5),
#                    (10,10,10), #(10,10,10), (10,10,10),
#                    (20,20,20), #(20,20,20), (20,20,20),
#                    (50,50,50), #(50,50,50), (50,50,50),
#                    (100,100,100), #(100,100,100), (100,100,100),
#                    (200,200,200), #(200,200,200), (200,200,200),
#                    ]
#simmulations_run = [(1,0,0), (0,1,0),
#                    (2,0,0), (0,2,0),
#                    (5,0,0), (0,5,0),
#                    (10,0,0), (0,10,0),
#                    (20,0,0), (0,20,0),
#                    (50,0,0), (0,50,0),
#                    (100,0,0), (0,100,0),
#                    (200,0,0), (0,200,0),
#                    (500,0,0), (0,500,0),
#                    (1000,0,0), (0,1000,0),
#                    (  1,1,0),
#                    (  2,1,0),
#                    (  5,1,0),
#                    ( 10,1,0),
#                    ( 20,1,0),
#                    ( 50,1,0),
#                    ( 100,1,0),
#                    ( 200,1,0),
#                    ( 500,1,0),
#                    (1000,1,0),
#                    (  1,2,0),
#                    (  2,2,0),
#                    (  5,2,0),
#                    ( 10,2,0),
#                    ( 20,2,0),
#                    ( 50,2,0),
#                    ( 100,2,0),
#                    ( 200,2,0),
#                    ( 500,2,0),
#                    (1000,2,0),
#                    (  1,5,0),
#                    (  2,5,0),
#                    (  5,5,0),
#                    ( 10,5,0),
#                    ( 20,5,0),
#                    ( 50,5,0),
#                    ( 100,5,0),
#                    ( 200,5,0),
#                    ( 500,5,0),
#                    (1000,5,0),
#                    (  1,10,0),
#                    (  2,10,0),
#                    (  5,10,0),
#                    ( 10,10,0),
#                    ( 20,10,0),
#                    ( 50,10,0),
#                    ( 100,10,0),
#                    ( 200,10,0),
#                    ( 500,10,0),
#                    (1000,10,0),
#                     (  1,20,0),
#                     (  2,20,0),
#                     (  5,20,0),
#                     ( 10,20,0),
#                     ( 20,20,0),
#                     ( 50,20,0),
#                    ( 100,20,0),
#                    ( 200,20,0),
#                    ( 500,20,0),
#                    (1000,20,0),
#                     (  1,50,0),
#                     (  2,50,0),
#                     (  5,50,0),
#                     ( 10,50,0),
#                     ( 20,50,0),
#                     ( 50,50,0),
#                    ( 100,50,0),
#                    ( 200,50,0),
#                    ( 500,50,0),
#                    (1000,50,0),
#                     ]
##simTime = 3600000
#simTime = 300
##                    (500,500,500), (500,500,500), (500,500,500),
                    
#subprocess.run(configure_command, shell=True, check=True)
#for seed in range(1,10):
#    for params in simmulations_run:
#        if simTime != 0 :
#            print("yep")
#            cmd = simulation_command[:-1] + f" --numUesCe0={params[0]} --numUesCe1={params[1]} --numUesCe2={params[2]} --simTime={simTime} --randomSeed={seed}\""
#        else:
#            cmd = simulation_command[:-1] + f" --numUesCe0={params[0]} --numUesCe1={params[1]} --numUesCe2={params[2]} --randomSeed={seed}\""
#        subprocess.run(cmd, shell=True, check=True)
#        sleep(5)
#
