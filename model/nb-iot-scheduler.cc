/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Tim Gebauer <tim.gebauer@tu-dortmund.de>
 * Modified by: 
 *      Pascal Jörke <pascal.joerke@tu-dortmund.de>
 */


#include "nb-iot-scheduler.h"
#include <istream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <ns3/build-profile.h>
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NbiotScheduler");

NS_OBJECT_ENSURE_REGISTERED (NbiotScheduler);

NbiotScheduler::NbiotScheduler (std::vector<NbIotRrcSap::NprachParametersNb> ces,
                                NbIotRrcSap::SystemInformationBlockType2Nb sib2)
{
  m_Amc = NbiotAmc ();
  m_ce0 = ces[0];
  m_ce1 = ces[1];
  m_ce2 = ces[2];
  m_sib2config = sib2;
  m_DciTimeOffsetRmaxSmall.reserve (8);
  m_DciTimeOffsetRmaxBig.reserve (8);
  m_Msg3TimeOffset.reserve (4);
  m_logdir = "";

  m_DciTimeOffsetRmaxSmall.insert (
      m_DciTimeOffsetRmaxSmall.begin (),
      {NbIotRrcSap::DciN1::NpdcchTimeOffset::ms0, NbIotRrcSap::DciN1::NpdcchTimeOffset::ms4,
       NbIotRrcSap::DciN1::NpdcchTimeOffset::ms8, NbIotRrcSap::DciN1::NpdcchTimeOffset::ms12,
       NbIotRrcSap::DciN1::NpdcchTimeOffset::ms16, NbIotRrcSap::DciN1::NpdcchTimeOffset::ms32,
       NbIotRrcSap::DciN1::NpdcchTimeOffset::ms64, NbIotRrcSap::DciN1::NpdcchTimeOffset::ms128});
  m_DciTimeOffsetRmaxBig.insert (
      m_DciTimeOffsetRmaxBig.begin (),
      {NbIotRrcSap::DciN1::NpdcchTimeOffset::ms0, NbIotRrcSap::DciN1::NpdcchTimeOffset::ms16,
       NbIotRrcSap::DciN1::NpdcchTimeOffset::ms32, NbIotRrcSap::DciN1::NpdcchTimeOffset::ms64,
       NbIotRrcSap::DciN1::NpdcchTimeOffset::ms128, NbIotRrcSap::DciN1::NpdcchTimeOffset::ms256,
       NbIotRrcSap::DciN1::NpdcchTimeOffset::ms512, NbIotRrcSap::DciN1::NpdcchTimeOffset::ms1024});
  m_Msg3TimeOffset.insert (
      m_Msg3TimeOffset.begin (),
      {NbIotRrcSap::UlGrant::SchedulingDelay::ms12, NbIotRrcSap::UlGrant::SchedulingDelay::ms16,
       NbIotRrcSap::UlGrant::SchedulingDelay::ms32, NbIotRrcSap::UlGrant::SchedulingDelay::ms64});
  m_DciTimeOffsetUplink.insert (m_DciTimeOffsetUplink.begin (),
                                {NbIotRrcSap::DciN0::NpuschSchedulingDelay::ms8,
                                 NbIotRrcSap::DciN0::NpuschSchedulingDelay::ms16,
                                 NbIotRrcSap::DciN0::NpuschSchedulingDelay::ms32,
                                 NbIotRrcSap::DciN0::NpuschSchedulingDelay::ms64});

  m_HarqTimeOffsets.insert (m_HarqTimeOffsets.begin (),
                            {NbIotRrcSap::HarqAckResource::TimeOffset::thirteen,
                             NbIotRrcSap::HarqAckResource::TimeOffset::fifteen,
                             NbIotRrcSap::HarqAckResource::TimeOffset::seventeen,
                             NbIotRrcSap::HarqAckResource::TimeOffset::eighteen});
  m_HarqSubcarrierIndex.insert (m_HarqSubcarrierIndex.begin (),
                                {NbIotRrcSap::HarqAckResource::SubcarrierIndex::zero,
                                 NbIotRrcSap::HarqAckResource::SubcarrierIndex::one,
                                 NbIotRrcSap::HarqAckResource::SubcarrierIndex::two,
                                 NbIotRrcSap::HarqAckResource::SubcarrierIndex::three});

  uint64_t numHyperframes = 1024;
  uint64_t numFrames = 1024;
  uint64_t numSubframes = 10;
  // SIB1 Scheduling
  NbIotRrcSap::MasterInformationBlockNb m_mibNb;
  m_mibNb.schedulingInfoSib1 = 2;
  bool sib1NbPeriod = false;
  uint16_t sib1NbRepetitions = 0;
  m_downlink.resize (numHyperframes * numFrames * numSubframes, 0);
  NbIotRrcSap::SystemInformationBlockType1Nb sib1;
  NbIotRrcSap::SchedulingInfoNb info;
  info.sibMappingInfo.push_back (2);
  info.siPeriodicity = NbIotRrcSap::SchedulingInfoNb::SiPeriodicityNb::rf512;
  info.siRepetitionPattern = NbIotRrcSap::SchedulingInfoNb::SiRepetitionPatternNb::every4thRF;
  info.siTb = NbIotRrcSap::SchedulingInfoNb::SiTbNb::b440;
  sib1.schedulingInfoList.push_back (info);
  sib1.siWindowLength = NbIotRrcSap::SystemInformationBlockType1Nb::SiWindowLengthNb::ms160;
  sib1.siRadioFrameOffset = 0;
  bool si = false;
  uint8_t siRepetitions = 0;
  uint8_t siRepetitionPattern = 0;
  uint8_t tmpSiRepetitions = 0;
  uint16_t siWindow = 0;
  for (size_t i = 0; i < m_downlink.size (); ++i)
    {
      if ((i % 10) == 0)
        {
          m_downlink[i] = -1; // MIB-NB
        }
      if ((i % 10) == 5)
        {
          m_downlink[i] = -2; // NPSS
        }
      if (((i % 10) == 9) && ((i / 10) % 2 == 1))
        {
          m_downlink[i] = -3; // NSSS
        }
      if (((i / 10) % 256) == 0)
        {
          sib1NbPeriod = true;
          switch (m_mibNb.schedulingInfoSib1)
            {
            case 0:
            case 3:
            case 6:
            case 9:
              sib1NbRepetitions = 4;
              break;
            case 1:
            case 4:
            case 7:
            case 10:
              sib1NbRepetitions = 8;
              break;
            default:
              sib1NbRepetitions = 16;
              break;
            }
        }
      if (sib1NbPeriod && (i / 160) % 2 == 0)
        {

          if (((i % 10) == 4) && ((i / 10) % 2 == 0))
            {
              m_downlink[i] = -4; // SIB1-NB
            }
        }
      else if (sib1NbPeriod && (i / 10) % 16 == 0 && i % 10 == 0)
        {
          sib1NbRepetitions--;
        }
      if (sib1NbRepetitions == 0)
        {
          sib1NbPeriod = false;
        }
      // SI Scheduling

      for (size_t j = 0; j < sib1.schedulingInfoList.size (); j++)
        {

          uint16_t lhs =
              i / 10 %
              NbIotRrcSap::ConvertSchedulingInfoPeriodicity2int (sib1.schedulingInfoList[j]);
          uint16_t x = (j) *NbIotRrcSap::ConvertSiWindowLength2int (sib1) + sib1.siRadioFrameOffset;
          uint16_t rhs = x / 10 + sib1.siRadioFrameOffset;
          if (lhs == rhs && !si)
            {
              si = true;
              if (NbIotRrcSap::ConvertSchedulingInfoTb2int (sib1.schedulingInfoList[j]) > 120)
                {
                  siRepetitions = 8;
                }
              else
                {
                  siRepetitions = 2;
                }
              siWindow = NbIotRrcSap::ConvertSiWindowLength2int (sib1);
              siRepetitionPattern = NbIotRrcSap::ConvertSchedulingInfoRepetitionPattern2int (
                  sib1.schedulingInfoList[j]);
              // Begin of SI Message
            }
        }
      if (si)
        {
          if (((i / 10) - sib1.siRadioFrameOffset) % siRepetitionPattern == 0 && i % 10 == 0)
            {
              tmpSiRepetitions = siRepetitions;
            }
          if (tmpSiRepetitions > 0)
            {
              if (m_downlink[i] == 0)
                {
                  m_downlink[i] = -5;
                  tmpSiRepetitions--;
                }
            }
        }
      if (siWindow > 0)
        {
          siWindow--;
        }
      else
        {
          si = false;
        }
    }

  if (m_only15KhzSpacing)
    {
      m_uplink.resize (12, std::vector<int> ());
      for (size_t i = 0; i < m_uplink.size (); ++i)
        {
          m_uplink[i].resize (numHyperframes * numFrames * numSubframes, 0);
        }
      for (std::vector<NbIotRrcSap::NprachParametersNb>::iterator it = ces.begin ();
           it != ces.end (); ++it)
        {
          uint64_t sendingTime = NbIotRrcSap::ConvertNprachStartTime2int (*it);
          double ts = 1000.0 / (15000.0 * 2048.0);
          double preambleSymbolTime = 8192.0 * ts;
          double preambleGroupTimeNoCP = 5.0 * preambleSymbolTime;
          double preambleGroupTime = 0.266 + preambleGroupTimeNoCP;
          double preambleRepetition = 4.0 * preambleGroupTime;
          double nprachduration =
              (NbIotRrcSap::ConvertNumRepetitionsPerPreambleAttempt2int (*it) * preambleRepetition);
          size_t subcarrierOffset = NbIotRrcSap::ConvertNprachSubcarrierOffset2int (*it);
          uint8_t numberSubcarriers = NbIotRrcSap::ConvertNprachNumSubcarriers2int (*it);
          double time_tmp = uint64_t (nprachduration) + 1;

          for (size_t i = 0; i < m_uplink[0].size (); ++i)
            {

              uint16_t window_condition =
                  (i / 10) % (NbIotRrcSap::ConvertNprachPeriodicity2int (*it) / 10);
              if (window_condition == 0)
                {
                  i += sendingTime;
                  for (size_t j = 0; j < time_tmp; ++j)
                    {
                      for (size_t k = 0; k < numberSubcarriers / 4; ++k)
                        {
                          m_uplink[subcarrierOffset / 4 + k][i + j] = -1;
                        }
                    }
                  i += time_tmp;
                }
            }
        }
    }
}

