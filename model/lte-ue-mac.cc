/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Nicola Baldo  <nbaldo@cttc.es>
 * Author: Marco Miozzo <mmiozzo@cttc.es>
 */

#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/packet.h>
#include <ns3/packet-burst.h>
#include <ns3/random-variable-stream.h>
#include <ns3/build-profile.h>

#include "lte-ue-mac.h"
#include "lte-ue-net-device.h"
#include "lte-radio-bearer-tag.h"
#include "nb-iot-data-volume-and-power-headroom-tag.h"
#include "nb-iot-buffer-status-report-tag.h"
#include <ns3/ff-mac-common.h>
#include <ns3/lte-control-messages.h>
#include <ns3/simulator.h>
#include <ns3/lte-common.h>
#include <fstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteUeMac");

NS_OBJECT_ENSURE_REGISTERED (LteUeMac);

///////////////////////////////////////////////////////////
// SAP forwarders
///////////////////////////////////////////////////////////

/// UeMemberLteUeCmacSapProvider class
class UeMemberLteUeCmacSapProvider : public LteUeCmacSapProvider
{
public:
  /**
   * Constructor
   *
   * \param mac the UE MAC
   */
  UeMemberLteUeCmacSapProvider (LteUeMac *mac);

  // inherited from LteUeCmacSapProvider
  virtual void ConfigureRach (RachConfig rc);
  virtual void ConfigureRadioResourceConfig (NbIotRrcSap::RadioResourceConfigCommonNb rc);
  virtual void StartContentionBasedRandomAccessProcedure ();
  virtual void StartRandomAccessProcedureNb (bool edt);
  virtual void StartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t preambleId,
                                                             uint8_t prachMask);
  virtual void SetRnti (uint16_t rnti);
  virtual void AddLc (uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig,
                      LteMacSapUser *msu);
  virtual void RemoveLc (uint8_t lcId);
  virtual void Reset ();
  virtual void NotifyConnectionSuccessful ();
  virtual void SetImsi (uint64_t imsi);
  virtual void NotifyEdrx();
  virtual void NotifyPsm();
  virtual void SetMsg5Buffer(uint32_t buffersize);
  virtual NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel GetCoverageEnhancementLevel();

private:
  LteUeMac *m_mac; ///< the UE MAC
};

UeMemberLteUeCmacSapProvider::UeMemberLteUeCmacSapProvider (LteUeMac *mac) : m_mac (mac)
{
}

void
UeMemberLteUeCmacSapProvider::ConfigureRach (RachConfig rc)
{
  m_mac->DoConfigureRach (rc);
}
void
UeMemberLteUeCmacSapProvider::ConfigureRadioResourceConfig (
    NbIotRrcSap::RadioResourceConfigCommonNb rc)
{
  m_mac->DoConfigureRadioResourceConfig (rc);
}
void
UeMemberLteUeCmacSapProvider::StartContentionBasedRandomAccessProcedure ()
{
  m_mac->DoStartContentionBasedRandomAccessProcedure ();
}
void
UeMemberLteUeCmacSapProvider::StartRandomAccessProcedureNb (bool edt)
{
  m_mac->DoStartRandomAccessProcedureNb (edt);
}
void
UeMemberLteUeCmacSapProvider::StartNonContentionBasedRandomAccessProcedure (uint16_t rnti,
                                                                            uint8_t preambleId,
                                                                            uint8_t prachMask)
{
  m_mac->DoStartNonContentionBasedRandomAccessProcedure (rnti, preambleId, prachMask);
}

void
UeMemberLteUeCmacSapProvider::SetRnti (uint16_t rnti)
{
  m_mac->DoSetRnti (rnti);
}

void
UeMemberLteUeCmacSapProvider::AddLc (uint8_t lcId, LogicalChannelConfig lcConfig,
                                     LteMacSapUser *msu)
{
  m_mac->DoAddLc (lcId, lcConfig, msu);
}

void
UeMemberLteUeCmacSapProvider::RemoveLc (uint8_t lcid)
{
  m_mac->DoRemoveLc (lcid);
}

void
UeMemberLteUeCmacSapProvider::Reset ()
{
  m_mac->DoReset ();
}

void
UeMemberLteUeCmacSapProvider::NotifyConnectionSuccessful ()
{
  m_mac->DoNotifyConnectionSuccessful ();
}

void
UeMemberLteUeCmacSapProvider::SetImsi (uint64_t imsi)
{
  m_mac->DoSetImsi (imsi);
}

void
UeMemberLteUeCmacSapProvider::NotifyEdrx()
{
  m_mac->DoNotifyEdrx();
}
void
UeMemberLteUeCmacSapProvider::NotifyPsm()
{
  m_mac->DoNotifyPsm();
}

void 
UeMemberLteUeCmacSapProvider::SetMsg5Buffer(uint32_t buffersize){
  m_mac->DoSetMsg5Buffer(buffersize);
}

NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel UeMemberLteUeCmacSapProvider::GetCoverageEnhancementLevel(){
  return m_mac->DoGetCoverageEnhancementLevel();
}

/// UeMemberLteMacSapProvider class
class UeMemberLteMacSapProvider : public LteMacSapProvider
{
public:
  /**
   * Constructor
   *
   * \param mac the UE MAC
   */
  UeMemberLteMacSapProvider (LteUeMac *mac);

  // inherited from LteMacSapProvider
  virtual void TransmitPdu (TransmitPduParameters params);
  virtual void ReportBufferStatus (ReportBufferStatusParameters params);
  virtual void ReportBufferStatusNb (ReportBufferStatusParameters params,
                                     NbIotRrcSap::NpdcchMessage::SearchSpaceType searchspace);
  virtual void ReportNoTransmissionNb(uint16_t rnti, uint8_t lcid);

private:
  LteUeMac *m_mac; ///< the UE MAC
};

UeMemberLteMacSapProvider::UeMemberLteMacSapProvider (LteUeMac *mac) : m_mac (mac)
{
}

void
UeMemberLteMacSapProvider::TransmitPdu (TransmitPduParameters params)
{
  m_mac->DoTransmitPdu (params);
}

void
UeMemberLteMacSapProvider::ReportBufferStatus (ReportBufferStatusParameters params)
{
  m_mac->DoReportBufferStatus (params);
}

void
UeMemberLteMacSapProvider::ReportBufferStatusNb (
    ReportBufferStatusParameters params, NbIotRrcSap::NpdcchMessage::SearchSpaceType searchspace)
{
  m_mac->DoReportBufferStatus (params);
}

void 
UeMemberLteMacSapProvider::ReportNoTransmissionNb(uint16_t rnti, uint8_t lcid){

}

/**
 * UeMemberLteUePhySapUser
 */
class UeMemberLteUePhySapUser : public LteUePhySapUser
{
public:
  /**
   * Constructor
   *
   * \param mac the UE MAC
   */
  UeMemberLteUePhySapUser (LteUeMac *mac);

  // inherited from LtePhySapUser
  virtual void ReceivePhyPdu (Ptr<Packet> p);
  virtual void SubframeIndication (uint32_t frameNo, uint32_t subframeNo);
  virtual void ReceiveLteControlMessage (Ptr<LteControlMessage> msg);
  virtual void NotifyAboutHarqOpportunity (std::vector<std::pair<uint64_t, std::vector<uint64_t>>> subframes);

private:
  LteUeMac *m_mac; ///< the UE MAC
};

UeMemberLteUePhySapUser::UeMemberLteUePhySapUser (LteUeMac *mac) : m_mac (mac)
{
}

void
UeMemberLteUePhySapUser::ReceivePhyPdu (Ptr<Packet> p)
{
  m_mac->DoReceivePhyPdu (p);
}

void
UeMemberLteUePhySapUser::SubframeIndication (uint32_t frameNo, uint32_t subframeNo)
{
  m_mac->DoSubframeIndication (frameNo, subframeNo);
}

void
UeMemberLteUePhySapUser::ReceiveLteControlMessage (Ptr<LteControlMessage> msg)
{
  m_mac->DoReceiveLteControlMessage (msg);
}

void
UeMemberLteUePhySapUser::NotifyAboutHarqOpportunity (
    std::vector<std::pair<uint64_t, std::vector<uint64_t>>> subframes)
{
  m_mac->DoNotifyAboutHarqOpportunity (subframes);
}

//////////////////////////////////////////////////////////
// LteUeMac methods
///////////////////////////////////////////////////////////

TypeId
LteUeMac::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::LteUeMac")
          .SetParent<Object> ()
          .SetGroupName ("Lte")
          .AddConstructor<LteUeMac> ()
          .AddTraceSource ("RaResponseTimeout", "trace fired upon RA response timeout",
                           MakeTraceSourceAccessor (&LteUeMac::m_raResponseTimeoutTrace),
                           "ns3::LteUeMac::RaResponseTimeoutTracedCallback")

      ;
  return tid;
}

LteUeMac::LteUeMac ()
    : m_bsrPeriodicity (MilliSeconds (1)), // ideal behavior
      m_bsrLast (MilliSeconds (0)),
      m_freshUlBsr (false),
      m_harqProcessId (0),
      m_rnti (0),
      m_imsi (0),
      m_rachConfigured (false),
      m_waitingForRaResponse (false),
      m_transmissionScheduled(false),
      m_listenToSearchSpaces(false)

