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
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 *         Nicola Baldo  <nbaldo@cttc.es>
 * Modified by:
 *          Danilo Abrignani <danilo.abrignani@unibo.it> (Carrier Aggregation - GSoC 2015)
 *          Biljana Bojovic <biljana.bojovic@cttc.es> (Carrier Aggregation)
 */

#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/packet.h>
#include <ns3/simulator.h>
#include <ns3/build-profile.h>
#include "lte-amc.h"
#include "lte-control-messages.h"
#include "lte-enb-net-device.h"
#include "lte-ue-net-device.h"

#include "ns3/lte-enb-mac.h"
#include <ns3/lte-radio-bearer-tag.h>
#include <ns3/lte-ue-phy.h>

#include "ns3/lte-mac-sap.h"
#include "ns3/lte-enb-cmac-sap.h"
#include <ns3/lte-common.h>

#include "nb-iot-data-volume-and-power-headroom-tag.h"
#include "nb-iot-buffer-status-report-tag.h"
#include "lte-rlc-am-header.h"

#include <algorithm>
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteEnbMac");

NS_OBJECT_ENSURE_REGISTERED (LteEnbMac);

// //////////////////////////////////////
// member SAP forwarders
// //////////////////////////////////////

/// EnbMacMemberLteEnbCmacSapProvider class
class EnbMacMemberLteEnbCmacSapProvider : public LteEnbCmacSapProvider
{
public:
  /**
   * Constructor
   *
   * \param mac the MAC
   */
  EnbMacMemberLteEnbCmacSapProvider (LteEnbMac *mac);

  // inherited from LteEnbCmacSapProvider
  virtual void ConfigureMac (uint16_t ulBandwidth, uint16_t dlBandwidth);
  virtual void AddUe (uint16_t rnti);
  virtual void RemoveUe (uint16_t rnti);
  virtual void MoveUeToResume(uint16_t rnti,uint64_t resumeId);
  virtual void ResumeUe(uint16_t rnti,uint64_t resumeId);
  virtual void RemoveUeFromScheduler(uint16_t rnti);
  virtual void AddLc (LcInfo lcinfo, LteMacSapUser *msu);
  virtual void ReconfigureLc (LcInfo lcinfo);
  virtual void ReleaseLc (uint16_t rnti, uint8_t lcid);
  virtual void UeUpdateConfigurationReq (UeConfig params);
  virtual RachConfig GetRachConfig ();
  virtual RachConfigNb GetRachConfigNb ();
  virtual void NotifyConnectionSuccessful(uint16_t rnti);
  virtual AllocateNcRaPreambleReturnValue AllocateNcRaPreamble (uint16_t rnti);
  virtual void SetLogDir(std::string logdir);

private:
  LteEnbMac *m_mac; ///< the MAC
};

EnbMacMemberLteEnbCmacSapProvider::EnbMacMemberLteEnbCmacSapProvider (LteEnbMac *mac) : m_mac (mac)
{
}

void
EnbMacMemberLteEnbCmacSapProvider::ConfigureMac (uint16_t ulBandwidth, uint16_t dlBandwidth)
{
  m_mac->DoConfigureMac (ulBandwidth, dlBandwidth);
}

void
EnbMacMemberLteEnbCmacSapProvider::AddUe (uint16_t rnti)
{
  m_mac->DoAddUe (rnti);
}

void
EnbMacMemberLteEnbCmacSapProvider::RemoveUe (uint16_t rnti)
{
  m_mac->DoRemoveUe (rnti);
}
void
EnbMacMemberLteEnbCmacSapProvider::MoveUeToResume(uint16_t rnti, uint64_t resumeId)
{
  m_mac->DoMoveUeToResume(rnti,resumeId);
}
void
EnbMacMemberLteEnbCmacSapProvider::RemoveUeFromScheduler(uint16_t rnti)
{
  m_mac->DoRemoveUeFromScheduler(rnti);
}
void
EnbMacMemberLteEnbCmacSapProvider::ResumeUe(uint16_t rnti, uint64_t resumeId)
{
  m_mac->DoResumeUe(rnti,resumeId);
}
void
EnbMacMemberLteEnbCmacSapProvider::AddLc (LcInfo lcinfo, LteMacSapUser *msu)
{
  m_mac->DoAddLc (lcinfo, msu);
}

void
EnbMacMemberLteEnbCmacSapProvider::ReconfigureLc (LcInfo lcinfo)
{
  m_mac->DoReconfigureLc (lcinfo);
}

void
EnbMacMemberLteEnbCmacSapProvider::ReleaseLc (uint16_t rnti, uint8_t lcid)
{
  m_mac->DoReleaseLc (rnti, lcid);
}

void
EnbMacMemberLteEnbCmacSapProvider::UeUpdateConfigurationReq (UeConfig params)
{
  m_mac->DoUeUpdateConfigurationReq (params);
}

LteEnbCmacSapProvider::RachConfig
EnbMacMemberLteEnbCmacSapProvider::GetRachConfig ()
{
  return m_mac->DoGetRachConfig ();
}

LteEnbCmacSapProvider::RachConfigNb
EnbMacMemberLteEnbCmacSapProvider::GetRachConfigNb ()
{
  return m_mac->DoGetRachConfigNb ();
}

void 
EnbMacMemberLteEnbCmacSapProvider::NotifyConnectionSuccessful(uint16_t rnti){
  m_mac->DoNotifyConnectionSuccessful(rnti);
}

LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue
EnbMacMemberLteEnbCmacSapProvider::AllocateNcRaPreamble (uint16_t rnti)
{
  return m_mac->DoAllocateNcRaPreamble (rnti);
}

void
EnbMacMemberLteEnbCmacSapProvider::SetLogDir(std::string logdir)
{
  return m_mac->DoSetLogDir(logdir);
}

/// EnbMacMemberFfMacSchedSapUser class
class EnbMacMemberFfMacSchedSapUser : public FfMacSchedSapUser
{
public:
  /**
   * Constructor
   * 
   * \param mac the MAC
   */
  EnbMacMemberFfMacSchedSapUser (LteEnbMac *mac);

  virtual void SchedDlConfigInd (const struct SchedDlConfigIndParameters &params);
  virtual void SchedUlConfigInd (const struct SchedUlConfigIndParameters &params);

private:
  LteEnbMac *m_mac; ///< the MAC
};

EnbMacMemberFfMacSchedSapUser::EnbMacMemberFfMacSchedSapUser (LteEnbMac *mac) : m_mac (mac)
{
}

void
EnbMacMemberFfMacSchedSapUser::SchedDlConfigInd (const struct SchedDlConfigIndParameters &params)
{
  m_mac->DoSchedDlConfigInd (params);
}

void
EnbMacMemberFfMacSchedSapUser::SchedUlConfigInd (const struct SchedUlConfigIndParameters &params)
{
  m_mac->DoSchedUlConfigInd (params);
}

/// EnbMacMemberFfMacCschedSapUser class
class EnbMacMemberFfMacCschedSapUser : public FfMacCschedSapUser
{
public:
  /**
   * Constructor
   *
   * \param mac the MAC
   */
  EnbMacMemberFfMacCschedSapUser (LteEnbMac *mac);

  virtual void CschedCellConfigCnf (const struct CschedCellConfigCnfParameters &params);
  virtual void CschedUeConfigCnf (const struct CschedUeConfigCnfParameters &params);
  virtual void CschedLcConfigCnf (const struct CschedLcConfigCnfParameters &params);
  virtual void CschedLcReleaseCnf (const struct CschedLcReleaseCnfParameters &params);
  virtual void CschedUeReleaseCnf (const struct CschedUeReleaseCnfParameters &params);
  virtual void CschedUeConfigUpdateInd (const struct CschedUeConfigUpdateIndParameters &params);
  virtual void CschedCellConfigUpdateInd (const struct CschedCellConfigUpdateIndParameters &params);

private:
  LteEnbMac *m_mac; ///< the MAC
};

EnbMacMemberFfMacCschedSapUser::EnbMacMemberFfMacCschedSapUser (LteEnbMac *mac) : m_mac (mac)
{
}

void
EnbMacMemberFfMacCschedSapUser::CschedCellConfigCnf (
    const struct CschedCellConfigCnfParameters &params)
{
  m_mac->DoCschedCellConfigCnf (params);
}

void
EnbMacMemberFfMacCschedSapUser::CschedUeConfigCnf (const struct CschedUeConfigCnfParameters &params)
{
  m_mac->DoCschedUeConfigCnf (params);
}

void
EnbMacMemberFfMacCschedSapUser::CschedLcConfigCnf (const struct CschedLcConfigCnfParameters &params)
{
  m_mac->DoCschedLcConfigCnf (params);
}

void
EnbMacMemberFfMacCschedSapUser::CschedLcReleaseCnf (
    const struct CschedLcReleaseCnfParameters &params)
{
  m_mac->DoCschedLcReleaseCnf (params);
}

void
EnbMacMemberFfMacCschedSapUser::CschedUeReleaseCnf (
    const struct CschedUeReleaseCnfParameters &params)
{
  m_mac->DoCschedUeReleaseCnf (params);
}

void
EnbMacMemberFfMacCschedSapUser::CschedUeConfigUpdateInd (
    const struct CschedUeConfigUpdateIndParameters &params)
{
  m_mac->DoCschedUeConfigUpdateInd (params);
}

void
EnbMacMemberFfMacCschedSapUser::CschedCellConfigUpdateInd (
    const struct CschedCellConfigUpdateIndParameters &params)
{
  m_mac->DoCschedCellConfigUpdateInd (params);
}

/// ---------- PHY-SAP
class EnbMacMemberLteEnbPhySapUser : public LteEnbPhySapUser
{
public:
  /**
   * Constructor
   *
   * \param mac the MAC
   */
  EnbMacMemberLteEnbPhySapUser (LteEnbMac *mac);

  // inherited from LteEnbPhySapUser
  virtual void ReceivePhyPdu (Ptr<Packet> p);
  virtual void SubframeIndication (uint32_t frameNo, uint32_t subframeNo);
  virtual void ReceiveLteControlMessage (Ptr<LteControlMessage> msg);
  virtual void ReceiveRachPreamble (uint32_t prachId);
  virtual void ReceiveNprachPreamble (uint32_t prachId, uint8_t subcarrierOffset, uint32_t ranti);
  virtual void UlCqiReport (FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi);
  virtual void UlCqiReportNb (std::vector<double> ulcqi);
  virtual void UlInfoListElementHarqFeeback (UlInfoListElement_s params);
  virtual void DlInfoListElementHarqFeeback (DlInfoListElement_s params);

private:
  LteEnbMac *m_mac; ///< the MAC
};

EnbMacMemberLteEnbPhySapUser::EnbMacMemberLteEnbPhySapUser (LteEnbMac *mac) : m_mac (mac)
{
}

void
EnbMacMemberLteEnbPhySapUser::ReceivePhyPdu (Ptr<Packet> p)
{
  m_mac->DoReceivePhyPdu (p);
}

void
EnbMacMemberLteEnbPhySapUser::SubframeIndication (uint32_t frameNo, uint32_t subframeNo)
{
  m_mac->DoSubframeIndicationNb (frameNo, subframeNo);
}

void
EnbMacMemberLteEnbPhySapUser::ReceiveLteControlMessage (Ptr<LteControlMessage> msg)
{
  m_mac->DoReceiveLteControlMessage (msg);
}

void
EnbMacMemberLteEnbPhySapUser::ReceiveRachPreamble (uint32_t prachId)
{
  m_mac->DoReceiveRachPreamble (prachId);
}

void
EnbMacMemberLteEnbPhySapUser::ReceiveNprachPreamble (uint32_t prachId, uint8_t subcarrierOffset,
                                                     uint32_t ranti)
{
  m_mac->DoReceiveNprachPreamble (prachId, subcarrierOffset, ranti);
}
void
EnbMacMemberLteEnbPhySapUser::UlCqiReport (FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi)
{
  m_mac->DoUlCqiReport (ulcqi);
}
void
EnbMacMemberLteEnbPhySapUser::UlCqiReportNb (std::vector<double> ulcqi)
{
  m_mac->DoUlCqiReportNb (ulcqi);
}
void
EnbMacMemberLteEnbPhySapUser::UlInfoListElementHarqFeeback (UlInfoListElement_s params)
{
  m_mac->DoUlInfoListElementHarqFeeback (params);
}