void
NbiotScheduler::DoDispose ()
{
  LogUplinkGrid();
  LogDownlinkGrid ();
  NS_LOG_FUNCTION (this);
}

void
NbiotScheduler::SetCeLevel (NbIotRrcSap::NprachParametersNb ce0,
                            NbIotRrcSap::NprachParametersNb ce1,
                            NbIotRrcSap::NprachParametersNb ce2)
{
  m_ce0 = ce0;
  m_ce1 = ce1;
  m_ce2 = ce2;
}
void
NbiotScheduler::SetUssSearchSpaces (NbIotRrcSap::NpdcchConfigDedicatedNb uss0,
                                    NbIotRrcSap::NpdcchConfigDedicatedNb uss1,
                                    NbIotRrcSap::NpdcchConfigDedicatedNb uss2)
{
  m_uss0 = uss0;
  m_uss1 = uss1;
  m_uss2 = uss2;
}
bool
NbiotScheduler::IsSearchSpaceBegin (SearchSpaceConfig ssc)
{
  uint32_t searchSpacePeriodicity = ssc.R_max * ssc.startSf;
  uint32_t searchSpaceConditionLeftSide =
      (10 * (m_frameNo - 1) + (m_subframeNo - 1)) % searchSpacePeriodicity;
  uint32_t searchSpaceConditionRightSide = ssc.offset * searchSpacePeriodicity;
  if (searchSpaceConditionLeftSide == searchSpaceConditionRightSide)
    {
      return true;
    }
  return false;
}

void
NbiotScheduler::SetRntiRsrpMap (std::map<uint16_t, double> map)
{
  m_rntiRsrpMap = map;
}

void
NbiotScheduler::ScheduleRarReq (NbIotRrcSap::NpdcchMessage msg, SearchSpaceConfig ssc)
{
  // NPDCCH Parameters taken from Liberg, Olof, et al. The Cellular Internet of Things 2017 p.305, In-Band-Deployment Table 8.9

  if (msg.ce == m_ce0.coverageEnhancementLevel)
    {
      msg.dciN0.dciRepetitions = NbIotRrcSap::DciN0::DciRepetitions::r2;
      msg.dciN1.dciRepetitions = NbIotRrcSap::DciN1::DciRepetitions::r2;
    }
  else if (msg.ce == m_ce1.coverageEnhancementLevel)
    {
      msg.dciN0.dciRepetitions = NbIotRrcSap::DciN0::DciRepetitions::r32;
      msg.dciN1.dciRepetitions = NbIotRrcSap::DciN1::DciRepetitions::r32;
    }
  else if (msg.ce == m_ce2.coverageEnhancementLevel)
    {
      msg.dciN0.dciRepetitions = NbIotRrcSap::DciN0::DciRepetitions::r256;
      msg.dciN1.dciRepetitions = NbIotRrcSap::DciN1::DciRepetitions::r256;
    }

  for (std::vector<NbIotRrcSap::Rar>::iterator it = msg.rars.begin (); it != msg.rars.end (); ++it)
    {
      m_rntiUeConfigMap[it->cellRnti] = UeConfig ();

      m_rntiUeConfigMap[it->cellRnti].lastDl = 0;
      m_rntiUeConfigMap[it->cellRnti].lastUl = 0;
      m_rntiUeConfigMap[it->cellRnti].rlcDlBuffer = 0;
      m_rntiUeConfigMap[it->cellRnti].rlcUlBuffer = 0;
      m_rntiUeConfigMap[it->cellRnti].rnti = it->cellRnti;
      m_rntiUeConfigMap[it->cellRnti].searchSpaceConfig = ssc;
    }
  m_rarQueue[ssc].push_back (msg);
}

std::vector<NbIotRrcSap::NpdcchMessage>
NbiotScheduler::Schedule (uint64_t frameNo, uint64_t subframeNo)
{
  m_frameNo = frameNo;
  m_subframeNo = subframeNo;
  std::vector<NbIotRrcSap::NpdcchMessage> ret = std::vector<NbIotRrcSap::NpdcchMessage> ();
  std::vector<NbIotRrcSap::NpdcchMessage> tmp;
  if (frameNo == 1 && subframeNo == 1)
    {
      return ret;
    }
  // check and Schedule DCIs for SearchSpaceType2 (RAR, HARQ, RRC)
  SearchSpaceConfig currentSearchSpace;
  if (IsSearchSpaceBegin (ConvertNprachParametersNb2SearchSpaceConfig (m_ce0)))
    {
      currentSearchSpace = ConvertNprachParametersNb2SearchSpaceConfig (m_ce0);
      tmp = ScheduleSearchSpace (currentSearchSpace);
      ret.reserve (ret.size () + std::distance (tmp.begin (), tmp.end ()));
      ret.insert (ret.end (), tmp.begin (), tmp.end ());
    }
  if (IsSearchSpaceBegin (ConvertNprachParametersNb2SearchSpaceConfig (m_ce1)))
    {
      currentSearchSpace = ConvertNprachParametersNb2SearchSpaceConfig (m_ce1);
      tmp = ScheduleSearchSpace (currentSearchSpace);
      ret.reserve (ret.size () + std::distance (tmp.begin (), tmp.end ()));
      ret.insert (ret.end (), tmp.begin (), tmp.end ());
    }
  if (IsSearchSpaceBegin (ConvertNprachParametersNb2SearchSpaceConfig (m_ce2)))
    {
      currentSearchSpace = ConvertNprachParametersNb2SearchSpaceConfig (m_ce2);
      tmp = ScheduleSearchSpace (currentSearchSpace);
      ret.reserve (ret.size () + std::distance (tmp.begin (), tmp.end ()));
      ret.insert (ret.end (), tmp.begin (), tmp.end ());
    }

  // check and Schedule DCIs for User specific searchspace
  //  if (IsSeachSpaceType2Begin (m_uss0))
  //    {
  //      tmp = ScheduleSearchSpace (NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2, m_ce0);
  //      ret.reserve(ret.size() + std::distance(tmp.begin(),tmp.end()));
  //      ret.insert(ret.end(),tmp.begin(),tmp.end());
  //    }
  //  if (IsSeachSpaceType2Begin (m_ce1))
  //    {
  //      tmp = ScheduleSearchSpace (NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2, m_ce1);
  //      ret.reserve(ret.size() + std::distance(tmp.begin(),tmp.end()));
  //      ret.insert(ret.end(),tmp.begin(),tmp.end());
  //    }
  //  if (IsSeachSpaceType2Begin (m_ce2))
  //    {
  //      tmp = ScheduleSearchSpace (NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2, m_ce2);
  //      ret.reserve(ret.size() + std::distance(tmp.begin(),tmp.end()));
  //      ret.insert(ret.end(),tmp.begin(),tmp.end());
  //    }
  //
  return ret;
}