{
  NS_LOG_FUNCTION (this);
  m_miUlHarqProcessesPacket.resize (HARQ_PERIOD);
  for (uint8_t i = 0; i < m_miUlHarqProcessesPacket.size (); i++)
    {
      Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
      m_miUlHarqProcessesPacket.at (i) = pb;
    }
  m_miUlHarqProcessesPacketTimer.resize (HARQ_PERIOD, 0);

  m_macSapProvider = new UeMemberLteMacSapProvider (this);
  m_cmacSapProvider = new UeMemberLteUeCmacSapProvider (this);
  m_uePhySapUser = new UeMemberLteUePhySapUser (this);
  m_raPreambleUniformVariable = CreateObject<UniformRandomVariable> ();
  m_componentCarrierId = 0;
  m_nextIsMsg5 = false;
}

LteUeMac::~LteUeMac ()
{
  NS_LOG_FUNCTION (this);
}

void
LteUeMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_miUlHarqProcessesPacket.clear ();
  delete m_macSapProvider;
  delete m_cmacSapProvider;
  delete m_uePhySapUser;
  Object::DoDispose ();
}

LteUePhySapUser *
LteUeMac::GetLteUePhySapUser (void)
{
  return m_uePhySapUser;
}

void
LteUeMac::SetLteUePhySapProvider (LteUePhySapProvider *s)
{
  m_uePhySapProvider = s;
}

LteMacSapProvider *
LteUeMac::GetLteMacSapProvider (void)
{
  return m_macSapProvider;
}

void
LteUeMac::SetLteUeCmacSapUser (LteUeCmacSapUser *s)
{
  m_cmacSapUser = s;
}

LteUeCmacSapProvider *
LteUeMac::GetLteUeCmacSapProvider (void)
{
  return m_cmacSapProvider;
}

