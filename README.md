# LENA-NB (Narrowband)
This repository includes LENA-NB, an NB-IoT extension to ns-3. Currently, the following features are implemented:

- RRC Connection Resume Procedure (3GPP Rel. 13)
- Cellular IoT Optimization (3GPP Rel. 13)
- Early Data Transmission (3GPP Rel. 15)
- Cross-Subframe Scheduling (including NPSS, NSSS, NPBCH, NPDCCH, NPDSCH & NPUSCH)
- Adaptive Modulation and Coding 
- Random Access Procedure with Coverage Enhancement Levels
- NB-IoT Energy State Machine (including eDRX and PSM)

If you are interested in the specific changes that were made to the original LENA source code, see commit [Implementation of NB-IoT
](https://github.com/tudo-cni/ns3-lena-nb/commit/aebee244bdc8f95264b85df34042edcb393057b1) and onwards for reference. Furthermore, additional information about the design decisions and the overall structure of LENA-NB can be found in the provided [document](https://github.com/tudo-cni/ns3-lena-nb/blob/main/LENA-NB_Additional_Information.pdf).

Currently, no NB-IoT specific error model is used. Instead, MATLAB was used to provide adequate configurations depending on a given SNR [1,2]. 
A lookup table is derived from MATLAB NB-IoT BLER simulations to find a decent UL and DL configuration corresponding to a target BLER.
In future work, an NB-IoT specific error model will be integrated in LENA-NB.

# Publications using LENA-NB
- P. Jörke, T. Gebauer, and C. Wietfeld, ‘From LENA to LENA-NB: Implementation and Performance Evaluation of NB-IoT and Early Data Transmission in ns-3’, in Proceedings of the 2022 Workshop on ns-3, 2022, pp. 73–80.
- P. Jörke, T. Gebauer, S. Böcker, and C. Wietfeld, ‘Scaling Dense NB-IoT Networks to the Max: Performance Benefits of Early Data Transmission’, in 2022 IEEE 95th Vehicular Technology Conference:(VTC2022-Spring), 2022, pp. 1–7.
- P. Jörke, D. Ronschka, and C. Wietfeld, ‘Performance Evaluation of Random Access for Small Data Transmissions in Highly Dense Public and Private NB-IoT Networks’, in 2023 IEEE 97th Vehicular Technology Conference (VTC2023-Spring), 2023, pp. 1–7.
- M. Štůsek et al., ‘Exploiting NB-IoT Network Performance and Capacity for Smart-Metering Use-Cases’, in In proceedings of the 15th International Congress on Ultra Modern Telecommunications and Control Systems and Workshops (ICUMT)., 2023, pp. 193–199.
- M.T. Abbas et al., ‘Evaluating the Impact of Pre-Configured Uplink Resources in Narrowband IoT.‘, in Sensors 2024, 24, 5706.
- P. Masek et al., ‘Quantifying NB-IoT Performance in 5G Use-Cases With Mixture of Regular and Stochastic Traffic‘, in IEEE Internet of Things Journal
  
If you use this code or results in your publications, please cite our work as mentioned in [Citation](#citation). Also, if you do not find your work in this list, please open a merge request. 
# How to use LENA-NB 

LENA-NB is based on the ns-3 release 3.32. We recommend using LENA-NB with ns-3 release 3.32, since other releases may have introduced changes in the source code 
which may interfere with our provided code. 
After downloading and building ns-3 the original lte folder, found in ns3/bake/source/ns-3.32/src, is replaced with the lte folder provided in this respository. 
Note that currently LENA-NB replaces the ns-3 LTE implementation. In future releases a coexistence will be introduced. 

# First Steps 

We have included an example script, called lena-nb-5G-scenario.cc, which can be used for testing the NB-IoT implementation in ns-3. This script expects the 
following parameters:

```
--simTime       # Time to be simulated, in milliseconds
--randomSeed    # Seed for Random Number Generator
--numUeAppA     # Number of UEs for the first application / use cases
--numUeAppB     # Number of UEs for the second application / use cases
--numUeAppC     # Number of UEs for the third application / use cases
--ciot          # Flag whether Cellular IoT Optimization should be used
--edt           # Flag whether Early Data Transmission should be used
```

With these parameters NB-IoT UEs are uniformly distributed on a disc with a diameter of ```cellsize=2500m```. Each UE transmits once a day a user payload 
of ```packetsize_app_a=49 Bytes```. Tn this case the payload for App A, B and C are equal, but can be changed in the script.

Additional Note: 
Our implementation uses an additional propagation model, which is available here: https://github.com/tudo-cni/ns3-propagation-winner-plus and needs to be included in the code for successful building the code.

## Note regarding actual Simulation Time
Note that the actual simulation time is 3*simTime. If simTime is set to 5 minutes, 15 minutes of simulation time are simulated, which is important for high-scaled 
scenarios. The first 5 minutes produce no significant results since devices at the beginning are scheduled in an empty cell and experience very good transmission 
conditions. After 5 minutes, new devices will find ongoing transmissions of previous devices, which enables a more realistic situation and produces significant 
results. Since devices that have started transmissions within the intermediate 5 minutes of the simulation may not complete their transmissions in this intermediate 
time slot, additional 5 minutes are simulated with more new transmissions to keep the channels busy and let the intermediate devices complete their transmissions.

# Automated Simulations  

For an automated execution of multiple simulations with different seeds and configurations, we provice a python script, called runner_example.py. This script 
generates the required lena-nb-5G-scenario.cc calls including all parameters. All simulations are queued using a class called taskQueue, which executes the simulations 
one-by-one and automatically restarts a simulation if an error occured. The parameter num_workers is used for parallel simulations. If the simulations are used by 
servers with e.g. 50 cores, the up to 50 workers can be set for up to 50 parallel simulations. Note that multiple parallel simulations consume much memory. 

# Citation
If you use this code or results in your paper, please cite our work as:
```
@inproceedings{10.1145/3532577.3532600,
author = {J\"{o}rke, Pascal and Gebauer, Tim and Wietfeld, Christian},
title = {From LENA to LENA-NB: Implementation and Performance Evaluation of NB-IoT and Early Data Transmission in Ns-3},
year = {2022},
isbn = {9781450396516},
publisher = {Association for Computing Machinery},
address = {New York, NY, USA},
url = {https://doi.org/10.1145/3532577.3532600},
doi = {10.1145/3532577.3532600},
pages = {73–80},
numpages = {8},
keywords = {Internet of Things, LENA-NB, Early Data Transmission, NB-IoT, implementation, ns-3, performance evaluation, simulation},
location = {Virtual Event, USA},
series = {WNS3 '22}
}
```

# Acknowledgement
This work has been carried out in the course of the PuLS project, funded by the Federal Ministry of Transport and Digital Infrastructure (BMVI) under grant agreement no. 03EMF0203B and the project 5hine and Competence Center 5G.NRW, funded by means of the Federal State NRW by the Ministry for Economic Affairs, Innovation, Digitalization and Energy (MWIDE) under the Funding ID 005-2108-0073 resp. 005-01903-0047, and was supported by the Collaborative Research Center SFB 876 “Providing Information by Resource-Constrained Analysis”, project A4.

# References

[1] MathWorks. 2021. NB-IoT NPDSCH Block Error Rate Simulation. Retrieved December 11, 2021 from https://www.mathworks.com/help/lte/ug/nb-iot-npdsch-block-error-rate-simulation.html
[2] MathWorks. 2021. NB-IoT NPUSCH Block Error Rate Simulation. Retrieved December 11, 2021 from https://www.mathworks.com/help/lte/ug/nb-iot-npusch-block-error-rate-simulation.html
