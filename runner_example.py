#!/usr/bin/python3
import os 
import subprocess
from time import sleep
from threading import Thread
import queue 
import time


configure_command = "cd ../../ && ./waf clean && CXXFLAGS=\"-O3 -w\" ./waf -d optimized configure --enable-examples --enable-modules=lte --disable-python"

simulation_command = "cd ../../ && ./waf --run \"lena-nb-5G-scenario"

callgrind_command = "cd ../../ && ./waf --command-template\"valgrind --tool=callgrind  \%\s\" --run \"lena-nb-udp-data-transfer"

#sim_command = "cd ../../ && ./build/src/lte/examples/ns3.32-lena-nb-5G-scenario-optimized"
sim_command = "cd ../../ && ./build/src/lte/examples/ns3.32-lena-nb-5G-scenario-debug"


class SimulationParameters:
    def __init__(self, simulation, simTime, randomSeed, num_ues_app_a, num_ues_app_b, num_ues_app_c,ciot,edt):
        self.simulation = simulation # .cc file to execute
        self.simTime = simTime # in MilliSeconds
        self.randomSeed = randomSeed # For Random Number Generator
        self.num_ues_app_a = num_ues_app_a # Number of UEs for the first application / use cases
        self.num_ues_app_b = num_ues_app_b # Number of UEs for the second application / use cases
        self.num_ues_app_c = num_ues_app_c # Number of UEs for the third application / use cases
        self.ciot = ciot # If Cellular IoT Optimization should be used
        self.edt = edt # If Early Data Transmission should be used


    def generateExecutableCall(self):
        call = self.simulation 
        call += f" --simTime={self.simTime}"
        call += f" --randomSeed={self.randomSeed}"
        call += f" --numUeAppA={self.num_ues_app_a}"
        call += f" --numUeAppB={self.num_ues_app_b}"
        call += f" --numUeAppC={self.num_ues_app_c}"
        call += f" --ciot={self.ciot}"
        call += f" --edt={self.edt}"
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
            cmd += f" --worker={id}"
            print(cmd)
            try:
                result = subprocess.run(cmd, shell=True, check=True)
                if result.returncode != 0:
                    # Task failed, maybe because of missing resources
                    # put simulation back on stac
                    print(f"Something failed! Return code: {result.returncode}. Output: {result.stdout}. Error: {result.stderr}")
                    self.put(simulationParameters)
            except:
                # Task failed, maybe because of missing resources
                # put simulation back on stac
                print("Something failed due to unknown exception...")
                self.put(simulationParameters)

            self.task_done()

start_time = time.time()
simTime = 300 # Simulation time in seconds
simu_queue = TaskQueue(3) # This is the number of parallel workers. This number should be below your number of CPU cores. Note that more parallel workers consume more RAM
seed = 1 # Number of runs. 5 means that simulations with seeds 1,2,3,4,5 will be started
for i in range(1,seed+1):
    # Place here your different scenario setups. In this example 15 devices are simulated using standard transmission, C-IoT Opt. or EDT
    simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i, num_ues_app_a=5, num_ues_app_b=5, num_ues_app_c=5, ciot=False, edt=False))
    simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i, num_ues_app_a=5, num_ues_app_b=5, num_ues_app_c=5, ciot=True, edt=False))
    simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i, num_ues_app_a=5, num_ues_app_b=5, num_ues_app_c=5, ciot=True, edt=True))
simu_queue.start_workers()

simu_queue.join()
print("--- %s seconds ---" % (time.time() - start_time))