void
LteUeMac::SetComponentCarrierId (uint8_t index)
{
  m_componentCarrierId = index;
}
uint64_t 
LteUeMac::GetBufferSize(){
  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it;
  uint64_t buffersize=0;
  for(it = m_ulBsrReceived.begin(); it != m_ulBsrReceived.end(); ++it){
      if((*it).second.lcid > 2){
        uint64_t data_per_lc =((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
        buffersize += data_per_lc;
      }
  }
  return buffersize;
}
uint64_t 
LteUeMac::GetBufferSizeComplete(){
  uint64_t buffersize=0;
  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it;
  for(it = m_ulBsrReceived.begin(); it != m_ulBsrReceived.end(); ++it){
        uint64_t data_per_lc =((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
        buffersize += data_per_lc;
  }
  return buffersize;
}
void
LteUeMac::DoTransmitPdu (LteMacSapProvider::TransmitPduParameters params)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_rnti == params.rnti, "RNTI mismatch between RLC and MAC");
  LteRadioBearerTag radioTag (params.rnti, params.lcid, 0 /* UE works in SISO mode*/);
  DataVolumeAndPowerHeadroomTag dprTag;
  BufferStatusReportTag bsrTag;
  uint64_t bsr =0;
  //DoSetTransmissionScheduled(false);
  if(m_msg5Buffer > 0){
    // We are just about to send MSG3, add DPR Element for MSG5 (potentially CIoT-Opt)
    //std::cout << " set payload" << std::endl;
    uint8_t dataVolumeIndex = DataVolumeDPR::BufferSize2DVId(m_msg5Buffer);
    m_msg5Buffer = 0;
    m_nextIsMsg5 = true;
    dprTag.SetDataVolumeValue(dataVolumeIndex);
    params.pdu->AddPacketTag(dprTag);
  }
  else{

    bsr = GetBufferSizeComplete();
    if(bsr > 0){

      bsrTag.SetBufferStatusReportIndex(BufferSizeLevelBsr::BufferSize2BsrId (bsr));
      params.pdu->AddPacketTag(bsrTag);
    }
    // Normal PDU just add BSR for next Packet
  }
  
  params.pdu->AddPacketTag (radioTag);
  // store pdu in HARQ buffer
  //m_miUlHarqProcessesPacket.at (m_harqProcessId)->AddPacket (params.pdu);
  //m_miUlHarqProcessesPacketTimer.at (m_harqProcessId) = HARQ_PERIOD;
  m_uePhySapProvider->SendMacPdu (params.pdu);
}

void
LteUeMac::DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_LOG_FUNCTION (this << (uint32_t) params.lcid);

  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it;

  it = m_ulBsrReceived.find (params.lcid);
  if (it != m_ulBsrReceived.end ())
    {
      // update entry
      (*it).second = params;
    }
  else
    {
      m_ulBsrReceived.insert (std::pair<uint8_t, LteMacSapProvider::ReportBufferStatusParameters> (
          params.lcid, params));
    }
  m_freshUlBsr = true;
}

void
LteUeMac::SendReportBufferStatus (void)
{
  NS_LOG_FUNCTION (this);

  if (m_rnti == 0)
    {
      NS_LOG_INFO ("MAC not initialized, BSR deferred");
      return;
    }

  if (m_ulBsrReceived.size () == 0)
    {
      NS_LOG_INFO ("No BSR report to transmit");
      return;
    }
  MacCeListElement_s bsr;
  bsr.m_rnti = m_rnti;
  bsr.m_macCeType = MacCeListElement_s::BSR;

  // BSR is reported for each LCG
  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it;
  std::vector<uint32_t> queue (4, 0); // one value per each of the 4 LCGs, initialized to 0
  for (it = m_ulBsrReceived.begin (); it != m_ulBsrReceived.end (); it++)
    {
      uint8_t lcid = it->first;
      std::map<uint8_t, LcInfo>::iterator lcInfoMapIt;
      lcInfoMapIt = m_lcInfoMap.find (lcid);
      NS_ASSERT (lcInfoMapIt != m_lcInfoMap.end ());
      NS_ASSERT_MSG ((lcid != 0) ||
                         (((*it).second.txQueueSize == 0) && ((*it).second.retxQueueSize == 0) &&
                          ((*it).second.statusPduSize == 0)),
                     "BSR should not be used for LCID 0");
      uint8_t lcg = lcInfoMapIt->second.lcConfig.logicalChannelGroup;
      queue.at (lcg) +=
          ((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
    }

  // FF API says that all 4 LCGs are always present
  bsr.m_macCeValue.m_bufferStatus.push_back (BufferSizeLevelBsr::BufferSize2BsrId (queue.at (0)));
  bsr.m_macCeValue.m_bufferStatus.push_back (BufferSizeLevelBsr::BufferSize2BsrId (queue.at (1)));
  bsr.m_macCeValue.m_bufferStatus.push_back (BufferSizeLevelBsr::BufferSize2BsrId (queue.at (2)));
  bsr.m_macCeValue.m_bufferStatus.push_back (BufferSizeLevelBsr::BufferSize2BsrId (queue.at (3)));

  // create the feedback to eNB
  Ptr<BsrLteControlMessage> msg = Create<BsrLteControlMessage> ();
  msg->SetBsr (bsr);
  m_uePhySapProvider->SendLteControlMessage (msg);
}

void
LteUeMac::RandomlySelectAndSendRaPreamble ()
{
  NS_LOG_FUNCTION (this);
  // 3GPP 36.321 5.1.1
  NS_ASSERT_MSG (m_rachConfigured, "RACH not configured");
  // assume that there is no Random Access Preambles group B
  m_raPreambleId =
      m_raPreambleUniformVariable->GetInteger (0, m_rachConfig.numberOfRaPreambles - 1);
  bool contention = true;
  SendRaPreamble (contention);
}

void
LteUeMac::RandomlySelectAndSendRaPreambleNb ()
{
  NS_LOG_FUNCTION (this);
  // 3GPP 36.321 5.1.1
  NS_ASSERT_MSG (m_nprachConfigured, "NPRACH not configured");
  // assume that there is no Random Access Preambles group B
  m_raPreambleId = m_raPreambleUniformVariable->GetInteger (0, NbIotRrcSap::ConvertNprachNumSubcarriers2int (m_CeLevel) - 1);
  bool contention = true;

  // NPRACH WINDOW STARTS at framenumber mod (NPRACH_PERIOD/10) = 0 (A Tutorial on NB-IoT Physical Layer Design, Mathhieu Kanj, et al.)
  //uint32_t currentsubframe = (m_frameNo - 1)*10 +(m_subframeNo-1);
  uint32_t currentsubframe = Simulator::Now().GetMilliSeconds();
  uint16_t window_condition = ( currentsubframe/10) % (NbIotRrcSap::ConvertNprachPeriodicity2int (m_CeLevel) / 10);
  uint32_t lastPeriodStart = (currentsubframe/10) - window_condition;
  uint32_t startSubframeNprachOccasion = lastPeriodStart*10 + NbIotRrcSap::ConvertNprachStartTime2int(m_CeLevel);
  if (startSubframeNprachOccasion != currentsubframe)
    {
      uint16_t subframesToWait = 0;
      if(currentsubframe < startSubframeNprachOccasion){
        subframesToWait = startSubframeNprachOccasion-currentsubframe;
      }else{
        subframesToWait = (NbIotRrcSap::ConvertNprachPeriodicity2int(m_CeLevel) - (currentsubframe % NbIotRrcSap::ConvertNprachPeriodicity2int(m_CeLevel)))+NbIotRrcSap::ConvertNprachStartTime2int(m_CeLevel);
      }

      //uint16_t frames_to_wait = (NbIotRrcSap::ConvertNprachPeriodicity2int (m_CeLevel) - window_condition*10) + (10-(m_subframeNo-1))%10;
      NS_BUILD_DEBUG(std::cout << m_frameNo*10+m_subframeNo << std::endl);
      m_logging.push_back(currentsubframe+subframesToWait);
      NS_BUILD_DEBUG(std::cout  << "Frames to wait:" << subframesToWait << std::endl);
      Simulator::Schedule (MilliSeconds (subframesToWait), &LteUeMac::SendRaPreambleNb, this,
                           contention);
    }
  else{
    SendRaPreambleNb(contention);
  }

}
void
LteUeMac::SendRaPreamble (bool contention)
{
  NS_LOG_FUNCTION (this << (uint32_t) m_raPreambleId << contention);
  // Since regular UL LteControlMessages need m_ulConfigured = true in
  // order to be sent by the UE, the rach preamble needs to be sent
  // with a dedicated primitive (not
  // m_uePhySapProvider->SendLteControlMessage (msg)) so that it can
  // bypass the m_ulConfigured flag. This is reasonable, since In fact
  // the RACH preamble is sent on 6RB bandwidth so the uplink
  // bandwidth does not need to be configured.
  NS_ASSERT (m_subframeNo > 0); // sanity check for subframe starting at 1
  m_raRnti = m_subframeNo - 1;
  m_uePhySapProvider->SendRachPreamble (m_raPreambleId, m_raRnti);
  NS_LOG_INFO (this << " sent preamble id " << (uint32_t) m_raPreambleId << ", RA-RNTI "
                    << (uint32_t) m_raRnti);
  // 3GPP 36.321 5.1.4
  //Time raWindowBegin = MilliSeconds (3);
  //Time raWindowEnd = MilliSeconds (3 + m_rachConfig.raResponseWindowSize);
  //Simulator::Schedule (raWindowBegin, &LteUeMac::StartWaitingForRaResponse, this);
  //m_noRaResponseReceivedEvent = Simulator::Schedule (raWindowEnd, &LteUeMac::RaResponseTimeout, this, contention);
}
void
LteUeMac::SendRaPreambleNb (bool contention)
{
  NS_LOG_FUNCTION (this << (uint32_t) m_raPreambleId << contention);

  NS_ASSERT (m_frameNo > 0); // sanity check for subframe starting at 1

  // ETSI 36.321 5.1.4
  m_raRnti = 1 + floor (m_frameNo / 4);

  m_radioResourceConfig.nprachConfig.nprachCpLength =
      NbIotRrcSap::NprachConfig::NprachCpLength::us266dot7;
  double ts = 1000.0 / (15000.0 * 2048.0);
  double preambleSymbolTime = 8192.0 * ts;
  double preambleGroupTimeNoCP = 5.0 * preambleSymbolTime;
  double preambleGroupTime =
      NbIotRrcSap::ConvertNprachCpLenght2double (m_radioResourceConfig.nprachConfig) +
      preambleGroupTimeNoCP;
  double preambleRepetition = 4.0 * preambleGroupTime;
  double time = NbIotRrcSap::ConvertNumRepetitionsPerPreambleAttempt2int (m_CeLevel) *
                                  preambleRepetition;
  
  m_cmacSapUser->NotifyEnergyState(NbiotEnergyModel::PowerState::RRC_CONNECTED_SENDING_NPRACH);
  //Schedule EnergyStateChange on the next subframe after transmission
  Simulator::Schedule (MilliSeconds (time+1), &LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE);
  Simulator::Schedule (MilliSeconds (time), &LteUePhySapProvider::SendNprachPreamble,
                       m_uePhySapProvider, m_raPreambleId, m_raRnti,
                       NbIotRrcSap::ConvertNprachSubcarrierOffset2int (m_CeLevel));
  NS_LOG_INFO (this << " sent preamble id " << (uint32_t) m_raPreambleId << ", RA-RNTI "
                    << (uint32_t) m_raRnti);

  // 3GPP 36.321 5.1.4
  Time raWindowBegin;
  Time raWindowEnd;
  uint32_t npdcchPeriod = NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int (m_CeLevel) *
                          NbIotRrcSap::ConvertNpdcchStartSfCssRa2double (m_CeLevel);

  if (NbIotRrcSap::ConvertNumRepetitionsPerPreambleAttempt2int (m_CeLevel) >= 64)
    {
      raWindowBegin = MilliSeconds (41);
      NS_BUILD_DEBUG(std::cout << (m_frameNo - 1) * 10 + (m_subframeNo - 1) + time + 41 +
                       NbIotRrcSap::ConvertRaResponseWindowSize2int (m_rachConfigCe) * npdcchPeriod
                << std::endl);
      raWindowEnd = MilliSeconds (
          time + 41 + NbIotRrcSap::ConvertRaResponseWindowSize2int (m_rachConfigCe) * npdcchPeriod);
    }
  else
    {
      raWindowBegin = MilliSeconds (4);
      NS_BUILD_DEBUG(std::cout << (m_frameNo - 1) * 10 + (m_subframeNo - 1) + time + 4 +
                       NbIotRrcSap::ConvertRaResponseWindowSize2int (m_rachConfigCe) * npdcchPeriod
                << std::endl);
      raWindowEnd = MilliSeconds (
          time + 4 + NbIotRrcSap::ConvertRaResponseWindowSize2int (m_rachConfigCe) * npdcchPeriod);
    }
  //Time raWindowEnd = MilliSeconds (4 + 8*10240);
  //Time raWindowEnd = MilliSeconds (4 + m_rachConfig.raResponseWindowSize);
  NS_BUILD_DEBUG(std::cout << (m_frameNo - 1) * 10 + (m_subframeNo - 1) + time << std::endl);
  Simulator::Schedule (raWindowBegin, &LteUeMac::StartWaitingForRaResponse, this);
  m_listenToSearchSpaces = true;
  m_noRaResponseReceivedEvent =
      Simulator::Schedule (raWindowEnd, &LteUeMac::RaResponseTimeoutNb, this, contention);
}
void
LteUeMac::StartWaitingForRaResponse ()
{
  NS_LOG_FUNCTION (this);
  m_waitingForRaResponse = true;
}
void
LteUeMac::StartWaitingForRaResponseNb ()
{
  NS_LOG_FUNCTION (this);
  m_waitingForRaResponse = true;
}

void
LteUeMac::RecvRaResponse (BuildRarListElement_s raResponse)
{
  NS_LOG_FUNCTION (this);
  m_waitingForRaResponse = false;
  m_noRaResponseReceivedEvent.Cancel ();
  NS_LOG_INFO ("got RAR for RAPID " << (uint32_t) m_raPreambleId
                                    << ", setting T-C-RNTI = " << raResponse.m_rnti);
  m_rnti = raResponse.m_rnti;
  m_cmacSapUser->SetTemporaryCellRnti (m_rnti);
  // in principle we should wait for contention resolution,
  // but in the current LTE model when two or more identical
  // preambles are sent no one is received, so there is no need
  // for contention resolution
  m_cmacSapUser->NotifyRandomAccessSuccessful (false);
  // trigger tx opportunity for Message 3 over LC 0
  // this is needed since Message 3's UL GRANT is in the RAR, not in UL-DCIs
  const uint8_t lc0Lcid = 0;
  std::map<uint8_t, LcInfo>::iterator lc0InfoIt = m_lcInfoMap.find (lc0Lcid);
  NS_ASSERT (lc0InfoIt != m_lcInfoMap.end ());
  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator lc0BsrIt =
      m_ulBsrReceived.find (lc0Lcid);
  if ((lc0BsrIt != m_ulBsrReceived.end ()) && (lc0BsrIt->second.txQueueSize > 0))
    {
      NS_ASSERT_MSG (raResponse.m_grant.m_tbSize > lc0BsrIt->second.txQueueSize,
                     "segmentation of Message 3 is not allowed");
      // this function can be called only from primary carrier
      if (m_componentCarrierId > 0)
        {
          NS_FATAL_ERROR ("Function called on wrong componentCarrier");
        }
      LteMacSapUser::TxOpportunityParameters txOpParams;
      txOpParams.bytes = raResponse.m_grant.m_tbSize;
      txOpParams.layer = 0;
      txOpParams.harqId = 0;
      txOpParams.componentCarrierId = m_componentCarrierId;
      txOpParams.rnti = m_rnti;
      txOpParams.lcid = lc0Lcid;
      lc0InfoIt->second.macSapUser->NotifyTxOpportunity (txOpParams);
      lc0BsrIt->second.txQueueSize = 0;
    }
}

void
LteUeMac::RecvRaResponseNb (NbIotRrcSap::RarPayload raResponse)
{
  NS_LOG_FUNCTION (this);
  m_waitingForRaResponse = false;
  m_noRaResponseReceivedEvent.Cancel ();
  NS_LOG_INFO ("got RAR for RAPID " << (uint32_t) m_raPreambleId
                                    << ", setting T-C-RNTI = " << raResponse.cellRnti);
  m_rnti = raResponse.cellRnti;
  m_cmacSapUser->SetTemporaryCellRnti (m_rnti);
  // in principle we should wait for contention resolution,
  // but in the current LTE model when two or more identical
  // preambles are sent no one is received, so there is no need
  // for contention resolution

  // To be comented in
  bool edt;
  if(raResponse.ulGrant.tbs_size > 88){
    // We got a grant for EDT 
    edt = true;
  }else{
    edt = false;
  }
  m_cmacSapUser->NotifyRandomAccessSuccessful (edt);

  // trigger tx opportunity for Message 3 over LC 0
  // this is needed since Message 3's UL GRANT is in the RAR, not in UL-DCIs
  const uint8_t lc0Lcid = 0;
  std::map<uint8_t, LcInfo>::iterator lc0InfoIt = m_lcInfoMap.find (lc0Lcid);
  NS_ASSERT (lc0InfoIt != m_lcInfoMap.end ());
  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator lc0BsrIt =
      m_ulBsrReceived.find (lc0Lcid);
  if ((lc0BsrIt != m_ulBsrReceived.end ()) && (lc0BsrIt->second.txQueueSize > 0))
    {
      // NS_ASSERT_MSG (raResponse.m_grant.m_tbSize > lc0BsrIt->second.txQueueSize,
      //               "segmentation of Message 3 is not allowed");
      // this function can be called only from primary carrier
      if (m_componentCarrierId > 0)
        {
          NS_FATAL_ERROR ("Function called on wrong componentCarrier");
        }
      LteMacSapUser::TxOpportunityParameters txOpParams;


      // Check if we 

      txOpParams.bytes = raResponse.ulGrant.tbs_size/8;
      txOpParams.layer = 0;
      txOpParams.harqId = 0;
      txOpParams.componentCarrierId = m_componentCarrierId;
      txOpParams.rnti = m_rnti;
      txOpParams.lcid = lc0Lcid;
      int subframes = raResponse.ulGrant.subframes.second.back() -
                      (10 * (m_frameNo - 1) + m_subframeNo - 1);

      uint32_t subframesTillNpusch = raResponse.ulGrant.subframes.second.front() - (10*(m_frameNo-1)+m_subframeNo-1);

      m_transmissionScheduled = true;
      Simulator::Schedule(MilliSeconds(subframesTillNpusch), &LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_SENDING_NPUSCH);

      //EnergyStateChange on the next Subframe after Transmission Completed
      Simulator::Schedule(MilliSeconds(subframes+1), &LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE);

      //Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
      //                     lc0InfoIt->second.macSapUser, txOpParams);
      lc0InfoIt->second.macSapUser->NotifyTxOpportunityNb(txOpParams,subframes);
      lc0BsrIt->second.txQueueSize = 0;
    }
}

void
LteUeMac::RaResponseTimeout (bool contention)
{
  NS_LOG_FUNCTION (this << contention);
  m_waitingForRaResponse = false;
  // 3GPP 36.321 5.1.4
  ++m_preambleTransmissionCounter;
  //fire RA response timeout trace
  m_raResponseTimeoutTrace (m_imsi, contention, m_preambleTransmissionCounter,
                            m_rachConfig.preambleTransMax + 1);
  if (m_preambleTransmissionCounter == m_rachConfig.preambleTransMax + 1)
    {
      NS_LOG_INFO ("RAR timeout, preambleTransMax reached => giving up");
      m_cmacSapUser->NotifyRandomAccessFailed ();
    }
  else
    {
      NS_LOG_INFO ("RAR timeout, re-send preamble");
      if (contention)
        {
          RandomlySelectAndSendRaPreamble ();
        }
      else
        {
          SendRaPreamble (contention);
        }
    }
}

void
LteUeMac::RaResponseTimeoutNb (bool contention)
{
  NS_LOG_FUNCTION (this << contention);
  m_waitingForRaResponse = false;
  NS_BUILD_DEBUG(std::cout << "Window End" << std::endl);
  // 3GPP 36.321 5.1.4
  ++m_preambleTransmissionCounter;
  //fire RA response timeout trace
  m_raResponseTimeoutTrace (m_imsi, contention, m_preambleTransmissionCounter,
                            m_rachConfig.preambleTransMax + 1);
  if (m_preambleTransmissionCounter ==
      NbIotRrcSap::ConvertMaxNumPreambleAttemptCE2int (m_CeLevel) + 1)
    {
      NS_LOG_INFO ("RAR timeout, preambleTransMax reached => giving up");

      m_cmacSapUser->NotifyRandomAccessFailed ();
    }
  else
    {
      NS_LOG_INFO ("RAR timeout, re-send preamble");
      if (contention)
        {
          RandomlySelectAndSendRaPreambleNb ();
        }
      else
        {
          SendRaPreambleNb (contention);
        }
    }
}

void
LteUeMac::DoConfigureRach (LteUeCmacSapProvider::RachConfig rc)
{
  NS_LOG_FUNCTION (this);
  m_rachConfig = rc;
  m_rachConfigured = true;
}
void
LteUeMac::DoConfigureRadioResourceConfig (NbIotRrcSap::RadioResourceConfigCommonNb rc)
{
  NS_LOG_FUNCTION (this);
  m_radioResourceConfig = rc;
  m_nprachConfigured = true;
}
void
LteUeMac::DoStartContentionBasedRandomAccessProcedure ()
{
  NS_LOG_FUNCTION (this);

  // 3GPP 36.321 5.1.1
  NS_ASSERT_MSG (m_rachConfigured, "RACH not configured");
  m_preambleTransmissionCounter = 0;
  m_backoffParameter = 0;
  RandomlySelectAndSendRaPreamble ();
}
void
LteUeMac::DoStartRandomAccessProcedureNb (bool edt)
{
  NS_LOG_FUNCTION (this);

  // 3GPP 36.321 5.1.1
  NS_ASSERT_MSG (m_nprachConfigured, "RACH not configured");
  m_preambleTransmissionCounter = 0;
  m_preambleTransmissionCounterCe = 0;
  // Check CE Level
  double rsrp = m_uePhySapProvider->GetRSRP ();
  NS_BUILD_DEBUG (std::cout << "RSRP: " << rsrp << "dBm"
                            << std::endl);
  // TODO Grenzfälle // PASCAL: Was genau fehlt hier?

  NbIotRrcSap::NprachParametersNbR14 tmp; // needed if EDT is enabled // Pascal: Hier Rel14, unten Rel15?

  if (rsrp <= m_radioResourceConfig.nprachConfig.rsrpThresholdsPrachInfoList.ce2_lowerbound)
    {
      // CE2
      m_CeLevel = m_radioResourceConfig.nprachConfig.nprachParametersList.nprachParametersNb2;
      m_rachConfigCe = m_radioResourceConfig.rachConfigCommon.rachInfoList.rachInfo3;
      if(edt){
        tmp = m_radioResourceConfig.nprachConfigR15.nprachParameterListEdt.nprachParametersNb2;
      }
    }
  else if (rsrp <= m_radioResourceConfig.nprachConfig.rsrpThresholdsPrachInfoList.ce1_lowerbound)
    {
      // CE1
      m_CeLevel = m_radioResourceConfig.nprachConfig.nprachParametersList.nprachParametersNb1;
      m_rachConfigCe = m_radioResourceConfig.rachConfigCommon.rachInfoList.rachInfo2;
      if(edt){
        tmp = m_radioResourceConfig.nprachConfigR15.nprachParameterListEdt.nprachParametersNb1;
      }
    }
  else if (rsrp > m_radioResourceConfig.nprachConfig.rsrpThresholdsPrachInfoList.ce1_lowerbound)
    {
      // CE0
      m_CeLevel = m_radioResourceConfig.nprachConfig.nprachParametersList.nprachParametersNb0;
      m_rachConfigCe = m_radioResourceConfig.rachConfigCommon.rachInfoList.rachInfo1;
      if(edt){
        tmp = m_radioResourceConfig.nprachConfigR15.nprachParameterListEdt.nprachParametersNb0;
      }
    }

  if(edt){
    // Overwrite R13 config with values for R15 EDT provided
    // easiest way to access data not include in NprachParameterNBR14
    m_CeLevel.coverageEnhancementLevel= tmp.coverageEnhancementLevel;
    m_CeLevel.nprachPeriodicity = tmp.nprachPeriodicity; 
    m_CeLevel.nprachStartTime = tmp.nprachStartTime;
    m_CeLevel.nprachSubcarrierOffset = tmp.nprachSubcarrierOffset;
    m_CeLevel.nprachNumSubcarriers = tmp.nprachNumSubcarriers;
    m_CeLevel.nprachSubcarrierMsg3RangeStart= tmp.nprachSubcarrierMsg3RangeStart;
    m_CeLevel.npdcchNumRepetitionsRA = tmp.npdcchNumRepetitionsRA;
    m_CeLevel.npdcchStartSfCssRa = tmp.npdcchStartSfCssRa;
    m_CeLevel.npdcchOffsetRa = tmp.npdcchOffsetRa;

  }
  m_backoffParameter = 0;
  RandomlySelectAndSendRaPreambleNb ();
}
void
LteUeMac::DoSetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  m_rnti = rnti;
}

void
LteUeMac::DoSetImsi (uint64_t imsi)
{
  NS_LOG_FUNCTION (this);
  m_imsi = imsi;
}

void
LteUeMac::DoStartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t preambleId,
                                                          uint8_t prachMask)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t) preambleId << (uint16_t) prachMask);
  NS_ASSERT_MSG (prachMask == 0,
                 "requested PRACH MASK = " << (uint32_t) prachMask
                                           << ", but only PRACH MASK = 0 is supported");
  m_rnti = rnti;
  m_raPreambleId = preambleId;
  m_preambleTransmissionCounter = 0;
  bool contention = false;
  SendRaPreamble (contention);
}

