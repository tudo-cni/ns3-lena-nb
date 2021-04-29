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
  m_downlink.resize (numHyperframes * numFrames * numSubframes, 0);
  for (size_t i = 0; i < m_downlink.size (); ++i)
    {
      if ((i % 10) == 0)
        {
          m_downlink[i] = -1; // MIB-NB
        }
      if ((i % 10) == 5)
        {
          m_downlink[i] = -1; // NPSS
        }
      if (((i % 10) == 9) && ((i / 10) % 2 == 1))
        {
          m_downlink[i] = -1; // NSSS
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

          for (size_t i = 0; i < m_uplink[0].size () / 400; ++i)
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
NbiotScheduler::SetUssSearchSpaces(NbIotRrcSap::NpdcchConfigDedicatedNb uss0,
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
  uint32_t searchSpaceConditionLeftSide = (10 * (m_frameNo - 1) + (m_subframeNo - 1)) % searchSpacePeriodicity;
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
  else if (msg.ce== m_ce2.coverageEnhancementLevel)
    {
      msg.dciN0.dciRepetitions = NbIotRrcSap::DciN0::DciRepetitions::r256;
      msg.dciN1.dciRepetitions = NbIotRrcSap::DciN1::DciRepetitions::r256;
    }

  for(std::vector<NbIotRrcSap::Rar>::iterator it = msg.rars.begin(); it != msg.rars.end(); ++it){
    m_rntiUeConfigMap[it->cellRnti] = UeConfig();

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
  if (IsSearchSpaceBegin (ConvertNprachParametersNb2SearchSpaceConfig(m_ce0)))
    { 
      currentSearchSpace =  ConvertNprachParametersNb2SearchSpaceConfig(m_ce0);
      tmp = ScheduleSearchSpace (currentSearchSpace);
      ret.reserve(ret.size() + std::distance(tmp.begin(),tmp.end()));
      ret.insert(ret.end(),tmp.begin(),tmp.end());
    }
  if (IsSearchSpaceBegin(ConvertNprachParametersNb2SearchSpaceConfig(m_ce1)))
    {
      currentSearchSpace =  ConvertNprachParametersNb2SearchSpaceConfig(m_ce1);
      tmp = ScheduleSearchSpace (currentSearchSpace);
      ret.reserve(ret.size() + std::distance(tmp.begin(),tmp.end()));
      ret.insert(ret.end(),tmp.begin(),tmp.end());
    }
  if (IsSearchSpaceBegin(ConvertNprachParametersNb2SearchSpaceConfig(m_ce2)))
    {
      currentSearchSpace =  ConvertNprachParametersNb2SearchSpaceConfig(m_ce2);
      tmp = ScheduleSearchSpace (currentSearchSpace);
      ret.reserve(ret.size() + std::distance(tmp.begin(),tmp.end()));
      ret.insert(ret.end(),tmp.begin(),tmp.end());
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
//std::vector<NbIotRrcSap::NpdcchMessage>
//NbiotScheduler::ScheduleSearchSpace (NbIotRrcSap::NpdcchMessage::SearchSpaceType seachspace,
//                                     NbIotRrcSap::NprachParametersNb ce)
//{
//  bool scheduleSuccessful = false;
//  uint64_tR_max;
//  std::vector<NbIotRrcSap::NpdcchMessage> scheduledMessages;
//  if (seachspace == NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2)
//    {
//      R_max = NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2uint64_t(ce);
//    }
//  /*
//  Scheduling Magic. For now FIFO
//  */
//
//  for (std::vector<NbIotRrcSap::NpdcchMessage>::iterator it = m_NpdcchQueue.begin ();
//       it != m_NpdcchQueue.end ();)
//    {
//      if (it->searchSpaceType == seachspace)
//        {
//          if (it->ce.nprachSubcarrierOffset == ce.nprachSubcarrierOffset)
//            {
//              if (it->dciType == NbIotRrcSap::NpdcchMessage::DciType::n1)
//                {
//                  std::vector<int> test = GetNextAvailableSearchSpaceCandidate (
//                    m_frameNo - 1, m_subframeNo - 1, R_max,
//                    NbIotRrcSap::ConvertDciN1Repetitions2uint64_t(it->dciN1));
//                  if (test.size () > 0) // WE GOT A DOWNLINK NPDCCH CANDIDATE
//                    {
//
//                      int subframesNpdsch =
//                          NbIotRrcSap::ConvertNumNpdschSubframesPerRepetition2int (it->dciN1) *
//                          NbIotRrcSap::ConvertNumNpdschRepetitions2int (it->dciN1);
//                      std::vector<int> npdschsubframes = GetNextAvailableNpdschCandidate (
//                          *(test.end () - 1), m_minSchedulingDelayDci2Downlink, subframesNpdsch,
//                          R_max);
//                      if (npdschsubframes.size () > 0) // WE GOT A DOWNLINK CANDIDATE
//                        {
//                          int subframesNpusch;
//                          std::pair<NbIotRrcSap::UlGrant, std::pair<int, std::vector<int>>> ulgrant;
//                          if (it->isRar)
//                            {
//                              for (std::vector<NbIotRrcSap::Rar>::iterator rar = it->rars.begin ();
//                                   rar != it->rars.end ();)
//                                {
//                                  // MSG3 SIZE
//                                  // See Joerke NBIoT_UE.py
//                                  /*
//                                  The MAC PDU consists of the following (ref. ETSI TS 136 321 V13.9.0 Fig. 6.1.6-4):
//                                  |-----------------------------------------MAC-Header-----------------------------------------|--MAC-CE--|...|--MAC-SDU--|--Padding(opt)--|
//                                  |--R/F2/E/LCID subheader--|...|--R/F2/E/LCID/F/K subheader--|--R/R/E/LCID padding subheader--|
//                                  In Msg3 the UE should transmit a Buffer Status Report MAC CE in order to inform the eNodeB about its UL buffer status
//                                  Since RRCConnectionResumeComplete-NB is transmitted in RLC TM, TMD PDU consists only of a data field and doesn't consists of any RLC headers (Ref. ETSI TS 136 331 V13.15.0 p.522 "RRCConnectionResumeRequest-NB" and ETSI TS 136 322 V13.4.0, 6.2.1.2)
//                                  RRCConnectionResumeRequest-NB ist transmitted using SRB0 (ref. ETSI TS 136 331 V13.15.0 p.522 "RRCConnectionResumeRequest-NB" and therefore does not go through PDCP (ref. ETSI TS 136 323 V13.6.0, 4.2.1: "Each RB (i.e. DRB, SLRB and SRB, except for SRB0 and SRB1bis) is associated with one PDCP entity"))
//                                  */
//
//                                  int size_rrc_conn_resume_req =
//                                      59; // 40 bits resumeID +16 bits shortResumeMAC-I +3 bits resumeCause ref. ETSI TS 136 331 V13.15.0 p.522 RRCConnectionResumeRequest-NB
//                                  size_rrc_conn_resume_req +=
//                                      (8 - size_rrc_conn_resume_req % 8); // Fill to full byte
//                                  int size_rlc_pdu = size_rrc_conn_resume_req;
//                                  //MAC_SUBHEADER_CE__R_F2_E_LCID = 8           # R/F2/E/LCID sub-header for MAC control element (ETSI TS 136 321 V13.9.0 (2018-07) Figure 6.1.2-2)
//                                  //MAC_SUBHEADER_SDU__R_F2_E_LCID_F_7L = 16    # R/F2/E/LCID/F/L sub-headerfor MAC SDU with 7-bits L field (ETSI TS 136 321 V13.9.0 (2018-07) Figure 6.1.2-1)
//                                  //MAC_CE_sBSR = 8                             # Short BSR and Truncated BSR MAC control element (ETSI TS 136 321 V13.9.0 (2018-07) Figure 6.1.3.1-1)
//                                  int size_mac_pdu = 8 + 16 + 8 + size_rlc_pdu;
//                                  int couplingloss;
//                                  if (rar->rapId < 24)
//                                    {
//                                      couplingloss = 159;
//                                    }
//                                  else if (rar->rapId < 36)
//                                    {
//                                      couplingloss = 149;
//                                    }
//                                  else if (rar->rapId < 48)
//                                    {
//                                      couplingloss = 139;
//                                    }
//                                  subframesNpusch = m_Amc.getMsg3Subframes (
//                                      couplingloss, size_mac_pdu, 15000, 15);
//                                  ulgrant = GetNextAvailableMsg3UlGrantCandidate (
//                                      *(npdschsubframes.end () - 1), subframesNpusch);
//                                  if (ulgrant.first.success) // WE GOT AN UPLINK MSG3 CANDIDATE
//                                    {
//                                      scheduleSuccessful = true;
//                                      rar->rarPayload.ulGrant = ulgrant.first;
//                                      rar->rarPayload.ulGrant.subframes = ulgrant.second;
//                                      NS_BUILD_DEBUG (std::cout << "Scheduling NPUSCH at ");
//                                      NS_BUILD_DEBUG (std::cout << " Subcarrier "
//                                                                << ulgrant.second.first << " ");
//                                      for (size_t i = 0; i < ulgrant.second.second.size (); i++)
//                                        {
//                                          m_uplink[ulgrant.second.first][ulgrant.second.second[i]] =
//                                              m_currenthyperindex;
//                                          NS_BUILD_DEBUG (std::cout << ulgrant.second.second[i]
//                                                                    << " ");
//                                        }
//                                      NS_BUILD_DEBUG (std::cout << "\n");
//                                      ++rar;
//                                    }
//                                  else
//                                    {
//                                      it->rars.erase (rar);
//                                      // Stuff if cant schedule npusch
//                                    }
//                                }
//                            }
//                          else
//                            {
//                              // Create HARQ Ressource
//                              int subframesNpuschHarq =
//                                  4; // Have to be set by higher layer | 4 for debugging
//                              std::vector<std::pair<int, std::vector<int>>> npuschharqsubframes =
//                                  GetNextAvailableNpuschCandidate (*(npdschsubframes.end () - 1), 0,
//                                                                   subframesNpuschHarq, true);
//                              if (npuschharqsubframes.size () > 0)
//                                {
//                                  NS_BUILD_DEBUG (std::cout << "Scheduling NPUSCH HARQ at ");
//                                  scheduleSuccessful = true;
//                                  for (size_t i = 0; i < npuschharqsubframes[0].second.size (); i++)
//                                    {
//                                      m_uplink[npuschharqsubframes[0].first]
//                                              [npuschharqsubframes[0].second[i]] =
//                                                  m_currenthyperindex;
//                                      NS_BUILD_DEBUG (std::cout << npuschharqsubframes[0].second[i]
//                                                                << " ");
//                                    }
//                                  NS_BUILD_DEBUG (std::cout << "\n");
//                                  it->dciN1.npuschOpportunity = npuschharqsubframes;
//                                }
//                            }
//                          if (scheduleSuccessful)
//                            {
//                              NS_BUILD_DEBUG (std::cout << "Scheduling NPDCCH at ");
//
//                              for (size_t j = 0; j < test.size (); ++j)
//                                {
//                                  m_downlink[test[j]] = m_currenthyperindex;
//                                  NS_BUILD_DEBUG (std::cout << test[j] << " ");
//                                }
//
//                              NS_BUILD_DEBUG (std::cout << "\n");
//                              NS_BUILD_DEBUG (std::cout << "Scheduling NPDSCH at ");
//
//                              for (size_t j = 0; j < npdschsubframes.size (); ++j)
//                                {
//                                  m_downlink[npdschsubframes[j]] = m_currenthyperindex;
//                                  NS_BUILD_DEBUG (std::cout << npdschsubframes[j] << " ");
//                                }
//
//                              it->dciRepetitionsubframes = test;
//                              it->dciN1.npdschOpportunity = npdschsubframes;
//                              scheduledMessages.push_back (*(it));
//                              NS_BUILD_DEBUG (std::cout << "\n");
//
//                              m_NpdcchQueue.erase (it);
//                              continue;
//                            }
//                          else
//                            {
//                              m_NpdcchQueue.erase (it);
//                              continue;
//                            }
//                        }
//                    }
//                }
//              else if (it->dciType == NbIotRrcSap::NpdcchMessage::DciType::n0)
//                {
//                  std::vector<int> test = GetNextAvailableSearchSpaceCandidate (
//                    m_frameNo - 1, m_subframeNo - 1, R_max,
//                    NbIotRrcSap::ConvertDciN0Repetitions2int (it->dciN0));
//
//                  if (test.size () > 0)
//                    {
//                      int subframesNpusch =
//                          NbIotRrcSap::ConvertNumResourceUnits2int (it->dciN0) *
//                          NbIotRrcSap::ConvertNumNpuschRepetitions2int (it->dciN0);
//                      // Have to be set by higher layer | 4 for debugging
//                      std::vector<std::pair<int, std::vector<int>>> npuschsubframes =
//                          GetNextAvailableNpuschCandidate (*(test.end () - 1), 0, subframesNpusch,
//                                                           true);
//
//                      if (npuschsubframes.size () > 0)
//                        {
//                          NS_BUILD_DEBUG (std::cout << "Scheduling NPUSCH at ");
//                          scheduleSuccessful = true;
//                          for (size_t i = 0; i < npuschsubframes[0].second.size (); i++)
//                            {
//                              m_uplink[npuschsubframes[0].first][npuschsubframes[0].second[i]] =
//                                  m_currenthyperindex;
//                              NS_BUILD_DEBUG (std::cout << npuschsubframes[0].second[i] << " ");
//                            }
//                          NS_BUILD_DEBUG (std::cout << "\n");
//
//                          it->dciRepetitionsubframes = test;
//                          it->dciN0.npuschOpportunity = npuschsubframes;
//                          scheduledMessages.push_back (*(it));
//                          NS_BUILD_DEBUG (std::cout << "\n");
//
//                          m_NpdcchQueue.erase (it);
//                          continue;
//                        }
//                    }
//                }
//            }
//          // TO FIX, HAS TO COUNT UP IN ALL CASSES that dont delete the current element
//        }
//
//      ++it;
//      scheduleSuccessful = false;
//    }
//
//  return scheduledMessages;
//}

//void NbiotScheduler::AddRntiDatatoNpdcchQueue(SearchSpaceConfig seachspace){
//  for(std::map<uint16_t, std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>>::iterator it = m_RntiRlcDlBuffer[seachspace].begin();
//      it != m_RntiRlcDlBuffer[seachspace].end();){
//      std::vector<NbIotRrcSap::NpdcchMessage>::iterator messageIt = m_NpdcchQueue.begin();
//      for(; messageIt != m_NpdcchQueue.end();++messageIt){
//                        if(messageIt->rnti == it->first && messageIt->dciType == NbIotRrcSap::NpdcchMessage::DciType::n1){
//            break;
//          }
//        }
//        NbIotRrcSap::NpdcchMessage dci = CreateDciNpdcchMessage(it->first,seachspace,NbIotRrcSap::NpdcchMessage::DciType::n1);
//        m_RntiRlcDlBuffer[seachspace].erase(it++);
//        if(messageIt != m_NpdcchQueue.end()){
//          *(messageIt) = dci;
//        }
//        else{
//          m_NpdcchQueue.push_back(dci);
//        }
//      }
//  for(std::map<uint16_t, uint64_t>::iterator it = m_RntiRlcUlBuffer[seachspace].begin();
//      it != m_RntiRlcUlBuffer[seachspace].end();){
//        NbIotRrcSap::NpdcchMessage dci = CreateDciNpdcchMessage(it->first,seachspace,NbIotRrcSap::NpdcchMessage::DciType::n0);
//        // Ue can only notify new BSR when a MAC PDU is transmitted, so we only delete the entry when all requested data is scheduled
//        if (m_RntiRlcUlBuffer[seachspace][it->first] == 0){
//        m_RntiRlcUlBuffer[seachspace].erase(it++);
//        }else{
//          it++;
//        }
//        m_NpdcchQueue.push_back(dci);
//      }
//}

void NbiotScheduler::SortBasedOnSelectedSchedulingAlgorithm(std::vector<uint16_t>& rntis){

}

bool NbiotScheduler::ScheduleNpdcchMessage(NbIotRrcSap::NpdcchMessage &message, SearchSpaceConfig ssc){

  bool scheduleSuccessful = false;
  
  if(message.dciType == NbIotRrcSap::NpdcchMessage::DciType::n1){
    std::vector<uint64_t> test = GetNextAvailableSearchSpaceCandidate (message.rnti,
      m_frameNo - 1, m_subframeNo - 1, ssc.R_max,
      NbIotRrcSap::ConvertDciN1Repetitions2int (message.dciN1));
    if (test.size () > 0) // WE GOT A DOWNLINK NPDCCH CANDIDATE
      {

        uint64_t subframesNpdsch =
            NbIotRrcSap::ConvertNumNpdschSubframesPerRepetition2int (message.dciN1) *
            NbIotRrcSap::ConvertNumNpdschRepetitions2int (message.dciN1);
        std::vector<uint64_t> npdschsubframes = GetNextAvailableNpdschCandidate (
            *(test.end () - 1), m_minSchedulingDelayDci2Downlink, subframesNpdsch,
            ssc.R_max);
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
                    if(message.isEdt){
                      if(message.ce == NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::zero){
                        size_mac_pdu = NbIotRrcSap::ConvertEdtTbs2int(m_sib2config.radioResourceConfigCommon.nprachConfigR15.edtTbsInfoList.edtTbsNb0);
                      }
                      else if(message.ce == NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::one){
                        size_mac_pdu = NbIotRrcSap::ConvertEdtTbs2int(m_sib2config.radioResourceConfigCommon.nprachConfigR15.edtTbsInfoList.edtTbsNb1);
                      }
                      else if(message.ce == NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::two){
                        size_mac_pdu = NbIotRrcSap::ConvertEdtTbs2int(m_sib2config.radioResourceConfigCommon.nprachConfigR15.edtTbsInfoList.edtTbsNb2);
                      }
                    }else{
                      size_mac_pdu = 88; // Fixed Ulgrant Size. See 136.331
                    }
                    uint64_t couplingloss = 0;
                    if (rar->ceLevel == NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::two)
                      {
                        couplingloss = 159;
                      }
                    else if (rar->ceLevel == NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::one)
                      {
                        couplingloss = 149;
                      }
                    else if (rar->ceLevel == NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::zero)
                      {
                        couplingloss = 139;
                      }
                    subframesNpusch = m_Amc.getMsg3Subframes (
                        couplingloss, size_mac_pdu, 15000, 15);
                    ulgrant = GetNextAvailableMsg3UlGrantCandidate (
                        npdschsubframes.back(), subframesNpusch);
                    if (ulgrant.first.success) // WE GOT AN UPLINK MSG3 CANDIDATE
                      {
                        scheduleSuccessful = true;
                        rar->rarPayload.ulGrant = ulgrant.first;
                        rar->rarPayload.ulGrant.subframes = ulgrant.second;
                        rar->rarPayload.ulGrant.tbs_size = size_mac_pdu;
                        NS_BUILD_DEBUG (std::cout << "Scheduling NPUSCH at ");
                        NS_BUILD_DEBUG (std::cout << " Subcarrier "
                                                  << ulgrant.second.first << " ");
                        for (size_t i = 0; i < ulgrant.second.second.size (); i++)
                          {
                            m_uplink[ulgrant.second.first][ulgrant.second.second[i]] =
                                m_currenthyperindex;
                            NS_BUILD_DEBUG (std::cout << ulgrant.second.second[i]
                                                      << " ");
                          }
                        NS_BUILD_DEBUG (std::cout << std::endl);
                        ++rar;
                        m_rntiUeConfigMap[rar->cellRnti].lastUl = ulgrant.second.second.back();
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
                    4; // Have to be set by higher layer | 4 for debugging
                std::vector<std::pair<uint64_t, std::vector<uint64_t>>> npuschharqsubframes =
                    GetNextAvailableNpuschCandidate (*(npdschsubframes.end () - 1), 0,
                                                      subframesNpuschHarq, true);
                if (npuschharqsubframes.size () > 0)
                  {
                    NS_BUILD_DEBUG (std::cout << "Scheduling NPUSCH HARQ at ");
                    scheduleSuccessful = true;
                    for (size_t i = 0; i < npuschharqsubframes[0].second.size (); i++)
                      {
                        m_uplink[npuschharqsubframes[0].first]
                                [npuschharqsubframes[0].second[i]] =
                                    m_currenthyperindex;
                        NS_BUILD_DEBUG (std::cout << npuschharqsubframes[0].second[i]
                                                  << " ");
                      }
                    NS_BUILD_DEBUG (std::cout << std::endl);
                    message.dciN1.npuschOpportunity = npuschharqsubframes;

                    m_rntiUeConfigMap[message.rnti].lastUl = npuschharqsubframes[0].second.back();
                  }
              }
            if (scheduleSuccessful)
              {
                NS_BUILD_DEBUG (std::cout << "Scheduling NPDCCH at ");

                for (size_t j = 0; j < test.size (); ++j)
                  {
                    m_downlink[test[j]] = m_currenthyperindex;
                    NS_BUILD_DEBUG (std::cout << test[j] << " ");
                  }

                NS_BUILD_DEBUG (std::cout << std::endl);
                NS_BUILD_DEBUG (std::cout << "Scheduling NPDSCH at ");

                for (size_t j = 0; j < npdschsubframes.size (); ++j)
                  {
                    m_downlink[npdschsubframes[j]] = m_currenthyperindex;
                    NS_BUILD_DEBUG (std::cout << npdschsubframes[j] << " ");
                  }

                message.dciRepetitionsubframes = test;
                message.dciN1.npdschOpportunity = npdschsubframes;
                message.dciN1.dciSubframes = test;
                NS_BUILD_DEBUG (std::cout << std::endl);
                return true;

              }

          }
    }
  }
  else if (message.dciType == NbIotRrcSap::NpdcchMessage::DciType::n0)
                {
                  std::vector<uint64_t> test = GetNextAvailableSearchSpaceCandidate (message.rnti,
                    m_frameNo - 1, m_subframeNo - 1, ssc.R_max,
                    NbIotRrcSap::ConvertDciN0Repetitions2int (message.dciN0));
                  if (test.size () > 0)
                    {
                      uint64_t subframesNpusch =
                          NbIotRrcSap::ConvertNumResourceUnits2int (message.dciN0) *
                          NbIotRrcSap::ConvertNumNpuschRepetitions2int (message.dciN0);
                      // Have to be set by higher layer | 4 for debugging
                      std::vector<std::pair<uint64_t, std::vector<uint64_t>>> npuschsubframes =
                          GetNextAvailableNpuschCandidate (*(test.end () - 1), 0, subframesNpusch,
                                                           true);

                      if (npuschsubframes.size () > 0)
                        {
                          NS_BUILD_DEBUG (std::cout << "Scheduling NPUSCH at ");
                          scheduleSuccessful = true;
                          for (size_t i = 0; i < npuschsubframes[0].second.size (); i++)
                            {
                              m_uplink[npuschsubframes[0].first][npuschsubframes[0].second[i]] =
                                  m_currenthyperindex;
                              NS_BUILD_DEBUG (std::cout << npuschsubframes[0].second[i] << " ");
                            }
                          NS_BUILD_DEBUG (std::cout << std::endl);

                          message.dciRepetitionsubframes = test;
                          message.dciN0.npuschOpportunity = npuschsubframes;
                          message.dciN0.dciSubframes = test;
                          m_rntiUeConfigMap[message.rnti].lastUl = npuschsubframes[0].second.back();
                          NS_BUILD_DEBUG (std::cout << std::endl);
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
  for(std::vector<NbIotRrcSap::NpdcchMessage>::iterator rar = m_rarQueue[ssc].begin(); rar != m_rarQueue[ssc].end();){
    if(ScheduleNpdcchMessage((*rar),ssc)){
      scheduledMessages.push_back(*rar);
      m_rarQueue[ssc].erase(rar);
    }
    else{
      ++rar;
    }
  }
  /*
  Scheduling Magic. For now FIFO
  */
  SortBasedOnSelectedSchedulingAlgorithm(m_searchSpaceRntiMap[ssc]);
  std::vector<uint16_t > test_tmp = m_searchSpaceRntiMap[ssc];
  for(std::vector<uint16_t>::iterator it = m_searchSpaceRntiMap[ssc].begin(); it != m_searchSpaceRntiMap[ssc].end(); ){
    NbIotRrcSap::NpdcchMessage dci_candidate;
    if(m_rntiUeConfigMap[(*it)].rlcDlBuffer > 0){

      dci_candidate= CreateDciNpdcchMessage((*it), NbIotRrcSap::NpdcchMessage::DciType::n1); 
      if(ScheduleNpdcchMessage(dci_candidate,ssc)){
        scheduledMessages.push_back(dci_candidate);
        m_rntiUeConfigMap[(*it)].rlcDlBuffer = 0;
      }
      
      
    }else if(m_rntiUeConfigMap[(*it)].rlcUlBuffer > 0){
      dci_candidate = CreateDciNpdcchMessage((*it), NbIotRrcSap::NpdcchMessage::DciType::n0); 
      if(ScheduleNpdcchMessage(dci_candidate,ssc)){
        scheduledMessages.push_back(dci_candidate);
        if(int(m_rntiUeConfigMap[(*it)].rlcUlBuffer - (dci_candidate.tbs/8)) > 0){
          m_rntiUeConfigMap[(*it)].rlcUlBuffer = m_rntiUeConfigMap[(*it)].rlcUlBuffer - (dci_candidate.tbs/8);
        }else{
          m_rntiUeConfigMap[(*it)].rlcUlBuffer =0;
        }
      }
    }
        ++it;
  }


//  for (std::vector<NbIotRrcSap::NpdcchMessage>::iterator it = m_NpdcchQueue.begin ();
//       it != m_NpdcchQueue.end ();)
//    {
//      if (it->searchSpaceType == seachspace)
//        {
//          if (it->ce == ssc.ce)
//            {
//              if (it->dciType == NbIotRrcSap::NpdcchMessage::DciType::n1)
//                {
//                  std::vector<uint64_t> test = GetNextAvailableSearchSpaceCandidate (it->rnti,
//                    m_frameNo - 1, m_subframeNo - 1, ssc.R_max,
//                    NbIotRrcSap::ConvertDciN1Repetitions2int (it->dciN1));
//                  if (test.size () > 0) // WE GOT A DOWNLINK NPDCCH CANDIDATE
//                    {
//
//                      uint64_t subframesNpdsch =
//                          NbIotRrcSap::ConvertNumNpdschSubframesPerRepetition2int (it->dciN1) *
//                          NbIotRrcSap::ConvertNumNpdschRepetitions2int (it->dciN1);
//                      std::vector<uint64_t> npdschsubframes = GetNextAvailableNpdschCandidate (
//                          *(test.end () - 1), m_minSchedulingDelayDci2Downlink, subframesNpdsch,
//                          ssc.R_max);
//                      if (npdschsubframes.size () > 0) // WE GOT A DOWNLINK CANDIDATE
//                        {
//                          uint64_t subframesNpusch;
//                          std::pair<NbIotRrcSap::UlGrant, std::pair<uint8_t, std::vector<uint64_t>>> ulgrant;
//                          if (it->isRar)
//                            {
//                              for (std::vector<NbIotRrcSap::Rar>::iterator rar = it->rars.begin ();
//                                   rar != it->rars.end ();)
//                                {
//                                  // MSG3 SIZE
//                                  // See Joerke NBIoT_UE.py
//                                  /*
//                                  The MAC PDU consists of the following (ref. ETSI TS 136 321 V13.9.0 Fig. 6.1.6-4):
//                                  |-----------------------------------------MAC-Header-----------------------------------------|--MAC-CE--|...|--MAC-SDU--|--Padding(opt)--|
//                                  |--R/F2/E/LCID subheader--|...|--R/F2/E/LCID/F/K subheader--|--R/R/E/LCID padding subheader--|
//                                  In Msg3 the UE should transmit a Buffer Status Report MAC CE in order to inform the eNodeB about its UL buffer status
//                                  Since RRCConnectionResumeComplete-NB is transmitted in RLC TM, TMD PDU consists only of a data field and doesn't consists of any RLC headers (Ref. ETSI TS 136 331 V13.15.0 p.522 "RRCConnectionResumeRequest-NB" and ETSI TS 136 322 V13.4.0, 6.2.1.2)
//                                  RRCConnectionResumeRequest-NB ist transmitted using SRB0 (ref. ETSI TS 136 331 V13.15.0 p.522 "RRCConnectionResumeRequest-NB" and therefore does not go through PDCP (ref. ETSI TS 136 323 V13.6.0, 4.2.1: "Each RB (i.e. DRB, SLRB and SRB, except for SRB0 and SRB1bis) is associated with one PDCP entity"))
//                                  */
//
//                                  //uint64_t size_rrc_conn_resume_req =
//                                  //    59; // 40 bits resumeID +16 bits shortResumeMAC-I +3 bits resumeCause ref. ETSI TS 136 331 V13.15.0 p.522 RRCConnectionResumeRequest-NB
//                                  //size_rrc_conn_resume_req +=
//                                  //    (8 - size_rrc_conn_resume_req % 8); // Fill to full byte
//                                  //uint64_t size_rlc_pdu = size_rrc_conn_resume_req;
//                                  ////MAC_SUBHEADER_CE__R_F2_E_LCID = 8           # R/F2/E/LCID sub-header for MAC control element (ETSI TS 136 321 V13.9.0 (2018-07) Figure 6.1.2-2)
//                                  ////MAC_SUBHEADER_SDU__R_F2_E_LCID_F_7L = 16    # R/F2/E/LCID/F/L sub-headerfor MAC SDU with 7-bits L field (ETSI TS 136 321 V13.9.0 (2018-07) Figure 6.1.2-1)
//                                  ////MAC_CE_sBSR = 8                             # Short BSR and Truncated BSR MAC control element (ETSI TS 136 321 V13.9.0 (2018-07) Figure 6.1.3.1-1)
//                                  //uint64_t size_mac_pdu = 8 + 16 + 8 + size_rlc_pdu;
//                                  uint64_t size_mac_pdu = 0;
//                                  if(it->isEdt){
//                                    if(it->ce == NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::zero){
//                                      size_mac_pdu = NbIotRrcSap::ConvertEdtTbs2int(m_sib2config.radioResourceConfigCommon.nprachConfigR15.edtTbsInfoList.edtTbsNb0);
//                                    }
//                                    else if(it->ce == NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::one){
//                                      size_mac_pdu = NbIotRrcSap::ConvertEdtTbs2int(m_sib2config.radioResourceConfigCommon.nprachConfigR15.edtTbsInfoList.edtTbsNb1);
//                                    }
//                                    else if(it->ce == NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::two){
//                                      size_mac_pdu = NbIotRrcSap::ConvertEdtTbs2int(m_sib2config.radioResourceConfigCommon.nprachConfigR15.edtTbsInfoList.edtTbsNb2);
//                                    }
//                                  }else{
//                                    size_mac_pdu = 88; // Fixed Ulgrant Size. See 136.331
//                                  }
//                                  uint64_t couplingloss = 0;
//                                  if (rar->ceLevel == NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::two)
//                                    {
//                                      couplingloss = 159;
//                                    }
//                                  else if (rar->ceLevel == NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::one)
//                                    {
//                                      couplingloss = 149;
//                                    }
//                                  else if (rar->ceLevel == NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::zero)
//                                    {
//                                      couplingloss = 139;
//                                    }
//                                  subframesNpusch = m_Amc.getMsg3Subframes (
//                                      couplingloss, size_mac_pdu, 15000, 15);
//                                  ulgrant = GetNextAvailableMsg3UlGrantCandidate (
//                                      npdschsubframes.back(), subframesNpusch);
//                                  if (ulgrant.first.success) // WE GOT AN UPLINK MSG3 CANDIDATE
//                                    {
//                                      scheduleSuccessful = true;
//                                      rar->rarPayload.ulGrant = ulgrant.first;
//                                      rar->rarPayload.ulGrant.subframes = ulgrant.second;
//                                      rar->rarPayload.ulGrant.tbs_size = size_mac_pdu;
//                                      NS_BUILD_DEBUG (std::cout << "Scheduling NPUSCH at ");
//                                      NS_BUILD_DEBUG (std::cout << " Subcarrier "
//                                                                << ulgrant.second.first << " ");
//                                      for (size_t i = 0; i < ulgrant.second.second.size (); i++)
//                                        {
//                                          m_uplink[ulgrant.second.first][ulgrant.second.second[i]] =
//                                              m_currenthyperindex;
//                                          NS_BUILD_DEBUG (std::cout << ulgrant.second.second[i]
//                                                                    << " ");
//                                        }
//                                      NS_BUILD_DEBUG (std::cout << std::endl);
//                                      ++rar;
//                                      m_lastUlSubframe[rar->cellRnti] = ulgrant.second.second.back();
//                                    }
//                                  else
//                                    {
//                                      it->rars.erase (rar);
//                                      // Stuff if cant schedule npusch
//                                    }
//                                }
//                            }
//                          else
//                            {
//                              // Create HARQ Ressource
//                              uint64_t subframesNpuschHarq =
//                                  4; // Have to be set by higher layer | 4 for debugging
//                              std::vector<std::pair<uint64_t, std::vector<uint64_t>>> npuschharqsubframes =
//                                  GetNextAvailableNpuschCandidate (*(npdschsubframes.end () - 1), 0,
//                                                                   subframesNpuschHarq, true);
//                              if (npuschharqsubframes.size () > 0)
//                                {
//                                  NS_BUILD_DEBUG (std::cout << "Scheduling NPUSCH HARQ at ");
//                                  scheduleSuccessful = true;
//                                  for (size_t i = 0; i < npuschharqsubframes[0].second.size (); i++)
//                                    {
//                                      m_uplink[npuschharqsubframes[0].first]
//                                              [npuschharqsubframes[0].second[i]] =
//                                                  m_currenthyperindex;
//                                      NS_BUILD_DEBUG (std::cout << npuschharqsubframes[0].second[i]
//                                                                << " ");
//                                    }
//                                  NS_BUILD_DEBUG (std::cout << std::endl);
//                                  it->dciN1.npuschOpportunity = npuschharqsubframes;
//
//                                  m_lastUlSubframe[it->rnti] = npuschharqsubframes[0].second.back();
//                                }
//                            }
//                          if (scheduleSuccessful)
//                            {
//                              NS_BUILD_DEBUG (std::cout << "Scheduling NPDCCH at ");
//
//                              for (size_t j = 0; j < test.size (); ++j)
//                                {
//                                  m_downlink[test[j]] = m_currenthyperindex;
//                                  NS_BUILD_DEBUG (std::cout << test[j] << " ");
//                                }
//
//                              NS_BUILD_DEBUG (std::cout << std::endl);
//                              NS_BUILD_DEBUG (std::cout << "Scheduling NPDSCH at ");
//
//                              for (size_t j = 0; j < npdschsubframes.size (); ++j)
//                                {
//                                  m_downlink[npdschsubframes[j]] = m_currenthyperindex;
//                                  NS_BUILD_DEBUG (std::cout << npdschsubframes[j] << " ");
//                                }
//
//                              it->dciRepetitionsubframes = test;
//                              it->dciN1.npdschOpportunity = npdschsubframes;
//                              it->dciN1.dciSubframes = test;
//                              scheduledMessages.push_back (*(it));
//                              NS_BUILD_DEBUG (std::cout << std::endl);
//
//                              m_NpdcchQueue.erase (it);
//                              continue;
//                            }
//                          else
//                            {
//
//                            }
//                        }
//                    }
//                }
//              else if (it->dciType == NbIotRrcSap::NpdcchMessage::DciType::n0)
//                {
//                  std::vector<uint64_t> test = GetNextAvailableSearchSpaceCandidate (it->rnti,
//                    m_frameNo - 1, m_subframeNo - 1, ssc.R_max,
//                    NbIotRrcSap::ConvertDciN0Repetitions2int (it->dciN0));
//                  if (test.size () > 0)
//                    {
//                      uint64_t subframesNpusch =
//                          NbIotRrcSap::ConvertNumResourceUnits2int (it->dciN0) *
//                          NbIotRrcSap::ConvertNumNpuschRepetitions2int (it->dciN0);
//                      // Have to be set by higher layer | 4 for debugging
//                      std::vector<std::pair<uint64_t, std::vector<uint64_t>>> npuschsubframes =
//                          GetNextAvailableNpuschCandidate (*(test.end () - 1), 0, subframesNpusch,
//                                                           true);
//
//                      if (npuschsubframes.size () > 0)
//                        {
//                          NS_BUILD_DEBUG (std::cout << "Scheduling NPUSCH at ");
//                          scheduleSuccessful = true;
//                          for (size_t i = 0; i < npuschsubframes[0].second.size (); i++)
//                            {
//                              m_uplink[npuschsubframes[0].first][npuschsubframes[0].second[i]] =
//                                  m_currenthyperindex;
//                              NS_BUILD_DEBUG (std::cout << npuschsubframes[0].second[i] << " ");
//                            }
//                          NS_BUILD_DEBUG (std::cout << std::endl);
//
//                          it->dciRepetitionsubframes = test;
//                          it->dciN0.npuschOpportunity = npuschsubframes;
//                          it->dciN0.dciSubframes = test;
//                          m_lastUlSubframe[it->rnti] = npuschsubframes[0].second.back();
//                          scheduledMessages.push_back (*(it));
//                          NS_BUILD_DEBUG (std::cout << std::endl);
//
//                          m_NpdcchQueue.erase (it);
//                          continue;
//                        }
//                    }
//                }
//            }
//          // TO FIX, HAS TO COUNT UP IN ALL CASSES that dont delete the current element
//        }
//
//      ++it;
//      scheduleSuccessful = false;
//    }
//   */
  return scheduledMessages;
}

std::vector<std::pair<uint64_t, std::vector<uint64_t>>>
NbiotScheduler::GetNextAvailableNpuschCandidate (uint64_t endSubframeNpdsch, uint64_t minSchedulingDelay,
                                                 uint64_t numSubframes, bool isHarq)
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
                  CheckforNContiniousSubframesUl (subframesOccupied, candidate, numSubframes, j);
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
          for (size_t j = 0; j < 4; ++j)
            { // For subcarrier 0-3 for 15khz Subcarrier spacing | needs change for 3.75 Khz
              NbIotRrcSap::DciN0 tmp;
              tmp.npuschSchedulingDelay = i;
              uint64_t candidate =
                  endSubframeNpdsch + NbIotRrcSap::ConvertNpuschSchedulingDelay2int (tmp)+1; // Start on next frame aber minSchedulingDelay
              std::vector<uint64_t> subframesOccupied =
                  GetUlSubframeRangeWithoutSystemResources (candidate, numSubframes, j);
              subframesOccupied =
                  CheckforNContiniousSubframesUl (subframesOccupied, candidate, numSubframes, j);
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
NbiotScheduler::GetNextAvailableMsg3UlGrantCandidate (uint64_t endSubframeMsg2, uint64_t numSubframes)
{
  for (auto &i : m_Msg3TimeOffset)
    {
      for (size_t j = 0; j < m_uplink.size (); ++j)
        {
          uint64_t candidate =
              endSubframeMsg2 + NbIotRrcSap::UlGrant::ConvertUlGrantSchedulingDelay2int (i)+1; // Start one subframe after delay
          std::vector<uint64_t> subframesOccupied =
              GetUlSubframeRangeWithoutSystemResources (candidate, numSubframes, j);
          subframesOccupied =
              CheckforNContiniousSubframesUl (subframesOccupied, candidate, numSubframes, j);
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
NbiotScheduler::GetNextAvailableNpdschCandidate (uint64_t endSubframeDci, uint64_t minSchedulingDelay,
                                                 uint64_t numSubframes, uint64_t R_max)
{

  uint64_t npdschCandidate = endSubframeDci + minSchedulingDelay + 1; // Start on the next Subframe of afer minSchedulingDelay
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
              CheckforNContiniousSubframesDl (subframesOccupied, tmpCandidate, numSubframes);
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
              CheckforNContiniousSubframesDl (subframesOccupied, tmpCandidate, numSubframes);
          if (subframesOccupied.size () > 0)
            {
              return subframesOccupied;
            }
        }
    }
  return std::vector<uint64_t> ();
}

std::vector<uint64_t>
NbiotScheduler::GetDlSubframeRangeWithoutSystemResources (uint64_t overallSubframeNo, uint64_t numSubframes)
{
  std::vector<uint64_t> subframeIndexes;
  size_t i = 0; // Starting on the given Subframe
  m_currenthyperindex = 1;
  while (numSubframes > 0)
    {
      size_t currentindex = overallSubframeNo + i;
      if ((m_downlink[currentindex] != -1))
        {
          subframeIndexes.push_back (currentindex);
          numSubframes--;
        }
      i++;
    }
  return subframeIndexes;
}

std::vector<uint64_t>
NbiotScheduler::GetUlSubframeRangeWithoutSystemResources (uint64_t overallSubframeNo, uint64_t numSubframes,
                                                          uint64_t carrier)
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
NbiotScheduler::CheckforNContiniousSubframesDl (std::vector<uint64_t> Subframes, uint64_t StartSubframe,
                                                uint64_t N)
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
      if (m_downlink[Subframes[startSubframeIndex + i]] == m_currenthyperindex)
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
NbiotScheduler::CheckforNContiniousSubframesUl (std::vector<uint64_t> Subframes, uint64_t StartSubframe,
                                                uint64_t N, uint64_t carrier)
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
      if (m_uplink[carrier][Subframes[startSubframeIndex + i]] == m_currenthyperindex)
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
NbiotScheduler::GetNextAvailableSearchSpaceCandidate (uint32_t rnti,uint64_t SearchSpaceStartFrame,
                                                      uint64_t SearchSpaceStartSubframe, uint64_t R_max,
                                                      uint64_t R)
{
  uint64_t u_max = ((R_max / R) - 1);
  uint64_t overallSubframe = 10 * (SearchSpaceStartFrame) + SearchSpaceStartSubframe;
  std::vector<uint64_t> subframes = GetDlSubframeRangeWithoutSystemResources (overallSubframe, R_max);
  for (size_t i = 0; i <= u_max; ++i)
    {
      // Calculate start of dci candidate
      std::vector<uint64_t> subframes_to_use =
          CheckforNContiniousSubframesDl (subframes, subframes[i * R], R);

      if (subframes_to_use.size () > 0)
        {
          if(subframes_to_use.front() > m_rntiUeConfigMap[rnti].lastUl+8){
            return subframes_to_use;
          }
        }
    }
  return std::vector<uint64_t> ();
}



NbIotRrcSap::NpdcchMessage NbiotScheduler::CreateDciNpdcchMessage(uint16_t rnti, NbIotRrcSap::NpdcchMessage::DciType dci_type){
  double correction_factor =
      10 *
      log10 (
          1.0 /
          12.0); // correctionfactor applied to rsrp because it's for earch subcarrier and tx power is for full spectrum

  NS_BUILD_DEBUG (std::cout << "MCL of " << rnti << " is "
                            << m_rntiRsrpMap[rnti] - 43.0 - correction_factor << std::endl);

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
  else{
      dciN1Repetitions = NbIotRrcSap::DciN1::DciRepetitions::r2;
      dciN0Repetitions = NbIotRrcSap::DciN0::DciRepetitions::r2;
      ceLevel = m_ce0.coverageEnhancementLevel;
  }
  
  NbIotRrcSap::NpdcchMessage msg;
  msg.isRar = false;
  msg.rnti = rnti;
  msg.ce = ceLevel;

  if(dci_type == NbIotRrcSap::NpdcchMessage::DciType::n1){
    uint64_t tbs = 0;

      if(m_rntiUeConfigMap[rnti].rlcDlBuffer*8 > 680){ // max TBS Downlink Rel. 13
        tbs = 680;
      }
      else{
        tbs = (m_rntiUeConfigMap[rnti].rlcDlBuffer)* 8;
      }
      std::pair<NbIotRrcSap::DciN1, uint64_t> dci_tbs = m_Amc.getBareboneDciN1 (
        m_rntiRsrpMap[rnti] - 43.0 - correction_factor, tbs, "inband");

      NbIotRrcSap::DciN1 dci = dci_tbs.first;

      dci.mCS = NbIotRrcSap::DciN1::MCS::one;
      dci.tbs = dci_tbs.second;
      dci.NDI = true;
      dci.dciRepetitions = dciN1Repetitions;

      msg.dciType = NbIotRrcSap::NpdcchMessage::DciType::n1;
      msg.dciN1 = dci;
      msg.tbs = dci_tbs.second;

  }
  else if (dci_type == NbIotRrcSap::NpdcchMessage::DciType::n0){
    uint64_t tbs = 0;

    if(m_rntiUeConfigMap[rnti].rlcUlBuffer*8 > 1000){ // max TBS Uplink Rel. 13
      tbs = 1000;

    }
    else{
      tbs = (10+m_rntiUeConfigMap[rnti].rlcUlBuffer)* 8;
    }

    std::pair<NbIotRrcSap::DciN0, uint64_t> dci_tbs = m_Amc.getBareboneDciN0 (
      m_rntiRsrpMap[rnti] - 43.0 - correction_factor, tbs, 15000, 15);

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

//void
//NbiotScheduler::ScheduleMsg5Req (uint64_t rnti)
//{
//  m_RntiRlcUlBuffer[NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2][rnti] = 14;
//}
void
NbiotScheduler::ScheduleDlRlcBufferReq (uint64_t rnti, std::map<uint8_t,LteMacSapProvider::ReportBufferStatusParameters> lcids)
{
  // Calculate RLC Buffer Size
  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it; 
  uint64_t buffer_size = 0;
  std::map<uint8_t, uint64_t> buffers;
    for(it= lcids.begin(); it != lcids.end(); ++it){
      if(it->second.txQueueSize != 0 || it->second.retxQueueSize != 0 || it->second.statusPduSize != 0){
        buffer_size +=  it->second.txQueueSize + it->second.retxQueueSize+it->second.statusPduSize;
        if(it->second.txQueueSize > 0){
          buffer_size += 4; // RLC Header
        }
    }
  }
  m_lastDlBuffer.push_back(std::pair<uint16_t, std::map<uint8_t,LteMacSapProvider::ReportBufferStatusParameters>>(rnti,lcids));

  m_rntiUeConfigMap[rnti].rlcDlBuffer = buffer_size;
  SearchSpaceConfig searchSpace= m_rntiUeConfigMap[rnti].searchSpaceConfig;
  std::vector<uint16_t>::iterator findit = std::find(m_searchSpaceRntiMap[searchSpace].begin(), m_searchSpaceRntiMap[searchSpace].end(), rnti);
  if(findit == m_searchSpaceRntiMap[searchSpace].end()){
    m_searchSpaceRntiMap[searchSpace].push_back(rnti);
  }
}
void
NbiotScheduler::ScheduleUlRlcBufferReq(uint64_t rnti, uint64_t dataSize)
{
  m_rntiUeConfigMap[rnti].rlcUlBuffer = dataSize;
  m_lastUlBuffer.push_back(std::pair<uint16_t, uint64_t>(rnti,dataSize));
  SearchSpaceConfig searchSpace= m_rntiUeConfigMap[rnti].searchSpaceConfig;
  std::vector<uint16_t>::iterator it = std::find(m_searchSpaceRntiMap[searchSpace].begin(), m_searchSpaceRntiMap[searchSpace].end(), rnti);
  if( it == m_searchSpaceRntiMap[searchSpace].end()){
    m_searchSpaceRntiMap[searchSpace].push_back(rnti);
  }
}

void
NbiotScheduler::AddToUlBufferReq(uint64_t rnti, uint64_t dataSize)
{
  m_rntiUeConfigMap[rnti].rlcUlBuffer += dataSize;
  m_lastUlBuffer.push_back(std::pair<uint16_t, uint64_t>(rnti,dataSize));
  SearchSpaceConfig searchSpace= m_rntiUeConfigMap[rnti].searchSpaceConfig;
  std::vector<uint16_t>::iterator it = std::find(m_searchSpaceRntiMap[searchSpace].begin(), m_searchSpaceRntiMap[searchSpace].end(), rnti);
  if( it == m_searchSpaceRntiMap[searchSpace].end()){
    m_searchSpaceRntiMap[searchSpace].push_back(rnti);
  }
}

SearchSpaceConfig NbiotScheduler::ConvertNpdcchConfigDedicatedNb2SearchSpaceConfig(NbIotRrcSap::NpdcchConfigDedicatedNb configDedicated){
  SearchSpaceConfig ssc;
  ssc.R_max = NbIotRrcSap::ConvertNpdcchNumRepetitions2int (configDedicated);
  ssc.startSf = NbIotRrcSap::ConvertNpdcchStartSfUss2double (configDedicated);
  ssc.offset =NbIotRrcSap::ConvertNpdcchOffsetUss2double (configDedicated); 
  // CE Level not used in USS, but for simplicity set to ce0
  ssc.ce = NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel::none;
  return ssc;
}
SearchSpaceConfig NbiotScheduler::ConvertNprachParametersNb2SearchSpaceConfig(NbIotRrcSap::NprachParametersNb ce){
  SearchSpaceConfig ssc;
  ssc.R_max = NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int (ce);
  ssc.startSf = NbIotRrcSap::ConvertNpdcchStartSfCssRa2double (ce);
  ssc.offset =NbIotRrcSap::ConvertNpdcchOffsetRa2double (ce); 
  ssc.ce = ce.coverageEnhancementLevel;
  return ssc;
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
