/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 *         Marco Miozzo <marco.miozzo@cttc.es>
 */

#ifndef NBIOT_MAC_SCHEDULER_H
#define NBIOT_MAC_SCHEDULER_H

#include <ns3/object.h>
#include "nb-iot-rrc-sap.h"
#include "lte-mac-sap.h"
#include <algorithm>
#include <unordered_map>
#include "nb-iot-amc.h"

namespace ns3 {

// struct ScheduledMessages{
//   std::vector<LteControlMessage> _controlmsgList;
//   std::vector<> _controlmsgList;
// };

// Overall size of UlGrant = 15 bit

struct NpdschMessage{
  uint64_t Tb;
};

class NbiotScheduler : public Object
{
public:

  NbiotScheduler(std::vector<NbIotRrcSap::NprachParametersNb> ces, NbIotRrcSap::SystemInformationBlockType2Nb sib2);

  //~NbiotScheduler();

  virtual void DoDispose (void);


bool IsSeachSpaceType2Begin(NbIotRrcSap::NprachParametersNb ce);
//bool IsSeachSpaceType1Begin(NbIotRrcSap::NprachParametersNb ce);
//bool IsSeachSpaceUeSpecificBegin(NbIotRrcSap::NprachParametersNb ce);
 
void ScheduleRarReq(uint64_t rnti, uint64_t rapid, NbIotRrcSap::NprachParametersNb ue, NbIotRrcSap::DciN1::DciRepetitions rep);
void ScheduleNpdcchMessageReq(NbIotRrcSap::NpdcchMessage msg);
void ScheduleMsg5Req(uint64_t rnti);
void SetCeLevel(NbIotRrcSap::NprachParametersNb ce0, NbIotRrcSap::NprachParametersNb ce1, NbIotRrcSap::NprachParametersNb ce2);
void SetRntiRsrpMap(std::map<uint16_t, double> map);
void ScheduleUlRlcBufferReq(uint64_t rnti, uint64_t dataSize,NbIotRrcSap::NpdcchMessage::SearchSpaceType searchspace); // Data in Byte
void ScheduleDlRlcBufferReq(LteMacSapProvider::ReportBufferStatusParameters params, NbIotRrcSap::NpdcchMessage::SearchSpaceType searchspace); // Data in Byte
void AddRntiDatatoNpdcchQueue(NbIotRrcSap::NpdcchMessage::SearchSpaceType seachspace);
std::vector<uint64_t> GetNextAvailableSearchSpaceCandidate(uint32_t rnti, uint64_t SearchSpaceStartFrame, uint64_t SearchSpaceStartSubframe, uint64_t R_max, uint64_t R);
std::vector<uint64_t> GetDlSubframeRangeWithoutSystemResources(uint64_t overallSubframeNo, uint64_t numSubframes);
std::vector<uint64_t> GetUlSubframeRangeWithoutSystemResources(uint64_t overallSubframeNo, uint64_t numSubframes, uint64_t carrier);
std::vector<uint64_t> CheckforNContiniousSubframesDl(std::vector<uint64_t> Subframes, uint64_t StartSubframe, uint64_t N);
std::vector<uint64_t> CheckforNContiniousSubframesUl(std::vector<uint64_t> Subframes, uint64_t StartSubframe, uint64_t N, uint64_t carrier);
std::vector<uint64_t> GetNextAvailableNpdschCandidate(uint64_t endSubframeDci, uint64_t minSchedulingDelay, uint64_t numSubframes, uint64_t R_max);
std::vector<std::pair<uint64_t,uint64_t>> GetAllPossibleSearchSpaceCandidates(std::vector<uint64_t> subframes, uint64_t R_max);
std::vector<NbIotRrcSap::NpdcchMessage> Schedule(uint64_t frameNo, uint64_t subframeNo);
std::vector<NbIotRrcSap::NpdcchMessage> ScheduleSearchSpace(NbIotRrcSap::NpdcchMessage::SearchSpaceType searchspace, NbIotRrcSap::NprachParametersNb ce);
std::vector<std::pair<uint64_t, std::vector<uint64_t>>> GetNextAvailableNpuschCandidate(uint64_t endSubframeNpdsch, uint64_t minSchedulingDelay, uint64_t numSubframes, bool isHarq);
std::pair<NbIotRrcSap::UlGrant, std::pair<uint64_t,std::vector<uint64_t>>> GetNextAvailableMsg3UlGrantCandidate(uint64_t endSubframeMsg2, uint64_t numSubframes);
NbIotRrcSap::NpdcchMessage CreateDciNpdcchMessage(uint16_t rnti,NbIotRrcSap::NpdcchMessage::SearchSpaceType searchspace, NbIotRrcSap::NpdcchMessage::DciType dci_type);
std::vector<int> m_downlink;
NbIotRrcSap::NprachParametersNb m_ce0;
protected:
  std::vector<std::vector<int>> m_uplink;
  std::vector<NbIotRrcSap::NpdcchMessage> m_rars_to_schedule;
  std::vector<NbIotRrcSap::NpdcchMessage> m_NpdcchQueue;
  std::vector<NbIotRrcSap::NpdcchMessage> m_NpdschQueue;
  std::vector<NbIotRrcSap::NpdcchMessage> m_NpuschQueue;
  std::vector<NbIotRrcSap::DciN1::NpdcchTimeOffset> m_DciTimeOffsetRmaxSmall;
  std::vector<NbIotRrcSap::DciN1::NpdcchTimeOffset> m_DciTimeOffsetRmaxBig;
  std::vector<NbIotRrcSap::UlGrant::SchedulingDelay> m_Msg3TimeOffset;
  std::vector<NbIotRrcSap::DciN0::NpuschSchedulingDelay> m_DciTimeOffsetUplink;
  std::vector<NbIotRrcSap::HarqAckResource::TimeOffset> m_HarqTimeOffsets;
  std::vector<NbIotRrcSap::HarqAckResource::SubcarrierIndex> m_HarqSubcarrierIndex;
  std::map<uint16_t, uint64_t> m_lastUlSubframe;
  NbIotRrcSap::NprachParametersNb m_ce1;
  NbIotRrcSap::NprachParametersNb m_ce2;
  std::map<NbIotRrcSap::NpdcchMessage::SearchSpaceType, std::map<uint16_t, std::map<uint8_t,LteMacSapProvider::ReportBufferStatusParameters>>> m_RntiRlcDlBuffer;
  std::map<NbIotRrcSap::NpdcchMessage::SearchSpaceType, std::map<uint16_t, uint64_t>> m_RntiRlcUlBuffer;
  uint64_t m_frameNo;
  bool m_only15KhzSpacing = true;
  uint64_t m_subframeNo;
  int m_currenthyperindex;
  const uint64_t m_minSchedulingDelayDci2Downlink = 4;
  const uint64_t m_minSchedulingDelayDci2Uplink = 8;
  bool m_log;
  std::map<uint16_t,double> m_rntiRsrpMap;
  NbiotAmc m_Amc;
  NbIotRrcSap::SystemInformationBlockType2Nb m_sib2config;

};

}  // namespace ns3

#endif /* FF_MAC_SCHEDULER_H */

