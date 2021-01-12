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

struct NpdcchUeInformation{
  NbIotRrcSap::NprachParametersNb ce;
  int ranti;
};
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
  NpdcchUeInformation ue;
  NbIotRrcSap::DciN1 dciN1;
  NbIotRrcSap::DciN0 dciN0;
  std::vector<int> npdschOpportunity;
  std::vector<int> npuschOpportunity;
  std::vector<int> dciRepetitionsubframes;
};

};


class NbiotScheduler : public Object
{
public:

  NbiotScheduler();

  //~NbiotScheduler();

  virtual void DoDispose (void);


bool IsSeachSpaceType2Begin(NbIotRrcSap::NprachParametersNb ce);
//bool IsSeachSpaceType1Begin(NbIotRrcSap::NprachParametersNb ce);
//bool IsSeachSpaceUeSpecificBegin(NbIotRrcSap::NprachParametersNb ce);
 
void ScheduleRarReq(int rnti, int rapid, NbIotRrcSap::NprachParametersNb ue, NbIotRrcSap::DciN1::DciRepetitions rep);
void ScheduleNpdcchMessageReq(NpdcchMessage msg);
std::vector<NpdcchMessage> Schedule(int frameNo, int subframeNo);
std::vector<NpdcchMessage> ScheduleSearchSpace(NpdcchMessage::SearchSpaceType searchspace, NbIotRrcSap::NprachParametersNb ce);
std::vector<int> GetNextAvailableSearchSpaceCandidate(int SearchSpaceStartFrame, int SearchSpaceStartSubframe, int R_max, int R);
std::vector<int> GetSubframeRangeWithoutSystemResources(int overallSubframeNo, int numSubframes);
std::vector<std::pair<int,int>> GetAllPossibleSearchSpaceCandidates(std::vector<int> subframes, int R_max);
std::vector<int> CheckforNContiniousSubframes(std::vector<int> Subframes, int StartSubframe, uint N);
std::vector<int> GetNextAvailableNpdschCandidate(int endSubframeDci, int minSchedulingDelay, int numSubframes, int R_max);
std::vector<int8_t> m_downlink;
void SetCeLevel(NbIotRrcSap::NprachParametersNb ce0, NbIotRrcSap::NprachParametersNb ce1, NbIotRrcSap::NprachParametersNb ce2);
NbIotRrcSap::NprachParametersNb m_ce0;
protected:
  std::vector<int8_t> m_uplink;
  std::vector<NpdcchMessage> m_NpdcchQueue;
  std::vector<NpdcchMessage> m_NpdschQueue;
  std::vector<NpdcchMessage> m_NpuschQueue;
  std::vector<NbIotRrcSap::DciN1::NpdcchTimeOffset> m_DciTimeOffsetRmaxSmall;
  std::vector<NbIotRrcSap::DciN1::NpdcchTimeOffset> m_DciTimeOffsetRmaxBig;
  NbIotRrcSap::NprachParametersNb m_ce1;
  NbIotRrcSap::NprachParametersNb m_ce2;
  int m_frameNo;
  int m_subframeNo;
  int m_currenthyperindex;
  const int m_minSchedulingDelayDci2Downlink = 4;
  const int m_minSchedulingDelayDci2Uplink = 8;
};

}  // namespace ns3

#endif /* FF_MAC_SCHEDULER_H */

