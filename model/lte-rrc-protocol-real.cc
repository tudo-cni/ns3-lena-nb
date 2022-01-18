/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2022 Communication Networks Institute at TU Dortmund University
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
 * Authors: Nicola Baldo <nbaldo@cttc.es>
 *          Lluis Parcerisa <lparcerisa@cttc.cat>
 * Modified by: 
 * 			Tim Gebauer <tim.gebauer@tu-dortmund.de> (NB-IoT)
 *			Pascal Jörke <pascal.joerke@tu-dortmund.de> (NB-IoT Extension)
 */

#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/nstime.h>
#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/simulator.h>
#include <ns3/build-profile.h>

#include "lte-rrc-protocol-real.h"
#include "lte-ue-rrc.h"
#include "lte-enb-rrc.h"
#include "lte-enb-net-device.h"
#include "lte-ue-net-device.h"

#include <chrono>
#include <iomanip>
#include <stdlib.h>
#include <ctime>    
#include <fstream>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteRrcProtocolReal");

/// RRC real message delay
const Time RRC_REAL_MSG_DELAY = MilliSeconds (0); 

NS_OBJECT_ENSURE_REGISTERED (LteUeRrcProtocolReal);

LteUeRrcProtocolReal::LteUeRrcProtocolReal ()
  :  m_ueRrcSapProvider (0),
    m_enbRrcSapProvider (0)
{
  m_logging = false;
  m_ueRrcSapUser = new MemberLteUeRrcSapUser<LteUeRrcProtocolReal> (this);
  m_completeSetupParameters.srb0SapUser = new LteRlcSpecificLteRlcSapUser<LteUeRrcProtocolReal> (this);
  m_completeSetupParameters.srb1SapUser = new LtePdcpSpecificLtePdcpSapUser<LteUeRrcProtocolReal> (this);    
}

LteUeRrcProtocolReal::~LteUeRrcProtocolReal ()
{
}

void
LteUeRrcProtocolReal::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_ueRrcSapUser;
  delete m_completeSetupParameters.srb0SapUser;
  delete m_completeSetupParameters.srb1SapUser;
  m_rrc = 0;
}

TypeId
LteUeRrcProtocolReal::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LteUeRrcProtocolReal")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddConstructor<LteUeRrcProtocolReal> ()
  ;
  return tid;
}

void 
LteUeRrcProtocolReal::SetLteUeRrcSapProvider (LteUeRrcSapProvider* p)
{
  m_ueRrcSapProvider = p;
}

LteUeRrcSapUser* 
LteUeRrcProtocolReal::GetLteUeRrcSapUser ()
{
  return m_ueRrcSapUser;
}

void 
LteUeRrcProtocolReal::SetUeRrc (Ptr<LteUeRrc> rrc)
{
  m_rrc = rrc;
}

void 
LteUeRrcProtocolReal::DoSetup (LteUeRrcSapUser::SetupParameters params)
{
  NS_LOG_FUNCTION (this);

  m_setupParameters.srb0SapProvider = params.srb0SapProvider;
  m_setupParameters.srb1SapProvider = params.srb1SapProvider; 
  m_ueRrcSapProvider->CompleteSetup (m_completeSetupParameters);
}

void 
LteUeRrcProtocolReal::DoSendRrcConnectionRequest (LteRrcSap::RrcConnectionRequest msg)
{
  // initialize the RNTI and get the EnbLteRrcSapProvider for the
  // eNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();

  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionRequestHeader rrcConnectionRequestHeader;
  rrcConnectionRequestHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionRequestHeader);

  LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = m_rnti;
  transmitPdcpPduParameters.lcid = 0;

  uint64_t imsi = msg.ueIdentity; 
  if (m_logging)
  {
    std::string logfile_path = m_logdir+"RRC.log";
    std::ofstream logfile;
    logfile.open(logfile_path, std::ios_base::app);
    logfile <<  imsi << ",DoSendRrcConnectionRequest," << Simulator::Now().GetMilliSeconds() << "\n";
    logfile.close();
  }

  m_setupParameters.srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
LteUeRrcProtocolReal::DoSendRrcConnectionResumeRequestNb (NbIotRrcSap::RrcConnectionResumeRequestNb msg)
{
  // initialize the RNTI and get the EnbLteRrcSapProvider for the
  // eNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();

  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionResumeRequestNbHeader rrcConnectionResumeRequestNbHeader;
  rrcConnectionResumeRequestNbHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionResumeRequestNbHeader);

  LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = m_rnti;
  transmitPdcpPduParameters.lcid = 0;

  uint64_t imsi = msg.resumeIdentity; 
  if (m_logging)
  {
    std::string logfile_path = m_logdir+"RRC.log";
    std::ofstream logfile;
    logfile.open(logfile_path, std::ios_base::app);
    logfile <<  imsi << ",DoSendRrcConnectionResumeRequestNb," << Simulator::Now().GetMilliSeconds() << "\n";
    logfile.close();
  }

  m_setupParameters.srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}