void
EnbMacMemberLteEnbPhySapUser::DlInfoListElementHarqFeeback (DlInfoListElement_s params)
{
  m_mac->DoDlInfoListElementHarqFeeback (params);
}

// //////////////////////////////////////
// generic LteEnbMac methods
// //////////////////////////////////////

TypeId
LteEnbMac::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::LteEnbMac")
          .SetParent<Object> ()
          .SetGroupName ("Lte")
          .AddConstructor<LteEnbMac> ()
          .AddAttribute ("NumberOfRaPreambles",
                         "how many random access preambles are available for the contention based "
                         "RACH process",
                         UintegerValue (52),
                         MakeUintegerAccessor (&LteEnbMac::m_numberOfRaPreambles),
                         MakeUintegerChecker<uint8_t> (4, 64))
          .AddAttribute ("PreambleTransMax",
                         "Maximum number of random access preamble transmissions",
                         UintegerValue (50), MakeUintegerAccessor (&LteEnbMac::m_preambleTransMax),
                         MakeUintegerChecker<uint8_t> (3, 200))
          .AddAttribute ("RaResponseWindowSize",
                         "length of the window (in TTIs) for the reception of the random access "
                         "response (RAR); the resulting RAR timeout is this value + 3 ms",
                         UintegerValue (3),
                         MakeUintegerAccessor (&LteEnbMac::m_raResponseWindowSize),
                         MakeUintegerChecker<uint8_t> (2, 10))
          .AddAttribute ("ConnEstFailCount", "how many time T300 timer can expire on the same cell",
                         UintegerValue (1), MakeUintegerAccessor (&LteEnbMac::m_connEstFailCount),
                         MakeUintegerChecker<uint8_t> (1, 4))
          .AddTraceSource ("DlScheduling", "Information regarding DL scheduling.",
                           MakeTraceSourceAccessor (&LteEnbMac::m_dlScheduling),
                           "ns3::LteEnbMac::DlSchedulingTracedCallback")
          .AddTraceSource ("UlScheduling", "Information regarding UL scheduling.",
                           MakeTraceSourceAccessor (&LteEnbMac::m_ulScheduling),
                           "ns3::LteEnbMac::UlSchedulingTracedCallback")
          .AddAttribute ("ComponentCarrierId",
                         "ComponentCarrier Id, needed to reply on the appropriate sap.",
                         UintegerValue (0), MakeUintegerAccessor (&LteEnbMac::m_componentCarrierId),
                         MakeUintegerChecker<uint8_t> (0, 4));

  return tid;
}

LteEnbMac::LteEnbMac () : m_ccmMacSapUser (0)
{
  NS_LOG_FUNCTION (this);
  m_macSapProvider = new EnbMacMemberLteMacSapProvider<LteEnbMac> (this);
  m_cmacSapProvider = new EnbMacMemberLteEnbCmacSapProvider (this);
  m_schedSapUser = new EnbMacMemberFfMacSchedSapUser (this);
  m_cschedSapUser = new EnbMacMemberFfMacCschedSapUser (this);
  m_enbPhySapUser = new EnbMacMemberLteEnbPhySapUser (this);
  m_ccmMacSapProvider = new MemberLteCcmMacSapProvider<LteEnbMac> (this);
  m_dropPreambleCollision = true;
}

LteEnbMac::~LteEnbMac ()
{
  NS_LOG_FUNCTION (this);
}

void
LteEnbMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_dlCqiReceived.clear ();
  m_ulCqiReceived.clear ();
  m_ulCeReceived.clear ();
  m_dlInfoListReceived.clear ();
  m_ulInfoListReceived.clear ();
  m_miDlHarqProcessesPackets.clear ();
  m_schedulerNb->DoDispose();
  delete m_macSapProvider;
  delete m_cmacSapProvider;
  delete m_schedSapUser;
  delete m_cschedSapUser;
  delete m_enbPhySapUser;
  delete m_ccmMacSapProvider;
}

void
LteEnbMac::SetComponentCarrierId (uint8_t index)
{
  m_componentCarrierId = index;
}

void
LteEnbMac::SetFfMacSchedSapProvider (FfMacSchedSapProvider *s)
{
  m_schedSapProvider = s;
}

FfMacSchedSapUser *
LteEnbMac::GetFfMacSchedSapUser (void)
{
  return m_schedSapUser;
}

void
LteEnbMac::SetFfMacCschedSapProvider (FfMacCschedSapProvider *s)
{
  m_cschedSapProvider = s;
}

FfMacCschedSapUser *
LteEnbMac::GetFfMacCschedSapUser (void)
{
  return m_cschedSapUser;
}

void
LteEnbMac::SetLteMacSapUser (LteMacSapUser *s)
{
  m_macSapUser = s;
}

LteMacSapProvider *
LteEnbMac::GetLteMacSapProvider (void)
{
  return m_macSapProvider;
}

void
LteEnbMac::SetLteEnbCmacSapUser (LteEnbCmacSapUser *s)
{
  m_cmacSapUser = s;
}

LteEnbCmacSapProvider *
LteEnbMac::GetLteEnbCmacSapProvider (void)
{
  return m_cmacSapProvider;
}

void
LteEnbMac::SetLteEnbPhySapProvider (LteEnbPhySapProvider *s)
{
  m_enbPhySapProvider = s;
}

LteEnbPhySapUser *
LteEnbMac::GetLteEnbPhySapUser ()
{
  return m_enbPhySapUser;
}

void
LteEnbMac::SetLteCcmMacSapUser (LteCcmMacSapUser *s)
{
  m_ccmMacSapUser = s;
}

LteCcmMacSapProvider *
LteEnbMac::GetLteCcmMacSapProvider ()
{
  return m_ccmMacSapProvider;
}

void
LteEnbMac::DoSubframeIndication (uint32_t frameNo, uint32_t subframeNo)
{
  NS_LOG_FUNCTION (this << " EnbMac - frame " << frameNo << " subframe " << subframeNo);

  // Store current frame / subframe number
  m_frameNo = frameNo;
  m_subframeNo = subframeNo;

  // --- DOWNLINK ---
  // Send Dl-CQI info to the scheduler
  if (m_dlCqiReceived.size () > 0)
    {
      FfMacSchedSapProvider::SchedDlCqiInfoReqParameters dlcqiInfoReq;
      dlcqiInfoReq.m_sfnSf = ((0x3FF & frameNo) << 4) | (0xF & subframeNo);
      dlcqiInfoReq.m_cqiList.insert (dlcqiInfoReq.m_cqiList.begin (), m_dlCqiReceived.begin (),
                                     m_dlCqiReceived.end ());
      m_dlCqiReceived.erase (m_dlCqiReceived.begin (), m_dlCqiReceived.end ());
      m_schedSapProvider->SchedDlCqiInfoReq (dlcqiInfoReq);
    }

  if (!m_receivedRachPreambleCount.empty ())
    {
      // process received RACH preambles and notify the scheduler
      FfMacSchedSapProvider::SchedDlRachInfoReqParameters rachInfoReqParams;
      NS_ASSERT (subframeNo > 0 && subframeNo <= 10); // subframe in 1..10
      for (std::map<uint8_t, uint32_t>::const_iterator it = m_receivedRachPreambleCount.begin ();
           it != m_receivedRachPreambleCount.end (); ++it)
        {
          NS_LOG_INFO (this << " preambleId " << (uint32_t) it->first << ": " << it->second
                            << " received");
          NS_ASSERT (it->second != 0);
          if (it->second > 1)
            {
              NS_LOG_INFO ("preambleId " << (uint32_t) it->first << ": collision");
              // in case of collision we assume that no preamble is
              // successfully received, hence no RAR is sent
            }
          else
            {
              uint16_t rnti;
              std::map<uint8_t, NcRaPreambleInfo>::iterator jt =
                  m_allocatedNcRaPreambleMap.find (it->first);
              if (jt != m_allocatedNcRaPreambleMap.end ())
                {
                  rnti = jt->second.rnti;
                  NS_LOG_INFO ("preambleId previously allocated for NC based RA, RNTI ="
                               << (uint32_t) rnti << ", sending RAR");
                }
              else
                {
                  rnti = m_cmacSapUser->AllocateTemporaryCellRnti ();
                  NS_LOG_INFO ("preambleId " << (uint32_t) it->first << ": allocated T-C-RNTI "
                                             << (uint32_t) rnti << ", sending RAR");
                }

              RachListElement_s rachLe;
              rachLe.m_rnti = rnti;
              rachLe.m_estimatedSize = 144; // to be confirmed
              rachInfoReqParams.m_rachList.push_back (rachLe);
              m_rapIdRntiMap.insert (std::pair<uint16_t, uint32_t> (rnti, it->first));
            }
        }
      m_schedSapProvider->SchedDlRachInfoReq (rachInfoReqParams);
      m_receivedRachPreambleCount.clear ();
    }
  // Get downlink transmission opportunities
  uint32_t dlSchedFrameNo = m_frameNo;
  uint32_t dlSchedSubframeNo = m_subframeNo;
  // NS_LOG_DEBUG (this << " sfn " << frameNo << " sbfn " << subframeNo);
  if (dlSchedSubframeNo + m_macChTtiDelay > 10)
    {
      dlSchedFrameNo++;
      dlSchedSubframeNo = (dlSchedSubframeNo + m_macChTtiDelay) % 10;
    }
  else
    {
      dlSchedSubframeNo = dlSchedSubframeNo + m_macChTtiDelay;
    }
  FfMacSchedSapProvider::SchedDlTriggerReqParameters dlparams;
  dlparams.m_sfnSf = ((0x3FF & dlSchedFrameNo) << 4) | (0xF & dlSchedSubframeNo);

  // Forward DL HARQ feebacks collected during last TTI
  if (m_dlInfoListReceived.size () > 0)
    {
      dlparams.m_dlInfoList = m_dlInfoListReceived;
      // empty local buffer
      m_dlInfoListReceived.clear ();
    }

  m_schedSapProvider->SchedDlTriggerReq (dlparams);

  // --- UPLINK ---
  // Send UL-CQI info to the scheduler
  for (uint16_t i = 0; i < m_ulCqiReceived.size (); i++)
    {
      if (subframeNo > 1)
        {
          m_ulCqiReceived.at (i).m_sfnSf = ((0x3FF & frameNo) << 4) | (0xF & (subframeNo - 1));
        }
      else
        {
          m_ulCqiReceived.at (i).m_sfnSf = ((0x3FF & (frameNo - 1)) << 4) | (0xF & 10);
        }
      m_schedSapProvider->SchedUlCqiInfoReq (m_ulCqiReceived.at (i));
    }
  m_ulCqiReceived.clear ();

  // Send BSR reports to the scheduler
  if (m_ulCeReceived.size () > 0)
    {
      FfMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters ulMacReq;
      ulMacReq.m_sfnSf = ((0x3FF & frameNo) << 4) | (0xF & subframeNo);
      ulMacReq.m_macCeList.insert (ulMacReq.m_macCeList.begin (), m_ulCeReceived.begin (),
                                   m_ulCeReceived.end ());
      m_ulCeReceived.erase (m_ulCeReceived.begin (), m_ulCeReceived.end ());
      m_schedSapProvider->SchedUlMacCtrlInfoReq (ulMacReq);
    }

  // Get uplink transmission opportunities
  uint32_t ulSchedFrameNo = m_frameNo;
  uint32_t ulSchedSubframeNo = m_subframeNo;
  //   NS_LOG_DEBUG (this << " sfn " << frameNo << " sbfn " << subframeNo);
  if (ulSchedSubframeNo + (m_macChTtiDelay + UL_PUSCH_TTIS_DELAY) > 10)
    {
      ulSchedFrameNo++;
      ulSchedSubframeNo = (ulSchedSubframeNo + (m_macChTtiDelay + UL_PUSCH_TTIS_DELAY)) % 10;
    }
  else
    {
      ulSchedSubframeNo = ulSchedSubframeNo + (m_macChTtiDelay + UL_PUSCH_TTIS_DELAY);
    }
  FfMacSchedSapProvider::SchedUlTriggerReqParameters ulparams;
  ulparams.m_sfnSf = ((0x3FF & ulSchedFrameNo) << 4) | (0xF & ulSchedSubframeNo);

  // Forward DL HARQ feebacks collected during last TTI
  if (m_ulInfoListReceived.size () > 0)
    {
      ulparams.m_ulInfoList = m_ulInfoListReceived;
      // empty local buffer
      m_ulInfoListReceived.clear ();
    }

  m_schedSapProvider->SchedUlTriggerReq (ulparams);
}