void
NbiotScheduler::SortBasedOnSelectedSchedulingAlgorithm (SearchSpaceConfig ssc)
{
  // When implementing new Scheduling Algorithms
  RoundRobinScheduling (ssc);
}

void
NbiotScheduler::RoundRobinScheduling (SearchSpaceConfig ssc)
{
  std::vector<uint16_t> &rntis = m_searchSpaceRntiMap[ssc];
  // Optional : Sort List by RNTI
  if (rntis.size () == 0)
    {
      // Nothing to sort
      return;
    }
  std::sort (rntis.begin (), rntis.end ());

  // Find last scheduled RNTI of this searchspace
  uint16_t start_rnti = m_RoundRobinLastScheduled[ssc];
  std::vector<uint16_t>::iterator it = std::find (rntis.begin (), rntis.end (), start_rnti);
  // Rnti not in list anymore, e.g. RrcRelease or moved to another Search Space
  uint16_t offset = 0;
  if (it != rntis.end ())
    {
      offset = it - rntis.begin () + 1;
    }
  else
    {
      while (it == rntis.end ())
        {
          start_rnti = ((start_rnti + 1) % 65535) + 1;
          it = std::find (rntis.begin (), rntis.end (), start_rnti);
        }
      offset = it - rntis.begin ();
    }
  std::rotate (rntis.begin (), rntis.begin () + offset, rntis.end ());
}

bool
NbiotScheduler::ScheduleNpdcchMessage (NbIotRrcSap::NpdcchMessage &message, SearchSpaceConfig ssc)
{

  bool scheduleSuccessful = false;

  if (message.dciType == NbIotRrcSap::NpdcchMessage::DciType::n1)
    {
      std::vector<uint64_t> test = GetNextAvailableSearchSpaceCandidate (
          message.rnti, m_frameNo - 1, m_subframeNo - 1, ssc.R_max,
          NbIotRrcSap::ConvertDciN1Repetitions2int (message.dciN1));
      if (test.size () > 0) // WE GOT A DOWNLINK NPDCCH CANDIDATE
        {

          uint64_t subframesNpdsch =
              NbIotRrcSap::ConvertNumNpdschSubframesPerRepetition2int (message.dciN1) *
              NbIotRrcSap::ConvertNumNpdschRepetitions2int (message.dciN1);
          std::vector<uint64_t> npdschsubframes = GetNextAvailableNpdschCandidate (
              *(test.end () - 1), m_minSchedulingDelayDci2Downlink, subframesNpdsch, ssc.R_max);
          if (npdschsubframes.size () > 0) // WE GOT A DOWNLINK CANDIDATE
            {
              uint64_t subframesNpusch;
              std::pair<NbIotRrcSap::UlGrant, std::pair<uint8_t, std::vector<uint64_t>>> ulgrant;
              if (message.isRar)
                {
                  for (std::vector<NbIotRrcSap::Rar>::iterator rar = message.rars.begin ();
                       rar != message.rars.end ();)
                    {
                      uint64_t size_mac_pdu = 0;
                      if (message.isEdt)
                        {
                          if (message.ce ==
                              NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::zero)
                            {
                              size_mac_pdu = NbIotRrcSap::ConvertEdtTbs2int (
                                  m_sib2config.radioResourceConfigCommon.nprachConfigR15
                                      .edtTbsInfoList.edtTbsNb0);
                            }
                          else if (message.ce ==
                                   NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::one)
                            {
                              size_mac_pdu = NbIotRrcSap::ConvertEdtTbs2int (
                                  m_sib2config.radioResourceConfigCommon.nprachConfigR15
                                      .edtTbsInfoList.edtTbsNb1);
                            }
                          else if (message.ce ==
                                   NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::two)
                            {
                              size_mac_pdu = NbIotRrcSap::ConvertEdtTbs2int (
                                  m_sib2config.radioResourceConfigCommon.nprachConfigR15
                                      .edtTbsInfoList.edtTbsNb2);
                            }
                        }
                      else
                        {
                          size_mac_pdu = 88; // Fixed Ulgrant Size. See 136.331
                        }
                      uint64_t couplingloss = 0;
                      if (rar->ceLevel ==
                          NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::two)
                        {
                          couplingloss = 159; // PASCAL: Woher diese Zahlen?
                        }
                      else if (rar->ceLevel ==
                               NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::one)
                        {
                          couplingloss = 149;
                        }
                      else if (rar->ceLevel ==
                               NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::zero)
                        {
                          couplingloss = 139;
                        }
                      subframesNpusch =
                          m_Amc.getMsg3Subframes (couplingloss, size_mac_pdu, 15000, 15);
                      ulgrant = GetNextAvailableMsg3UlGrantCandidate (npdschsubframes.back (),
                                                                      subframesNpusch);
                      if (ulgrant.first.success) // WE GOT AN UPLINK MSG3 CANDIDATE
                        {
                          scheduleSuccessful = true;
                          rar->rarPayload.ulGrant = ulgrant.first;
                          rar->rarPayload.ulGrant.subframes = ulgrant.second;
                          rar->rarPayload.ulGrant.tbs_size = size_mac_pdu;
                          //NS_BUILD_DEBUG (std::cout << "Scheduling NPUSCH at ");
                          //NS_BUILD_DEBUG (std::cout << " Subcarrier " << ulgrant.second.first << " ");
                          for (size_t i = 0; i < ulgrant.second.second.size (); i++)
                            {
                              m_uplink[ulgrant.second.first][ulgrant.second.second[i]] =
                                  message.rnti;
                                  //m_currenthyperindex;
                                  
                              //NS_BUILD_DEBUG (std::cout << ulgrant.second.second[i] << " ");
                            }
                          //NS_BUILD_DEBUG (std::cout << std::endl);
                          ++rar;
                          m_rntiUeConfigMap[rar->cellRnti].lastUl = ulgrant.second.second.back ();
                        }
                      else
                        {
                          message.rars.erase (rar);
                          // Stuff if cant schedule npusch
                        }
                    }
                }
              else
                {
                  // Create HARQ Ressource
                  uint64_t subframesNpuschHarq =
                      16; // Have to be set by higher layer | 4 for debugging
                  std::vector<std::pair<uint64_t, std::vector<uint64_t>>> npuschharqsubframes =
                      GetNextAvailableNpuschCandidate (*(npdschsubframes.end () - 1), 0,
                                                       subframesNpuschHarq, true);
                  if (npuschharqsubframes.size () > 0)
                    {
                      //NS_BUILD_DEBUG (std::cout << "Scheduling NPUSCH HARQ at ");
                      scheduleSuccessful = true;
                      for (size_t i = 0; i < npuschharqsubframes[0].second.size (); i++)
                        {
                          m_uplink[npuschharqsubframes[0].first][npuschharqsubframes[0].second[i]] =
                              message.rnti;
                              //m_currenthyperindex;
                          //NS_BUILD_DEBUG (std::cout << npuschharqsubframes[0].second[i] << " ");
                        }
                      //NS_BUILD_DEBUG (std::cout << std::endl);
                      message.dciN1.npuschOpportunity = npuschharqsubframes;

                      m_rntiUeConfigMap[message.rnti].lastUl =
                          npuschharqsubframes[0].second.back ();
                    }
                }
              if (scheduleSuccessful)
                {
                  //NS_BUILD_DEBUG (std::cout << "Scheduling NPDCCH at ");

                  for (size_t j = 0; j < test.size (); ++j)
                    {
                      m_downlink[test[j]] = message.rnti;//m_currenthyperindex;
                      //NS_BUILD_DEBUG (std::cout << test[j] << " ");
                    }

                  //NS_BUILD_DEBUG (std::cout << std::endl);
                  //NS_BUILD_DEBUG (std::cout << "Scheduling NPDSCH at ");

                  for (size_t j = 0; j < npdschsubframes.size (); ++j)
                    {
                      m_downlink[npdschsubframes[j]] = message.rnti;//m_currenthyperindex;
                      //NS_BUILD_DEBUG (std::cout << npdschsubframes[j] << " ");
                    }

                  message.dciRepetitionsubframes = test;
                  message.dciN1.npdschOpportunity = npdschsubframes;
                  message.dciN1.dciSubframes = test;
                  //NS_BUILD_DEBUG (std::cout << std::endl);
                  return true;
                }
            }
        }
    }
  else if (message.dciType == NbIotRrcSap::NpdcchMessage::DciType::n0)
    {
      std::vector<uint64_t> test = GetNextAvailableSearchSpaceCandidate (
          message.rnti, m_frameNo - 1, m_subframeNo - 1, ssc.R_max,
          NbIotRrcSap::ConvertDciN0Repetitions2int (message.dciN0));
      if (test.size () > 0)
        {
          uint64_t len_ru = NbIotRrcSap::ConvertLenResourceUnits2int(message.dciN0); // 15 khz spacing 
          uint64_t subframesNpusch = NbIotRrcSap::ConvertNumResourceUnits2int (message.dciN0)*len_ru *
                                     NbIotRrcSap::ConvertNumNpuschRepetitions2int (message.dciN0);
          // Have to be set by higher layer | 4 for debugging
          std::vector<std::pair<uint64_t, std::vector<uint64_t>>> npuschsubframes =
              GetNextAvailableNpuschCandidate (*(test.end () - 1), 0, subframesNpusch, true);

          if (npuschsubframes.size () > 0)
            {
              //NS_BUILD_DEBUG (std::cout << "Scheduling NPDCCH at ");

              for (size_t j = 0; j < test.size (); ++j)
                {
                  m_downlink[test[j]] = message.rnti;//m_currenthyperindex;
                  //NS_BUILD_DEBUG (std::cout << test[j] << " ");
                }

              //NS_BUILD_DEBUG (std::cout << std::endl);
              //NS_BUILD_DEBUG (std::cout << "Scheduling NPUSCH at ");
              scheduleSuccessful = true;
              for (size_t i = 0; i < npuschsubframes[0].second.size (); i++)
                {
                  m_uplink[npuschsubframes[0].first][npuschsubframes[0].second[i]] =
                      message.rnti;
                      //m_currenthyperindex;
                  //NS_BUILD_DEBUG (std::cout << npuschsubframes[0].second[i] << " ");
                }
              //NS_BUILD_DEBUG (std::cout << std::endl);

              message.dciRepetitionsubframes = test;
              message.dciN0.npuschOpportunity = npuschsubframes;
              message.dciN0.dciSubframes = test;
              m_rntiUeConfigMap[message.rnti].lastUl = npuschsubframes[0].second.back ();
              //NS_BUILD_DEBUG (std::cout << std::endl);
              return true;
            }
        }
    }
  return false;
}