void 
LteUeRrcProtocolReal::DoSendRrcConnectionSetupCompleted (LteRrcSap::RrcConnectionSetupCompleted msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionSetupCompleteHeader rrcConnectionSetupCompleteHeader;
  rrcConnectionSetupCompleteHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionSetupCompleteHeader);

  LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;

  if (m_setupParameters.srb1SapProvider)
    {
      m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
    }
}

void 
LteUeRrcProtocolReal::DoSendRrcConnectionResumeCompletedNb (NbIotRrcSap::RrcConnectionResumeCompleteNb msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionResumeCompleteNbHeader rrcConnectionResumeCompleteNbHeader;
  rrcConnectionResumeCompleteNbHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionResumeCompleteNbHeader);
  //uint32_t size = packet->GetSerializedSize();
  if(msg.dedicatedInfoNas->GetSize() > 0){
    packet->AddAtEnd(msg.dedicatedInfoNas);
  }
  //packet->AddByteTag(NasTag(),size,msg.dedicatedInfoNas->GetSerializedSize());

  LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;

  if (m_setupParameters.srb1SapProvider)
    {
      uint64_t imsi = msg.rrcTransactionIdentifier; 
      if (m_logging)
      {
        std::string logfile_path = m_logdir+"RRC.log";
        std::ofstream logfile;
        logfile.open(logfile_path, std::ios_base::app);
        logfile <<  imsi << ",DoSendRrcConnectionResumeCompletedNb," << Simulator::Now().GetMilliSeconds() << "\n";
        logfile.close();
      }
      m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
    }
}