void LteEnbMac::CheckPreambleReceptionForAllCoverageClases(){
  //CE0 
  CheckIfPreambleWasReceived(m_ce0Parameter, false);
  //CE1 
  CheckIfPreambleWasReceived(m_ce1Parameter, false);
  //CE2 
  CheckIfPreambleWasReceived(m_ce2Parameter, false);
  if(m_edt){

    //CE0 Edt
    CheckIfPreambleWasReceived(m_ce0ParameterEdt, true);
    //CE1 Edt
    CheckIfPreambleWasReceived(m_ce1ParameterEdt, true);
    //CE2 Edt
    CheckIfPreambleWasReceived(m_ce2ParameterEdt, true);
  }
}

void
LteEnbMac::CheckIfPreambleWasReceived (NbIotRrcSap::NprachParametersNb ce, bool edt) 
{
  uint32_t currentsubframe = Simulator::Now().GetMilliSeconds();
  uint16_t window_condition = ( currentsubframe/10) % (NbIotRrcSap::ConvertNprachPeriodicity2int (ce) / 10);
  uint32_t lastPeriodStart = (currentsubframe/10) - window_condition;
  uint32_t startSubframeNprachOccasion = lastPeriodStart*10 + NbIotRrcSap::ConvertNprachStartTime2int(ce);
  uint16_t timeSinceOcassion = currentsubframe - startSubframeNprachOccasion;

  m_sib2Nb.radioResourceConfigCommon.nprachConfig.nprachCpLength =
      NbIotRrcSap::NprachConfig::NprachCpLength::us266dot7;
  double ts = 1000.0 / (15000.0 * 2048.0);
  double preambleSymbolTime = 8192.0 * ts;
  double preambleGroupTimeNoCP = 5.0 * preambleSymbolTime;
  double preambleGroupTime =
      NbIotRrcSap::ConvertNprachCpLenght2double (m_sib2Nb.radioResourceConfigCommon.nprachConfig) +
      preambleGroupTimeNoCP;
  double preambleRepetition = 4.0 * preambleGroupTime;
  double time = NbIotRrcSap::ConvertNumRepetitionsPerPreambleAttempt2int (ce) *
                                  preambleRepetition;

  if (std::ceil(time)+1 != timeSinceOcassion){ 
    return;
  }



  std::map<uint8_t, uint32_t> receivedNprachs;
  uint8_t subcarrierOffset = NbIotRrcSap::ConvertNprachSubcarrierOffset2int (ce);

  receivedNprachs = m_receivedNprachPreambleCount[subcarrierOffset];

  std::vector<std::pair<int, NbIotRrcSap::Rar>> m_rarQueue; // Mapping Ranti -> Rar
  NbIotRrcSap::NpdcchMessage rar_dci;


  if (receivedNprachs.size () > 0)
    {
      //int rnti = m_cmacSapUser->AllocateTemporaryCellRnti ();

      for (std::map<uint8_t, uint32_t>::iterator iter = receivedNprachs.begin ();
           iter != receivedNprachs.end (); ++iter)
        {
          if (iter->second == 1)
            { // sanity check. Actually should be always equal

              NS_BUILD_DEBUG (std::cout << "Preamble received of offset " << int (subcarrierOffset)
                                        << " at Subframe "
                                        << (10 * (m_frameNo - 1) + (m_subframeNo - 1)) << std::endl);
              NbIotRrcSap::Rar rar;
              rar.cellRnti = m_cmacSapUser->AllocateTemporaryCellRnti ();
              rar.rapId = subcarrierOffset + iter->first;
              rar.rarPayload.cellRnti = rar.cellRnti;
              m_rarQueue.push_back (
                  std::make_pair (m_rapIdRantiMap[subcarrierOffset + iter->first], rar));
              m_receivedNprachPreambleCount[subcarrierOffset].erase (iter->first);
            }
          else if (iter->second > 1)
            {
              // Collision
              // Action depends on how to handle the collisions
              // 2 possible actions:
              // a) Preambles interfere with each other, so all UEs lose
              // b) eNB does cotention resolution magic and one UE surives
              NS_BUILD_DEBUG(std::cout << "==================================================" <<  std::endl);
              NS_BUILD_DEBUG (std::cout << "Collision" << std::endl);
              NS_BUILD_DEBUG(std::cout << "==================================================" <<  std::endl);
              if (m_dropPreambleCollision)
                {
                  m_receivedNprachPreambleCount[subcarrierOffset].erase (iter->first);
                  continue;
                }
              else
                {
                  m_rapIdCollisionMap[subcarrierOffset + iter->first] = true;
                  NS_BUILD_DEBUG (std::cout << "Preamble received of offset "
                                            << int (subcarrierOffset) << " at Subframe "
                                            << (10 * (m_frameNo - 1) + (m_subframeNo - 1)) << std::endl);
                  NbIotRrcSap::Rar rar;
                  rar.cellRnti = m_cmacSapUser->AllocateTemporaryCellRnti ();
                  rar.rapId = subcarrierOffset + iter->first;
                  rar.rarPayload.cellRnti = rar.cellRnti;
                  rar_dci.isRar = true;
                  rar_dci.isEdt = edt;
                  rar_dci.rars.push_back (rar);
                  rar_dci.ranti = m_rapIdRantiMap[subcarrierOffset + iter->first];
                  m_receivedNprachPreambleCount[subcarrierOffset].erase (iter->first);
                  m_RntiCeMap.insert (
                      std::pair<uint32_t,
                                NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel> (
                          rar.cellRnti, ce.coverageEnhancementLevel));
                }
            }
        }
      std::vector<NbIotRrcSap::NpdcchMessage> rar_dcis;
      if (m_rarQueue.size () > 0)
        {
          int size_mac_pdu = 0;
          size_mac_pdu += 8; // E/T/R/R/BI Header 8 bit ETSI 136.321 13.7 6.1.5-2
          int max_pdu_size_mac = 680;
          int size_mac_sdu_header = 8; // E/T/RAPID Header 8 bit ETSI 136.321 13.7 6.1.5-1
          int size_rar = 48; // MAC RAR for NB-IoT UEs 48 bit ETSI 136.321 13.7 6.1.5-3b
          while ((m_rarQueue.size () > 0))
            {
              // Create new DCI to be scheduled
              rar_dcis.push_back (NbIotRrcSap::NpdcchMessage ());
              rar_dcis.back ().ranti = m_rarQueue.front ().first;
              rar_dcis.back ().npdcchFormat = NbIotRrcSap::NpdcchMessage::NpdcchFormat::format1;
              rar_dcis.back ().dciType = NbIotRrcSap::NpdcchMessage::DciType::n1;
              rar_dcis.back ().searchSpaceType = NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2;
              rar_dcis.back ().ce = ce.coverageEnhancementLevel;
              rar_dcis.back ().isRar = true;
              rar_dcis.back ().isEdt = edt;
              rar_dcis.back ().dciN1.numNpdschSubframesPerRepetition =
                  NbIotRrcSap::DciN1::NumNpdschSubframesPerRepetition::s2;
              rar_dcis.back ().dciN1.numNpdschRepetitions =
                  NbIotRrcSap::DciN1::NumNpdschRepetitions::r2;
              // while there is space in the MAC PDU fill in as many RARs as fit. Otherwise create new DCI
              while ((m_rarQueue.size () > 0) &&
                     (size_mac_pdu + size_mac_sdu_header + size_rar < max_pdu_size_mac))
                {
                  size_mac_pdu += size_mac_sdu_header;
                  size_mac_pdu += size_rar;
                  std::pair<int, NbIotRrcSap::Rar> rar = m_rarQueue.front ();
                  rar_dcis.back ().rars.push_back (rar.second);
                  m_rarQueue.erase (m_rarQueue.begin ());
                }
            }

          // Dci set depending on coverage level.... yet static
          //rar_dci.dciN1.dciRepetitions = NbIotRrcSap::DciN1::DciRepetitions::r1;
          for (std::vector<NbIotRrcSap::NpdcchMessage>::iterator it = rar_dcis.begin ();
               it != rar_dcis.end (); ++it)
            {
              m_schedulerNb->ScheduleRarReq(*it,NbiotScheduler::ConvertNprachParametersNb2SearchSpaceConfig(ce));
            }
        }
    }
}

