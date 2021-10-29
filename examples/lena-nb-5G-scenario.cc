/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC), 2021 TU Dortmund University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Jaume Nin <jaume.nin@cttc.cat>
 *          Manuel Requena <manuel.requena@cttc.es>
 *          Tim Gebauer <tim.gebauer@tu-dortmund.de>
 *          Pascal Jörke <pascal.joerke@tu-dortmund.de>
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/lte-module.h"
#include <ns3/winner-plus-propagation-loss-model.h>
//#include "ns3/gtk-config-store.h"
#include <chrono>
#include <iomanip>
#include <stdlib.h>
#include <ctime>    
#include <fstream>
using namespace ns3;




/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeBs,
 * attaches one UE per eNodeB starts a flow for each UE to and from a remote host.
 * It also starts another flow between each UE pair.
 */

NS_LOG_COMPONENT_DEFINE ("LenaNb5G");

int
main (int argc, char *argv[])
{
  Time simTime = Minutes(6);
  //double distance = 50000.0;
  uint64_t ues_to_consider = 0;

  bool scenario = true;
  uint8_t worker = 0;
  int seed = 1;
  std::string path = "scenarios/release_13_and_ciot/5.0.csv";
  std::string simName = "test";
  double cellsize = 2500; // in meters
  //std::vector<std::vector<std::string>> ue_configs;
  int num_ues_app_a = 1;
  int num_ues_app_b = 2;
  int num_ues_app_c = 3;
  int packetsize_app_a = 20;
  int packetsize_app_b = 50;
  int packetsize_app_c = 100;
  Time packetinterval_app_a = Days(1);
  Time packetinterval_app_b = Days(1);
  Time packetinterval_app_c = Days(1);
  bool ciot = false;
  bool edt = false;
  // Command line arguments
  CommandLine cmd (__FILE__);
  cmd.AddValue ("simTime", "Total duration of the simulation", simTime);
  cmd.AddValue ("path", "Total duration of the simulation", path);
  cmd.AddValue ("simName", "Total duration of the simulation", simName);
  cmd.AddValue ("scenario", "1 if should use scenario csv", scenario);
  cmd.AddValue ("worker", "worker id when using multithreading to not confuse logging", worker);
  cmd.AddValue ("randomSeed", "randomSeed",seed);
  cmd.AddValue ("numUeAppA", "Number of UEs for Application A",num_ues_app_a);
  cmd.AddValue ("numUeAppB", "Number of UEs for Application B",num_ues_app_b);
  cmd.AddValue ("numUeAppC", "Number of UEs for Application C",num_ues_app_c);
  cmd.Parse (argc, argv);
  std::cout << "Number of UEs of Application A: " << num_ues_app_a << std::endl;
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // parse again so you can override default values from the command line

  //Ptr<OkumuraHataPropagationLossModel> propagationLossModel = CreateObject<OkumuraHataPropagationLossModel> ();
  //propagationLossModel->SetAttribute ("Frequency", DoubleValue (869e6));
  //propagationLossModel->SetAttribute ("Environment", EnumValue (UrbanEnvironment));
  //propagationLossModel->SetAttribute ("CitySize", EnumValue (LargeCity));

  Ptr<WinnerPlusPropagationLossModel> propagationLossModel = CreateObject<WinnerPlusPropagationLossModel> ();
  propagationLossModel->SetFrequency(869e6);
  propagationLossModel->SetAttribute ("LineOfSight", BooleanValue (false));
  propagationLossModel->SetAttribute ("Environment", EnumValue (UMaEnvironment));
  propagationLossModel->SetAttribute ("HeightBasestation", DoubleValue (20.0));

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");
  lteHelper->SetUeAntennaModelType ("ns3::IsotropicAntennaModel");
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::WinnerPlusPropagationLossModel"));
  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));
  Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));

  // if(scenario){
  //   std::ifstream scenario(path);
  //   std::string row;
  //   std::getline (scenario, row);
  //   while (!scenario.eof ())
  //     {
  //       std::getline (scenario, row);
  //       if (scenario.bad () || scenario.fail ())
  //         {
  //           break;
  //         }
  //       ue_configs.push_back(readCSVRow(row));
  //       std::vector<std::string> it= ue_configs.back();

  //       //stod(fields[3])*1000.0
  //     }

  // }

  // Calculate UES to consider
  ues_to_consider = num_ues_app_a + num_ues_app_b + num_ues_app_c;
  std::cout << "ues_to_consider: " << ues_to_consider<< std::endl;

  Ptr<Node> pgw = epcHelper->GetPgwNode ();
   // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer enbNodes;
  enbNodes.Create (1);
  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (2500, 2500, 25));

  MobilityHelper mobilityEnb;
  mobilityEnb.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityEnb.SetPositionAllocator(positionAlloc);
  mobilityEnb.Install(enbNodes);

  NodeContainer ueNodes;
  if(scenario){
    ueNodes.Create (ues_to_consider*3); // Pre-Run, Run, Post-Run
  }
  Ptr<ListPositionAllocator> positionAllocUe = CreateObject<ListPositionAllocator> ();

  for (uint32_t j = 0; j<3; j++){
    // Install Mobility Model for Application A
    ObjectFactory pos_a;
    pos_a.SetTypeId ("ns3::UniformDiscPositionAllocator");
    pos_a.Set ("X", StringValue (std::to_string(cellsize/2)));
    pos_a.Set ("Y", StringValue (std::to_string(cellsize/2)));
    pos_a.Set ("Z", StringValue ("1.5"));
    pos_a.Set ("rho", DoubleValue (cellsize/2));
    Ptr<PositionAllocator> m_position = pos_a.Create ()->GetObject<PositionAllocator> ();
    for (uint32_t i = 0; i < num_ues_app_a; ++i){
      Vector position = m_position->GetNext ();
      positionAllocUe->Add (position);
      //std::cout << "a," << position.x << "," << position.y << "," << position.z << std::endl;
    }
    // Install Mobility Model for Application B
    ObjectFactory pos_b;
    pos_b.SetTypeId ("ns3::UniformDiscPositionAllocator");
    pos_b.Set ("X", StringValue (std::to_string(cellsize/2)));
    pos_b.Set ("Y", StringValue (std::to_string(cellsize/2)));
    pos_b.Set ("Z", StringValue ("0.0"));
    pos_b.Set ("rho", DoubleValue (cellsize/2));    
    m_position = pos_b.Create ()->GetObject<PositionAllocator> ();
    for (uint32_t i = 0; i < num_ues_app_b; ++i){
      Vector position = m_position->GetNext ();
      positionAllocUe->Add (position);
      //std::cout << "b," << position.x << "," << position.y << "," << position.z << std::endl;
    }
    // Install Mobility Model for Application C
    ObjectFactory pos_c;
    pos_c.SetTypeId ("ns3::UniformDiscPositionAllocator");
    pos_c.Set ("X", StringValue (std::to_string(cellsize/2)));
    pos_c.Set ("Y", StringValue (std::to_string(cellsize/2)));
    pos_c.Set ("Z", StringValue ("-1.5"));
    pos_c.Set ("rho", DoubleValue (cellsize/2));    
    m_position = pos_c.Create ()->GetObject<PositionAllocator> ();
    for (uint32_t i = 0; i < num_ues_app_c; ++i){
      Vector position = m_position->GetNext ();
      positionAllocUe->Add (position);
      //std::cout << "c," << position.x << "," << position.y << "," << position.z << std::endl;
    }
  }
  
  MobilityHelper mobilityUe;
  //mobilityUe.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
  //                               "X", StringValue (std::to_string(cellsize/2)),
  //                               "Y", StringValue (std::to_string(cellsize/2)),
  //                               "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=" + std::to_string(cellsize) + "]"));
  mobilityUe.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityUe.SetPositionAllocator(positionAllocUe);
  mobilityUe.Install (ueNodes);



  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }
  // Attach one UE per eNodeB
  RngSeedManager::SetSeed (seed);  // Changes seed from default of 1 to 3
  Ptr<UniformRandomVariable> RaUeUniformVariable = CreateObject<UniformRandomVariable> ();






  // Install and start applications on UEs and remote host
  uint16_t ulPort = 2000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  
  
  // Pre-Run
  for (uint16_t i = 0; i < ues_to_consider; i++)
    {
      int access = RaUeUniformVariable->GetInteger (50, simTime.GetMilliSeconds());
      lteHelper->AttachSuspendedNb(ueLteDevs.Get(i), enbLteDevs.Get(0));

      Ptr<LteUeNetDevice> ueLteDevice = ueLteDevs.Get(i)->GetObject<LteUeNetDevice> ();
      Ptr<LteUeRrc> ueRrc = ueLteDevice->GetRrc();
      if(ciot == true){
        std::cout << "ciot" << std::endl;
        ueRrc->SetAttribute("CIoT-Opt", BooleanValue(true));
      }
      else{
        ueRrc->SetAttribute("CIoT-Opt", BooleanValue(false));
      }
      if(edt == true){
        std::cout << "EDT" << std::endl;
        ueRrc->SetAttribute("EDT", BooleanValue(true));
      }
      else{
        ueRrc->SetAttribute("EDT", BooleanValue(false));
      }

      ++ulPort;
      UdpEchoServerHelper server (ulPort);
      serverApps.Add(server.Install (remoteHost));
      //
      // Create a UdpEchoClient application to send UDP datagrams from node zero to
      // node one.
      //
      if (i < num_ues_app_a){
        uint packetsize = packetsize_app_a;
        UdpEchoClientHelper ulClient (remoteHostAddr, ulPort);
        ulClient.SetAttribute ("Interval", TimeValue (packetinterval_app_a));
        ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
        ulClient.SetAttribute ("PacketSize", UintegerValue(packetsize));
        clientApps.Add (ulClient.Install (ueNodes.Get(i)));

        serverApps.Get(i)->SetStartTime (MilliSeconds (access));
        clientApps.Get(i)->SetStartTime (MilliSeconds (access));
      }
      else if (i < num_ues_app_a+num_ues_app_b){
        uint packetsize = packetsize_app_b;
        UdpEchoClientHelper ulClient (remoteHostAddr, ulPort);
        ulClient.SetAttribute ("Interval", TimeValue (packetinterval_app_b));
        ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
        ulClient.SetAttribute ("PacketSize", UintegerValue(packetsize));
        clientApps.Add (ulClient.Install (ueNodes.Get(i)));

        serverApps.Get(i)->SetStartTime (MilliSeconds (access));
        clientApps.Get(i)->SetStartTime (MilliSeconds (access));
      }
      else {
        uint packetsize = packetsize_app_c;
        UdpEchoClientHelper ulClient (remoteHostAddr, ulPort);
        ulClient.SetAttribute ("Interval", TimeValue (packetinterval_app_c));
        ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
        ulClient.SetAttribute ("PacketSize", UintegerValue(packetsize));
        clientApps.Add (ulClient.Install (ueNodes.Get(i)));

        serverApps.Get(i)->SetStartTime (MilliSeconds (access));
        clientApps.Get(i)->SetStartTime (MilliSeconds (access));
      }
    }


  // UEs to be considered 
  for (uint16_t i = ues_to_consider; i < ues_to_consider*2; i++)
    {
      int access = RaUeUniformVariable->GetInteger (simTime.GetMilliSeconds(), 2*simTime.GetMilliSeconds());
      lteHelper->AttachSuspendedNb(ueLteDevs.Get(i), enbLteDevs.Get(0));

      Ptr<LteUeNetDevice> ueLteDevice = ueLteDevs.Get(i)->GetObject<LteUeNetDevice> ();
      Ptr<LteUeRrc> ueRrc = ueLteDevice->GetRrc();
      ueRrc->EnableLogging();
      if(ciot == true){
        std::cout << "ciot" << std::endl;
        ueRrc->SetAttribute("CIoT-Opt", BooleanValue(true));
      }
      else{
        ueRrc->SetAttribute("CIoT-Opt", BooleanValue(false));
      }
      if(edt == true){
        std::cout << "EDT" << std::endl;
        ueRrc->SetAttribute("EDT", BooleanValue(true));
      }
      else{
        ueRrc->SetAttribute("EDT", BooleanValue(false));
      }

      ++ulPort;
      UdpEchoServerHelper server (ulPort);
      serverApps.Add(server.Install (remoteHost));
      //
      // Create a UdpEchoClient application to send UDP datagrams from node zero to
      // node one.
      //

      if (i < ues_to_consider+num_ues_app_a){
        uint packetsize = packetsize_app_a;
        UdpEchoClientHelper ulClient (remoteHostAddr, ulPort);
        ulClient.SetAttribute ("Interval", TimeValue (packetinterval_app_a));
        ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
        ulClient.SetAttribute ("PacketSize", UintegerValue(packetsize));
        clientApps.Add (ulClient.Install (ueNodes.Get(i)));

        serverApps.Get(i)->SetStartTime (MilliSeconds (access));
        clientApps.Get(i)->SetStartTime (MilliSeconds (access));
      }
      else if (i < ues_to_consider+num_ues_app_a+num_ues_app_b){
        uint packetsize = packetsize_app_b;
        UdpEchoClientHelper ulClient (remoteHostAddr, ulPort);
        ulClient.SetAttribute ("Interval", TimeValue (packetinterval_app_b));
        ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
        ulClient.SetAttribute ("PacketSize", UintegerValue(packetsize));
        clientApps.Add (ulClient.Install (ueNodes.Get(i)));

        serverApps.Get(i)->SetStartTime (MilliSeconds (access));
        clientApps.Get(i)->SetStartTime (MilliSeconds (access));
      }
      else {
        uint packetsize = packetsize_app_c;
        UdpEchoClientHelper ulClient (remoteHostAddr, ulPort);
        ulClient.SetAttribute ("Interval", TimeValue (packetinterval_app_c));
        ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
        ulClient.SetAttribute ("PacketSize", UintegerValue(packetsize));
        clientApps.Add (ulClient.Install (ueNodes.Get(i)));

        serverApps.Get(i)->SetStartTime (MilliSeconds (access));
        clientApps.Get(i)->SetStartTime (MilliSeconds (access));
      }
    }
  


  // Post-Run
  for (uint16_t i = ues_to_consider*2; i < ues_to_consider*3; i++)
    {
      int access = RaUeUniformVariable->GetInteger (simTime.GetMilliSeconds()*2, simTime.GetMilliSeconds()*3);
      lteHelper->AttachSuspendedNb(ueLteDevs.Get(i), enbLteDevs.Get(0));

      Ptr<LteUeNetDevice> ueLteDevice = ueLteDevs.Get(i)->GetObject<LteUeNetDevice> ();
      Ptr<LteUeRrc> ueRrc = ueLteDevice->GetRrc();
      if(ciot == true){
        std::cout << "ciot" << std::endl;
        ueRrc->SetAttribute("CIoT-Opt", BooleanValue(true));
      }
      else{
        ueRrc->SetAttribute("CIoT-Opt", BooleanValue(false));
      }
      if(edt == true){
        std::cout << "EDT" << std::endl;
        ueRrc->SetAttribute("EDT", BooleanValue(true));
      }
      else{
        ueRrc->SetAttribute("EDT", BooleanValue(false));
      }

      ++ulPort;
      UdpEchoServerHelper server (ulPort);
      serverApps.Add(server.Install (remoteHost));
      //
      // Create a UdpEchoClient application to send UDP datagrams from node zero to
      // node one.
      //
      if (i < ues_to_consider*2 + num_ues_app_a){
        uint packetsize = packetsize_app_a;
        UdpEchoClientHelper ulClient (remoteHostAddr, ulPort);
        ulClient.SetAttribute ("Interval", TimeValue (packetinterval_app_a));
        ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
        ulClient.SetAttribute ("PacketSize", UintegerValue(packetsize));
        clientApps.Add (ulClient.Install (ueNodes.Get(i)));

        serverApps.Get(i)->SetStartTime (MilliSeconds (access));
        clientApps.Get(i)->SetStartTime (MilliSeconds (access));
      }
      else if (i < ues_to_consider*2 + num_ues_app_a+num_ues_app_b){
        uint packetsize = packetsize_app_b;
        UdpEchoClientHelper ulClient (remoteHostAddr, ulPort);
        ulClient.SetAttribute ("Interval", TimeValue (packetinterval_app_b));
        ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
        ulClient.SetAttribute ("PacketSize", UintegerValue(packetsize));
        clientApps.Add (ulClient.Install (ueNodes.Get(i)));

        serverApps.Get(i)->SetStartTime (MilliSeconds (access));
        clientApps.Get(i)->SetStartTime (MilliSeconds (access));
      }
      else {
        uint packetsize = packetsize_app_c;
        UdpEchoClientHelper ulClient (remoteHostAddr, ulPort);
        ulClient.SetAttribute ("Interval", TimeValue (packetinterval_app_c));
        ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
        ulClient.SetAttribute ("PacketSize", UintegerValue(packetsize));
        clientApps.Add (ulClient.Install (ueNodes.Get(i)));

        serverApps.Get(i)->SetStartTime (MilliSeconds (access));
        clientApps.Get(i)->SetStartTime (MilliSeconds (access));
      }
    }




  auto start = std::chrono::system_clock::now(); 
  std::time_t start_time = std::chrono::system_clock::to_time_t(start);
  std::cout << "started computation at " << std::ctime(&start_time);
     std::string logdir = "logs/";
  std::string makedir = "mkdir -p ";
  //auto start = std::chrono::system_clock::now();
  std::string techdir = makedir;

  techdir += logdir;
  int z = std::system(techdir.c_str());
  std::cout << z;
  techdir += "/";
  techdir += simName;
  techdir += "/";
  z = std::system(techdir.c_str());
  std::cout << z;
  logdir += simName;
  logdir += "/";
  logdir += std::to_string(ueNodes.GetN());
  logdir += "_";
  logdir += std::to_string(simTime.GetInteger());
  
  std::string top_dirmakedir = makedir+logdir; 
  int a = std::system(top_dirmakedir.c_str());
  std::cout << a << std::endl;
  logdir += "/";
  

  auto tm = *std::localtime(&start_time);
  std::stringstream ss;
  ss << std::put_time(&tm, "%d_%m_%Y_%H_%M_%S");
  logdir += ss.str();
  logdir += "_";
  logdir += std::to_string(worker);
  logdir += "_";
  //std::cout << logfile << "\n";

  for (uint16_t i = 0; i < ueNodes.GetN(); i++){

    Ptr<LteUeNetDevice> ueLteDevice = ueLteDevs.Get(i)->GetObject<LteUeNetDevice> ();
    Ptr<LteUeRrc> ueRrc = ueLteDevice->GetRrc();
    ueRrc->SetLogDir(logdir); // Will be changed to real ns3 traces later on
  }
  Ptr<LteEnbNetDevice> enbLteDevice = enbLteDevs.Get(0)->GetObject<LteEnbNetDevice>();
  Ptr<LteEnbRrc> enbRrc = enbLteDevice->GetRrc();
  enbRrc->SetLogDir(logdir);
  
  //lteHelper->EnableTraces ();
  // Uncomment to enable PCAP tracing
  //p2ph.EnablePcapAll("lena-simple-epc");

  Simulator::Stop (3*simTime);
  Simulator::Run ();
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::time_t end_time = std::chrono::system_clock::to_time_t(end);
  /*GtkConfigStore config;
  config.ConfigureAttributes();*/
  std::cout << "finished computation at " << std::ctime(&end_time)
              << "elapsed time: " << elapsed_seconds.count() << "s\n";
  Simulator::Destroy ();
  return 0;
}