void 
LteUeRrcProtocolReal::DoSendRrcEarlyDataRequestNb (NbIotRrcSap::RrcEarlyDataRequestNb msg)
{
  m_rnti = m_rrc->GetRnti ();
  Ptr<Packet> packet = Create<Packet> ();

  RrcEarlyDataRequestNbHeader rrcEarlyDataRequestNbHeader;
  rrcEarlyDataRequestNbHeader.SetMessage (msg);

  packet->AddHeader (rrcEarlyDataRequestNbHeader);
  //uint32_t size = packet->GetSerializedSize();

  if(msg.dedicatedInfoNas->GetSize() > 0){
    packet->AddAtEnd(msg.dedicatedInfoNas);
  }
  //packet->AddByteTag(NasTag(),size,msg.dedicatedInfoNas->GetSerializedSize());
  LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = m_rnti;
  transmitPdcpPduParameters.lcid = 0;

  uint64_t imsi = msg.sTmsiNb.mTmsi;
  if (m_logging)
  {
    std::string logfile_path = m_logdir+"RRC.log";
    std::ofstream logfile;
    logfile.open(logfile_path, std::ios_base::app);
    logfile <<  m_rnti << ",DoSendRrcEarlyDataRequestNb," << Simulator::Now().GetMilliSeconds() << ",imsi: " << imsi << "\n";
    logfile.close();
  }

  m_setupParameters.srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
LteUeRrcProtocolReal::DoSendRrcConnectionReconfigurationCompleted (LteRrcSap::RrcConnectionReconfigurationCompleted msg)
{
  // re-initialize the RNTI and get the EnbLteRrcSapProvider for the
  // eNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();

  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReconfigurationCompleteHeader rrcConnectionReconfigurationCompleteHeader;
  rrcConnectionReconfigurationCompleteHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReconfigurationCompleteHeader);

  LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;

  m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void 
LteUeRrcProtocolReal::DoSendMeasurementReport (LteRrcSap::MeasurementReport msg)
{
  // re-initialize the RNTI and get the EnbLteRrcSapProvider for the
  // eNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();

  Ptr<Packet> packet = Create<Packet> ();

  MeasurementReportHeader measurementReportHeader;
  measurementReportHeader.SetMessage (msg);

  packet->AddHeader (measurementReportHeader);

  LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;

  m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void
LteUeRrcProtocolReal::DoSendIdealUeContextRemoveRequest (uint16_t rnti)
{
  NS_LOG_FUNCTION(this<<rnti);
  uint16_t cellId = m_rrc->GetCellId ();
  // re-initialize the RNTI and get the EnbLteRrcSapProvider for the
  // eNB we are currently attached to or attempting random access to
  // a target eNB
  m_rnti = m_rrc->GetRnti ();

  NS_LOG_DEBUG ("RNTI " << rnti
                << " sending UE context remove request to cell id " << cellId);
  NS_ABORT_MSG_IF (m_rnti != rnti, "RNTI mismatch");

  SetEnbRrcSapProvider (); //the provider has to be reset since the cell might
                          // have changed due to handover
  //ideally informing eNB
  Simulator::Schedule (RRC_REAL_MSG_DELAY, &LteEnbRrcSapProvider::RecvIdealUeContextRemoveRequest,
                       m_enbRrcSapProvider, rnti);
}

void 
LteUeRrcProtocolReal::DoSendRrcConnectionReestablishmentRequest (LteRrcSap::RrcConnectionReestablishmentRequest msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReestablishmentRequestHeader rrcConnectionReestablishmentRequestHeader;
  rrcConnectionReestablishmentRequestHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReestablishmentRequestHeader);

  LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = m_rnti;
  transmitPdcpPduParameters.lcid = 0;

  m_setupParameters.srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
LteUeRrcProtocolReal::DoSendRrcConnectionReestablishmentComplete (LteRrcSap::RrcConnectionReestablishmentComplete msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReestablishmentCompleteHeader rrcConnectionReestablishmentCompleteHeader;
  rrcConnectionReestablishmentCompleteHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReestablishmentCompleteHeader);

  LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;

  m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}


void 
LteUeRrcProtocolReal::SetEnbRrcSapProvider ()
{
  NS_LOG_FUNCTION (this);

  uint16_t cellId = m_rrc->GetCellId ();

  NS_LOG_DEBUG ("RNTI " << m_rnti << " connected to cell " << cellId);

  // walk list of all nodes to get the peer eNB
  Ptr<LteEnbNetDevice> enbDev;
  NodeList::Iterator listEnd = NodeList::End ();
  bool found = false;
  for (NodeList::Iterator i = NodeList::Begin (); 
       (i != listEnd) && (!found); 
       ++i)
    {
      Ptr<Node> node = *i;
      int nDevs = node->GetNDevices ();
      for (int j = 0; 
           (j < nDevs) && (!found);
           j++)
        {
          enbDev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
          if (enbDev == 0)
            {
              continue;
            }
          else
            {
              if (enbDev->HasCellId (cellId))
                {
                  found = true;
                  break;
                }
            }
        }
    }
  NS_ASSERT_MSG (found, " Unable to find eNB with CellId =" << cellId);
  m_enbRrcSapProvider = enbDev->GetRrc ()->GetLteEnbRrcSapProvider ();
  Ptr<LteEnbRrcProtocolReal> enbRrcProtocolReal = enbDev->GetRrc ()->GetObject<LteEnbRrcProtocolReal> ();
  enbRrcProtocolReal->SetUeRrcSapProvider (m_rnti, m_ueRrcSapProvider);
}

void
LteUeRrcProtocolReal::DoReceivePdcpPdu (Ptr<Packet> p)
{
  // Get type of message received
  RrcDlCcchMessage rrcDlCcchMessage;
  p->PeekHeader (rrcDlCcchMessage);

  // Declare possible headers to receive
  RrcConnectionReestablishmentHeader rrcConnectionReestablishmentHeader;
  RrcConnectionReestablishmentRejectHeader rrcConnectionReestablishmentRejectHeader;
  RrcConnectionSetupHeader rrcConnectionSetupHeader;
  RrcConnectionRejectHeader rrcConnectionRejectHeader;
  RrcEarlyDataCompleteNbHeader rrcEarlyDataCompleteNbHeader;

  // Declare possible messages
  LteRrcSap::RrcConnectionReestablishment rrcConnectionReestablishmentMsg;
  LteRrcSap::RrcConnectionReestablishmentReject rrcConnectionReestablishmentRejectMsg;
  LteRrcSap::RrcConnectionSetup rrcConnectionSetupMsg;
  LteRrcSap::RrcConnectionReject rrcConnectionRejectMsg;
  NbIotRrcSap::RrcEarlyDataCompleteNb rrcEarlyDataCompleteNbMsg;

  // Deserialize packet and call member recv function with appropriate structure
  switch ( rrcDlCcchMessage.GetMessageType () )
    {
    case 0:
      // RrcConnectionReestablishment
      p->RemoveHeader (rrcConnectionReestablishmentHeader);
      rrcConnectionReestablishmentMsg = rrcConnectionReestablishmentHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionReestablishment (rrcConnectionReestablishmentMsg);
      break;
    case 1:
      // RrcConnectionReestablishmentReject
      p->RemoveHeader (rrcConnectionReestablishmentRejectHeader);
      rrcConnectionReestablishmentRejectMsg = rrcConnectionReestablishmentRejectHeader.GetMessage ();
      // m_ueRrcSapProvider->RecvRrcConnectionReestablishmentReject (rrcConnectionReestablishmentRejectMsg);
      break;
    case 2:
      // RrcConnectionReject
      p->RemoveHeader (rrcConnectionRejectHeader);
      rrcConnectionRejectMsg = rrcConnectionRejectHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionReject (rrcConnectionRejectMsg);
      break;
    case 3:
      // RrcConnectionSetup
      p->RemoveHeader (rrcConnectionSetupHeader);
      rrcConnectionSetupMsg = rrcConnectionSetupHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionSetup (rrcConnectionSetupMsg);
      break;
    case 4:
      p->RemoveHeader(rrcEarlyDataCompleteNbHeader);
      rrcEarlyDataCompleteNbMsg = rrcEarlyDataCompleteNbHeader.GetMessage();
      rrcEarlyDataCompleteNbMsg.dedicatedInfoNas = p;
      m_ueRrcSapProvider->RecvRrcEarlyDataCompleteNb(rrcEarlyDataCompleteNbMsg);


    }
}

void
LteUeRrcProtocolReal::DoReceivePdcpSdu (LtePdcpSapUser::ReceivePdcpSduParameters params)
{
  // Get type of message received
  RrcDlDcchMessage rrcDlDcchMessage;
  params.pdcpSdu->PeekHeader (rrcDlDcchMessage);

  // Declare possible headers to receive
  RrcConnectionReconfigurationHeader rrcConnectionReconfigurationHeader;
  RrcConnectionReleaseHeader rrcConnectionReleaseHeader;
  RrcConnectionResumeNbHeader rrcConnectionResumeNbHeader;
  RrcConnectionReleaseNbHeader rrcConnectionReleaseNbHeader;

  // Declare possible messages to receive
  LteRrcSap::RrcConnectionReconfiguration rrcConnectionReconfigurationMsg;
  LteRrcSap::RrcConnectionRelease rrcConnectionReleaseMsg;
  NbIotRrcSap::RrcConnectionResumeNb rrcConnectionResumeNbMsg;
  NbIotRrcSap::RrcConnectionReleaseNb rrcConnectionReleaseNbMsg;

  // Deserialize packet and call member recv function with appropriate structure
  switch ( rrcDlDcchMessage.GetMessageType () )
    {
    case 4:
      params.pdcpSdu->RemoveHeader (rrcConnectionReconfigurationHeader);
      rrcConnectionReconfigurationMsg = rrcConnectionReconfigurationHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionReconfiguration (rrcConnectionReconfigurationMsg);
      break;
    case 5:
      params.pdcpSdu->RemoveHeader (rrcConnectionReleaseHeader);
      rrcConnectionReleaseMsg = rrcConnectionReleaseHeader.GetMessage ();
      //m_ueRrcSapProvider->RecvRrcConnectionRelease (rrcConnectionReleaseMsg);
    case 12:
      params.pdcpSdu->RemoveHeader (rrcConnectionResumeNbHeader);
      rrcConnectionResumeNbMsg = rrcConnectionResumeNbHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionResumeNb (rrcConnectionResumeNbMsg);
      break;
    case 13:
      params.pdcpSdu->RemoveHeader (rrcConnectionReleaseNbHeader);
      rrcConnectionReleaseNbMsg = rrcConnectionReleaseNbHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionReleaseNb (rrcConnectionReleaseNbMsg);
      break;
    }
}

void LteUeRrcProtocolReal::DoSetLogDir(std::string dirname){
  m_logdir = dirname;
  m_logging = true;
}

NS_OBJECT_ENSURE_REGISTERED (LteEnbRrcProtocolReal);

LteEnbRrcProtocolReal::LteEnbRrcProtocolReal ()
  :  m_enbRrcSapProvider (0)
{
  NS_LOG_FUNCTION (this);
  m_logging = true;
  m_enbRrcSapUser = new MemberLteEnbRrcSapUser<LteEnbRrcProtocolReal> (this);
}

LteEnbRrcProtocolReal::~LteEnbRrcProtocolReal ()
{
  NS_LOG_FUNCTION (this);
}

void
LteEnbRrcProtocolReal::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_enbRrcSapUser;
  for (std::map<uint16_t, LteEnbRrcSapProvider::CompleteSetupUeParameters>::iterator 
         it = m_completeSetupUeParametersMap.begin ();
       it != m_completeSetupUeParametersMap.end ();
       ++it)
    {     
      delete it->second.srb0SapUser;
      delete it->second.srb1SapUser;
    }
  m_completeSetupUeParametersMap.clear ();
}