void
LteEnbMac::VerySimpleNbiotDownlinkScheduler ()
{
  //bool isCe0SearchSpace = false;
  uint32_t searchSpacePeriodicityce0 =
      NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int (m_ce0Parameter) *
      NbIotRrcSap::ConvertNpdcchStartSfCssRa2double (m_ce0Parameter);
  uint32_t searchSpaceConditionLeftSidece0 =
      (10 * (m_frameNo - 1) + (m_subframeNo - 1)) % searchSpacePeriodicityce0;
  uint32_t searchSpaceConditionRightSidece0 =
      NbIotRrcSap::ConvertNpdcchOffsetRa2double (m_ce0Parameter) * searchSpacePeriodicityce0;
  int8_t subcarrieroffset = NbIotRrcSap::ConvertNprachSubcarrierOffset2int (m_ce0Parameter);
  if (searchSpaceConditionLeftSidece0 == searchSpaceConditionRightSidece0)
    {
      // IS CE1 SEACH SPACE
      m_SearchSpaceType2C0 = true;
      m_SearchSpaceType2C0SfBegin = 0;

      if (m_DlDcis[subcarrieroffset].size () > 0)
        {
          Ptr<DlDciN1NbiotControlMessage> msg = Create<DlDciN1NbiotControlMessage> ();
          msg->SetDci (*(m_DlDcis[subcarrieroffset].begin ()));
          m_enbPhySapProvider->SendLteControlMessage (msg);
          m_currentRepetitions = R - 1;
        }
    }
  else if (m_SearchSpaceType2C0 && (m_currentRepetitions > 0))
    {
      if (m_DlDcis[subcarrieroffset].size () > 0)
        {
          Ptr<DlDciN1NbiotControlMessage> msg = Create<DlDciN1NbiotControlMessage> ();
          msg->SetDci (*(m_DlDcis[subcarrieroffset].begin ()));
          m_enbPhySapProvider->SendLteControlMessage (msg);
          m_currentRepetitions--;
        }
    }
  // else if ((m_SearchSpaceType2C0SfBegin + R ==  && m_SearchSpaceType2C0 && (m_currentRepetitions==0)){
  //   m_DlDcis[subcarrieroffset].erase(m_DlDcis[subcarrieroffset].begin());
  //   if (m_DlDcis[subcarrieroffset].size() > 0){
  //       Ptr<DlDciN1NbiotControlMessage> msg = Create<DlDciN1NbiotControlMessage>();
  //       msg->SetDci(*(m_DlDcis[subcarrieroffset].begin()));
  //       m_enbPhySapProvider->SendLteControlMessage (msg);
  //       m_currentRepetitions = R-1;
  //   }else{
  //     std::cout << "No DCI to send" << std::endl;
  //   }
  // }
  if (m_SearchSpaceType2C0)
    {
      m_SearchSpaceType2C0SfBegin++;
    }
  // CE1 Type2
  //bool isCe1SearchSpace = false;
  uint32_t searchSpacePeriodicityce1 =
      NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int (m_ce1Parameter) *
      NbIotRrcSap::ConvertNpdcchStartSfCssRa2double (m_ce1Parameter);
  uint32_t searchSpaceConditionLeftSidece1 =
      (10 * (m_frameNo - 1) + (m_subframeNo - 1)) % searchSpacePeriodicityce1;
  uint32_t searchSpaceConditionRightSidece1 =
      NbIotRrcSap::ConvertNpdcchOffsetRa2double (m_ce1Parameter) * searchSpacePeriodicityce1;
  if (searchSpaceConditionLeftSidece1 == searchSpaceConditionRightSidece1)
    {
    }

  // CE2 Type 2
  //bool isCe2SearchSpace = false;
  uint32_t searchSpacePeriodicityce2 =
      NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int (m_ce2Parameter) *
      NbIotRrcSap::ConvertNpdcchStartSfCssRa2double (m_ce2Parameter);
  uint32_t searchSpaceConditionLeftSidece2 =
      (10 * (m_frameNo - 1) + (m_subframeNo - 1)) % searchSpacePeriodicityce2;
  uint32_t searchSpaceConditionRightSidece2 =
      NbIotRrcSap::ConvertNpdcchOffsetRa2double (m_ce2Parameter) * searchSpacePeriodicityce2;
  if (searchSpaceConditionLeftSidece2 == searchSpaceConditionRightSidece2)
    {
    }
}
void LteEnbMac::SetCoverageLevelAndSib2Nb(){
  m_sib2Nb = m_cmacSapUser->GetCurrentSystemInformationBlockType2Nb ();
  m_ce0Parameter =
      m_sib2Nb.radioResourceConfigCommon.nprachConfig.nprachParametersList.nprachParametersNb0;
  m_ce1Parameter =
          m_sib2Nb.radioResourceConfigCommon.nprachConfig.nprachParametersList.nprachParametersNb1;
  m_ce2Parameter =
          m_sib2Nb.radioResourceConfigCommon.nprachConfig.nprachParametersList.nprachParametersNb2;

  // Temp variables to assign relevant edt parameters
  NbIotRrcSap::NprachParametersNb tmp;
  NbIotRrcSap::NprachParametersNbR14 tmpr14;

  // EDT CE0
  tmp = m_sib2Nb.radioResourceConfigCommon.nprachConfig.nprachParametersList.nprachParametersNb0;
  tmpr14 = m_sib2Nb.radioResourceConfigCommon.nprachConfigR15.nprachParameterListEdt.nprachParametersNb0;
  tmp.coverageEnhancementLevel= tmpr14.coverageEnhancementLevel;
  tmp.nprachPeriodicity = tmpr14.nprachPeriodicity; 
  tmp.nprachStartTime = tmpr14.nprachStartTime;
  tmp.nprachSubcarrierOffset = tmpr14.nprachSubcarrierOffset;
  tmp.nprachNumSubcarriers = tmpr14.nprachNumSubcarriers;
  tmp.nprachSubcarrierMsg3RangeStart= tmpr14.nprachSubcarrierMsg3RangeStart;
  tmp.npdcchNumRepetitionsRA = tmpr14.npdcchNumRepetitionsRA;
  tmp.npdcchStartSfCssRa = tmpr14.npdcchStartSfCssRa;
  tmp.npdcchOffsetRa = tmpr14.npdcchOffsetRa;

  m_ce0ParameterEdt = tmp;
 
  // EDT CE1
  tmp = m_sib2Nb.radioResourceConfigCommon.nprachConfig.nprachParametersList.nprachParametersNb1;
  tmpr14 = m_sib2Nb.radioResourceConfigCommon.nprachConfigR15.nprachParameterListEdt.nprachParametersNb1;
  tmp.coverageEnhancementLevel= tmpr14.coverageEnhancementLevel;
  tmp.nprachPeriodicity = tmpr14.nprachPeriodicity; 
  tmp.nprachStartTime = tmpr14.nprachStartTime;
  tmp.nprachSubcarrierOffset = tmpr14.nprachSubcarrierOffset;
  tmp.nprachNumSubcarriers = tmpr14.nprachNumSubcarriers;
  tmp.nprachSubcarrierMsg3RangeStart= tmpr14.nprachSubcarrierMsg3RangeStart;
  tmp.npdcchNumRepetitionsRA = tmpr14.npdcchNumRepetitionsRA;
  tmp.npdcchStartSfCssRa = tmpr14.npdcchStartSfCssRa;
  tmp.npdcchOffsetRa = tmpr14.npdcchOffsetRa;     

  m_ce1ParameterEdt = tmp;
 
  // EDT CE2
  tmp = m_sib2Nb.radioResourceConfigCommon.nprachConfig.nprachParametersList.nprachParametersNb2;
  tmpr14 = m_sib2Nb.radioResourceConfigCommon.nprachConfigR15.nprachParameterListEdt.nprachParametersNb2;
  tmp.coverageEnhancementLevel= tmpr14.coverageEnhancementLevel;
  tmp.nprachPeriodicity = tmpr14.nprachPeriodicity; 
  tmp.nprachStartTime = tmpr14.nprachStartTime;
  tmp.nprachSubcarrierOffset = tmpr14.nprachSubcarrierOffset;
  tmp.nprachNumSubcarriers = tmpr14.nprachNumSubcarriers;
  tmp.nprachSubcarrierMsg3RangeStart= tmpr14.nprachSubcarrierMsg3RangeStart;
  tmp.npdcchNumRepetitionsRA = tmpr14.npdcchNumRepetitionsRA;
  tmp.npdcchStartSfCssRa = tmpr14.npdcchStartSfCssRa;
  tmp.npdcchOffsetRa = tmpr14.npdcchOffsetRa;     

  m_ce2ParameterEdt = tmp;

}
void
LteEnbMac::DoSubframeIndicationNb (uint32_t frameNo, uint32_t subframeNo)
{
  NS_LOG_FUNCTION (this << " EnbMac - frame " << frameNo << " subframe " << subframeNo);
  m_edt = true;
  m_frameNo = frameNo;
  m_subframeNo = subframeNo;
  if (m_schedulerNb == nullptr)
    {
      SetCoverageLevelAndSib2Nb();

      m_schedulerNb = new NbiotScheduler (
          std::vector<NbIotRrcSap::NprachParametersNb>{m_ce0Parameter, m_ce1Parameter,
                                                       m_ce2Parameter},
          m_sib2Nb);
      m_schedulerNb->SetLogDir(m_logdir);
    }
  // Implement NB-IoT DCI Searchspaces Type2-CSS All AL2  Liberg et al. p 282
  // Find out if current subframe is start of Type2/UE-specific search space
  // A Tutorial to NB-IoT Design zeugs


  CheckPreambleReceptionForAllCoverageClases();
  //m_schedulerNb->SetCeLevel (m_ce0Parameter, m_ce1Parameter, m_ce2Parameter);
  m_schedulerNb->SetRntiRsrpMap (m_ulRsrpReceivedNb);
  std::vector<NbIotRrcSap::NpdcchMessage> scheduled = m_schedulerNb->Schedule (frameNo, subframeNo);

  int currentsubframe = 10 * (m_frameNo - 1) + (m_subframeNo - 1);
  std::map<int16_t, bool> contention_resolution;
  for (std::vector<NbIotRrcSap::NpdcchMessage>::iterator it = scheduled.begin ();
       it != scheduled.end (); ++it)
    {

      if (it->isRar)
        {
          Ptr<RarNbiotControlMessage> msg = Create<RarNbiotControlMessage> ();
          for (std::vector<NbIotRrcSap::Rar>::iterator rar = it->rars.begin ();
               rar != it->rars.end (); ++rar)
            {
              msg->AddRar (*rar);
            }
          msg->SetRaRnti (it->ranti);
          m_connectionSuccessful[it->rnti] = false;
          int subframestowait = *(it->dciN1.npdschOpportunity.end () - 1) - currentsubframe;
          Simulator::Schedule (MilliSeconds (subframestowait),
                               &LteEnbPhySapProvider::SendLteControlMessage, m_enbPhySapProvider,
                               msg);
        }
      else
        {
          if (it->dciType == NbIotRrcSap::NpdcchMessage::DciType::n1)
            {
              if (!contention_resolution[it->rnti])
                {
                  int subframestowait = *(it->dciN1.npdschOpportunity.end () - 1) - currentsubframe;
                  std::map<uint16_t, std::map<uint8_t, LteMacSapUser *>>::iterator rntiIt =
                      m_rlcAttached.find (it->rnti);
                  NS_ASSERT_MSG (rntiIt != m_rlcAttached.end (), "could not find RNTI" << it->rnti);
                  //NS_LOG_DEBUG (this << " rnti= " << rnti << " lcid= " << (uint32_t) lcid << " layer= " << k);
                  std::vector<uint8_t> activeLcs;
                  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
                  for (itBsr = m_lastDlBSR[it->rnti].begin (); itBsr != m_lastDlBSR[it->rnti].end (); itBsr++)
                  {
                    if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) ||
                        ((*itBsr).second.txQueueSize > 0))
                      {
                        activeLcs.push_back(itBsr->first);
                      }
                  }
                  LteMacSapUser::TxOpportunityParameters txOpParams;
                  // Prioritise SRBs over DataBs
                  uint64_t bytesforallLc = it->tbs/8;
                  for(std::vector<uint8_t>::iterator lcit = activeLcs.begin(); lcit != activeLcs.end(); ++lcit){
                    std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator bsr = m_lastDlBSR[it->rnti].find((*lcit));
                    std::map<uint8_t, LteMacSapUser *>::iterator lcidIt = rntiIt->second.find (bsr->second.lcid);
                    if ((bsr->second.statusPduSize > 0) &&
                            (bytesforallLc >= bsr->second.statusPduSize))
                      {
                        txOpParams.bytes = bsr->second.statusPduSize;
                        txOpParams.layer = 0;
                        txOpParams.harqId = 0;
                        txOpParams.componentCarrierId = m_componentCarrierId;
                        txOpParams.rnti = bsr->second.rnti;
                        txOpParams.lcid = bsr->second.lcid;
                        (*lcidIt).second->NotifyTxOpportunityNb(txOpParams, subframestowait);
                        //Simulator::Schedule (MilliSeconds (subframestowait), &LteMacSapUser::NotifyTxOpportunity,
                        //   (*lcidIt).second, txOpParams);
                        bytesforallLc -= bsr->second.statusPduSize;
                        bsr->second.statusPduSize = 0;
                      }
                    else
                      {
                        if (bsr->second.statusPduSize > bytesforallLc)
                          {
                            NS_FATAL_ERROR (
                                "Insufficient Tx Opportunity for sending a status message");
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
                            //Simulator::Schedule (MilliSeconds (subframestowait), &LteMacSapUser::NotifyTxOpportunity,
                            //  (*lcidIt).second, txOpParams);
                            (*lcidIt).second->NotifyTxOpportunityNb(txOpParams, subframestowait);
                          }
                        else if (bsr->second.txQueueSize > 0)
                          {

                            if(bsr->second.txQueueSize > bytesforallLc){
                              txOpParams.bytes = bytesforallLc;
                              bsr->second.txQueueSize -= bytesforallLc;
                              bytesforallLc = 0;
                            }else{
                              if(bsr->second.txQueueSize+4 < 7){
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

                            //Simulator::Schedule (MilliSeconds (subframestowait), &LteMacSapUser::NotifyTxOpportunity,
                            //  (*lcidIt).second, txOpParams);
                            (*lcidIt).second->NotifyTxOpportunityNb(txOpParams,subframestowait);
                            
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
                  std::vector<uint8_t> remaining;
                  for (itBsr = m_lastDlBSR[it->rnti].begin (); itBsr != m_lastDlBSR[it->rnti].end (); itBsr++)
                  {
                    if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) ||
                        ((*itBsr).second.txQueueSize > 0))
                      {
                        remaining.push_back(itBsr->first);
                      }
                  }
                  if(remaining.size()>0){
                    m_schedulerNb->ScheduleDlRlcBufferReq(it->rnti,m_lastDlBSR[it->rnti]);
                  }
                }
            }
        }
      if (!contention_resolution[it->rnti])
        {
          if (it->dciType == NbIotRrcSap::NpdcchMessage::DciType::n1)
            {

              Ptr<DlDciN1NbiotControlMessage> msg = Create<DlDciN1NbiotControlMessage> ();
              msg->SetDci (it->dciN1);
              msg->SetRnti (it->rnti);
              int subframestowait = *(it->dciRepetitionsubframes.end () - 1) - currentsubframe;
              Simulator::Schedule (MilliSeconds (subframestowait),
                                   &LteEnbPhySapProvider::SendLteControlMessage,
                                   m_enbPhySapProvider, msg);

              // Implement DataInactivity-Timer 
              // Notify RRC about last scheduled NPDSCH Transmission for the rnti
              if(it->rnti != 0){ 
                int subframestillDataInactivity = it->dciN1.npdschOpportunity.back()- currentsubframe;
                m_cmacSapUser->NotifyDataActivitySchedulerNb(it->rnti);

                if(!m_noDataIndicators[it->rnti].IsExpired()){
                  m_noDataIndicators[it->rnti].Cancel();
                }
                m_noDataIndicators[it->rnti] = Simulator::Schedule (MilliSeconds (subframestillDataInactivity),
                                    &LteEnbCmacSapUser::NotifyDataInactivitySchedulerNb,
                                    m_cmacSapUser, it->rnti);
              }

            }
          else if (it->dciType == NbIotRrcSap::NpdcchMessage::DciType::n0){
            Ptr<UlDciN0NbiotControlMessage> msg = Create<UlDciN0NbiotControlMessage> ();
            msg->SetDci(it->dciN0);
            msg->SetRnti(it->rnti);
            int subframestowait = *(it->dciRepetitionsubframes.end () - 1) - currentsubframe;
            Simulator::Schedule (MilliSeconds (subframestowait),
                                  &LteEnbPhySapProvider::SendLteControlMessage,
                                  m_enbPhySapProvider, msg);
            // Implement DataInactivity-Timer 
            // Notify RRC about last scheduled NPDSCH Transmission for the rnti
            if(it->rnti != 0){
              int subframestillDataInactivity = it->dciN0.npuschOpportunity.back().second.back() - currentsubframe;
              m_cmacSapUser->NotifyDataActivitySchedulerNb(it->rnti);
              if(!m_noDataIndicators[it->rnti].IsExpired()){
                  m_noDataIndicators[it->rnti].Cancel();
              }
              m_noDataIndicators[it->rnti] = Simulator::Schedule (MilliSeconds (subframestillDataInactivity),
                                  &LteEnbCmacSapUser::NotifyDataInactivitySchedulerNb,
                                  m_cmacSapUser, it->rnti);
            }
          }
        }
    }
    scheduled.clear();

  //// CE0 Type2
  //// --- DOWNLINK ---
  //// Send Dl-CQI info to the scheduler
  //if (m_dlCqiReceived.size () > 0)
  //  {
  //    FfMacSchedSapProvider::SchedDlCqiInfoReqParameters dlcqiInfoReq;
  //    dlcqiInfoReq.m_sfnSf = ((0x3FF & frameNo) << 4) | (0xF & subframeNo);
  //    dlcqiInfoReq.m_cqiList.insert (dlcqiInfoReq.m_cqiList.begin (), m_dlCqiReceived.begin (), m_dlCqiReceived.end ());
  //    m_dlCqiReceived.erase (m_dlCqiReceived.begin (), m_dlCqiReceived.end ());
  //    m_schedSapProvider->SchedDlCqiInfoReq (dlcqiInfoReq);
  //  }

  //if (!m_receivedRachPreambleCount.empty ())
  //  {
  //    // process received RACH preambles and notify the scheduler
  //    FfMacSchedSapProvider::SchedDlRachInfoReqParameters rachInfoReqParams;
  //    NS_ASSERT (subframeNo > 0 && subframeNo <= 10); // subframe in 1..10
  //    for (std::map<uint8_t, uint32_t>::const_iterator it = m_receivedRachPreambleCount.begin ();
  //         it != m_receivedRachPreambleCount.end ();
  //         ++it)
  //      {
  //        NS_LOG_INFO (this << " preambleId " << (uint32_t) it->first << ": " << it->second << " received");
  //        NS_ASSERT (it->second != 0);
  //        if (it->second > 1)
  //          {
  //            NS_LOG_INFO ("preambleId " << (uint32_t) it->first << ": collision");
  //            // in case of collision we assume that no preamble is
  //            // successfully received, hence no RAR is sent
  //          }
  //        else
  //          {
  //            uint16_t rnti;
  //            std::map<uint8_t, NcRaPreambleInfo>::iterator jt = m_allocatedNcRaPreambleMap.find (it->first);
  //            if (jt != m_allocatedNcRaPreambleMap.end ())
  //              {
  //                rnti = jt->second.rnti;
  //                NS_LOG_INFO ("preambleId previously allocated for NC based RA, RNTI =" << (uint32_t) rnti << ", sending RAR");
  //
  //              }
  //            else
  //              {
  //                rnti = m_cmacSapUser->AllocateTemporaryCellRnti ();
  //                NS_LOG_INFO ("preambleId " << (uint32_t) it->first << ": allocated T-C-RNTI " << (uint32_t) rnti << ", sending RAR");
  //              }

  //            RachListElement_s rachLe;
  //            rachLe.m_rnti = rnti;
  //            rachLe.m_estimatedSize = 144; // to be confirmed
  //            rachInfoReqParams.m_rachList.push_back (rachLe);
  //            m_rapIdRntiMap.insert (std::pair <uint16_t, uint32_t> (rnti, it->first));
  //          }
  //      }
  //    m_schedSapProvider->SchedDlRachInfoReq (rachInfoReqParams);
  //    m_receivedRachPreambleCount.clear ();
  //  }
  //// Get downlink transmission opportunities
  //uint32_t dlSchedFrameNo = m_frameNo;
  //uint32_t dlSchedSubframeNo = m_subframeNo;
  ////   NS_LOG_DEBUG (this << " sfn " << frameNo << " sbfn " << subframeNo);
  //if (dlSchedSubframeNo + m_macChTtiDelay > 10)
  //  {
  //    dlSchedFrameNo++;
  //    dlSchedSubframeNo = (dlSchedSubframeNo + m_macChTtiDelay) % 10;
  //  }
  //else
  //  {
  //    dlSchedSubframeNo = dlSchedSubframeNo + m_macChTtiDelay;
  //  }
  //FfMacSchedSapProvider::SchedDlTriggerReqParameters dlparams;
  //dlparams.m_sfnSf = ((0x3FF & dlSchedFrameNo) << 4) | (0xF & dlSchedSubframeNo);

  //// Forward DL HARQ feebacks collected during last TTI
  //if (m_dlInfoListReceived.size () > 0)
  //  {
  //    dlparams.m_dlInfoList = m_dlInfoListReceived;
  //    // empty local buffer
  //    m_dlInfoListReceived.clear ();
  //  }

  //m_schedSapProvider->SchedDlTriggerReq (dlparams);

  //// --- UPLINK ---
  //// Send UL-CQI info to the scheduler
  //for (uint16_t i = 0; i < m_ulCqiReceived.size (); i++)
  //  {
  //    if (subframeNo > 1)
  //      {
  //        m_ulCqiReceived.at (i).m_sfnSf = ((0x3FF & frameNo) << 4) | (0xF & (subframeNo - 1));
  //      }
  //    else
  //      {
  //        m_ulCqiReceived.at (i).m_sfnSf = ((0x3FF & (frameNo - 1)) << 4) | (0xF & 10);
  //      }
  //    m_schedSapProvider->SchedUlCqiInfoReq (m_ulCqiReceived.at (i));
  //  }
  //  m_ulCqiReceived.clear ();

  //// Send BSR reports to the scheduler
  //if (m_ulCeReceived.size () > 0)
  //  {
  //    FfMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters ulMacReq;
  //    ulMacReq.m_sfnSf = ((0x3FF & frameNo) << 4) | (0xF & subframeNo);
  //    ulMacReq.m_macCeList.insert (ulMacReq.m_macCeList.begin (), m_ulCeReceived.begin (), m_ulCeReceived.end ());
  //    m_ulCeReceived.erase (m_ulCeReceived.begin (), m_ulCeReceived.end ());
  //    m_schedSapProvider->SchedUlMacCtrlInfoReq (ulMacReq);
  //  }

  //// Get uplink transmission opportunities
  //uint32_t ulSchedFrameNo = m_frameNo;
  //uint32_t ulSchedSubframeNo = m_subframeNo;
  ////   NS_LOG_DEBUG (this << " sfn " << frameNo << " sbfn " << subframeNo);
  //if (ulSchedSubframeNo + (m_macChTtiDelay + UL_PUSCH_TTIS_DELAY) > 10)
  //  {
  //    ulSchedFrameNo++;
  //    ulSchedSubframeNo = (ulSchedSubframeNo + (m_macChTtiDelay + UL_PUSCH_TTIS_DELAY)) % 10;
  //  }
  //else
  //  {
  //    ulSchedSubframeNo = ulSchedSubframeNo + (m_macChTtiDelay + UL_PUSCH_TTIS_DELAY);
  //  }
  //FfMacSchedSapProvider::SchedUlTriggerReqParameters ulparams;
  //ulparams.m_sfnSf = ((0x3FF & ulSchedFrameNo) << 4) | (0xF & ulSchedSubframeNo);

  //// Forward DL HARQ feebacks collected during last TTI
  //if (m_ulInfoListReceived.size () > 0)
  //  {
  //   ulparams.m_ulInfoList = m_ulInfoListReceived;
  //    // empty local buffer
  //    m_ulInfoListReceived.clear ();
  //  }

  //m_schedSapProvider->SchedUlTriggerReq (ulparams);
}

void
LteEnbMac::DoReceiveLteControlMessage (Ptr<LteControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);
  if (msg->GetMessageType () == LteControlMessage::DL_CQI)
    {
      Ptr<DlCqiLteControlMessage> dlcqi = DynamicCast<DlCqiLteControlMessage> (msg);
      ReceiveDlCqiLteControlMessage (dlcqi);
    }
  else if (msg->GetMessageType () == LteControlMessage::BSR)
    {
      Ptr<BsrLteControlMessage> bsr = DynamicCast<BsrLteControlMessage> (msg);
      ReceiveBsrMessage (bsr->GetBsr ());
    }
  else if (msg->GetMessageType () == LteControlMessage::DL_HARQ)
    {
      Ptr<DlHarqFeedbackLteControlMessage> dlharq =
          DynamicCast<DlHarqFeedbackLteControlMessage> (msg);
      DoDlInfoListElementHarqFeeback (dlharq->GetDlHarqFeedback ());
    }
  else if (msg->GetMessageType () == LteControlMessage::DL_HARQ_NB)
    {
      Ptr<DlHarqFeedbackNbiotControlMessage> dlharq =
          DynamicCast<DlHarqFeedbackNbiotControlMessage> (msg);
      // If connectionSuccessful == false, device hasnt completed its Connection yet
      // Device has received MSG4 and neeeds UL-Resources for MSG5
      if (!m_connectionSuccessful[dlharq->GetRnti ()])
        {
          // MIGHT BE NOT NEEDED ANYMORE
          m_schedulerNb->ScheduleUlRlcBufferReq(dlharq->GetRnti (), m_ueStoredBSR[dlharq->GetRnti()]);
          m_connectionSuccessful[dlharq->GetRnti ()] = true;
          m_ueStoredBSR[dlharq->GetRnti()] = 0;
        }
    }
  else
    {
      NS_LOG_LOGIC (this << " LteControlMessage type " << msg->GetMessageType ()
                         << " not recognized");
    }
}