std::vector<NbIotRrcSap::NpdcchMessage>
NbiotScheduler::ScheduleSearchSpace (SearchSpaceConfig ssc)
{
  std::vector<NbIotRrcSap::NpdcchMessage> scheduledMessages;

  //AddRntiDatatoNpdcchQueue(seachspace);

  // priorities rar
  for (std::vector<NbIotRrcSap::NpdcchMessage>::iterator rar = m_rarQueue[ssc].begin ();
       rar != m_rarQueue[ssc].end ();)
    {
      if (ScheduleNpdcchMessage ((*rar), ssc))
        {
          scheduledMessages.push_back (*rar);
          m_rarQueue[ssc].erase (rar);
        }
      else
        {
          ++rar;
        }
    }
  /*
  Scheduling Magic. For now FIFO
  */
  SortBasedOnSelectedSchedulingAlgorithm (ssc);
  std::vector<uint16_t> test_tmp = m_searchSpaceRntiMap[ssc];
  for (std::vector<uint16_t>::iterator it = m_searchSpaceRntiMap[ssc].begin ();
       it != m_searchSpaceRntiMap[ssc].end ();)
    {
      NbIotRrcSap::NpdcchMessage dci_candidate;
      if (m_rntiUeConfigMap[(*it)].priority == UeConfig::SchedulePriority::DOWNLINK)
        {
          if (m_rntiUeConfigMap[(*it)].rlcDlBuffer > 0)
            {
              dci_candidate =
                  CreateDciNpdcchMessage ((*it), NbIotRrcSap::NpdcchMessage::DciType::n1);
              if (ScheduleNpdcchMessage (dci_candidate, ssc))
                {
                  scheduledMessages.push_back (dci_candidate);
                  m_rntiUeConfigMap[(*it)].rlcDlBuffer = 0;
                  m_RoundRobinLastScheduled[ssc] = (*it);
                  m_rntiUeConfigMap[(*it)].priority = UeConfig::SchedulePriority::UPLINK;
                }
            }
              else if (m_rntiUeConfigMap[(*it)].rlcUlBuffer > 0)
                {
                  dci_candidate =
                      CreateDciNpdcchMessage ((*it), NbIotRrcSap::NpdcchMessage::DciType::n0);
                  if (ScheduleNpdcchMessage (dci_candidate, ssc))
                    {
                      scheduledMessages.push_back (dci_candidate);
                      if (int (m_rntiUeConfigMap[(*it)].rlcUlBuffer - (dci_candidate.tbs / 8)) > 0)
                        {
                          m_rntiUeConfigMap[(*it)].rlcUlBuffer =
                              m_rntiUeConfigMap[(*it)].rlcUlBuffer - (dci_candidate.tbs / 8);
                        }
                      else
                        {
                          m_rntiUeConfigMap[(*it)].rlcUlBuffer = 0;
                        }
                      m_RoundRobinLastScheduled[ssc] = (*it);
                    }
                }
            }
      else if (m_rntiUeConfigMap[(*it)].priority == UeConfig::SchedulePriority::UPLINK)
        {
          if (m_rntiUeConfigMap[(*it)].rlcUlBuffer > 0)
            {
              dci_candidate =
                  CreateDciNpdcchMessage ((*it), NbIotRrcSap::NpdcchMessage::DciType::n0);
              if (ScheduleNpdcchMessage (dci_candidate, ssc))
                {
                  scheduledMessages.push_back (dci_candidate);
                  if (int (m_rntiUeConfigMap[(*it)].rlcUlBuffer - (dci_candidate.tbs / 8)) > 0)
                    {
                      m_rntiUeConfigMap[(*it)].rlcUlBuffer =
                          m_rntiUeConfigMap[(*it)].rlcUlBuffer - (dci_candidate.tbs / 8);
                    }
                  else
                    {
                      m_rntiUeConfigMap[(*it)].rlcUlBuffer = 0;
                    }
                  m_RoundRobinLastScheduled[ssc] = (*it);
                  m_rntiUeConfigMap[(*it)].priority = UeConfig::SchedulePriority::DOWNLINK;
                }
            }
          else if (m_rntiUeConfigMap[(*it)].rlcDlBuffer > 0)
            {
              dci_candidate =
                  CreateDciNpdcchMessage ((*it), NbIotRrcSap::NpdcchMessage::DciType::n1);
              if (ScheduleNpdcchMessage (dci_candidate, ssc))
                {
                  scheduledMessages.push_back (dci_candidate);
                  m_rntiUeConfigMap[(*it)].rlcDlBuffer = 0;
                  m_RoundRobinLastScheduled[ssc] = (*it);
                }
            }
        }
      ++it;
    }

  return scheduledMessages;
}