TypeId
LteEnbRrcProtocolReal::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LteEnbRrcProtocolReal")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddConstructor<LteEnbRrcProtocolReal> ()
  ;
  return tid;
}

void 
LteEnbRrcProtocolReal::SetLteEnbRrcSapProvider (LteEnbRrcSapProvider* p)
{
  m_enbRrcSapProvider = p;
}

LteEnbRrcSapUser* 
LteEnbRrcProtocolReal::GetLteEnbRrcSapUser ()
{
  return m_enbRrcSapUser;
}

void 
LteEnbRrcProtocolReal::SetCellId (uint16_t cellId)
{
  m_cellId = cellId;
}

LteUeRrcSapProvider* 
LteEnbRrcProtocolReal::GetUeRrcSapProvider (uint16_t rnti)
{
  std::map<uint16_t, LteUeRrcSapProvider*>::const_iterator it;
  it = m_enbRrcSapProviderMap.find (rnti);
  NS_ASSERT_MSG (it != m_enbRrcSapProviderMap.end (), "could not find RNTI = " << rnti);
  return it->second;
}

void 
LteEnbRrcProtocolReal::SetUeRrcSapProvider (uint16_t rnti, LteUeRrcSapProvider* p)
{
  std::map<uint16_t, LteUeRrcSapProvider*>::iterator it;
  it = m_enbRrcSapProviderMap.find (rnti);
  NS_ASSERT_MSG (it != m_enbRrcSapProviderMap.end (), "Cell id " << m_cellId
                                         << " could not find RNTI = " << rnti);
  it->second = p;
}