void
LteEnbMac::DoReceiveRachPreamble (uint8_t rapId)
{
  NS_LOG_FUNCTION (this << (uint32_t) rapId);
  // just record that the preamble has been received; it will be processed later
  ++m_receivedRachPreambleCount[rapId]; // will create entry if not exists
}
void
LteEnbMac::DoReceiveNprachPreamble (uint8_t rapId, uint8_t subcarrierOffset, uint32_t ranti)
{
  NS_LOG_FUNCTION (this << (uint32_t) rapId);
  // just record that the preamble has been received; it will be processed later

  ++(m_receivedNprachPreambleCount[subcarrierOffset][rapId]); // will create entry if not exists
  m_rapIdRantiMap[subcarrierOffset + rapId] = ranti;
}

void
LteEnbMac::DoUlCqiReport (FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi)
{
  if (ulcqi.m_ulCqi.m_type == UlCqi_s::PUSCH)
    {
      NS_LOG_DEBUG (this << " eNB rxed an PUSCH UL-CQI");
    }
  else if (ulcqi.m_ulCqi.m_type == UlCqi_s::SRS)
    {
      NS_LOG_DEBUG (this << " eNB rxed an SRS UL-CQI");
    }
  m_ulCqiReceived.push_back (ulcqi);
}

void
LteEnbMac::DoUlCqiReportNb (std::vector<double> cqi)
{
  NS_BUILD_DEBUG (std::cout << "Received CQI: ");
  for (std::vector<double>::iterator it = cqi.begin (); it != cqi.end (); ++it)
    {
      NS_BUILD_DEBUG (std::cout << *it << " ");
    }
  NS_BUILD_DEBUG (std::cout << std::endl);
  m_ulCqiReceivedNb.push_back (cqi);
}

