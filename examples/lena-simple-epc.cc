/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/lte-module.h"
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

NS_LOG_COMPONENT_DEFINE ("LenaSimpleEpc");

std::vector<std::string> readCSVRow (const std::string &row)
{
  CSVState state = CSVState::UnquotedField;
  std::vector<std::string> fields{""};
  size_t i = 0; // index of the current field
  for (char c : row)
    {
      switch (state)
        {
        case CSVState::UnquotedField:
          switch (c)
            {
            case ',': // end of field
              fields.push_back ("");
              i++;
              break;
            case '"':
              state = CSVState::QuotedField;
              break;
            default:
              fields[i].push_back (c);
              break;
            }
          break;
        case CSVState::QuotedField:
          switch (c)
            {
            case '"':
              state = CSVState::QuotedQuote;
              break;
            default:
              fields[i].push_back (c);
              break;
            }
          break;
        case CSVState::QuotedQuote:
          switch (c)
            {
            case ',': // , after closing quote
              fields.push_back ("");
              i++;
              state = CSVState::UnquotedField;
              break;
            case '"': // "" -> "
              fields[i].push_back ('"');
              state = CSVState::QuotedField;
              break;
            default: // end of quote
              state = CSVState::UnquotedField;
              break;
            }
          break;
        }
    }
  return fields;
}