void 
LteEnbRrcProtocolReal::DoSetupUe (uint16_t rnti, LteEnbRrcSapUser::SetupUeParameters params)
{
  NS_LOG_FUNCTION (this << rnti);

  // // walk list of all nodes to get the peer UE RRC SAP Provider
  // Ptr<LteUeRrc> ueRrc;
  // NodeList::Iterator listEnd = NodeList::End ();
  // bool found = false;
  // for (NodeList::Iterator i = NodeList::Begin (); (i != listEnd) && (found == false); i++)
  //   {
  //     Ptr<Node> node = *i;
  //     int nDevs = node->GetNDevices ();
  //     for (int j = 0; j < nDevs; j++)
  //       {
  //         Ptr<LteUeNetDevice> ueDev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
  //         if (!ueDev)
  //           {
  //             continue;
  //           }
  //         else
  //           {
  //             ueRrc = ueDev->GetRrc ();
  //             if ((ueRrc->GetRnti () == rnti) && (ueRrc->GetCellId () == m_cellId))
  //               {
  //              found = true;
  //              break;
  //               }
  //           }
  //       }
  //   }
  // NS_ASSERT_MSG (found , " Unable to find UE with RNTI=" << rnti << " cellId=" << m_cellId);
  // m_enbRrcSapProviderMap[rnti] = ueRrc->GetLteUeRrcSapProvider ();

  // just create empty entry, the UeRrcSapProvider will be set by the
  // ue upon connection request or connection reconfiguration
  // completed 
  m_enbRrcSapProviderMap[rnti] = 0;

  // Store SetupUeParameters
  m_setupUeParametersMap[rnti] = params;

  LteEnbRrcSapProvider::CompleteSetupUeParameters completeSetupUeParameters;
  std::map<uint16_t, LteEnbRrcSapProvider::CompleteSetupUeParameters>::iterator 
    csupIt = m_completeSetupUeParametersMap.find (rnti);
  if (csupIt == m_completeSetupUeParametersMap.end ())
    {
      // Create LteRlcSapUser, LtePdcpSapUser
      LteRlcSapUser* srb0SapUser = new RealProtocolRlcSapUser (this,rnti);
      LtePdcpSapUser* srb1SapUser = new LtePdcpSpecificLtePdcpSapUser<LteEnbRrcProtocolReal> (this);
      completeSetupUeParameters.srb0SapUser = srb0SapUser;
      completeSetupUeParameters.srb1SapUser = srb1SapUser;
      // Store LteRlcSapUser, LtePdcpSapUser
      m_completeSetupUeParametersMap[rnti] = completeSetupUeParameters;      
    }
  else
    {
      completeSetupUeParameters = csupIt->second;
    }
  m_enbRrcSapProvider->CompleteSetupUe (rnti, completeSetupUeParameters);
}

void 
LteEnbRrcProtocolReal::DoResumeUe(uint16_t rnti, uint64_t resumeId){
  m_completeSetupUeParametersMap[rnti] = m_resumeCompleteSetupUeParametersMap[resumeId];
  m_resumeCompleteSetupUeParametersMap.erase(resumeId);
  m_enbRrcSapProviderMap[rnti] = m_resumeEnbRrcSapProviderMap[resumeId];
  m_resumeEnbRrcSapProviderMap.erase (resumeId);
  m_setupUeParametersMap[rnti] = m_resumeSetupUeParametersMap[resumeId];
  m_setupUeParametersMap.erase (resumeId);
}

void 
LteEnbRrcProtocolReal::DoRemoveUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  std::map<uint16_t, LteEnbRrcSapProvider::CompleteSetupUeParameters>::iterator 
    it = m_completeSetupUeParametersMap.find (rnti);
  NS_ASSERT (it != m_completeSetupUeParametersMap.end ());
  delete it->second.srb0SapUser;
  delete it->second.srb1SapUser;
  m_completeSetupUeParametersMap.erase (it);
  m_enbRrcSapProviderMap.erase (rnti);
  m_setupUeParametersMap.erase (rnti);
}

void 
LteEnbRrcProtocolReal::DoRemoveUe (uint16_t rnti, bool resumed)
{
  NS_LOG_FUNCTION (this << rnti);
  std::map<uint16_t, LteEnbRrcSapProvider::CompleteSetupUeParameters>::iterator 
    it = m_completeSetupUeParametersMap.find (rnti);
  NS_ASSERT (it != m_completeSetupUeParametersMap.end ());
  if(!resumed){
    delete it->second.srb0SapUser;
    delete it->second.srb1SapUser;
  }
  m_completeSetupUeParametersMap.erase (it);
  m_enbRrcSapProviderMap.erase (rnti);
  m_setupUeParametersMap.erase (rnti);
}

void 
LteEnbRrcProtocolReal::DoMoveUeToResume(uint16_t rnti, uint64_t resumeId){
  m_resumeCompleteSetupUeParametersMap[resumeId] = m_completeSetupUeParametersMap[rnti];
  m_resumeEnbRrcSapProviderMap[resumeId] = m_enbRrcSapProviderMap[rnti];
  m_resumeSetupUeParametersMap[resumeId] = m_setupUeParametersMap[rnti];
}