void
LteEnbMac::ReceiveDlCqiLteControlMessage (Ptr<DlCqiLteControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);

  CqiListElement_s dlcqi = msg->GetDlCqi ();
  //
  NS_LOG_LOGIC (this << "Enb Received DL-CQI rnti" << dlcqi.m_rnti);
  NS_ASSERT (dlcqi.m_rnti != 0);
  m_ulRsrpReceivedNb.insert (std::pair<uint16_t, double> (dlcqi.m_rnti, msg->rsrp));
  //m_dlCqiReceived.push_back (dlcqi);
}

void
LteEnbMac::ReceiveBsrMessage (MacCeListElement_s bsr)
{
  NS_LOG_FUNCTION (this);
  m_ccmMacSapUser->UlReceiveMacCe (bsr, m_componentCarrierId);
}

void
LteEnbMac::DoReportMacCeToScheduler (MacCeListElement_s bsr)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG (this << " bsr Size " << (uint16_t) m_ulCeReceived.size ());
  //send to LteCcmMacSapUser
  m_ulCeReceived.push_back (
      bsr); // this to called when LteUlCcmSapProvider::ReportMacCeToScheduler is called
  NS_LOG_DEBUG (this << " bsr Size after push_back " << (uint16_t) m_ulCeReceived.size ());
}

void
LteEnbMac::DoReceivePhyPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
  LteRadioBearerTag tag;
  p->RemovePacketTag (tag);

  // store info of the packet received

  //   std::map <uint16_t,UlInfoListElement_s>::iterator it;
  //   u_int rnti = tag.GetRnti ();
  //  u_int lcid = tag.GetLcid ();
  //   it = m_ulInfoListElements.find (tag.GetRnti ());
  //   if (it == m_ulInfoListElements.end ())
  //     {
  //       // new RNTI
  //       UlInfoListElement_s ulinfonew;
  //       ulinfonew.m_rnti = tag.GetRnti ();
  //       // always allocate full size of ulReception vector, initializing all elements to 0
  //       ulinfonew.m_ulReception.assign (MAX_LC_LIST+1, 0);
  //       // set the element for the current LCID
  //       ulinfonew.m_ulReception.at (tag.GetLcid ()) = p->GetSize ();
  //       ulinfonew.m_receptionStatus = UlInfoListElement_s::Ok;
  //       ulinfonew.m_tpc = 0; // Tx power control not implemented at this stage
  //       m_ulInfoListElements.insert (std::pair<uint16_t, UlInfoListElement_s > (tag.GetRnti (), ulinfonew));
  //
  //     }
  //   else
  //     {
  //       // existing RNTI: we just set the value for the current
  //       // LCID. Note that the corresponding element had already been
  //       // allocated previously.
  //       NS_ASSERT_MSG ((*it).second.m_ulReception.at (tag.GetLcid ()) == 0, "would overwrite previously written ulReception element");
  //       (*it).second.m_ulReception.at (tag.GetLcid ()) = p->GetSize ();
  //       (*it).second.m_receptionStatus = UlInfoListElement_s::Ok;
  //     }

  // forward the packet to the correspondent RLC
  uint16_t rnti = tag.GetRnti ();
  uint8_t lcid = tag.GetLcid ();

  DataVolumeAndPowerHeadroomTag dprTag;
  BufferStatusReportTag bsrTag;
  uint32_t buffersize;
  if(p->RemovePacketTag(dprTag)){ // it's MSG3
    buffersize = DataVolumeDPR::DVId2BufferSize(dprTag.GetDataVolumeValue());
    m_ueStoredBSR[rnti] = buffersize;

  }else if (p->RemovePacketTag(bsrTag)){
  buffersize = BufferSizeLevelBsr::BsrId2BufferSize(bsrTag.GetBufferStatusReportIndex());
  buffersize += 4; // Compensate RLC Header etc
  NS_BUILD_DEBUG(std::cout << "-------------------------" << std::endl);
  NS_BUILD_DEBUG(std::cout << "Buffersize: " << buffersize << std::endl);
  NS_BUILD_DEBUG(std::cout << "-------------------------" << std::endl);
  m_schedulerNb->ScheduleUlRlcBufferReq(rnti,buffersize);
  
  }
  std::map<uint16_t, std::map<uint8_t, LteMacSapUser *>>::iterator rntiIt =
      m_rlcAttached.find (rnti);
  NS_ASSERT_MSG (rntiIt != m_rlcAttached.end (), "could not find RNTI" << rnti);
  std::map<uint8_t, LteMacSapUser *>::iterator lcidIt = rntiIt->second.find (lcid);
  //NS_ASSERT_MSG (lcidIt != rntiIt->second.end (), "could not find LCID" << lcid);

  LteMacSapUser::ReceivePduParameters rxPduParams;
  rxPduParams.p = p;
  rxPduParams.rnti = rnti;
  rxPduParams.lcid = lcid;


  //Receive PDU only if LCID is found
  if (lcidIt != rntiIt->second.end ())
    {
      (*lcidIt).second->ReceivePdu (rxPduParams);
    }
}

// ////////////////////////////////////////////
// CMAC SAP
// ////////////////////////////////////////////

void
LteEnbMac::DoConfigureMac (uint16_t ulBandwidth, uint16_t dlBandwidth)
{
  NS_LOG_FUNCTION (this << " ulBandwidth=" << ulBandwidth << " dlBandwidth=" << dlBandwidth);
  FfMacCschedSapProvider::CschedCellConfigReqParameters params;
  // Configure the subset of parameters used by FfMacScheduler
  params.m_ulBandwidth = ulBandwidth;
  params.m_dlBandwidth = dlBandwidth;
  m_macChTtiDelay = m_enbPhySapProvider->GetMacChTtiDelay ();
  // ...more parameters can be configured
  m_cschedSapProvider->CschedCellConfigReq (params);
}

void
LteEnbMac::DoAddUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << " rnti=" << rnti);
  std::map<uint8_t, LteMacSapUser *> empty;
  std::pair<std::map<uint16_t, std::map<uint8_t, LteMacSapUser *>>::iterator, bool> ret =
      m_rlcAttached.insert (std::pair<uint16_t, std::map<uint8_t, LteMacSapUser *>> (rnti, empty));
  NS_ASSERT_MSG (ret.second, "element already present, RNTI already existed");

  FfMacCschedSapProvider::CschedUeConfigReqParameters params;
  params.m_rnti = rnti;
  params.m_transmissionMode =
      0; // set to default value (SISO) for avoiding random initialization (valgrind error)

  m_cschedSapProvider->CschedUeConfigReq (params);

  // Create DL transmission HARQ buffers
  std::vector<Ptr<PacketBurst>> dlHarqLayer0pkt;
  dlHarqLayer0pkt.resize (8);
  for (uint8_t i = 0; i < 8; i++)
    {
      Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
      dlHarqLayer0pkt.at (i) = pb;
    }
  std::vector<Ptr<PacketBurst>> dlHarqLayer1pkt;
  dlHarqLayer1pkt.resize (8);
  for (uint8_t i = 0; i < 8; i++)
    {
      Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
      dlHarqLayer1pkt.at (i) = pb;
    }
  DlHarqProcessesBuffer_t buf;
  buf.push_back (dlHarqLayer0pkt);
  buf.push_back (dlHarqLayer1pkt);
  m_miDlHarqProcessesPackets.insert (std::pair<uint16_t, DlHarqProcessesBuffer_t> (rnti, buf));
}

void 
LteEnbMac::DoMoveUeToResume(uint16_t rnti, uint64_t resumeId){
  m_resumeRlcAttached[resumeId] = m_rlcAttached[rnti];
  m_connectionSuccessful[rnti]= false;
}
void 
LteEnbMac::DoResumeUe(uint16_t rnti, uint64_t resumeId){
  m_rlcAttached[rnti] = m_resumeRlcAttached[resumeId];
  m_resumeRlcAttached.erase(resumeId);

  // Reinitialize HARQ 
  // Create DL transmission HARQ buffers
  std::vector<Ptr<PacketBurst>> dlHarqLayer0pkt;
  dlHarqLayer0pkt.resize (8);
  for (uint8_t i = 0; i < 8; i++)
    {
      Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
      dlHarqLayer0pkt.at (i) = pb;
    }
  std::vector<Ptr<PacketBurst>> dlHarqLayer1pkt;
  dlHarqLayer1pkt.resize (8);
  for (uint8_t i = 0; i < 8; i++)
    {
      Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
      dlHarqLayer1pkt.at (i) = pb;
    }
  DlHarqProcessesBuffer_t buf;
  buf.push_back (dlHarqLayer0pkt);
  buf.push_back (dlHarqLayer1pkt);
  m_miDlHarqProcessesPackets.insert (std::pair<uint16_t, DlHarqProcessesBuffer_t> (rnti, buf));
}

void LteEnbMac::DoRemoveUeFromScheduler(uint16_t rnti){
  m_schedulerNb->RemoveUe(rnti);
}
void LteEnbMac::DoSetLogDir(std::string logdir){
  m_logdir = logdir;
}
void
LteEnbMac::DoRemoveUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << " rnti=" << rnti);
  FfMacCschedSapProvider::CschedUeReleaseReqParameters params;
  params.m_rnti = rnti;
  m_cschedSapProvider->CschedUeReleaseReq (params);
  m_rlcAttached.erase (rnti);
  m_miDlHarqProcessesPackets.erase (rnti);

  NS_LOG_DEBUG ("start checking for unprocessed preamble for rnti: " << rnti);
  //remove unprocessed preamble received for RACH during handover
  std::map<uint8_t, NcRaPreambleInfo>::iterator jt = m_allocatedNcRaPreambleMap.begin ();
  while (jt != m_allocatedNcRaPreambleMap.end ())
    {
      if (jt->second.rnti == rnti)
        {
          std::map<uint8_t, uint32_t>::const_iterator it =
              m_receivedRachPreambleCount.find (jt->first);
          if (it != m_receivedRachPreambleCount.end ())
            {
              m_receivedRachPreambleCount.erase (it->first);
            }
          jt = m_allocatedNcRaPreambleMap.erase (jt);
        }
      else
        {
          ++jt;
        }
    }

  std::vector<MacCeListElement_s>::iterator itCeRxd = m_ulCeReceived.begin ();
  while (itCeRxd != m_ulCeReceived.end ())
    {
      if (itCeRxd->m_rnti == rnti)
        {
          itCeRxd = m_ulCeReceived.erase (itCeRxd);
        }
      else
        {
          itCeRxd++;
        }
    }

}