int
main (int argc, char *argv[])
{
  uint16_t numUesCe0 = 1;
  uint16_t numUesCe1 = 1;
  uint16_t numUesCe2 = 1;
  Time simTime = MilliSeconds (3600000);
  //double distance = 50000.0;
  double distanceCe0 =  469531.7428251784;
  double distanceCe1 = 1484789.7410759863;
  double distanceCe2 = 4695317.428251784;
  

  Time interPacketInterval = MilliSeconds (100);
  bool useCa = false;
  bool disableDl = false;
  bool disableUl = false;
  bool disablePl = false;
  bool scenario = false;
  int seed = 0;

  // Command line arguments
  CommandLine cmd (__FILE__);
  cmd.AddValue ("numUesCe0", "Number of eNodeBs + UE pairs", numUesCe0);
  cmd.AddValue ("numUesCe1", "Number of eNodeBs + UE pairs", numUesCe1);
  cmd.AddValue ("numUesCe2", "Number of eNodeBs + UE pairs", numUesCe2);
  cmd.AddValue ("simTime", "Total duration of the simulation", simTime);
  cmd.AddValue ("distanceCe0", "Distance between eNBs [m]", distanceCe0);
  cmd.AddValue ("distanceCe1", "Distance between eNBs [m]", distanceCe1);
  cmd.AddValue ("distanceCe2", "Distance between eNBs [m]", distanceCe2);
  cmd.AddValue ("interPacketInterval", "Inter packet interval", interPacketInterval);
  cmd.AddValue ("useCa", "Whether to use carrier aggregation.", useCa);
  cmd.AddValue ("disableDl", "Disable downlink data flows", disableDl);
  cmd.AddValue ("disableUl", "Disable uplink data flows", disableUl);
  cmd.AddValue ("disablePl", "Disable data flows between peer UEs", disablePl);
  cmd.AddValue ("scenario", "1 if should use scenario csv", scenario);
  cmd.AddValue ("randomSeed", "randomSeed",seed);
  cmd.Parse (argc, argv);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);
  std::cout << simTime << std::endl;
  if (useCa)
   {
     Config::SetDefault ("ns3::LteHelper::UseCa", BooleanValue (useCa));
     Config::SetDefault ("ns3::LteHelper::NumberOfComponentCarriers", UintegerValue (2));
     Config::SetDefault ("ns3::LteHelper::EnbComponentCarrierManager", StringValue ("ns3::RrComponentCarrierManager"));
   }

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");
  lteHelper->SetUeAntennaModelType ("ns3::IsotropicAntennaModel");
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel"));
  Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (true));

  std::vector<double> positions;
  if(scenario){
    std::ifstream scenario("src/lte/csv/scenario.csv");
    std::string row;
    std::getline (scenario, row);
    while (!scenario.eof ())
      {
        std::getline (scenario, row);
        if (scenario.bad () || scenario.fail ())
          {
            break;
          }
        auto fields = readCSVRow (row);
        positions.push_back(stod(fields[3])*1000.0);
      }
    for(std::vector<double>::iterator it = positions.begin(); it != positions.end(); ++it){
      std::cout << *it << std::endl;
    }
  }
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
  //Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create (1);
  if(scenario){
    ueNodes.Create (positions.size());
  }
  else{
    ueNodes.Create (numUesCe0+numUesCe1+numUesCe2);
  }
  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();


  positionAlloc->Add (Vector (0, 0, 0));
  if(scenario){
      for(std::vector<double>::iterator it = positions.begin(); it != positions.end(); ++it){
      positionAlloc->Add (Vector (*it, 0, 0));
      }
  }
  else{
  // positions for CE0
    for (uint16_t i = 1; i <= numUesCe0;i++)
      {
        positionAlloc->Add (Vector (distanceCe0, 0, 0));
        //positionAlloc->Add (Vector (distance*(i%positions)+1000, 0, 0));
      }
    for (uint16_t i = 1; i <= numUesCe1;i++)
      {
        positionAlloc->Add (Vector (distanceCe1, 0, 0));
        //positionAlloc->Add (Vector (distance*(i%positions)+1000, 0, 0));
      }
    for (uint16_t i = 1; i <= numUesCe2;i++)
      {
        positionAlloc->Add (Vector (distanceCe2, 0, 0));
        //positionAlloc->Add (Vector (distance*(i%positions)+1000, 0, 0));
      }
  }
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);

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

  for (uint16_t i = 0; i < ueNodes.GetN(); i++)
    {

      int access = RaUeUniformVariable->GetInteger (0, simTime.GetMilliSeconds());
      std::cout << access << "\n";
      lteHelper->AttachAtTime (ueLteDevs.Get(i), access); //, enbLteDevs.Get(i)
      //lteHelper->Attach(ueLteDevs.Get(i)); //, enbLteDevs.Get(i)
      // side effect: the default EPS bearer will be activated
    }

  auto start = std::chrono::system_clock::now(); 
  std::time_t start_time = std::chrono::system_clock::to_time_t(start);
  std::cout << "started computation at " << std::ctime(&start_time);
  std::string logfile = "logs/";
  std::string makedir = "mkdir -p ";
  //auto start = std::chrono::system_clock::now();

  logfile += "RA_";
  logfile += std::to_string(ueNodes.GetN());
  logfile += "_";
  logfile += std::to_string(simTime.GetInteger());
  makedir += logfile; 
  std::system(makedir.c_str());
  logfile += "/";
  auto tm = *std::localtime(&start_time);
  std::stringstream ss;
  ss << std::put_time(&tm, "ra_%d_%m_%Y_%H_%M_%S");
  logfile += ss.str();
  if (scenario){
    logfile += "_";
    logfile += "predifined_scenario";
    logfile += "_";
  }
  else{
    logfile += "_";
    logfile += std::to_string(numUesCe0);
    logfile += "_";
    logfile += std::to_string(distanceCe0);
    logfile += "_";
    logfile += std::to_string(numUesCe1);
    logfile += "_";
    logfile += std::to_string(distanceCe1);
    logfile += "_";
    logfile += std::to_string(numUesCe2);
    logfile += "_";
    logfile += std::to_string(distanceCe2);
  }
  logfile += ".log";
  std::cout << logfile << "\n";
  for (uint16_t i = 0; i < ueNodes.GetN(); i++){

    Ptr<LteUeNetDevice> ueLteDevice = ueLteDevs.Get(i)->GetObject<LteUeNetDevice> ();
    Ptr<LteUeRrc> ueRrc = ueLteDevice->GetRrc();
    ueRrc->SetLogFile(logfile);
  }
  //// Install and start applications on UEs and remote host
  //uint16_t dlPort = 1100;
  //uint16_t ulPort = 2000;
  //uint16_t otherPort = 3000;
  //ApplicationContainer clientApps;
  //ApplicationContainer serverApps;
  //for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  //  {
  //    if (!disableDl)
  //      {
  //        PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
  //        serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (u)));

  //        UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
  //        dlClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
  //        dlClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
  //        clientApps.Add (dlClient.Install (remoteHost));
  //      }

  //    if (!disableUl)
  //      {
  //        ++ulPort;
  //        PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
  //        serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

  //        UdpClientHelper ulClient (remoteHostAddr, ulPort);
  //        ulClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
  //        ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
  //        clientApps.Add (ulClient.Install (ueNodes.Get(u)));
  //      }

  //    if (!disablePl && numNodePairs > 1)
  //      {
  //        ++otherPort;
  //        PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
  //        serverApps.Add (packetSinkHelper.Install (ueNodes.Get (u)));

  //        UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
  //        client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  //        client.SetAttribute ("MaxPackets", UintegerValue (1000000));
  //        clientApps.Add (client.Install (ueNodes.Get ((u + 1) % numNodePairs)));
  //      }
  //  }

  //serverApps.Start (MilliSeconds (500));
  //clientApps.Start (MilliSeconds (500));
  lteHelper->EnableTraces ();
  // Uncomment to enable PCAP tracing
  //p2ph.EnablePcapAll("lena-simple-epc");

  Simulator::Stop (simTime);
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