void 
LteEnbRrcProtocolReal::DoSendSystemInformation (uint16_t cellId, LteRrcSap::SystemInformation msg)
{
  NS_LOG_FUNCTION (this << cellId);
  // walk list of all nodes to get UEs with this cellId
  Ptr<LteUeRrc> ueRrc;
  for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
    {
      Ptr<Node> node = *i;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; ++j)
        {
          Ptr<LteUeNetDevice> ueDev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
          if (ueDev != 0)
            {
              Ptr<LteUeRrc> ueRrc = ueDev->GetRrc ();
              NS_LOG_LOGIC ("considering UE IMSI " << ueDev->GetImsi () << " that has cellId " << ueRrc->GetCellId ());
              if (ueRrc->GetCellId () == cellId)
                {
                  NS_LOG_LOGIC ("sending SI to IMSI " << ueDev->GetImsi ());

                  Simulator::ScheduleWithContext (node->GetId (),
                                                  RRC_REAL_MSG_DELAY,
                                                  &LteUeRrcSapProvider::RecvSystemInformation,
                                                  ueRrc->GetLteUeRrcSapProvider (),
                                                  msg);
                }
            }
        }
    } 
}
void 

LteEnbRrcProtocolReal::DoSendSystemInformationNb (uint16_t cellId, NbIotRrcSap::SystemInformationNb msg)
{
  NS_LOG_FUNCTION (this << cellId);
  // walk list of all nodes to get UEs with this cellId
  Ptr<LteUeRrc> ueRrc;
  for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
    {
      Ptr<Node> node = *i;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; ++j)
        {
          Ptr<LteUeNetDevice> ueDev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
          if (ueDev != 0)
            {
              Ptr<LteUeRrc> ueRrc = ueDev->GetRrc ();
              NS_LOG_LOGIC ("considering UE IMSI " << ueDev->GetImsi () << " that has cellId " << ueRrc->GetCellId ());
              if (ueRrc->GetCellId () == cellId)
                {
                  NS_LOG_LOGIC ("sending SI to IMSI " << ueDev->GetImsi ());

                  Simulator::ScheduleWithContext (node->GetId (),
                                                  RRC_REAL_MSG_DELAY,
                                                  &LteUeRrcSapProvider::RecvSystemInformationNb,
                                                  ueRrc->GetLteUeRrcSapProvider (),
                                                  msg);
                }
            }
        }
    } 
}

void 
LteEnbRrcProtocolReal::DoSendRrcConnectionSetup (uint16_t rnti, LteRrcSap::RrcConnectionSetup msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionSetupHeader rrcConnectionSetupHeader;
  rrcConnectionSetupHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionSetupHeader);

  LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = rnti;
  transmitPdcpPduParameters.lcid = 0;
  NS_BUILD_DEBUG(std::cout << "Send connection setup" << std::endl);
  m_setupUeParametersMap.at (rnti).srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
LteEnbRrcProtocolReal::DoSendRrcConnectionResumeNb (uint16_t rnti, NbIotRrcSap::RrcConnectionResumeNb msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionResumeNbHeader rrcConnectionResumeNbHeader;
  rrcConnectionResumeNbHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionResumeNbHeader);
  LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = rnti;
  transmitPdcpSduParameters.lcid = 1;

  uint64_t imsi = msg.rrcTransactionIdentifier; 
  if (m_logging)
  {
    std::string logfile_path = m_logdir+"RRC.log";
    std::ofstream logfile;
    logfile.open(logfile_path, std::ios_base::app);
    logfile <<  imsi << ",DoSendRrcConnectionResumeNb," << Simulator::Now().GetMilliSeconds() << "\n";
    logfile.close();
  }

  m_setupUeParametersMap[rnti].srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void 
LteEnbRrcProtocolReal::DoSendRrcConnectionReject (uint16_t rnti, LteRrcSap::RrcConnectionReject msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionRejectHeader rrcConnectionRejectHeader;
  rrcConnectionRejectHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionRejectHeader);

  LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = rnti;
  transmitPdcpPduParameters.lcid = 0;

  m_setupUeParametersMap[rnti].srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
LteEnbRrcProtocolReal::DoSendRrcConnectionReconfiguration (uint16_t rnti, LteRrcSap::RrcConnectionReconfiguration msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReconfigurationHeader rrcConnectionReconfigurationHeader;
  rrcConnectionReconfigurationHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReconfigurationHeader);

  LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = rnti;
  transmitPdcpSduParameters.lcid = 1;

  m_setupUeParametersMap[rnti].srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void 
LteEnbRrcProtocolReal::DoSendRrcConnectionReestablishment (uint16_t rnti, LteRrcSap::RrcConnectionReestablishment msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReestablishmentHeader rrcConnectionReestablishmentHeader;
  rrcConnectionReestablishmentHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReestablishmentHeader);

  LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = rnti;
  transmitPdcpPduParameters.lcid = 0;

  m_setupUeParametersMap[rnti].srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
LteEnbRrcProtocolReal::DoSendRrcConnectionReestablishmentReject (uint16_t rnti, LteRrcSap::RrcConnectionReestablishmentReject msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReestablishmentRejectHeader rrcConnectionReestablishmentRejectHeader;
  rrcConnectionReestablishmentRejectHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReestablishmentRejectHeader);

  LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = rnti;
  transmitPdcpPduParameters.lcid = 0;

  m_setupUeParametersMap[rnti].srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
LteEnbRrcProtocolReal::DoSendRrcConnectionRelease (uint16_t rnti, LteRrcSap::RrcConnectionRelease msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReleaseHeader rrcConnectionReleaseHeader;
  rrcConnectionReleaseHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReleaseHeader);

  LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = rnti;
  transmitPdcpSduParameters.lcid = 1;

  m_setupUeParametersMap[rnti].srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}