void
LteEnbMac::DoAddLc (LteEnbCmacSapProvider::LcInfo lcinfo, LteMacSapUser *msu)
{
  NS_LOG_FUNCTION (this << lcinfo.rnti << (uint16_t) lcinfo.lcId);

  std::map<LteFlowId_t, LteMacSapUser *>::iterator it;

  LteFlowId_t flow (lcinfo.rnti, lcinfo.lcId);

  std::map<uint16_t, std::map<uint8_t, LteMacSapUser *>>::iterator rntiIt =
      m_rlcAttached.find (lcinfo.rnti);
  NS_ASSERT_MSG (rntiIt != m_rlcAttached.end (), "RNTI not found");
  std::map<uint8_t, LteMacSapUser *>::iterator lcidIt = rntiIt->second.find (lcinfo.lcId);
  if (lcidIt == rntiIt->second.end ())
    {
      rntiIt->second.insert (std::pair<uint8_t, LteMacSapUser *> (lcinfo.lcId, msu));
    }
  else
    {
      NS_LOG_ERROR ("LC already exists");
    }

  // CCCH (LCID 0) is pre-configured
  // see FF LTE MAC Scheduler
  // Interface Specification v1.11,
  // 4.3.4 logicalChannelConfigListElement
  if (lcinfo.lcId != 0)
    {
      struct FfMacCschedSapProvider::CschedLcConfigReqParameters params;
      params.m_rnti = lcinfo.rnti;
      params.m_reconfigureFlag = false;

      struct LogicalChannelConfigListElement_s lccle;
      lccle.m_logicalChannelIdentity = lcinfo.lcId;
      lccle.m_logicalChannelGroup = lcinfo.lcGroup;
      lccle.m_direction = LogicalChannelConfigListElement_s::DIR_BOTH;
      lccle.m_qosBearerType = lcinfo.isGbr ? LogicalChannelConfigListElement_s::QBT_GBR
                                           : LogicalChannelConfigListElement_s::QBT_NON_GBR;
      lccle.m_qci = lcinfo.qci;
      lccle.m_eRabMaximulBitrateUl = lcinfo.mbrUl;
      lccle.m_eRabMaximulBitrateDl = lcinfo.mbrDl;
      lccle.m_eRabGuaranteedBitrateUl = lcinfo.gbrUl;
      lccle.m_eRabGuaranteedBitrateDl = lcinfo.gbrDl;
      params.m_logicalChannelConfigList.push_back (lccle);

      m_cschedSapProvider->CschedLcConfigReq (params);
    }
}

void
LteEnbMac::DoReconfigureLc (LteEnbCmacSapProvider::LcInfo lcinfo)
{
  NS_FATAL_ERROR ("not implemented");
}

void
LteEnbMac::DoReleaseLc (uint16_t rnti, uint8_t lcid)
{
  NS_LOG_FUNCTION (this);

  //Find user based on rnti and then erase lcid stored against the same
  std::map<uint16_t, std::map<uint8_t, LteMacSapUser *>>::iterator rntiIt =
      m_rlcAttached.find (rnti);
  rntiIt->second.erase (lcid);

  struct FfMacCschedSapProvider::CschedLcReleaseReqParameters params;
  params.m_rnti = rnti;
  params.m_logicalChannelIdentity.push_back (lcid);
  m_cschedSapProvider->CschedLcReleaseReq (params);
}

void
LteEnbMac::DoUeUpdateConfigurationReq (LteEnbCmacSapProvider::UeConfig params)
{
  NS_LOG_FUNCTION (this);

  // propagates to scheduler
  FfMacCschedSapProvider::CschedUeConfigReqParameters req;
  req.m_rnti = params.m_rnti;
  req.m_transmissionMode = params.m_transmissionMode;
  req.m_reconfigureFlag = true;
  m_cschedSapProvider->CschedUeConfigReq (req);
}

LteEnbCmacSapProvider::RachConfig
LteEnbMac::DoGetRachConfig ()
{
  struct LteEnbCmacSapProvider::RachConfig rc;
  rc.numberOfRaPreambles = m_numberOfRaPreambles;
  rc.preambleTransMax = m_preambleTransMax;
  rc.raResponseWindowSize = m_raResponseWindowSize;
  rc.connEstFailCount = m_connEstFailCount;
  return rc;
}
LteEnbCmacSapProvider::RachConfigNb
LteEnbMac::DoGetRachConfigNb ()
{
  struct LteEnbCmacSapProvider::RachConfigNb rc;
  //rc.numberOfRaPreambles = m_numberOfRaPreambles;
  //rc.preambleTransMax = m_preambleTransMax;
  //rc.raResponseWindowSize = m_raResponseWindowSize;
  //rc.connEstFailCount = m_connEstFailCount;
  return rc;
}
LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue
LteEnbMac::DoAllocateNcRaPreamble (uint16_t rnti)
{
  bool found = false;
  uint8_t preambleId;
  for (preambleId = m_numberOfRaPreambles; preambleId < 64; ++preambleId)
    {
      std::map<uint8_t, NcRaPreambleInfo>::iterator it =
          m_allocatedNcRaPreambleMap.find (preambleId);
      /**
       * Allocate preamble only if its free. The non-contention preamble
       * assigned to UE during handover or PDCCH order is valid only until the
       * time duration of the “expiryTime” of the preamble is reached. This
       * timer value is only maintained at the eNodeB and the UE has no way of
       * knowing if this timer has expired. If the UE tries to send the preamble
       * again after the expiryTime and the preamble is re-assigned to another
       * UE, it results in errors. This has been solved by re-assigning the
       * preamble to another UE only if it is not being used (An UE can be using
       * the preamble even after the expiryTime duration).
       */
      if ((it != m_allocatedNcRaPreambleMap.end ()) && (it->second.expiryTime < Simulator::Now ()))
        {
          if (!m_cmacSapUser->IsRandomAccessCompleted (rnti))
            {
              //random access of the UE is not completed,
              //check other preambles
              continue;
            }
        }
      if ((it == m_allocatedNcRaPreambleMap.end ()) || (it->second.expiryTime < Simulator::Now ()))
        {
          found = true;
          NcRaPreambleInfo preambleInfo;
          uint32_t expiryIntervalMs =
              (uint32_t) m_preambleTransMax * ((uint32_t) m_raResponseWindowSize + 5);

          preambleInfo.expiryTime = Simulator::Now () + MilliSeconds (expiryIntervalMs);
          preambleInfo.rnti = rnti;
          NS_LOG_INFO ("allocated preamble for NC based RA: preamble "
                       << preambleId << ", RNTI " << preambleInfo.rnti << ", exiryTime "
                       << preambleInfo.expiryTime);
          m_allocatedNcRaPreambleMap[preambleId] =
              preambleInfo; // create if not exist, update otherwise
          break;
        }
    }
  LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue ret;
  if (found)
    {
      ret.valid = true;
      ret.raPreambleId = preambleId;
      ret.raPrachMaskIndex = 0;
    }
  else
    {
      ret.valid = false;
      ret.raPreambleId = 0;
      ret.raPrachMaskIndex = 0;
    }
  return ret;
}

// ////////////////////////////////////////////
// MAC SAP
// ////////////////////////////////////////////

void
LteEnbMac::DoTransmitPdu (LteMacSapProvider::TransmitPduParameters params)
{
  NS_LOG_FUNCTION (this);

  // Peek RLC Header to issue StatusPDU scheduling if needed

  LteRlcAmHeader rlcAmHeader;
  if(params.pdu->PeekHeader(rlcAmHeader) != 0){
    // Is RLC AM, check if status pdu is requested
    if ( rlcAmHeader.GetPollingBit () == LteRlcAmHeader::STATUS_REPORT_IS_REQUESTED )
      {
        m_schedulerNb->AddToUlBufferReq(params.rnti, 4); // Add Status Pdu to be scheduled (4 Byte)
      }
  }

  LteRadioBearerTag tag (params.rnti, params.lcid, params.layer);
  params.pdu->AddPacketTag (tag);
  params.componentCarrierId = m_componentCarrierId;
  // Store pkt in HARQ buffer
  //std::map<uint16_t, DlHarqProcessesBuffer_t>::iterator it =
  //    m_miDlHarqProcessesPackets.find (params.rnti);
  //NS_ASSERT (it != m_miDlHarqProcessesPackets.end ());
  //NS_LOG_DEBUG (this << " LAYER " << (uint16_t) tag.GetLayer () << " HARQ ID "
  //                   << (uint16_t) params.harqProcessId);

  ////(*it).second.at (params.layer).at (params.harqProcessId) = params.pdu;//->Copy ();
  //(*it).second.at (params.layer).at (params.harqProcessId)->AddPacket (params.pdu);
  m_enbPhySapProvider->SendMacPdu (params.pdu);
}

void
LteEnbMac::DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_LOG_FUNCTION (this);
  FfMacSchedSapProvider::SchedDlRlcBufferReqParameters req;
  req.m_rnti = params.rnti;
  req.m_logicalChannelIdentity = params.lcid;
  req.m_rlcTransmissionQueueSize = params.txQueueSize;
  req.m_rlcTransmissionQueueHolDelay = params.txQueueHolDelay;
  req.m_rlcRetransmissionQueueSize = params.retxQueueSize;
  req.m_rlcRetransmissionHolDelay = params.retxQueueHolDelay;
  req.m_rlcStatusPduSize = params.statusPduSize;
  m_schedSapProvider->SchedDlRlcBufferReq (req);
}

void
LteEnbMac::DoReportBufferStatusNb (LteMacSapProvider::ReportBufferStatusParameters params,
                                   NbIotRrcSap::NpdcchMessage::SearchSpaceType searchspace)
{
  NS_LOG_FUNCTION (this);
  m_lastDlBSR[params.rnti][params.lcid] = params;
  if(params.lcid == 1 || params.lcid == 3){
    // We are using SRB1 or DRB -> RLC AM Header
  //m_lastDlBSR[params.rnti][params.lcid].txQueueSize;

  }
  m_schedulerNb->ScheduleDlRlcBufferReq(params.rnti, m_lastDlBSR[params.rnti]);
  
}
// ////////////////////////////////////////////
// SCHED SAP
// ////////////////////////////////////////////