void
LteUeMac::DoAddLc (uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig,
                   LteMacSapUser *msu)
{
  NS_LOG_FUNCTION (this << " lcId" << (uint32_t) lcId);
  NS_ASSERT_MSG (m_lcInfoMap.find (lcId) == m_lcInfoMap.end (),
                 "cannot add channel because LCID " << (uint16_t) lcId << " is already present");

  LcInfo lcInfo;
  lcInfo.lcConfig = lcConfig;
  lcInfo.macSapUser = msu;
  m_lcInfoMap[lcId] = lcInfo;
}

void
LteUeMac::DoRemoveLc (uint8_t lcId)
{
  NS_LOG_FUNCTION (this << " lcId" << lcId);
  NS_ASSERT_MSG (m_lcInfoMap.find (lcId) != m_lcInfoMap.end (), "could not find LCID " << lcId);
  m_lcInfoMap.erase (lcId);
}

void
LteUeMac::DoReset ()
{
  NS_LOG_FUNCTION (this);
  std::map<uint8_t, LcInfo>::iterator it = m_lcInfoMap.begin ();
  while (it != m_lcInfoMap.end ())
    {
      // don't delete CCCH)
      if (it->first == 0)
        {
          ++it;
        }
      else
        {
          // note: use of postfix operator preserves validity of iterator
          m_lcInfoMap.erase (it++);
        }
    }
  // note: rnti will be assigned by the eNB using RA response message
  m_rnti = 0;
  m_noRaResponseReceivedEvent.Cancel ();
  m_rachConfigured = false;
  m_freshUlBsr = false;
  m_ulBsrReceived.clear ();
}

