#!/usr/bin/python3
import subprocess
from time import sleep
from threading import Thread
import queue 
import time


configure_command = "cd ../../ && ./waf clean && CXXFLAGS=\"-O3 -w\" ./waf -d optimized configure --enable-examples --enable-modules=lte --disable-python"

simulation_command = "cd ../../ && ./waf --run \"lena-nb-udp-data-transfer --scenario 0"

callgrind_command = "cd ../../ && ./waf --command-template\"valgrind --tool=callgrind  \%\s\" --run \"lena-nb-udp-data-transfer"


class SimulationParameters:
    def __init__(self, numUesCe0, numUesCe1, numUesCe2, simulation, simTime, randomSeed,edt, ciot):
        self.numUesCe0 = numUesCe0 
        self.numUesCe1 = numUesCe1 
        self.numUesCe2 = numUesCe2 
        self.simulation = simulation # .cc file to execute
        self.simTime = simTime # in MilliSeconds
        self.randomSeed = randomSeed
        self.edt = edt
        self.ciot = ciot

    def generateExecutableCall(self):
        #--numUesCe0={params[0]} --numUesCe1={params[1]} --numUesCe2={params[2]} --simTime={simTime} --randomSeed={seed}
        call = self.simulation 
        call += f" --numUesCe0={self.numUesCe0}"
        call += f" --numUesCe1={self.numUesCe1}"
        call += f" --numUesCe2={self.numUesCe2}"
        call += f" --simTime={self.simTime}"
        call += f" --randomSeed={self.randomSeed}"
        call += f" --scenario=0"
        call += f" --edt={str(self.edt).lower()}"
        call += f" --ciot={str(self.ciot).lower()}"
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
            result = subprocess.run(cmd, shell=True, check=False)
            if result.returncode != 0:
                # Task failed, maybe because of missing resources
                # put simulation back on stack
                self.put(simulationParameters)

            self.task_done()

start_time = time.time()
simTime = 600 
simu_queue = TaskQueue(4)
seed = 2
at_least = 100

for i in range(1,100):
    simu_queue.add_task(SimulationParameters(numUesCe1=1,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=1,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=1,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

for i in range(1,10):
    simu_queue.add_task(SimulationParameters(numUesCe1=10,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=10,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=10,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=20,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=20,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=20,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=30,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=30,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=30,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=40,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=40,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=40,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=50,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=50,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=50,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=60,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=60,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=60,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=70,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=70,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=70,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=80,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=80,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=80,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=90,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=90,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=90,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

for i in range(1,5):
    simu_queue.add_task(SimulationParameters(numUesCe1=100,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=100,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=100,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=200,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=200,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=200,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=300,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=300,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=300,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=400,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=400,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=400,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=500,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=500,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=500,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=600,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=600,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=600,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=700,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=700,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=700,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=800,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=800,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=800,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=900,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=900,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=900,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

for i in range(1,2):
    simu_queue.add_task(SimulationParameters(numUesCe1=1000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=1000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=1000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=2000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=2000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=2000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=3000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=3000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=3000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=4000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=4000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=4000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=5000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=5000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=5000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=6000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=6000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=6000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=7000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=7000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=7000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=8000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=8000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=8000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))

    simu_queue.add_task(SimulationParameters(numUesCe1=9000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=9000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=True,ciot=False))
    simu_queue.add_task(SimulationParameters(numUesCe1=9000,numUesCe0=0,numUesCe2=0,simulation=simulation_command,simTime=simTime,randomSeed=i,edt=False,ciot=True))



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