void 
LteEnbRrcProtocolReal::DoSendRrcConnectionReleaseNb (uint16_t rnti, NbIotRrcSap::RrcConnectionReleaseNb msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReleaseNbHeader rrcConnectionReleaseNbHeader;
  rrcConnectionReleaseNbHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReleaseNbHeader);

  LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = rnti;
  transmitPdcpSduParameters.lcid = 1;

  uint64_t imsi = msg.rrcTransactionIdentifier;
  if (m_logging)
  {
    std::string logfile_path = m_logdir+"RRC.log";
    std::ofstream logfile;
    logfile.open(logfile_path, std::ios_base::app);
    logfile <<  imsi << ",DoSendRrcConnectionReleaseNb," << Simulator::Now().GetMilliSeconds() << "\n";
    logfile.close();
  }

  m_setupUeParametersMap[rnti].srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void
LteEnbRrcProtocolReal::DoReceivePdcpPdu (uint16_t rnti, Ptr<Packet> p)
{
  // Get type of message received
  RrcUlCcchMessage rrcUlCcchMessage;
  p->PeekHeader (rrcUlCcchMessage);

  // Declare possible headers to receive
  RrcConnectionReestablishmentRequestHeader rrcConnectionReestablishmentRequestHeader;
  RrcConnectionRequestHeader rrcConnectionRequestHeader;
  RrcConnectionResumeRequestNbHeader rrcConnectionResumeRequestNbHeader;
  RrcEarlyDataRequestNbHeader rrcEarlyDataRequestNbHeader;
  if (m_logging)
  {
    std::string logfile_path = m_logdir+"RRC.log";
    std::ofstream logfile;
    logfile.open(logfile_path, std::ios_base::app);
    logfile <<  rnti << ",DoReceivePdcpPdu," << Simulator::Now().GetMilliSeconds() << ",MessageType," << rrcUlCcchMessage.GetMessageType () << "\n";
    logfile.close();
  }

  // Deserialize packet and call member recv function with appropriate structure
  switch ( rrcUlCcchMessage.GetMessageType () )
    {
    case 0:
      p->RemoveHeader (rrcConnectionReestablishmentRequestHeader);
      LteRrcSap::RrcConnectionReestablishmentRequest rrcConnectionReestablishmentRequestMsg;
      rrcConnectionReestablishmentRequestMsg = rrcConnectionReestablishmentRequestHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionReestablishmentRequest (rnti,rrcConnectionReestablishmentRequestMsg);
      break;
    case 1:
      p->RemoveHeader (rrcConnectionRequestHeader);
      LteRrcSap::RrcConnectionRequest rrcConnectionRequestMsg;
      rrcConnectionRequestMsg = rrcConnectionRequestHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionRequest (rnti,rrcConnectionRequestMsg);
      break;
    case 2:
      p->RemoveHeader(rrcConnectionResumeRequestNbHeader);
      NbIotRrcSap::RrcConnectionResumeRequestNb rrcConnectionResumeRequestNbMsg;
      rrcConnectionResumeRequestNbMsg = rrcConnectionResumeRequestNbHeader.GetMessage();
      m_enbRrcSapProvider->RecvRrcConnectionResumeRequestNb (rnti, rrcConnectionResumeRequestNbMsg);
      break;
    case 3:
      p->RemoveHeader(rrcEarlyDataRequestNbHeader);
      NbIotRrcSap::RrcEarlyDataRequestNb rrcEarlyDataRequestNbMsg;
      rrcEarlyDataRequestNbMsg = rrcEarlyDataRequestNbHeader.GetMessage();
      rrcEarlyDataRequestNbMsg.dedicatedInfoNas= p;
      m_enbRrcSapProvider->RecvRrcEarlyDataRequestNb (rnti, rrcEarlyDataRequestNbMsg);
      break;
    }
}
void 
LteEnbRrcProtocolReal::DoSendRrcEarlyDataCompleteNb (uint16_t rnti, NbIotRrcSap::RrcEarlyDataCompleteNb msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcEarlyDataCompleteNbHeader rrcEarlyDataCompleteNbHeader;
  rrcEarlyDataCompleteNbHeader.SetMessage (msg);

  packet->AddHeader (rrcEarlyDataCompleteNbHeader);
  //uint32_t size = packet->GetSerializedSize();
  if(msg.dedicatedInfoNas->GetSize() > 0){
    packet->AddAtEnd(msg.dedicatedInfoNas);
  }
  //packet->AddByteTag(NasTag(),size,msg.dedicatedInfoNas->GetSerializedSize());

  LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = rnti;
  transmitPdcpPduParameters.lcid = 0;

  if (m_logging)
  {
    std::string logfile_path = m_logdir+"RRC.log";
    std::ofstream logfile;
    logfile.open(logfile_path, std::ios_base::app);
    logfile <<  rnti << ",DoSendRrcEarlyDataCompleteNb," << Simulator::Now().GetMilliSeconds() << "\n";
    logfile.close();
  }

  m_setupUeParametersMap[rnti].srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}
