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

sim_command = "cd ../../ && ./build/src/lte/examples/ns3.32-lena-nb-5G-scenario-optimized"


class SimulationParameters:
    def __init__(self, simulation, simTime, randomSeed, path, num_ues_app_a, num_ues_app_b, num_ues_app_c):
        self.simulation = simulation # .cc file to execute
        self.simTime = simTime # in MilliSeconds
        self.randomSeed = randomSeed
        self.path = path
        self.num_ues_app_a = num_ues_app_a
        self.num_ues_app_b = num_ues_app_b
        self.num_ues_app_c = num_ues_app_c


    def generateExecutableCall(self):
        #--numUesCe0={params[0]} --numUesCe1={params[1]} --numUesCe2={params[2]} --simTime={simTime} --randomSeed={seed}
        call = self.simulation 
        call += f" --simTime={self.simTime}"
        call += f" --randomSeed={self.randomSeed}"
        call += f" --scenario=1"
        call += f" --path={self.path}"
        call += f" --numUeAppA={self.num_ues_app_a}"
        call += f" --numUeAppB={self.num_ues_app_b}"
        call += f" --numUeAppC={self.num_ues_app_c}"
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
            #print("Bla")
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
                    print("Something failed")
                    self.put(simulationParameters)
            except:
                # Task failed, maybe because of missing resources
                # put simulation back on stac
                print("Something Failed")
                self.put(simulationParameters)

            self.task_done()

start_time = time.time()
simTime =300
simu_queue = TaskQueue(40)
seed =16
#num_ues_app_a = 10 # Outdoor
#num_ues_app_b = 10 # Indoor
##num_ues_app_c = 10 # Deep Indoor
to_simulate = "../../scenarios"
for i in range(1,seed):
    for filename in os.listdir(to_simulate):
        #print(filename)
        #filepath=to_simulate[6:]+"/"+filename
        #filepath.replace("/","\/")
        #filepath = "\'"+filepath+"\'"
        #print(filepath)
        simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i,path=to_simulate[6:]+"/"+filename, num_ues_app_a=1, num_ues_app_b=1, num_ues_app_c=1))
        simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i,path=to_simulate[6:]+"/"+filename, num_ues_app_a=2, num_ues_app_b=2, num_ues_app_c=2))
        simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i,path=to_simulate[6:]+"/"+filename, num_ues_app_a=5, num_ues_app_b=5, num_ues_app_c=5))
        simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i,path=to_simulate[6:]+"/"+filename, num_ues_app_a=10, num_ues_app_b=10, num_ues_app_c=10))
        simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i,path=to_simulate[6:]+"/"+filename, num_ues_app_a=20, num_ues_app_b=20, num_ues_app_c=20))
        simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i,path=to_simulate[6:]+"/"+filename, num_ues_app_a=50, num_ues_app_b=50, num_ues_app_c=50))
        simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i,path=to_simulate[6:]+"/"+filename, num_ues_app_a=100, num_ues_app_b=100, num_ues_app_c=100))
        simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i,path=to_simulate[6:]+"/"+filename, num_ues_app_a=200, num_ues_app_b=200, num_ues_app_c=200))
        simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i,path=to_simulate[6:]+"/"+filename, num_ues_app_a=500, num_ues_app_b=500, num_ues_app_c=500))
        simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i,path=to_simulate[6:]+"/"+filename, num_ues_app_a=1000, num_ues_app_b=1000, num_ues_app_c=1000))
        simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i,path=to_simulate[6:]+"/"+filename, num_ues_app_a=2000, num_ues_app_b=2000, num_ues_app_c=2000))
        simu_queue.add_task(SimulationParameters(simTime=simTime,simulation=sim_command, randomSeed=i,path=to_simulate[6:]+"/"+filename, num_ues_app_a=5000, num_ues_app_b=5000, num_ues_app_c=5000))

simu_queue.start_workers()

simu_queue.join()
print("--- %s seconds ---" % (time.time() - start_time))