std::vector<std::pair<uint64_t, std::vector<uint64_t>>>
NbiotScheduler::GetNextAvailableNpuschCandidate (uint64_t endSubframeNpdsch,
                                                 uint64_t minSchedulingDelay, uint64_t numSubframes,
                                                 bool isHarq)
{
  std::vector<std::pair<uint64_t, std::vector<uint64_t>>> allocation;
  if (isHarq)
    {

      for (auto &i : m_HarqTimeOffsets)
        {
          for (size_t j = 0; j < 4; ++j)
            { // For subcarrier 0-3 for 15khz Subcarrier spacing | needs change for 3.75 Khz
              uint64_t candidate =
                  endSubframeNpdsch + NbIotRrcSap::HarqAckResource::ConvertHarqTimeOffset2int (i);
              std::vector<uint64_t> subframesOccupied =
                  GetUlSubframeRangeWithoutSystemResources (candidate, numSubframes, j);
              subframesOccupied =
                  CheckforNContinuousSubframesUl (subframesOccupied, candidate, numSubframes, j);
              if (subframesOccupied.size () > 0)
                {
                  allocation.push_back (std::make_pair (j, subframesOccupied));
                  return allocation;
                }
            }
        }
    }
  else
    {
      /*TODO NPUSCH DELAYS ETC*/
      for (auto &i : m_DciTimeOffsetUplink)
        {
          for (size_t j = 0; j < 12; ++j)
            { // For subcarrier 0-3 for 15khz Subcarrier spacing | needs change for 3.75 Khz
              NbIotRrcSap::DciN0 tmp;
              tmp.npuschSchedulingDelay = i;
              uint64_t candidate = endSubframeNpdsch +
                                   NbIotRrcSap::ConvertNpuschSchedulingDelay2int (tmp) +
                                   1; // Start on next frame aber minSchedulingDelay
              std::vector<uint64_t> subframesOccupied =
                  GetUlSubframeRangeWithoutSystemResources (candidate, numSubframes, j);
              subframesOccupied =
                  CheckforNContinuousSubframesUl (subframesOccupied, candidate, numSubframes, j);
              if (subframesOccupied.size () > 0)
                {
                  return allocation;
                }
            }
        }
    }
  return std::vector<std::pair<uint64_t, std::vector<uint64_t>>> ();
}

std::pair<NbIotRrcSap::UlGrant, std::pair<uint64_t, std::vector<uint64_t>>>
NbiotScheduler::GetNextAvailableMsg3UlGrantCandidate (uint64_t endSubframeMsg2,
                                                      uint64_t numSubframes)
{
  for (auto &i : m_Msg3TimeOffset)
    {
      for (size_t j = 0; j < m_uplink.size (); ++j)
        {
          uint64_t candidate = endSubframeMsg2 +
                               NbIotRrcSap::UlGrant::ConvertUlGrantSchedulingDelay2int (i) +
                               1; // Start one subframe after delay
          std::vector<uint64_t> subframesOccupied =
              GetUlSubframeRangeWithoutSystemResources (candidate, numSubframes, j);
          subframesOccupied =
              CheckforNContinuousSubframesUl (subframesOccupied, candidate, numSubframes, j);
          if (subframesOccupied.size () > 0)
            {
              NbIotRrcSap::UlGrant ret;
              ret.schedulingDelay = i;
              ret.msg3Repetitions = NbIotRrcSap::UlGrant::Msg3Repetitions::r4;
              ret.subcarrierIndication = j;
              ret.Subcarrierspacing = 1;
              ret.success = true;
              ret.subframes = std::make_pair (j, subframesOccupied);
              return std::make_pair (ret, std::make_pair (j, subframesOccupied));
            }
        }
    }
  NbIotRrcSap::UlGrant ret;
  ret.success = false;
  return std::make_pair (ret, std::make_pair (uint64_t (), std::vector<uint64_t> ()));
}
std::vector<uint64_t>
NbiotScheduler::GetNextAvailableNpdschCandidate (uint64_t endSubframeDci,
                                                 uint64_t minSchedulingDelay, uint64_t numSubframes,
                                                 uint64_t R_max)
{

  uint64_t npdschCandidate = endSubframeDci + minSchedulingDelay +
                             1; // Start on the next Subframe of afer minSchedulingDelay
      // |0|1|2|3|4|5|6|7|8|9|10|
      //     |^  |^      |^     |
      //     |DCI|Delay  |NPDSCH|
  if (R_max < 128)
    {
      for (auto &i : m_DciTimeOffsetRmaxSmall)
        {
          NbIotRrcSap::DciN1 tmp; /// FIX AS SOON AS POSSIBLE
          tmp.npdcchTimeOffset = i;
          uint64_t tmpCandidate = npdschCandidate + NbIotRrcSap::ConvertNpdcchTimeOffset2int (tmp);
          std::vector<uint64_t> subframesOccupied =
              GetDlSubframeRangeWithoutSystemResources (tmpCandidate, numSubframes);
          subframesOccupied =
              CheckforNContinuousSubframesDl (subframesOccupied, tmpCandidate, numSubframes);
          if (subframesOccupied.size () > 0)
            {
              return subframesOccupied;
            }
        }
    }
  else
    {
      for (auto &i : m_DciTimeOffsetRmaxBig)
        {
          NbIotRrcSap::DciN1 tmp; /// FIX AS SOON AS POSSIBLE
          tmp.npdcchTimeOffset = i;
          uint64_t tmpCandidate = npdschCandidate + NbIotRrcSap::ConvertNpdcchTimeOffset2int (tmp);
          std::vector<uint64_t> subframesOccupied =
              GetDlSubframeRangeWithoutSystemResources (tmpCandidate, numSubframes);
          subframesOccupied =
              CheckforNContinuousSubframesDl (subframesOccupied, tmpCandidate, numSubframes);
          if (subframesOccupied.size () > 0)
            {
              return subframesOccupied;
            }
        }
    }
  return std::vector<uint64_t> ();
}

std::vector<uint64_t>
NbiotScheduler::GetDlSubframeRangeWithoutSystemResources (uint64_t overallSubframeNo,
                                                          uint64_t numSubframes)
{
  std::vector<uint64_t> subframeIndexes;
  size_t i = 0; // Starting on the given Subframe
  m_currenthyperindex = 1;
  while (numSubframes > 0)
    {
      size_t currentindex = overallSubframeNo + i;
      if ((m_downlink[currentindex] == 0))
        {
          subframeIndexes.push_back (currentindex);
          numSubframes--;
        }
      i++;
    }
  return subframeIndexes;
}