void
LteEnbRrcProtocolReal::DoReceivePdcpSdu (LtePdcpSapUser::ReceivePdcpSduParameters params)
{
  // Get type of message received
  RrcUlDcchMessage rrcUlDcchMessage;
  params.pdcpSdu->PeekHeader (rrcUlDcchMessage);
  // Declare possible headers to receive
  MeasurementReportHeader measurementReportHeader;
  RrcConnectionReconfigurationCompleteHeader rrcConnectionReconfigurationCompleteHeader;
  RrcConnectionReestablishmentCompleteHeader rrcConnectionReestablishmentCompleteHeader;
  RrcConnectionSetupCompleteHeader rrcConnectionSetupCompleteHeader;
  RrcConnectionResumeCompleteNbHeader rrcConnectionResumeCompleteNbHeader;

  // Declare possible messages to receive
  LteRrcSap::MeasurementReport measurementReportMsg;
  LteRrcSap::RrcConnectionReconfigurationCompleted rrcConnectionReconfigurationCompleteMsg;
  LteRrcSap::RrcConnectionReestablishmentComplete rrcConnectionReestablishmentCompleteMsg;
  LteRrcSap::RrcConnectionSetupCompleted rrcConnectionSetupCompletedMsg;
  NbIotRrcSap::RrcConnectionResumeCompleteNb rrcConnectionResumeCompleteNbMsg;

  // Deserialize packet and call member recv function with appropriate structure
  switch ( rrcUlDcchMessage.GetMessageType () )
    {
    case 1:
      params.pdcpSdu->RemoveHeader (measurementReportHeader);
      measurementReportMsg = measurementReportHeader.GetMessage ();
      m_enbRrcSapProvider->RecvMeasurementReport (params.rnti,measurementReportMsg);
      break;
    case 2:
      params.pdcpSdu->RemoveHeader (rrcConnectionReconfigurationCompleteHeader);
      rrcConnectionReconfigurationCompleteMsg = rrcConnectionReconfigurationCompleteHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionReconfigurationCompleted (params.rnti,rrcConnectionReconfigurationCompleteMsg);
      break;
    case 3:
      params.pdcpSdu->RemoveHeader (rrcConnectionReestablishmentCompleteHeader);
      rrcConnectionReestablishmentCompleteMsg = rrcConnectionReestablishmentCompleteHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionReestablishmentComplete (params.rnti,rrcConnectionReestablishmentCompleteMsg);
      break;
    case 4:
      params.pdcpSdu->RemoveHeader (rrcConnectionSetupCompleteHeader);
      rrcConnectionSetupCompletedMsg = rrcConnectionSetupCompleteHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionSetupCompleted (params.rnti, rrcConnectionSetupCompletedMsg);
      break;
    case 14:
      params.pdcpSdu->RemoveHeader (rrcConnectionResumeCompleteNbHeader);
      rrcConnectionResumeCompleteNbMsg = rrcConnectionResumeCompleteNbHeader.GetMessage ();
      rrcConnectionResumeCompleteNbMsg.dedicatedInfoNas= params.pdcpSdu;
      m_enbRrcSapProvider->RecvRrcConnectionResumeCompletedNb (params.rnti, rrcConnectionResumeCompleteNbMsg);
      break;
    }
}

Ptr<Packet> 
LteEnbRrcProtocolReal::DoEncodeHandoverPreparationInformation (LteRrcSap::HandoverPreparationInfo msg)
{
  HandoverPreparationInfoHeader h;
  h.SetMessage (msg);

  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  return p;
}

LteRrcSap::HandoverPreparationInfo 
LteEnbRrcProtocolReal::DoDecodeHandoverPreparationInformation (Ptr<Packet> p)
{
  HandoverPreparationInfoHeader h;
  p->RemoveHeader (h);
  LteRrcSap::HandoverPreparationInfo msg = h.GetMessage ();
  return msg;
}

Ptr<Packet> 
LteEnbRrcProtocolReal::DoEncodeHandoverCommand (LteRrcSap::RrcConnectionReconfiguration msg)
{
  RrcConnectionReconfigurationHeader h;
  h.SetMessage (msg);
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  return p;
}

LteRrcSap::RrcConnectionReconfiguration
LteEnbRrcProtocolReal::DoDecodeHandoverCommand (Ptr<Packet> p)
{
  RrcConnectionReconfigurationHeader h;
  p->RemoveHeader (h);
  LteRrcSap::RrcConnectionReconfiguration msg = h.GetMessage ();
  return msg;
}

void LteEnbRrcProtocolReal::DoSetLogDir(std::string dirname){
  m_logdir = dirname;
  m_logging = true;
}

//////////////////////////////////////////////////////

RealProtocolRlcSapUser::RealProtocolRlcSapUser (LteEnbRrcProtocolReal* pdcp, uint16_t rnti)
  : m_pdcp (pdcp),
    m_rnti (rnti)
{
}

RealProtocolRlcSapUser::RealProtocolRlcSapUser ()
{
}

void
RealProtocolRlcSapUser::ReceivePdcpPdu (Ptr<Packet> p)
{
  m_pdcp->DoReceivePdcpPdu (m_rnti, p);
}

} // namespace ns3