void
LteEnbMac::DoSchedDlConfigInd (FfMacSchedSapUser::SchedDlConfigIndParameters ind)
{
  NS_LOG_FUNCTION (this);
  // Create DL PHY PDU
  Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
  std::map<LteFlowId_t, LteMacSapUser *>::iterator it;
  LteMacSapUser::TxOpportunityParameters txOpParams;

  for (unsigned int i = 0; i < ind.m_buildDataList.size (); i++)
    {
      for (uint16_t layer = 0; layer < ind.m_buildDataList.at (i).m_dci.m_ndi.size (); layer++)
        {
          if (ind.m_buildDataList.at (i).m_dci.m_ndi.at (layer) == 1)
            {
              // new data -> force emptying correspondent harq pkt buffer
              std::map<uint16_t, DlHarqProcessesBuffer_t>::iterator it =
                  m_miDlHarqProcessesPackets.find (ind.m_buildDataList.at (i).m_rnti);
              NS_ASSERT (it != m_miDlHarqProcessesPackets.end ());
              for (uint16_t lcId = 0; lcId < (*it).second.size (); lcId++)
                {
                  Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
                  (*it).second.at (lcId).at (ind.m_buildDataList.at (i).m_dci.m_harqProcess) = pb;
                }
            }
        }
      for (unsigned int j = 0; j < ind.m_buildDataList.at (i).m_rlcPduList.size (); j++)
        {
          for (uint16_t k = 0; k < ind.m_buildDataList.at (i).m_rlcPduList.at (j).size (); k++)
            {
              if (ind.m_buildDataList.at (i).m_dci.m_ndi.at (k) == 1)
                {
                  // New Data -> retrieve it from RLC
                  uint16_t rnti = ind.m_buildDataList.at (i).m_rnti;
                  uint8_t lcid = ind.m_buildDataList.at (i)
                                     .m_rlcPduList.at (j)
                                     .at (k)
                                     .m_logicalChannelIdentity;
                  std::map<uint16_t, std::map<uint8_t, LteMacSapUser *>>::iterator rntiIt =
                      m_rlcAttached.find (rnti);
                  NS_ASSERT_MSG (rntiIt != m_rlcAttached.end (), "could not find RNTI" << rnti);
                  std::map<uint8_t, LteMacSapUser *>::iterator lcidIt = rntiIt->second.find (lcid);
                  NS_ASSERT_MSG (lcidIt != rntiIt->second.end (),
                                 "could not find LCID" << (uint32_t) lcid << " carrier id:"
                                                       << (uint16_t) m_componentCarrierId);
                  NS_LOG_DEBUG (this << " rnti= " << rnti << " lcid= " << (uint32_t) lcid
                                     << " layer= " << k);
                  txOpParams.bytes = ind.m_buildDataList.at (i).m_rlcPduList.at (j).at (k).m_size;
                  txOpParams.layer = k;
                  txOpParams.harqId = ind.m_buildDataList.at (i).m_dci.m_harqProcess;
                  txOpParams.componentCarrierId = m_componentCarrierId;
                  txOpParams.rnti = rnti;
                  txOpParams.lcid = lcid;
                  (*lcidIt).second->NotifyTxOpportunity (txOpParams);
                }
              else
                {
                  if (ind.m_buildDataList.at (i).m_dci.m_tbsSize.at (k) > 0)
                    {
                      // HARQ retransmission -> retrieve TB from HARQ buffer
                      std::map<uint16_t, DlHarqProcessesBuffer_t>::iterator it =
                          m_miDlHarqProcessesPackets.find (ind.m_buildDataList.at (i).m_rnti);
                      NS_ASSERT (it != m_miDlHarqProcessesPackets.end ());
                      Ptr<PacketBurst> pb =
                          (*it).second.at (k).at (ind.m_buildDataList.at (i).m_dci.m_harqProcess);
                      for (std::list<Ptr<Packet>>::const_iterator j = pb->Begin (); j != pb->End ();
                           ++j)
                        {
                          Ptr<Packet> pkt = (*j)->Copy ();
                          m_enbPhySapProvider->SendMacPdu (pkt);
                        }
                    }
                }
            }
        }
      // send the relative DCI
      Ptr<DlDciLteControlMessage> msg = Create<DlDciLteControlMessage> ();
      msg->SetDci (ind.m_buildDataList.at (i).m_dci);
      m_enbPhySapProvider->SendLteControlMessage (msg);
    }

  // Fire the trace with the DL information
  for (uint32_t i = 0; i < ind.m_buildDataList.size (); i++)
    {
      // Only one TB used
      if (ind.m_buildDataList.at (i).m_dci.m_tbsSize.size () == 1)
        {
          DlSchedulingCallbackInfo dlSchedulingCallbackInfo;
          dlSchedulingCallbackInfo.frameNo = m_frameNo;
          dlSchedulingCallbackInfo.subframeNo = m_subframeNo;
          dlSchedulingCallbackInfo.rnti = ind.m_buildDataList.at (i).m_dci.m_rnti;
          dlSchedulingCallbackInfo.mcsTb1 = ind.m_buildDataList.at (i).m_dci.m_mcs.at (0);
          dlSchedulingCallbackInfo.sizeTb1 = ind.m_buildDataList.at (i).m_dci.m_tbsSize.at (0);
          dlSchedulingCallbackInfo.mcsTb2 = 0;
          dlSchedulingCallbackInfo.sizeTb2 = 0;
          dlSchedulingCallbackInfo.componentCarrierId = m_componentCarrierId;
          m_dlScheduling (dlSchedulingCallbackInfo);
        }
      // Two TBs used
      else if (ind.m_buildDataList.at (i).m_dci.m_tbsSize.size () == 2)
        {
          DlSchedulingCallbackInfo dlSchedulingCallbackInfo;
          dlSchedulingCallbackInfo.frameNo = m_frameNo;
          dlSchedulingCallbackInfo.subframeNo = m_subframeNo;
          dlSchedulingCallbackInfo.rnti = ind.m_buildDataList.at (i).m_dci.m_rnti;
          dlSchedulingCallbackInfo.mcsTb1 = ind.m_buildDataList.at (i).m_dci.m_mcs.at (0);
          dlSchedulingCallbackInfo.sizeTb1 = ind.m_buildDataList.at (i).m_dci.m_tbsSize.at (0);
          dlSchedulingCallbackInfo.mcsTb2 = ind.m_buildDataList.at (i).m_dci.m_mcs.at (1);
          dlSchedulingCallbackInfo.sizeTb2 = ind.m_buildDataList.at (i).m_dci.m_tbsSize.at (1);
          dlSchedulingCallbackInfo.componentCarrierId = m_componentCarrierId;
          m_dlScheduling (dlSchedulingCallbackInfo);
        }
      else
        {
          NS_FATAL_ERROR ("Found element with more than two transport blocks");
        }
    }

  // Random Access procedure: send RARs
  Ptr<RarLteControlMessage> rarMsg = Create<RarLteControlMessage> ();
  // see TS 36.321 5.1.4;  preambles were sent two frames ago
  // (plus 3GPP counts subframes from 0, not 1)
  uint16_t raRnti;
  if (m_subframeNo < 3)
    {
      raRnti = m_subframeNo + 7; // equivalent to +10-3
    }
  else
    {
      raRnti = m_subframeNo - 3;
    }
  rarMsg->SetRaRnti (raRnti);
  for (unsigned int i = 0; i < ind.m_buildRarList.size (); i++)
    {
      std::map<uint16_t, uint32_t>::iterator itRapId =
          m_rapIdRntiMap.find (ind.m_buildRarList.at (i).m_rnti);
      if (itRapId == m_rapIdRntiMap.end ())
        {
          NS_FATAL_ERROR ("Unable to find rapId of RNTI " << ind.m_buildRarList.at (i).m_rnti);
        }
      RarLteControlMessage::Rar rar;
      rar.rapId = itRapId->second;
      rar.rarPayload = ind.m_buildRarList.at (i);
      rarMsg->AddRar (rar);
      NS_LOG_INFO (this << " Send RAR message to RNTI " << ind.m_buildRarList.at (i).m_rnti
                        << " rapId " << itRapId->second);
    }
  if (ind.m_buildRarList.size () > 0)
    {
      m_enbPhySapProvider->SendLteControlMessage (rarMsg);
    }
  m_rapIdRntiMap.clear ();
}

void
LteEnbMac::DoSchedUlConfigInd (FfMacSchedSapUser::SchedUlConfigIndParameters ind)
{
  NS_LOG_FUNCTION (this);

  for (unsigned int i = 0; i < ind.m_dciList.size (); i++)
    {
      // send the correspondent ul dci
      Ptr<UlDciLteControlMessage> msg = Create<UlDciLteControlMessage> ();
      msg->SetDci (ind.m_dciList.at (i));
      m_enbPhySapProvider->SendLteControlMessage (msg);
    }

  // Fire the trace with the UL information
  for (uint32_t i = 0; i < ind.m_dciList.size (); i++)
    {
      m_ulScheduling (m_frameNo, m_subframeNo, ind.m_dciList.at (i).m_rnti,
                      ind.m_dciList.at (i).m_mcs, ind.m_dciList.at (i).m_tbSize,
                      m_componentCarrierId);
    }
}

// ////////////////////////////////////////////
// CSCHED SAP
// ////////////////////////////////////////////

void
LteEnbMac::DoCschedCellConfigCnf (FfMacCschedSapUser::CschedCellConfigCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
LteEnbMac::DoCschedUeConfigCnf (FfMacCschedSapUser::CschedUeConfigCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
LteEnbMac::DoCschedLcConfigCnf (FfMacCschedSapUser::CschedLcConfigCnfParameters params)
{
  NS_LOG_FUNCTION (this);
  // Call the CSCHED primitive
  // m_cschedSap->LcConfigCompleted();
}

void
LteEnbMac::DoCschedLcReleaseCnf (FfMacCschedSapUser::CschedLcReleaseCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
LteEnbMac::DoCschedUeReleaseCnf (FfMacCschedSapUser::CschedUeReleaseCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
LteEnbMac::DoCschedUeConfigUpdateInd (FfMacCschedSapUser::CschedUeConfigUpdateIndParameters params)
{
  NS_LOG_FUNCTION (this);
  // propagates to RRC
  LteEnbCmacSapUser::UeConfig ueConfigUpdate;
  ueConfigUpdate.m_rnti = params.m_rnti;
  ueConfigUpdate.m_transmissionMode = params.m_transmissionMode;
  m_cmacSapUser->RrcConfigurationUpdateInd (ueConfigUpdate);
}

void
LteEnbMac::DoCschedCellConfigUpdateInd (
    FfMacCschedSapUser::CschedCellConfigUpdateIndParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
LteEnbMac::DoUlInfoListElementHarqFeeback (UlInfoListElement_s params)
{
  NS_LOG_FUNCTION (this);
  m_ulInfoListReceived.push_back (params);
}

void
LteEnbMac::DoDlInfoListElementHarqFeeback (DlInfoListElement_s params)
{
  NS_LOG_FUNCTION (this);
  // Update HARQ buffer
  std::map<uint16_t, DlHarqProcessesBuffer_t>::iterator it =
      m_miDlHarqProcessesPackets.find (params.m_rnti);
  NS_ASSERT (it != m_miDlHarqProcessesPackets.end ());
  for (uint8_t layer = 0; layer < params.m_harqStatus.size (); layer++)
    {
      if (params.m_harqStatus.at (layer) == DlInfoListElement_s::ACK)
        {
          // discard buffer
          Ptr<PacketBurst> emptyBuf = CreateObject<PacketBurst> ();
          (*it).second.at (layer).at (params.m_harqProcessId) = emptyBuf;
          NS_LOG_DEBUG (this << " HARQ-ACK UE " << params.m_rnti << " harqId "
                             << (uint16_t) params.m_harqProcessId << " layer " << (uint16_t) layer);
        }
      else if (params.m_harqStatus.at (layer) == DlInfoListElement_s::NACK)
        {
          NS_LOG_DEBUG (this << " HARQ-NACK UE " << params.m_rnti << " harqId "
                             << (uint16_t) params.m_harqProcessId << " layer " << (uint16_t) layer);
        }
      else
        {
          NS_FATAL_ERROR (" HARQ functionality not implemented");
        }
    }
  m_dlInfoListReceived.push_back (params);
}

void LteEnbMac::DoNotifyConnectionSuccessful(uint16_t rnti){
  m_connectionSuccessful[rnti] = true;
  //if (m_ueStoredBSR[rnti] > 0){
  //    uint64_t dataSize = BufferSizeLevelBsr::BsrId2BufferSize(m_ueStoredBSR[rnti]);
  //    m_schedulerNb->ScheduleUlRlcBufferReq(rnti,dataSize,NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2);
  //    m_ueStoredBSR[rnti]=0;
  //}
}

void LteEnbMac::CheckForDataInactivity(uint16_t rnti){
 std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it; 
  bool buffer_remaining = false;
  for(it = m_lastDlBSR[rnti].begin(); it != m_lastDlBSR[rnti].end(); ++it){
    if(!(it->second.txQueueSize == 0 && it->second.retxQueueSize == 0 && it->second.statusPduSize == 0)){
      buffer_remaining = true;
      break;
    }
  }
  if(!buffer_remaining && m_ueStoredBSR[rnti] ==0){
    m_cmacSapUser->NotifyDataInactivitySchedulerNb(rnti);
  }
}
void LteEnbMac::DoReportNoTransmissionNb(uint16_t rnti, uint8_t lcid){
}
} // namespace ns3