std::vector<uint64_t>
NbiotScheduler::GetUlSubframeRangeWithoutSystemResources (uint64_t overallSubframeNo,
                                                          uint64_t numSubframes, uint64_t carrier)
{
  std::vector<uint64_t> subframeIndexes;
  size_t i = 0;
  m_currenthyperindex = 1;
  while (numSubframes > 0)
    {
      size_t currentindex = overallSubframeNo + i;
      if ((m_uplink[carrier][currentindex] != -1))
        {
          subframeIndexes.push_back (currentindex);
          numSubframes--;
        }
      i++;
    }
  return subframeIndexes;
}
std::vector<uint64_t>
NbiotScheduler::CheckforNContinuousSubframesDl (std::vector<uint64_t> Subframes,
                                                uint64_t StartSubframe, uint64_t N)
{
  int startSubframeIndex = -1;
  std::vector<uint64_t> range;
  for (size_t i = 0; i < Subframes.size (); ++i)
    {
      if (Subframes[i] == StartSubframe)
        {
          startSubframeIndex = i;
          break;
        }
    }
  if (startSubframeIndex == -1)
    {
      return std::vector<uint64_t> ();
    }

  for (size_t i = 0; i < N; i++)
    {
      if (m_downlink[Subframes[startSubframeIndex + i]] > 0) // if > 0, then subframes are already used by user specific data
        {
          return std::vector<uint64_t> ();
        }
      else
        {
          range.push_back (Subframes[startSubframeIndex + i]);
        }
    }
  return range;
}
std::vector<uint64_t>
NbiotScheduler::CheckforNContinuousSubframesUl (std::vector<uint64_t> Subframes,
                                                uint64_t StartSubframe, uint64_t N,
                                                uint64_t carrier)
{
  int startSubframeIndex = -1;
  std::vector<uint64_t> range;
  for (size_t i = 0; i < Subframes.size (); ++i)
    {
      if (Subframes[i] == StartSubframe)
        {
          startSubframeIndex = i;
          break;
        }
    }
  if (startSubframeIndex == -1)
    {
      return std::vector<uint64_t> ();
    }

  for (size_t i = 0; i < N; i++)
    {
      if (m_uplink[carrier][Subframes[startSubframeIndex + i]] > 0) // if > 0, then subframes are already used by user specific data
        {
          return std::vector<uint64_t> ();
        }
      else
        {
          range.push_back (Subframes[startSubframeIndex + i]);
        }
    }
  return range;
}
std::vector<uint64_t>
NbiotScheduler::GetNextAvailableSearchSpaceCandidate (uint32_t rnti, uint64_t SearchSpaceStartFrame,
                                                      uint64_t SearchSpaceStartSubframe,
                                                      uint64_t R_max, uint64_t R)
{
  uint64_t u_max = ((R_max / R) - 1);
  uint64_t overallSubframe = 10 * (SearchSpaceStartFrame) + SearchSpaceStartSubframe;
  std::vector<uint64_t> subframes =
      GetDlSubframeRangeWithoutSystemResources (overallSubframe, R_max);
  for (size_t i = 0; i <= u_max; ++i)
    {
      // Calculate start of dci candidate
      std::vector<uint64_t> subframes_to_use =
          CheckforNContinuousSubframesDl (subframes, subframes[i * R], R);

      if (subframes_to_use.size () > 0)
        {
          if (subframes_to_use.front () > m_rntiUeConfigMap[rnti].lastUl + 3)
            {
              return subframes_to_use;
            }
        }
    }
  return std::vector<uint64_t> ();
}

NbIotRrcSap::NpdcchMessage
NbiotScheduler::CreateDciNpdcchMessage (uint16_t rnti, NbIotRrcSap::NpdcchMessage::DciType dci_type)
{
  double correction_factor =
      10 *
      log10 (
          1.0 /
          12.0); // correctionfactor applied to rsrp because it's for earch subcarrier and tx power is for full spectrum

  //NS_BUILD_DEBUG (std::cout << "MCL of " << rnti << " is " << m_rntiRsrpMap[rnti] - 43.0 - correction_factor << std::endl);

  NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel ceLevel;
  NbIotRrcSap::DciN1::DciRepetitions dciN1Repetitions;
  NbIotRrcSap::DciN0::DciRepetitions dciN0Repetitions;

  if (m_rntiRsrpMap[rnti] < m_sib2config.radioResourceConfigCommon.nprachConfig
                                .rsrpThresholdsPrachInfoList.ce2_lowerbound)
    {
      dciN1Repetitions = NbIotRrcSap::DciN1::DciRepetitions::r256;
      dciN0Repetitions = NbIotRrcSap::DciN0::DciRepetitions::r256;
      ceLevel = m_ce2.coverageEnhancementLevel;
    }
  else if (m_rntiRsrpMap[rnti] < m_sib2config.radioResourceConfigCommon.nprachConfig
                                     .rsrpThresholdsPrachInfoList.ce1_lowerbound)
    {
      dciN1Repetitions = NbIotRrcSap::DciN1::DciRepetitions::r32;
      dciN0Repetitions = NbIotRrcSap::DciN0::DciRepetitions::r32;
      ceLevel = m_ce1.coverageEnhancementLevel;
    }
  else if (m_rntiRsrpMap[rnti] > m_sib2config.radioResourceConfigCommon.nprachConfig
                                     .rsrpThresholdsPrachInfoList.ce1_lowerbound)
    {
      dciN1Repetitions = NbIotRrcSap::DciN1::DciRepetitions::r2;
      dciN0Repetitions = NbIotRrcSap::DciN0::DciRepetitions::r2;
      ceLevel = m_ce0.coverageEnhancementLevel;
    }
  else
    {
      dciN1Repetitions = NbIotRrcSap::DciN1::DciRepetitions::r2;
      dciN0Repetitions = NbIotRrcSap::DciN0::DciRepetitions::r2;
      ceLevel = m_ce0.coverageEnhancementLevel;
    }

  NbIotRrcSap::NpdcchMessage msg;
  msg.isRar = false;
  msg.rnti = rnti;
  msg.ce = ceLevel;

  if (dci_type == NbIotRrcSap::NpdcchMessage::DciType::n1)
    {
      uint64_t tbs = 0;

      if (m_rntiUeConfigMap[rnti].rlcDlBuffer * 8 > 680)
        { // max TBS Downlink Rel. 13
          tbs = 680;
        }
      else
        {
          tbs = (m_rntiUeConfigMap[rnti].rlcDlBuffer) * 8;
        }
      //std::cout << "Coupling Loss (N1) " <<m_rntiRsrpMap[rnti] - 43.0 - correction_factor << std::endl;
      std::pair<NbIotRrcSap::DciN1, uint64_t> dci_tbs = m_Amc.getBareboneDciN1 (
          m_rntiRsrpMap[rnti] - 43.0 - correction_factor, tbs, "standalone");

      NbIotRrcSap::DciN1 dci = dci_tbs.first;

      dci.mCS = NbIotRrcSap::DciN1::MCS::one;
      dci.tbs = dci_tbs.second;
      dci.NDI = true;
      dci.dciRepetitions = dciN1Repetitions;

      msg.dciType = NbIotRrcSap::NpdcchMessage::DciType::n1;
      msg.dciN1 = dci;
      msg.tbs = dci_tbs.second;
    }
  else if (dci_type == NbIotRrcSap::NpdcchMessage::DciType::n0)
    {
      uint64_t tbs = 0;
      //std::cout << m_rntiRsrpMap[rnti] << std::endl;
      //NpuschMeasurementValues val = m_Amc.getMaxTbsforCl(m_rntiRsrpMap[rnti] - 43.0 - correction_factor,15000,15);

      if (m_rntiUeConfigMap[rnti].rlcUlBuffer * 8 >= 1000)
        { // max TBS Uplink Rel. 13
          tbs = 1000;
        }
      else
        {
          tbs = (10+m_rntiUeConfigMap[rnti].rlcUlBuffer) * 8;
        }
      //std::cout << "Coupling Loss (N0) " <<m_rntiRsrpMap[rnti] - 43.0 - correction_factor << std::endl;
      std::pair<NbIotRrcSap::DciN0, uint64_t> dci_tbs =
          m_Amc.getBareboneDciN0 (m_rntiRsrpMap[rnti] - 43.0 - correction_factor, tbs, 15000, 15);

      NbIotRrcSap::DciN0 dci = dci_tbs.first;
      dci.mCS = NbIotRrcSap::DciN0::MCS::one;
      dci.tbs = dci_tbs.second;
      dci.NDI = true;
      dci.dciRepetitions = dciN0Repetitions;

      msg.dciType = NbIotRrcSap::NpdcchMessage::DciType::n0;
      msg.dciN0 = dci;
      msg.tbs = dci_tbs.second;
      msg.lcid = 0;
    }

  return msg;
}



