#!/usr/bin/python3
import subprocess
from time import sleep
from threading import Thread
import queue 
import time


configure_command = "cd ../../ && ./waf clean && CXXFLAGS=\"-O3 -w\" ./waf -d optimized configure --enable-examples --enable-modules=lte --disable-python"

simulation_command = "cd ../../ && ./waf --run \"lena-nb-udp-data-transfer --scenario 0"

class SimulationParameters:
    def __init__(self, numUesCe0, numUesCe1, numUesCe2, simulation, simTime, randomSeed):
        self.numUesCe0 = numUesCe0 
        self.numUesCe1 = numUesCe1 
        self.numUesCe2 = numUesCe2 
        self.simulation = simulation # .cc file to execute
        self.simTime = simTime # in MilliSeconds
        self.randomSeed = randomSeed

    def generateExecutableCall(self):
        #--numUesCe0={params[0]} --numUesCe1={params[1]} --numUesCe2={params[2]} --simTime={simTime} --randomSeed={seed}
        call = self.simulation 
        call += f" --numUesCe0={self.numUesCe0}"
        call += f" --numUesCe1={self.numUesCe1}"
        call += f" --numUesCe2={self.numUesCe2}"
        call += f" --simTime={self.simTime}"
        call += f" --randomSeed={self.randomSeed}"
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
        print("blub")
        print(id)
        while True:
            print("grrr")
            sleep(5) 
            print("graaa")
            simulationParameters = self.get()
            print(simulationParameters)
            print("grooo")
            cmd = simulationParameters[0].generateExecutableCall()
            cmd += f" --worker={id}\""
            print("Hallo")
            subprocess.run(cmd, shell=True, check=True)
            self.task_done()

start_time = time.time()
simTime = 300
simu_queue = TaskQueue(5)
seed = 11
for i in range(1,seed):
    simu_queue.add_task(SimulationParameters(numUesCe0=10,numUesCe1=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i))

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