void
LteUeMac::DoNotifyConnectionSuccessful ()
{
  NS_LOG_FUNCTION (this);
  m_uePhySapProvider->NotifyConnectionSuccessful ();
}

void
LteUeMac::DoReceivePhyPdu (Ptr<Packet> p)
{
  LteRadioBearerTag tag;
  p->RemovePacketTag (tag);
  if (tag.GetRnti () == m_rnti)
    {
      // packet is for the current user
      std::map<uint8_t, LcInfo>::const_iterator it = m_lcInfoMap.find (tag.GetLcid ());
      if (it != m_lcInfoMap.end ())
        {
          LteMacSapUser::ReceivePduParameters rxPduParams;
          rxPduParams.p = p;
          rxPduParams.rnti = m_rnti;
          rxPduParams.lcid = tag.GetLcid ();
          it->second.macSapUser->ReceivePdu (rxPduParams);
          // NB-IOT Specific Send HARQ at advertised Subframe
          if (m_nextPossibleHarqOpportunity.size () > 0)
            {
              uint32_t currentsubframe = 10 * (m_frameNo - 1) + (m_subframeNo - 1);
              uint32_t subframestillHarqF2 = m_nextPossibleHarqOpportunity[0].second.front()- currentsubframe;
              uint32_t subframestowait = m_nextPossibleHarqOpportunity[0].second.back() - currentsubframe;
              NS_BUILD_DEBUG (std::cout << "Sending HARQ Response at "
                                        << currentsubframe + subframestowait << std::endl);

              Simulator::Schedule (MilliSeconds (subframestowait),
                                   &LteUePhySapProvider::SendHarqAckResponse, m_uePhySapProvider,
                                   true);
              Simulator::Schedule(MilliSeconds(subframestillHarqF2),&LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_SENDING_NPUSCH_F2);
              Simulator::Schedule(MilliSeconds(subframestowait+1),&LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE);
              m_nextPossibleHarqOpportunity.clear();
              NS_BUILD_DEBUG (std::cout << m_rnti << " Got to MSG4-HARQ \n");
            }
        }
      else
        {
          NS_LOG_WARN ("received packet with unknown lcid " << (uint32_t) tag.GetLcid ());
        }
    }
}
void 
LteUeMac::DoSetTransmissionScheduled(bool scheduled){
  m_transmissionScheduled = scheduled;
}