void 
NbiotScheduler::SchedulePurNb(NbIotRrcSap::InfoPurRequest infoPurRequest)
{
  NbIotRrcSap::PurSetupRequest purSetupRequest = infoPurRequest.purSetupRequest;
  uint64_t imsi = infoPurRequest.imsi;
  uint16_t rnti = infoPurRequest.rnti;
  double rsrp = infoPurRequest.rsrp;
  double correction_factor =
      10 *
      log10 (
          1.0 /
          12.0); // correction factor applied to rsrp because it's for each subcarrier and tx power is for full spectrum

  //std::cout << "Beep5" << std::endl;
  uint32_t periodicity = NbIotRrcSap::ConvertPurPeriodicity2int(purSetupRequest.requestedPeriodicityR16); // in ms
  uint32_t nextAccess = purSetupRequest.requestedOffsetR16 * 10240; // purSetupRequest.requestedOffsetR16 in HSF (10.24s)
  bool infiniteOccasions = false;
  if (purSetupRequest.requestedNumOccasionsR16 == NbIotRrcSap::PurSetupRequest::RequestedNumOccasionsR16::infinite){
    infiniteOccasions = true;
  } else{
    infiniteOccasions = false;
  }
  std::cout << "SchedulePurNb: " << periodicity << ", " << nextAccess << ", " << infiniteOccasions << "\n";

  uint64_t tbs = NbIotRrcSap::ConvertRequestedTbs2int(purSetupRequest.requestedTbsR16);
  std::cout << "SchedulePurNb: imsi: "<< imsi<< "\n";
  std::cout << "SchedulePurNb: RNTI: "<< rnti<< "\n";
  int couplingloss = rsrp - 43.0 - correction_factor;
  std::cout << "SchedulePurNb: Couplingloss: " << couplingloss<< "\n";

  NpuschMeasurementValues npusch = m_Amc.getBareboneNpusch (m_rntiRsrpMap[rnti] - 43.0 - correction_factor, tbs, 15000, 15);
  if(ScheduleNpuschPur(npusch,rnti,periodicity, nextAccess,infiniteOccasions)){
    // Do something
  }

  // if (ScheduleNpdcchMessage (dci_candidate, ssc))
  //             {
  //               scheduledMessages.push_back (dci_candidate);
  //               if (int (m_rntiUeConfigMap[(*it)].rlcUlBuffer - (dci_candidate.tbs / 8)) > 0)
  //                 {
  //                   m_rntiUeConfigMap[(*it)].rlcUlBuffer =
  //                       m_rntiUeConfigMap[(*it)].rlcUlBuffer - (dci_candidate.tbs / 8);
  //                 }
  //               else
  //                 {
  //                   m_rntiUeConfigMap[(*it)].rlcUlBuffer = 0;
  //                 }
  //               m_RoundRobinLastScheduled[ssc] = (*it);
  //               m_rntiUeConfigMap[(*it)].priority = UeConfig::SchedulePriority::DOWNLINK;
  //             }

}

bool 
NbiotScheduler::ScheduleNpuschPur(NpuschMeasurementValues npusch, uint16_t rnti, uint32_t periodicity, uint32_t nextAccess, bool infiniteOcassions){
  bool scheduleSuccessful = false;
  
  uint8_t numRus = npusch.NRU;
  uint8_t lenRu = 0; // length of a single Resource Unit in [ms]. This depends on the number of subcarriers: 1SC: 8ms, 3SC: 4ms, 6SC: 2ms, 8SC: 1ms
  switch (npusch.NRUSC)
  {
  case 1:
    lenRu = 8;
    break;
  case 3:
    lenRu = 4;
    break;
  case 6:
    lenRu = 2;
    break;
   case 12:
    lenRu = 1;
    break;
  default:
    break;
  }
  uint16_t subframesNpusch = lenRu * npusch.NRU * npusch.NRep;

  std::cout << "ScheduleNpuschPur: " << npusch.SCS << ", " << npusch.NRUSC << ", " << npusch.NRU << ", " << npusch.TTI << "\n";


  std::vector<std::pair<uint64_t, std::vector<uint64_t>>> purGrant = GetNextAvailablePurNpuschCandidate (periodicity, nextAccess, subframesNpusch);

  // if (test.size () > 0)
  //   {
  //     uint64_t size_ru = 8; // 15 khz bandwidth // TODO Pascal: This was 1 by Tim. Why? Since BW is fixed to 15 KHz for now, each RU is 8ms long.
  //     uint64_t subframesNpusch = NbIotRrcSap::ConvertNumResourceUnits2int (message.dciN0)*size_ru *
  //                                 NbIotRrcSap::ConvertNumNpuschRepetitions2int (message.dciN0);
  //     // Have to be set by higher layer | 4 for debugging
  //     std::vector<std::pair<uint64_t, std::vector<uint64_t>>> npuschsubframes =
  //         GetNextAvailableNpuschCandidate (*(test.end () - 1), 0, subframesNpusch, true);

  //     if (npuschsubframes.size () > 0)
  //       {
  //         //NS_BUILD_DEBUG (std::cout << "Scheduling NPDCCH at ");

  //         for (size_t j = 0; j < test.size (); ++j)
  //           {
  //             m_downlink[test[j]] = message.rnti;//m_currenthyperindex;
  //             //NS_BUILD_DEBUG (std::cout << test[j] << " ");
  //           }

  //         //NS_BUILD_DEBUG (std::cout << std::endl);
  //         //NS_BUILD_DEBUG (std::cout << "Scheduling NPUSCH at ");
  //         scheduleSuccessful = true;
  //         for (size_t i = 0; i < npuschsubframes[0].second.size (); i++)
  //           {
  //             m_uplink[npuschsubframes[0].first][npuschsubframes[0].second[i]] =
  //                 message.rnti;
  //                 //m_currenthyperindex;
  //             //NS_BUILD_DEBUG (std::cout << npuschsubframes[0].second[i] << " ");
  //           }
  //         //NS_BUILD_DEBUG (std::cout << std::endl);

  //         message.dciRepetitionsubframes = test;
  //         message.dciN0.npuschOpportunity = npuschsubframes;
  //         message.dciN0.dciSubframes = test;
  //         m_rntiUeConfigMap[message.rnti].lastUl = npuschsubframes[0].second.back ();
  //         //NS_BUILD_DEBUG (std::cout << std::endl);
  //         return true;
  //       }
// }
  return false;
}

std::vector<std::pair<uint64_t, std::vector<uint64_t>>>
NbiotScheduler::GetNextAvailablePurNpuschCandidate (uint32_t periodicity, uint32_t nextAccess, uint64_t numSubframes)
{
  std::cout << "GetNextAvailablePurNpuschCandidate - Frame No: " << m_frameNo << ", Subframe No: " << m_subframeNo << "\n";
  std::vector<std::pair<uint64_t, std::vector<uint64_t>>> allocation;    
  uint16_t period = 0;
  uint64_t candidate = m_frameNo*10 + m_subframeNo + nextAccess + periodicity*period; // calculate the absolute time of the first PUR occasion
  bool purResourcesFound = false;
  while (purResourcesFound == false) // For all periodic occasions free resources must be found
  {
    bool firstPurResourceFound = false;
    while (candidate < (2*60*60 + 54*60 + 46)*1000) // Check for free resources in the complete hyperframe cycle
    {
      while (firstPurResourceFound == false) // Look for the UL grant of the first PUR occasion
      {
        for (size_t j = 0; j < 12; ++j) // check for all subcarriers
        { // For subcarrier 0-11 for 15khz Subcarrier spacing | needs change for 3.75 Khz
          //uint64_t candidate = m_frameNo*10 + m_subframeNo + nextAccess; // Start on next subframe
          std::vector<uint64_t> subframesRequired = GetUlSubframeRangeWithoutSystemResources (candidate, numSubframes, j); // This checks for subframes that aren't occupied by network-specific resources such as Random Access Windows
          std::vector<uint64_t> subframesOccupied = CheckforNContinuousSubframesUl (subframesRequired, candidate, numSubframes, j); // This checks if the required subframes are not already used by other users. If they are, check for other subframe
          if (subframesOccupied.size () > 0)
          {
            allocation.push_back (std::make_pair (j, subframesOccupied));
            firstPurResourceFound = true;
          }
        }
        candidate = candidate + 1; // if all subcarriers are occupied, try the next subframe
        if (candidate == (2*60*60 + 54*60 + 46)*1000) // End of hyperframe cycle
        {
          return std::vector<std::pair<uint64_t, std::vector<uint64_t>>> (); // No free resources were found
        }
      }
      
      for (size_t j = 0; j < 12; ++j)
      { // For subcarrier 0-11 for 15khz Subcarrier spacing | needs change for 3.75 Khz
        //uint64_t candidate = m_frameNo*10 + m_subframeNo + nextAccess; // Start on next subframe
        std::vector<uint64_t> subframesRequired = GetUlSubframeRangeWithoutSystemResources (candidate, numSubframes, j); // This checks for subframes that aren't occupied by network-specific resources such as Random Access Windows
        std::vector<uint64_t> subframesOccupied = CheckforNContinuousSubframesUl (subframesRequired, candidate, numSubframes, j); // This checks if the required subframes are not already used by other users. If they are, check for other subframe
        if (subframesOccupied.size () > 0)
          {
            allocation.push_back (std::make_pair (j, subframesOccupied));
            firstPurResourceFound = true;
          }
      }
    }
    return std::vector<std::pair<uint64_t, std::vector<uint64_t>>> ();
  }
}















