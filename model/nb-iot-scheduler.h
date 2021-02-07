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
#include "lte-control-messages.h"
#include <algorithm>
#include <unordered_map>


namespace ns3 {

// struct ScheduledMessages{
//   std::vector<LteControlMessage> _controlmsgList;
//   std::vector<> _controlmsgList;
// };

// Overall size of UlGrant = 15 bit


struct NpdcchMessage{
  enum SearchSpaceType{
    type1,
    type2,
    ueSpecific
  } searchSpaceType;
  enum NpdcchFormat {
    format1,
    format2
  } npdcchFormat;
  enum DciType{
    n0,
    n1,
    n2
  } dciType;
  NbIotRrcSap::NprachParametersNb ce;
  NbIotRrcSap::DciN1 dciN1;
  NbIotRrcSap::DciN0 dciN0;
  std::vector<RarNbiotControlMessage::Rar> rars;
  std::vector<int> npdschOpportunity;
  std::vector<int> npuschOpportunity;
  std::vector<int> dciRepetitionsubframes;
  int ranti;
  bool isRar;

};

struct NpdschMessage{
  int Tb;
};

class NbiotScheduler : public Object
{
public:

  NbiotScheduler(std::vector<NbIotRrcSap::NprachParametersNb> ces);

  //~NbiotScheduler();

  virtual void DoDispose (void);


bool IsSeachSpaceType2Begin(NbIotRrcSap::NprachParametersNb ce);
//bool IsSeachSpaceType1Begin(NbIotRrcSap::NprachParametersNb ce);
//bool IsSeachSpaceUeSpecificBegin(NbIotRrcSap::NprachParametersNb ce);
 
void ScheduleRarReq(int rnti, int rapid, NbIotRrcSap::NprachParametersNb ue, NbIotRrcSap::DciN1::DciRepetitions rep);
void ScheduleNpdcchMessageReq(NbIotRrcSap::NpdcchMessage msg);
void ScheduleDlRlcBufferReq(LteMacSapProvider::ReportBufferStatusParameters params, NbIotRrcSap::NpdcchMessage::SearchSpaceType searchspace);
void ScheduleMsg5Req(int rnti);
void SetCeLevel(NbIotRrcSap::NprachParametersNb ce0, NbIotRrcSap::NprachParametersNb ce1, NbIotRrcSap::NprachParametersNb ce2);

std::vector<int> GetNextAvailableSearchSpaceCandidate(int SearchSpaceStartFrame, int SearchSpaceStartSubframe, int R_max, int R);
std::vector<int> GetDlSubframeRangeWithoutSystemResources(int overallSubframeNo, int numSubframes);
std::vector<int> GetUlSubframeRangeWithoutSystemResources(int overallSubframeNo, int numSubframes, int carrier);
std::vector<int> CheckforNContiniousSubframesDl(std::vector<int> Subframes, int StartSubframe, uint N);
std::vector<int> CheckforNContiniousSubframesUl(std::vector<int> Subframes, int StartSubframe, uint N, uint carrier);
std::vector<int> GetNextAvailableNpdschCandidate(int endSubframeDci, int minSchedulingDelay, int numSubframes, int R_max);
std::vector<std::pair<int,int>> GetAllPossibleSearchSpaceCandidates(std::vector<int> subframes, int R_max);
std::vector<NbIotRrcSap::NpdcchMessage> Schedule(int frameNo, int subframeNo);
std::vector<NbIotRrcSap::NpdcchMessage> ScheduleSearchSpace(NbIotRrcSap::NpdcchMessage::SearchSpaceType searchspace, NbIotRrcSap::NprachParametersNb ce);
std::vector<std::pair<int, std::vector<int>>> GetNextAvailableNpuschCandidate(int endSubframeNpdsch, int minSchedulingDelay, int numSubframes, bool isHarq);
std::pair<NbIotRrcSap::UlGrant, std::pair<int,std::vector<int>>> GetNextAvailableMsg3UlGrantCandidate(int endSubframeMsg2, int numSubframes);

std::vector<int8_t> m_downlink;
NbIotRrcSap::NprachParametersNb m_ce0;
protected:
  std::vector<std::vector<int8_t>> m_uplink;
  std::vector<NbIotRrcSap::NpdcchMessage> m_rars_to_schedule;
  std::vector<NbIotRrcSap::NpdcchMessage> m_NpdcchQueue;
  std::vector<NbIotRrcSap::NpdcchMessage> m_NpdschQueue;
  std::vector<NbIotRrcSap::NpdcchMessage> m_NpuschQueue;
  std::vector<NbIotRrcSap::DciN1::NpdcchTimeOffset> m_DciTimeOffsetRmaxSmall;
  std::vector<NbIotRrcSap::DciN1::NpdcchTimeOffset> m_DciTimeOffsetRmaxBig;
  std::vector<NbIotRrcSap::UlGrant::SchedulingDelay> m_Msg3TimeOffset;
  std::vector<NbIotRrcSap::HarqAckResource::TimeOffset> m_HarqTimeOffsets;
  std::vector<NbIotRrcSap::HarqAckResource::SubcarrierIndex> m_HarqSubcarrierIndex;
  NbIotRrcSap::NprachParametersNb m_ce1;
  NbIotRrcSap::NprachParametersNb m_ce2;
  int m_frameNo;
  bool m_only15KhzSpacing = true;
  int m_subframeNo;
  int m_currenthyperindex;
  const int m_minSchedulingDelayDci2Downlink = 4;
  const int m_minSchedulingDelayDci2Uplink = 8;
  bool m_log;
};

}  // namespace ns3

#endif /* FF_MAC_SCHEDULER_H */