void
LteUeMac::DoReceiveLteControlMessage (Ptr<LteControlMessage> msg)
{
  NS_LOG_FUNCTION (this);
  if (msg->GetMessageType () == LteControlMessage::UL_DCI)
    {
      Ptr<UlDciLteControlMessage> msg2 = DynamicCast<UlDciLteControlMessage> (msg);
      UlDciListElement_s dci = msg2->GetDci ();
      if (dci.m_ndi == 1)
        {
          // New transmission -> empty pkt buffer queue (for deleting eventual pkts not acked )
          Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
          m_miUlHarqProcessesPacket.at (m_harqProcessId) = pb;
          // Retrieve data from RLC
          std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
          uint16_t activeLcs = 0;
          uint32_t statusPduMinSize = 0;
          for (itBsr = m_ulBsrReceived.begin (); itBsr != m_ulBsrReceived.end (); itBsr++)
            {
              if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) ||
                  ((*itBsr).second.txQueueSize > 0))
                {
                  activeLcs++;
                  if (((*itBsr).second.statusPduSize != 0) &&
                      ((*itBsr).second.statusPduSize < statusPduMinSize))
                    {
                      statusPduMinSize = (*itBsr).second.statusPduSize;
                    }
                  if (((*itBsr).second.statusPduSize != 0) && (statusPduMinSize == 0))
                    {
                      statusPduMinSize = (*itBsr).second.statusPduSize;
                    }
                }
            }
          if (activeLcs == 0)
            {
              NS_LOG_ERROR (this << " No active flows for this UL-DCI");
              return;
            }
          std::map<uint8_t, LcInfo>::iterator it;
          uint32_t bytesPerActiveLc = dci.m_tbSize / activeLcs;
          bool statusPduPriority = false;
          if ((statusPduMinSize != 0) && (bytesPerActiveLc < statusPduMinSize))
            {
              // send only the status PDU which has highest priority
              statusPduPriority = true;
              NS_LOG_DEBUG (this << " Reduced resource -> send only Status, b ytes "
                                 << statusPduMinSize);
              if (dci.m_tbSize < statusPduMinSize)
                {
                  NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                }
            }
          NS_LOG_LOGIC (this << " UE " << m_rnti << ": UL-CQI notified TxOpportunity of "
                             << dci.m_tbSize << " => " << bytesPerActiveLc << " bytes per active LC"
                             << " statusPduMinSize " << statusPduMinSize);

          LteMacSapUser::TxOpportunityParameters txOpParams;

          for (it = m_lcInfoMap.begin (); it != m_lcInfoMap.end (); it++)
            {
              itBsr = m_ulBsrReceived.find ((*it).first);
              NS_LOG_DEBUG (this << " Processing LC " << (uint32_t) (*it).first
                                 << " bytesPerActiveLc " << bytesPerActiveLc);
              if ((itBsr != m_ulBsrReceived.end ()) &&
                  (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) ||
                   ((*itBsr).second.txQueueSize > 0)))
                {
                  if ((statusPduPriority) && ((*itBsr).second.statusPduSize == statusPduMinSize))
                    {
                      txOpParams.bytes = (*itBsr).second.statusPduSize;
                      txOpParams.layer = 0;
                      txOpParams.harqId = 0;
                      txOpParams.componentCarrierId = m_componentCarrierId;
                      txOpParams.rnti = m_rnti;
                      txOpParams.lcid = (*it).first;
                      (*it).second.macSapUser->NotifyTxOpportunity (txOpParams);
                      NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " send  "
                                         << (*itBsr).second.statusPduSize << " status bytes to LC "
                                         << (uint32_t) (*it).first << " statusQueue "
                                         << (*itBsr).second.statusPduSize << " retxQueue"
                                         << (*itBsr).second.retxQueueSize << " txQueue"
                                         << (*itBsr).second.txQueueSize);
                      (*itBsr).second.statusPduSize = 0;
                      break;
                    }
                  else
                    {
                      uint32_t bytesForThisLc = bytesPerActiveLc;
                      NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " bytes to LC "
                                         << (uint32_t) (*it).first << " statusQueue "
                                         << (*itBsr).second.statusPduSize << " retxQueue"
                                         << (*itBsr).second.retxQueueSize << " txQueue"
                                         << (*itBsr).second.txQueueSize);
                      if (((*itBsr).second.statusPduSize > 0) &&
                          (bytesForThisLc > (*itBsr).second.statusPduSize))
                        {
                          txOpParams.bytes = (*itBsr).second.statusPduSize;
                          txOpParams.layer = 0;
                          txOpParams.harqId = 0;
                          txOpParams.componentCarrierId = m_componentCarrierId;
                          txOpParams.rnti = m_rnti;
                          txOpParams.lcid = (*it).first;
                          (*it).second.macSapUser->NotifyTxOpportunity (txOpParams);
                          bytesForThisLc -= (*itBsr).second.statusPduSize;
                          NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
                          (*itBsr).second.statusPduSize = 0;
                        }
                      else
                        {
                          if ((*itBsr).second.statusPduSize > bytesForThisLc)
                            {
                              NS_FATAL_ERROR (
                                  "Insufficient Tx Opportunity for sending a status message");
                            }
                        }

                      if ((bytesForThisLc > 7) // 7 is the min TxOpportunity useful for Rlc
                          && (((*itBsr).second.retxQueueSize > 0) ||
                              ((*itBsr).second.txQueueSize > 0)))
                        {
                          if ((*itBsr).second.retxQueueSize > 0)
                            {
                              NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);
                              txOpParams.bytes = bytesForThisLc;
                              txOpParams.layer = 0;
                              txOpParams.harqId = 0;
                              txOpParams.componentCarrierId = m_componentCarrierId;
                              txOpParams.rnti = m_rnti;
                              txOpParams.lcid = (*it).first;
                              (*it).second.macSapUser->NotifyTxOpportunity (txOpParams);
                              if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
                                {
                                  (*itBsr).second.retxQueueSize -= bytesForThisLc;
                                }
                              else
                                {
                                  (*itBsr).second.retxQueueSize = 0;
                                }
                            }
                          else if ((*itBsr).second.txQueueSize > 0)
                            {
                              uint16_t lcid = (*it).first;
                              uint32_t rlcOverhead;
                              if (lcid == 1)
                                {
                                  // for SRB1 (using RLC AM) it's better to
                                  // overestimate RLC overhead rather than
                                  // underestimate it and risk unneeded
                                  // segmentation which increases delay
                                  rlcOverhead = 4;
                                }
                              else
                                {
                                  // minimum RLC overhead due to header
                                  rlcOverhead = 2;
                                }
                              NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc
                                                 << ", RLC overhead " << rlcOverhead);
                              txOpParams.bytes = bytesForThisLc;
                              txOpParams.layer = 0;
                              txOpParams.harqId = 0;
                              txOpParams.componentCarrierId = m_componentCarrierId;
                              txOpParams.rnti = m_rnti;
                              txOpParams.lcid = (*it).first;
                              (*it).second.macSapUser->NotifyTxOpportunity (txOpParams);
                              if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
                                {
                                  (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;
                                }
                              else
                                {
                                  (*itBsr).second.txQueueSize = 0;
                                }
                            }
                        }
                      else
                        {
                          if (((*itBsr).second.retxQueueSize > 0) ||
                              ((*itBsr).second.txQueueSize > 0))
                            {
                              // resend BSR info for updating eNB peer MAC
                              m_freshUlBsr = true;
                            }
                        }
                      NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << "\t new queues "
                                         << (uint32_t) (*it).first << " statusQueue "
                                         << (*itBsr).second.statusPduSize << " retxQueue"
                                         << (*itBsr).second.retxQueueSize << " txQueue"
                                         << (*itBsr).second.txQueueSize);
                    }
                }
            }
        }
      else
        {
          // HARQ retransmission -> retrieve data from HARQ buffer
          NS_LOG_DEBUG (this << " UE MAC RETX HARQ " << (uint16_t) m_harqProcessId);
          Ptr<PacketBurst> pb = m_miUlHarqProcessesPacket.at (m_harqProcessId);
          for (std::list<Ptr<Packet>>::const_iterator j = pb->Begin (); j != pb->End (); ++j)
            {
              Ptr<Packet> pkt = (*j)->Copy ();
              m_uePhySapProvider->SendMacPdu (pkt);
            }
          m_miUlHarqProcessesPacketTimer.at (m_harqProcessId) = HARQ_PERIOD;
        }
    }
  else if (msg->GetMessageType () == LteControlMessage::RAR)
    {
      if (m_waitingForRaResponse)
        {
          Ptr<RarLteControlMessage> rarMsg = DynamicCast<RarLteControlMessage> (msg);
          uint16_t raRnti = rarMsg->GetRaRnti ();
          NS_LOG_LOGIC (this << "got RAR with RA-RNTI " << (uint32_t) raRnti << ", expecting "
                             << (uint32_t) m_raRnti);
          if (raRnti == m_raRnti) // RAR corresponds to TX subframe of preamble
            {
              for (std::list<RarLteControlMessage::Rar>::const_iterator it =
                       rarMsg->RarListBegin ();
                   it != rarMsg->RarListEnd (); ++it)
                {
                  if (it->rapId == m_raPreambleId) // RAR is for me
                    {
                      RecvRaResponse (it->rarPayload);
                      /// \todo RRC generates the RecvRaResponse messaged
                      /// for avoiding holes in transmission at PHY layer
                      /// (which produce erroneous UL CQI evaluation)
                    }
                }
            }
        }
    }
  else if (msg->GetMessageType () == LteControlMessage::RAR_NB)
    {
      if (m_waitingForRaResponse)
        {
          Ptr<RarNbiotControlMessage> rarMsg = DynamicCast<RarNbiotControlMessage> (msg);
          uint16_t raRnti = rarMsg->GetRaRnti ();
          NS_LOG_LOGIC (this << "got RAR with RA-RNTI " << (uint32_t) raRnti << ", expecting "
                             << (uint32_t) m_raRnti);
          if (raRnti == m_raRnti) // RAR corresponds to TX subframe of preamble
            {
              for (std::list<NbIotRrcSap::Rar>::const_iterator it = rarMsg->RarListBegin ();
                   it != rarMsg->RarListEnd (); ++it)
                {
                  if (it->rapId == NbIotRrcSap::ConvertNprachSubcarrierOffset2int (m_CeLevel) +
                                       m_raPreambleId) // RAR is for me
                    {
                      RecvRaResponseNb (it->rarPayload);
                      /// \todo RRC generates the RecvRaResponse messaged
                      /// for avoiding holes in transmission at PHY layer
                      /// (which produce erroneous UL CQI evaluation)
                    }
                }
            }
        }
    }
  else if (msg->GetMessageType () == LteControlMessage::DL_DCI_NB){
      Ptr<DlDciN1NbiotControlMessage> msg2 = DynamicCast<DlDciN1NbiotControlMessage> (msg);
      NbIotRrcSap::DciN1 dci = msg2->GetDci ();
      //Handle Energy State Dci Reception
      m_cmacSapUser->NotifyEnergyState(NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE);

      uint32_t subframesTillNpdschBegin = dci.npdschOpportunity.front() - (10*(m_frameNo-1)+m_subframeNo-1);
      uint32_t subframesTillNpdschEnd = dci.npdschOpportunity.back() - (10 * (m_frameNo - 1) + m_subframeNo - 1);
      m_transmissionScheduled = true;
      Simulator::Schedule(MilliSeconds(subframesTillNpdschBegin), &LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_RECEIVING_NPDSCH);
      Simulator::Schedule(MilliSeconds(subframesTillNpdschEnd+1), &LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE);

      // Liberg p. 286 "After the device completes its NPUSCH Format 2 transmission, it is not required to monitor NPDCCH search space for 3 ms."
      if (m_nextPossibleHarqOpportunity.size() > 0){
        uint32_t subframesTransmissionEnd = m_nextPossibleHarqOpportunity[0].second.back() - (10*(m_frameNo-1)+m_subframeNo-1);
        Simulator::Schedule(MilliSeconds(subframesTransmissionEnd+3),&LteUeMac::DoSetTransmissionScheduled, this,false); // Transmission done, ready to listen to new NPDCCH
      }else{
        Simulator::Schedule(MilliSeconds(subframesTillNpdschEnd+3), &LteUeMac::DoSetTransmissionScheduled, this, false);
      }


  }
  else if (msg->GetMessageType () == LteControlMessage::UL_DCI_NB)
    {
      Ptr<UlDciN0NbiotControlMessage> msg2 = DynamicCast<UlDciN0NbiotControlMessage> (msg);
      NbIotRrcSap::DciN0 dci = msg2->GetDci ();

      m_cmacSapUser->NotifyEnergyState(NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE);

      uint32_t subframesTillNpusch = dci.npuschOpportunity[0].second.front() - (10*(m_frameNo-1)+m_subframeNo-1);
      uint32_t subframes = *(dci.npuschOpportunity[0].second.end () - 1) -
          (10 * (m_frameNo - 1) + m_subframeNo - 1);

      m_transmissionScheduled = true;
      Simulator::Schedule(MilliSeconds(subframesTillNpusch), &LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_SENDING_NPUSCH);
      Simulator::Schedule(MilliSeconds(subframes+1), &LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE);


      // Liberg p. 283 "After the device completes its NPUSCH transmission, there is at least a 3-ms gap to allow the device to switch from transmission mode to reception mode and be ready for monitoring the next NPDCCH search space candidate."
      uint32_t subframesTransmissionEnd = dci.npuschOpportunity[0].second.back() - (10*(m_frameNo-1)+m_subframeNo-1);
      Simulator::Schedule(MilliSeconds(subframesTransmissionEnd+3),&LteUeMac::DoSetTransmissionScheduled, this,false); // Transmission done, ready to listen to new NPDCCH

      if (dci.NDI)
        {
          // New transmission -> empty pkt buffer queue (for deleting eventual pkts not acked )
          Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
          m_miUlHarqProcessesPacket.at (m_harqProcessId) = pb;
          // Retrieve data from RLC
          std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
          std::vector<uint8_t> activeLcs;
          for (itBsr = m_ulBsrReceived.begin (); itBsr != m_ulBsrReceived.end (); itBsr++)
          {
            if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) ||
                ((*itBsr).second.txQueueSize > 0))
              {
                if(m_nextIsMsg5){
                  // We might get a bigger TxOp as the size of the MSG5, so we don't want user data transmitted there
                  if(itBsr->first >2){
                    continue;
                  }
                }
                activeLcs.push_back(itBsr->first);
              }
          }
          if(m_nextIsMsg5){
            m_nextIsMsg5 = false;
          }
          LteMacSapUser::TxOpportunityParameters txOpParams;
          // Prioritise SRBs over DataBs
          uint64_t bytesforallLc = dci.tbs/8;
          for(std::vector<uint8_t>::iterator lcit = activeLcs.begin(); lcit != activeLcs.end(); ++lcit){
            std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator bsr = m_ulBsrReceived.find((*lcit));
            std::map<uint8_t, LcInfo>::iterator lcidIt = m_lcInfoMap.find (bsr->second.lcid);
            if ((bsr->second.statusPduSize > 0) &&
                    (bytesforallLc > bsr->second.statusPduSize))
              {
                txOpParams.bytes = bsr->second.statusPduSize;
                txOpParams.layer = 0;
                txOpParams.harqId = 0;
                txOpParams.componentCarrierId = m_componentCarrierId;
                txOpParams.rnti = bsr->second.rnti;
                txOpParams.lcid = bsr->second.lcid;
                //Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
                //    (*lcidIt).second.macSapUser, txOpParams);
                (*lcidIt).second.macSapUser->NotifyTxOpportunityNb(txOpParams,subframes);
                bytesforallLc -= bsr->second.statusPduSize;
                bsr->second.statusPduSize = 0;
              }
            else
              {
                if (bsr->second.statusPduSize > bytesforallLc)
                  {
                    //NS_FATAL_ERROR (
                    //    "Insufficient Tx Opportunity for sending a status message");
                  }
              }

            if ((bytesforallLc> 7) // 7 is the min TxOpportunity useful for Rlc
                && ((bsr->second.retxQueueSize > 0) ||
                    (bsr->second.txQueueSize > 0)))
              {
                if (bsr->second.retxQueueSize > 0)
                  {
                    NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesforallLc);
                    if(bsr->second.retxQueueSize > bytesforallLc){
                      txOpParams.bytes = bytesforallLc;
                      bsr->second.retxQueueSize -= bytesforallLc;
                      bytesforallLc = 0;
                    }else{
                      if(bsr->second.retxQueueSize +4 < 7){
                        txOpParams.bytes = 7;
                        bytesforallLc -= 7;
                      }else{
                        txOpParams.bytes = bsr->second.retxQueueSize+4;
                        bytesforallLc -= bsr->second.retxQueueSize+4;
                      }
                        bsr->second.retxQueueSize = 0;
                    }
                    txOpParams.layer = 0;
                    txOpParams.harqId = 0;
                    txOpParams.componentCarrierId = m_componentCarrierId;
                    txOpParams.rnti = bsr->second.rnti;
                    txOpParams.lcid = bsr->second.lcid;
                    //Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
                    //  (*lcidIt).second.macSapUser, txOpParams);
                    (*lcidIt).second.macSapUser->NotifyTxOpportunityNb(txOpParams,subframes);
                  }
                else if (bsr->second.txQueueSize > 0)
                  {
                    uint16_t lcid = bsr->second.lcid;
                    uint32_t rlcOverhead;
                    if (lcid == 1 || lcid == 3)
                      {
                        // for SRB1 (using RLC AM) it's better to
                        // overestimate RLC overhead rather than
                        // underestimate it and risk unneeded
                        // segmentation which increases delay
                        rlcOverhead = 4;
                      }
                    else
                      {
                        // minimum RLC overhead due to header
                        rlcOverhead = 2;
                      }
                    NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesforallLc 
                                        << ", RLC overhead " << rlcOverhead);
                    if(bsr->second.txQueueSize > bytesforallLc){
                      txOpParams.bytes = bytesforallLc;
                      bsr->second.txQueueSize -= bytesforallLc-rlcOverhead;
                      bytesforallLc = 0;
                    }else{
                      if(bsr->second.txQueueSize +4 < 7){
                        txOpParams.bytes = 7;
                        bytesforallLc -= 7;
                      }else{
                        txOpParams.bytes = bsr->second.txQueueSize+4;
                        bytesforallLc -= bsr->second.txQueueSize+4;
                      }

                      bsr->second.txQueueSize = 0;
                    }
                    txOpParams.layer = 0;
                    txOpParams.harqId = 0;
                    txOpParams.componentCarrierId = m_componentCarrierId;
                    txOpParams.rnti = bsr->second.rnti;
                    txOpParams.lcid = bsr->second.lcid;

                    //Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
                    //  (*lcidIt).second.macSapUser, txOpParams);
                    (*lcidIt).second.macSapUser->NotifyTxOpportunityNb(txOpParams,subframes);
                    
                  }
                  }
                else
                  {
                    if ((bsr->second.retxQueueSize > 0) ||
                        (bsr->second.txQueueSize > 0))
                      {
                        NS_BUILD_DEBUG(std::cout << "Not enough space" << std::endl);
                      }
                  }
          }
          //uint16_t activeLcs = 0;
          //uint32_t statusPduMinSize = 0;
          //for (itBsr = m_ulBsrReceived.begin (); itBsr != m_ulBsrReceived.end (); itBsr++)
          //  {
          //    if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) ||
          //        ((*itBsr).second.txQueueSize > 0))
          //      {
          //        activeLcs++;
          //        if (((*itBsr).second.statusPduSize != 0) &&
          //            ((*itBsr).second.statusPduSize < statusPduMinSize))
          //          {
          //            statusPduMinSize = (*itBsr).second.statusPduSize;
          //          }
          //        if (((*itBsr).second.statusPduSize != 0) && (statusPduMinSize == 0))
          //          {
          //            statusPduMinSize = (*itBsr).second.statusPduSize;
          //          }
          //      }
          //  }
          //if (activeLcs == 0)
          //  {
          //    NS_LOG_ERROR (this << " No active flows for this UL-DCI");
          //    return;
          //  }
          //std::map<uint8_t, LcInfo>::iterator it;
          //uint32_t bytesPerActiveLc = (dci.tbs/8)/ activeLcs;
          //bool statusPduPriority = false;
          //if ((statusPduMinSize != 0) && (bytesPerActiveLc < statusPduMinSize))
          //  {
          //    // send only the status PDU which has highest priority
          //    statusPduPriority = true;
          //    NS_LOG_DEBUG (this << " Reduced resource -> send only Status, b ytes "
          //                       << statusPduMinSize);
          //    if (dci.tbs/8< statusPduMinSize)
          //      {
          //        NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
          //      }
          //  }
          //NS_LOG_LOGIC (this << " UE " << m_rnti << ": UL-CQI notified TxOpportunity of "
          //                   << dci.tbs << " => " << bytesPerActiveLc << " bytes per active LC"
          //                   << " statusPduMinSize " << statusPduMinSize);

          //LteMacSapUser::TxOpportunityParameters txOpParams;

          //for (it = m_lcInfoMap.begin (); it != m_lcInfoMap.end (); it++)
          //  {
          //    itBsr = m_ulBsrReceived.find ((*it).first);
          //    NS_LOG_DEBUG (this << " Processing LC " << (uint32_t) (*it).first
          //                       << " bytesPerActiveLc " << bytesPerActiveLc);
          //    if ((itBsr != m_ulBsrReceived.end ()) &&
          //        (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) ||
          //         ((*itBsr).second.txQueueSize > 0)))
          //      {
          //        if ((statusPduPriority) && ((*itBsr).second.statusPduSize == statusPduMinSize))
          //          {
          //            txOpParams.bytes = (*itBsr).second.statusPduSize;
          //            txOpParams.layer = 0;
          //            txOpParams.harqId = 0;
          //            txOpParams.componentCarrierId = m_componentCarrierId;
          //            txOpParams.rnti = m_rnti;
          //            txOpParams.lcid = (*it).first;
          //            Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
          //                    it->second.macSapUser, txOpParams);
          //            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " send  "
          //                               << (*itBsr).second.statusPduSize << " status bytes to LC "
          //                               << (uint32_t) (*it).first << " statusQueue "
          //                               << (*itBsr).second.statusPduSize << " retxQueue"
          //                               << (*itBsr).second.retxQueueSize << " txQueue"
          //                               << (*itBsr).second.txQueueSize);
          //            (*itBsr).second.statusPduSize = 0;
          //            break;
          //          }
          //        else
          //          {
          //            uint32_t bytesForThisLc = bytesPerActiveLc;
          //            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " bytes to LC "
          //                               << (uint32_t) (*it).first << " statusQueue "
          //                               << (*itBsr).second.statusPduSize << " retxQueue"
          //                               << (*itBsr).second.retxQueueSize << " txQueue"
          //                               << (*itBsr).second.txQueueSize);
          //            if (((*itBsr).second.statusPduSize > 0) &&
          //                (bytesForThisLc > (*itBsr).second.statusPduSize))
          //              {
          //                txOpParams.bytes = (*itBsr).second.statusPduSize;
          //                txOpParams.layer = 0;
          //                txOpParams.harqId = 0;
          //                txOpParams.componentCarrierId = m_componentCarrierId;
          //                txOpParams.rnti = m_rnti;
          //                txOpParams.lcid = (*it).first;
          //                Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
          //                    it->second.macSapUser, txOpParams);
          //                bytesForThisLc -= (*itBsr).second.statusPduSize;
          //                NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
          //                (*itBsr).second.statusPduSize = 0;
          //              }
          //            else
          //              {
          //                if ((*itBsr).second.statusPduSize > bytesForThisLc)
          //                  {
          //                    NS_FATAL_ERROR (
          //                        "Insufficient Tx Opportunity for sending a status message");
          //                  }
          //              }

          //            if ((bytesForThisLc > 7) // 7 is the min TxOpportunity useful for Rlc
          //                && (((*itBsr).second.retxQueueSize > 0) ||
          //                    ((*itBsr).second.txQueueSize > 0)))
          //              {
          //                if ((*itBsr).second.retxQueueSize > 0)
          //                  {
          //                    NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);
          //                    txOpParams.bytes = bytesForThisLc;
          //                    txOpParams.layer = 0;
          //                    txOpParams.harqId = 0;
          //                    txOpParams.componentCarrierId = m_componentCarrierId;
          //                    txOpParams.rnti = m_rnti;
          //                    txOpParams.lcid = (*it).first;
          //                    Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
          //                      it->second.macSapUser, txOpParams);
          //                    if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
          //                      {
          //                        (*itBsr).second.retxQueueSize -= bytesForThisLc;
          //                      }
          //                    else
          //                      {
          //                        (*itBsr).second.retxQueueSize = 0;
          //                      }
          //                  }
          //                else if ((*itBsr).second.txQueueSize > 0)
          //                  {
          //                    uint16_t lcid = (*it).first;
          //                    uint32_t rlcOverhead;
          //                    if (lcid == 1)
          //                      {
          //                        // for SRB1 (using RLC AM) it's better to
          //                        // overestimate RLC overhead rather than
          //                        // underestimate it and risk unneeded
          //                        // segmentation which increases delay
          //                        rlcOverhead = 4;
          //                      }
          //                    else
          //                      {
          //                        // minimum RLC overhead due to header
          //                        rlcOverhead = 2;
          //                      }
          //                    NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc
          //                                       << ", RLC overhead " << rlcOverhead);
          //                    txOpParams.bytes = bytesForThisLc;
          //                    txOpParams.layer = 0;
          //                    txOpParams.harqId = 0;
          //                    txOpParams.componentCarrierId = m_componentCarrierId;
          //                    txOpParams.rnti = m_rnti;
          //                    txOpParams.lcid = (*it).first;

          //                    Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
          //                      it->second.macSapUser, txOpParams);
          //                    if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
          //                      {
          //                        (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;
          //                      }
          //                    else
          //                      {
          //                        (*itBsr).second.txQueueSize = 0;
          //                      }
          //                  }
          //              }
          //            else
          //              {
          //                if (((*itBsr).second.retxQueueSize > 0) ||
          //                    ((*itBsr).second.txQueueSize > 0))
          //                  {
          //                    // resend BSR info for updating eNB peer MAC
          //                    m_freshUlBsr = true;
          //                  }
          //              }
          //            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << "\t new queues "
          //                               << (uint32_t) (*it).first << " statusQueue "
          //                               << (*itBsr).second.statusPduSize << " retxQueue"
          //                               << (*itBsr).second.retxQueueSize << " txQueue"
          //                               << (*itBsr).second.txQueueSize);
          //          }
          //      }
          //  }
        }
      else
        {
          // HARQ retransmission -> retrieve data from HARQ buffer
          NS_LOG_DEBUG (this << " UE MAC RETX HARQ " << (uint16_t) m_harqProcessId);
          Ptr<PacketBurst> pb = m_miUlHarqProcessesPacket.at (m_harqProcessId);
          for (std::list<Ptr<Packet>>::const_iterator j = pb->Begin (); j != pb->End (); ++j)
            {
              Ptr<Packet> pkt = (*j)->Copy ();
              Simulator::Schedule (MilliSeconds (subframes), &LteUePhySapProvider::SendMacPdu ,
                m_uePhySapProvider, pkt);
              //m_uePhySapProvider->SendMacPdu (pkt);
            }
          m_miUlHarqProcessesPacketTimer.at (m_harqProcessId) = HARQ_PERIOD;
        }
    }
  else
    {
      NS_LOG_WARN (this << " LteControlMessage not recognized");
    }
}
void
LteUeMac::DoNotifyAboutHarqOpportunity (std::vector<std::pair<uint64_t, std::vector<uint64_t>>> subframes)
{
  m_nextPossibleHarqOpportunity = subframes;
}