//void
//NbiotScheduler::ScheduleMsg5Req (uint64_t rnti)
//{
//  m_RntiRlcUlBuffer[NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2][rnti] = 14;
//}
void
NbiotScheduler::ScheduleDlRlcBufferReq (
    uint64_t rnti, std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters> lcids)
{
  // Calculate RLC Buffer Size
  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it;
  uint64_t buffer_size = 0;
  std::map<uint8_t, uint64_t> buffers;
  for (it = lcids.begin (); it != lcids.end (); ++it)
    {
      if (it->second.txQueueSize != 0 || it->second.retxQueueSize != 0 ||
          it->second.statusPduSize != 0)
        {
          buffer_size +=
              it->second.txQueueSize + it->second.retxQueueSize + it->second.statusPduSize;
          if (it->second.txQueueSize > 0)
            {
              buffer_size += 4; // RLC Header
            }
        }
    }


  m_rntiUeConfigMap[rnti].rlcDlBuffer = buffer_size;
  SearchSpaceConfig searchSpace = m_rntiUeConfigMap[rnti].searchSpaceConfig;
  std::vector<uint16_t>::iterator findit = std::find (
      m_searchSpaceRntiMap[searchSpace].begin (), m_searchSpaceRntiMap[searchSpace].end (), rnti);
  if (findit == m_searchSpaceRntiMap[searchSpace].end ())
    {
      m_searchSpaceRntiMap[searchSpace].push_back (rnti);
    }
}
void
NbiotScheduler::ScheduleUlRlcBufferReq (uint64_t rnti, uint64_t dataSize)
{
  m_rntiUeConfigMap[rnti].rlcUlBuffer = dataSize;
  SearchSpaceConfig searchSpace = m_rntiUeConfigMap[rnti].searchSpaceConfig;
  std::vector<uint16_t>::iterator it = std::find (m_searchSpaceRntiMap[searchSpace].begin (),
                                                  m_searchSpaceRntiMap[searchSpace].end (), rnti);
  if (it == m_searchSpaceRntiMap[searchSpace].end ())
    {
      m_searchSpaceRntiMap[searchSpace].push_back (rnti);
    }
}

void
NbiotScheduler::AddToUlBufferReq (uint64_t rnti, uint64_t dataSize)
{
  if(m_rntiUeConfigMap.find(rnti) != m_rntiUeConfigMap.end()){
    m_rntiUeConfigMap[rnti].rlcUlBuffer += dataSize;
    SearchSpaceConfig searchSpace = m_rntiUeConfigMap[rnti].searchSpaceConfig;
    std::vector<uint16_t>::iterator it = std::find (m_searchSpaceRntiMap[searchSpace].begin (),
                                                    m_searchSpaceRntiMap[searchSpace].end (), rnti);
    if (it == m_searchSpaceRntiMap[searchSpace].end ())
      {
        m_searchSpaceRntiMap[searchSpace].push_back (rnti);
      }
  }
}

SearchSpaceConfig
NbiotScheduler::ConvertNpdcchConfigDedicatedNb2SearchSpaceConfig (
    NbIotRrcSap::NpdcchConfigDedicatedNb configDedicated)
{
  SearchSpaceConfig ssc;
  ssc.R_max = NbIotRrcSap::ConvertNpdcchNumRepetitions2int (configDedicated);
  ssc.startSf = NbIotRrcSap::ConvertNpdcchStartSfUss2double (configDedicated);
  ssc.offset = NbIotRrcSap::ConvertNpdcchOffsetUss2double (configDedicated);
  // CE Level not used in USS, but for simplicity set to ce0
  ssc.ce = NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::none;
  return ssc;
}
SearchSpaceConfig
NbiotScheduler::ConvertNprachParametersNb2SearchSpaceConfig (NbIotRrcSap::NprachParametersNb ce)
{
  SearchSpaceConfig ssc;
  ssc.R_max = NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int (ce);
  ssc.startSf = NbIotRrcSap::ConvertNpdcchStartSfCssRa2double (ce);
  ssc.offset = NbIotRrcSap::ConvertNpdcchOffsetRa2double (ce);
  ssc.ce = ce.coverageEnhancementLevel;
  return ssc;
}

void
NbiotScheduler::RemoveUe (uint16_t rnti)
{
  if (m_rntiUeConfigMap.find (rnti) == m_rntiUeConfigMap.end ())
    {
      // Ue was not added to Scheduler
      return;
    }
  UeConfig ue = m_rntiUeConfigMap[rnti];
  std::vector<uint16_t>::iterator it =
      std::find (m_searchSpaceRntiMap[ue.searchSpaceConfig].begin (),
                 m_searchSpaceRntiMap[ue.searchSpaceConfig].end (), rnti);
  if (it != m_searchSpaceRntiMap[ue.searchSpaceConfig].end ())
    {
      m_searchSpaceRntiMap[ue.searchSpaceConfig].erase (it);
    }
  m_rntiUeConfigMap.erase (rnti);
}

void
NbiotScheduler::LogUplinkGrid ()
{
  std::string logfile_path = m_logdir+"Spectral_Uplink.log";
  std::ofstream logfile;
  logfile.open (logfile_path, std::ios_base::app);
  // for (size_t i = 0; i < m_uplink.size (); i++)
  //   {
  //     for (int64_t j = 0; j < Simulator::Now ().GetMilliSeconds (); j++)
  //       {
  //         logfile << m_uplink[i][j] << ",";
  //       }
  //     logfile << "\n";
  //   }
    for (int64_t j = 0; j < Simulator::Now ().GetMilliSeconds (); j++){
      for (size_t i = 0; i < m_uplink.size (); i++){
        logfile << m_uplink[i][j] << ",";
      }
      logfile << "\n";
    }

    
  logfile.close ();
}

void NbiotScheduler::SetLogDir(std::string logdir){
  std::cout << logdir << std::endl;
  m_logdir = logdir;
}

void
NbiotScheduler::LogDownlinkGrid ()
{
  std::string logfile_path = m_logdir+"Spectral_Downlink.log";
  std::ofstream logfile;
  logfile.open (logfile_path, std::ios_base::app);
  for (int64_t i = 0; i < Simulator::Now ().GetMilliSeconds (); i++)
    {
      //if((i/10)% 16 == 0 && (i%10) == 0){
      //  logfile << "\n";
      //}
      if (m_downlink[i] > -1)
        {
          logfile << " ";
        }
      //logfile  << " "<< m_downlink[i]<< ",";
      logfile << " " << m_downlink[i] << "\n";
    }

  logfile.close ();
}

// Stuff that could be useful later... could(!!!!!)

//std::vector<std::pair<uint64_t, uint64_t>>
//NbiotScheduler::GetAllPossibleSearchSpaceCandidates (std::vector<uint64_t> subframes, uint64_t R_max)
//{
//  std::vector<std::pair<uint64_t, uint64_t>> candidates;
//  m_currenthyperindex = 1;
//  uint64_t start_sf;
//  uint64_t length = 0;
//  uint64_t i = 0;
//  start_sf = subframes[0];
//  while (R_max > 0)
//    {
//      if (m_downlink[subframes[i]] != m_currenthyperindex)
//        {
//          length++;
//        }
//      else
//        {
//          if (length > 0)
//            {
//              candidates.push_back (std::make_pair (start_sf, length));
//            }
//          start_sf = subframes[i];
//          length = 0;
//        }
//      R_max--;
//      i++;
//    }
//  if (candidates.size () == 0 && length > 0)
//    {
//      candidates.push_back (std::make_pair (start_sf, length));
//    }
//  return candidates;
//}

} // namespace ns3