void
LteUeMac::RefreshHarqProcessesPacketBuffer (void)
{
  NS_LOG_FUNCTION (this);

  for (uint16_t i = 0; i < m_miUlHarqProcessesPacketTimer.size (); i++)
    {
      if (m_miUlHarqProcessesPacketTimer.at (i) == 0)
        {
          if (m_miUlHarqProcessesPacket.at (i)->GetSize () > 0)
            {
              // timer expired: drop packets in buffer for this process
              NS_LOG_INFO (this << " HARQ Proc Id " << i << " packets buffer expired");
              Ptr<PacketBurst> emptyPb = CreateObject<PacketBurst> ();
              m_miUlHarqProcessesPacket.at (i) = emptyPb;
            }
        }
      else
        {
          m_miUlHarqProcessesPacketTimer.at (i)--;
        }
    }
}

void
LteUeMac::DoSubframeIndication (uint32_t frameNo, uint32_t subframeNo)
{
  NS_LOG_FUNCTION (this);
  m_frameNo = frameNo;
  m_subframeNo = subframeNo;
  //RefreshHarqProcessesPacketBuffer ();
  //
  if(m_edrx && GetBufferSizeComplete() == 0 && !m_transmissionScheduled){
    m_listenToSearchSpaces = false;
    m_cmacSapUser->NotifyEnergyState(NbiotEnergyModel::PowerState::RRC_SUSPENDED_EDRX);
    m_edrx = false;
    // TODO Activate Paging Occasion listening
  }
  if(m_psm && GetBufferSizeComplete() == 0 && !m_transmissionScheduled){
    m_listenToSearchSpaces = false;
    m_cmacSapUser->NotifyEnergyState(NbiotEnergyModel::PowerState::RRC_SUSPENDED_PSM);
    // TODO Activate Paging Occasion listening
    m_psm = false;
  }
  if(m_listenToSearchSpaces){
    // Energy Model Start Receiving on my SearchSpaceBegin
    uint32_t searchSpacePeriodicity = NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int (m_CeLevel) *
                                      NbIotRrcSap::ConvertNpdcchStartSfCssRa2double (m_CeLevel);
    uint32_t searchSpaceConditionLeftSide =
        (10 * (m_frameNo - 1) + (m_subframeNo - 1)) % searchSpacePeriodicity;
    uint32_t searchSpaceConditionRightSide =
        NbIotRrcSap::ConvertNpdcchOffsetRa2double (m_CeLevel) * searchSpacePeriodicity;

    if (searchSpaceConditionLeftSide == searchSpaceConditionRightSide) 
      {
        m_inSearchSpace=true;
        m_subframesInSearchSpace = 0;
      }
    if (m_inSearchSpace){ 
      if(!m_transmissionScheduled && m_cmacSapUser->GetEnergyState() == NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE){
        // We just moved from another state into IDLE
        // According to Liberg p.286 we still have to monitor the rest of the NPDCCH
        // Offset like the 3ms after NPUSCH F2 schould be handled by the m_transmissionScheduled flag
        m_cmacSapUser->NotifyEnergyState(NbiotEnergyModel::PowerState::RRC_CONNECTED_RECEIVING_NPDCCH);
      }
      if (((m_subframeNo-1) != 0) && ((m_subframeNo-1) != 5) && !((m_subframeNo-1) == 9 && ((m_frameNo-1) % 2) == 1)) // Current Subframe is not NPBCH, NPSS and NSSS, and SI #TODO add SI
        {
          m_subframesInSearchSpace++; 
        }
      if(m_subframesInSearchSpace == NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int (m_CeLevel)){
        m_inSearchSpace = false;
        m_subframesInSearchSpace = 0;
        if(m_cmacSapUser->GetEnergyState() == NbiotEnergyModel::PowerState::RRC_CONNECTED_RECEIVING_NPDCCH){
          m_cmacSapUser->NotifyEnergyState(NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE); // Device listened to the whole SearchSpace without Dci scheduled
        }
      }
    }
  }
  if ((Simulator::Now () >= m_bsrLast + m_bsrPeriodicity) && (m_freshUlBsr == true))
    {
      if (m_componentCarrierId == 0)
        {
          //Send BSR through primary carrier
          SendReportBufferStatus ();
        }
      m_bsrLast = Simulator::Now ();
      m_freshUlBsr = false;
    }
  m_harqProcessId = (m_harqProcessId + 1) % HARQ_PERIOD;
}

int64_t
LteUeMac::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_raPreambleUniformVariable->SetStream (stream);
  return 1;
}
void 
LteUeMac::DoNotifyEdrx(){
  m_edrx = true;
}

void 
LteUeMac::DoNotifyPsm(){
  m_psm = true;
}

NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel 
LteUeMac::DoGetCoverageEnhancementLevel(){
  return m_CeLevel.coverageEnhancementLevel;
}

void LteUeMac::DoSetMsg5Buffer(uint32_t buffersize){
  m_msg5Buffer = buffersize;
}

} // namespace ns3
