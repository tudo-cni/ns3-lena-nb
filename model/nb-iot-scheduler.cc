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
  m_DciTimeOffsetUplink.insert (
      m_DciTimeOffsetUplink.begin (),
      {NbIotRrcSap::DciN0::NpuschSchedulingDelay::ms8, NbIotRrcSap::DciN0::NpuschSchedulingDelay::ms16,
       NbIotRrcSap::DciN0::NpuschSchedulingDelay::ms32, NbIotRrcSap::DciN0::NpuschSchedulingDelay::ms64});

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

  int numHyperframes = 1024;
  int numFrames = 1024;
  int numSubframes = 10;
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
      m_uplink.resize (12, std::vector<int8_t> ());
      for (size_t i = 0; i < m_uplink.size (); ++i)
        {
          m_uplink[i].resize (numHyperframes * numFrames * numSubframes, 0);
        }
      for (std::vector<NbIotRrcSap::NprachParametersNb>::iterator it = ces.begin ();
           it != ces.end (); ++it)
        {
          int sendingTime = NbIotRrcSap::ConvertNprachStartTime2int (*it);
          double ts = 1000.0 / (15000.0 * 2048.0);
          double preambleSymbolTime = 8192.0 * ts;
          double preambleGroupTimeNoCP = 5.0 * preambleSymbolTime;
          double preambleGroupTime = 0.266 + preambleGroupTimeNoCP;
          double preambleRepetition = 4.0 * preambleGroupTime;
          double nprachduration =
              (NbIotRrcSap::ConvertNumRepetitionsPerPreambleAttempt2int (*it) * preambleRepetition);
          size_t subcarrierOffset = NbIotRrcSap::ConvertNprachSubcarrierOffset2int (*it);
          uint8_t numberSubcarriers = NbIotRrcSap::ConvertNprachNumSubcarriers2int (*it);
          double time_tmp = int (nprachduration) + 1;

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
bool
NbiotScheduler::IsSeachSpaceType2Begin (NbIotRrcSap::NprachParametersNb ce)
{
  uint32_t searchSpacePeriodicity = NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int (ce) *
                                    NbIotRrcSap::ConvertNpdcchStartSfCssRa2double (ce);
  uint32_t searchSpaceConditionLeftSide =
      (10 * (m_frameNo - 1) + (m_subframeNo - 1)) % searchSpacePeriodicity;
  uint32_t searchSpaceConditionRightSide =
      NbIotRrcSap::ConvertNpdcchOffsetRa2double (ce) * searchSpacePeriodicity;
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
NbiotScheduler::ScheduleRarReq (int rnti, int rapid, NbIotRrcSap::NprachParametersNb ce,
                                NbIotRrcSap::DciN1::DciRepetitions rep)
{
  NbIotRrcSap::NpdcchMessage rar;
  //  rar.npdcchFormat = NpdcchMessage::NpdcchFormat::format1;
  //  rar.dciType = NpdcchMessage::DciType::n1;
  //  rar.searchSpaceType = NpdcchMessage::SearchSpaceType::type2;
  //  // Dci set depending on coverage level.... yet static
  //  rar.dciN1.dciRepetitions = rep;
  //  rar.dciN1.m_rapId = rapid;
  //  rar.dciN1.numNpdschSubframesPerRepetition = NbIotRrcSap::DciN1::NumNpdschSubframesPerRepetition::s2;
  //  rar.dciN1.numNpdschRepetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r2;
  //  rar.ue.ce = ce;
  //  rar.ue.ranti = rnti;
  m_NpdcchQueue.push_back (rar);
}

void
NbiotScheduler::ScheduleNpdcchMessageReq (NbIotRrcSap::NpdcchMessage msg)
{
  // NPDCCH Parameters taken from Liberg, Olof, et al. The Cellular Internet of Things 2017 p.305, In-Band-Deployment Table 8.9
  if (msg.ce.nprachSubcarrierOffset == m_ce0.nprachSubcarrierOffset)
    {
      msg.dciN0.dciRepetitions = NbIotRrcSap::DciN0::DciRepetitions::r2;
      msg.dciN1.dciRepetitions = NbIotRrcSap::DciN1::DciRepetitions::r2;
    }
  else if (msg.ce.nprachSubcarrierOffset == m_ce1.nprachSubcarrierOffset)
    {
      msg.dciN0.dciRepetitions = NbIotRrcSap::DciN0::DciRepetitions::r32;
      msg.dciN1.dciRepetitions = NbIotRrcSap::DciN1::DciRepetitions::r32;
    }
  else if (msg.ce.nprachSubcarrierOffset == m_ce2.nprachSubcarrierOffset)
    {
      msg.dciN0.dciRepetitions = NbIotRrcSap::DciN0::DciRepetitions::r256;
      msg.dciN1.dciRepetitions = NbIotRrcSap::DciN1::DciRepetitions::r256;
    }
  m_NpdcchQueue.push_back (msg);
}

std::vector<NbIotRrcSap::NpdcchMessage>
NbiotScheduler::Schedule (int frameNo, int subframeNo)
{
  m_frameNo = frameNo;
  m_subframeNo = subframeNo;
  std::vector<NbIotRrcSap::NpdcchMessage> ret = std::vector<NbIotRrcSap::NpdcchMessage> ();
  if (frameNo == 1 && subframeNo == 1)
    {
      return ret;
    }
  // check and Schedule DCIs for SearchSpaceType2 (RAR, HARQ, RRC)
  if (IsSeachSpaceType2Begin (m_ce0))
    {
      ret = ScheduleSearchSpace (NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2, m_ce0);
    }
  if (IsSeachSpaceType2Begin (m_ce1))
    {
      ret = ScheduleSearchSpace (NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2, m_ce1);
    }
  if (IsSeachSpaceType2Begin (m_ce2))
    {
      ret = ScheduleSearchSpace (NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2, m_ce2);
    }

  return ret;
}

std::vector<NbIotRrcSap::NpdcchMessage>
NbiotScheduler::ScheduleSearchSpace (NbIotRrcSap::NpdcchMessage::SearchSpaceType seachspace,
                                     NbIotRrcSap::NprachParametersNb ce)
{
  bool scheduleSuccessful = false;
  int R_max;
  std::vector<NbIotRrcSap::NpdcchMessage> scheduledMessages;
  if (seachspace == NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2)
    {
      R_max = NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int (ce);
    }
  /*
  Scheduling Magic. For now FIFO
  */
  for (std::vector<NbIotRrcSap::NpdcchMessage>::iterator it = m_NpdcchQueue.begin ();
       it != m_NpdcchQueue.end ();)
    {
      if (it->searchSpaceType == seachspace)
        {
          if (it->ce.nprachSubcarrierOffset == ce.nprachSubcarrierOffset)
            {
              std::vector<int> test = GetNextAvailableSearchSpaceCandidate (
                  m_frameNo - 1, m_subframeNo - 1, R_max,
                  NbIotRrcSap::ConvertDciN1Repetitions2int (it->dciN1));

              if (it->dciType == NbIotRrcSap::NpdcchMessage::DciType::n1)
                {
                  if (test.size () > 0) // WE GOT A DOWNLINK NPDCCH CANDIDATE
                    {

                      int subframesNpdsch =
                          NbIotRrcSap::ConvertNumNpdschSubframesPerRepetition2int (it->dciN1) *
                          NbIotRrcSap::ConvertNumNpdschRepetitions2int (it->dciN1);
                      std::vector<int> npdschsubframes = GetNextAvailableNpdschCandidate (
                          *(test.end () - 1), m_minSchedulingDelayDci2Downlink, subframesNpdsch,
                          R_max);
                      if (npdschsubframes.size () > 0) // WE GOT A DOWNLINK CANDIDATE
                        {
                          int subframesNpusch;
                          std::pair<NbIotRrcSap::UlGrant, std::pair<int, std::vector<int>>> ulgrant;
                          if (it->isRar)
                            {
                              for (std::vector<NbIotRrcSap::Rar>::iterator rar = it->rars.begin ();
                                   rar != it->rars.end ();)
                                {
                                  // MSG3 SIZE
                                  // See Joerke NBIoT_UE.py
                                  /*
                                  The MAC PDU consists of the following (ref. ETSI TS 136 321 V13.9.0 Fig. 6.1.6-4):
                                  |-----------------------------------------MAC-Header-----------------------------------------|--MAC-CE--|...|--MAC-SDU--|--Padding(opt)--|
                                  |--R/F2/E/LCID subheader--|...|--R/F2/E/LCID/F/K subheader--|--R/R/E/LCID padding subheader--|
                                  In Msg3 the UE should transmit a Buffer Status Report MAC CE in order to inform the eNodeB about its UL buffer status
                                  Since RRCConnectionResumeComplete-NB is transmitted in RLC TM, TMD PDU consists only of a data field and doesn't consists of any RLC headers (Ref. ETSI TS 136 331 V13.15.0 p.522 "RRCConnectionResumeRequest-NB" and ETSI TS 136 322 V13.4.0, 6.2.1.2)
                                  RRCConnectionResumeRequest-NB ist transmitted using SRB0 (ref. ETSI TS 136 331 V13.15.0 p.522 "RRCConnectionResumeRequest-NB" and therefore does not go through PDCP (ref. ETSI TS 136 323 V13.6.0, 4.2.1: "Each RB (i.e. DRB, SLRB and SRB, except for SRB0 and SRB1bis) is associated with one PDCP entity"))
                                  */

                                  int size_rrc_conn_resume_req =
                                      59; // 40 bits resumeID +16 bits shortResumeMAC-I +3 bits resumeCause ref. ETSI TS 136 331 V13.15.0 p.522 RRCConnectionResumeRequest-NB
                                  size_rrc_conn_resume_req +=
                                      (8 - size_rrc_conn_resume_req % 8); // Fill to full byte
                                  int size_rlc_pdu = size_rrc_conn_resume_req;
                                  //MAC_SUBHEADER_CE__R_F2_E_LCID = 8           # R/F2/E/LCID sub-header for MAC control element (ETSI TS 136 321 V13.9.0 (2018-07) Figure 6.1.2-2)
                                  //MAC_SUBHEADER_SDU__R_F2_E_LCID_F_7L = 16    # R/F2/E/LCID/F/L sub-headerfor MAC SDU with 7-bits L field (ETSI TS 136 321 V13.9.0 (2018-07) Figure 6.1.2-1)
                                  //MAC_CE_sBSR = 8                             # Short BSR and Truncated BSR MAC control element (ETSI TS 136 321 V13.9.0 (2018-07) Figure 6.1.3.1-1)
                                  int size_mac_pdu = 8 + 16 + 8 + size_rlc_pdu;
                                  int couplingloss;
                                  if (rar->rapId < 24)
                                    {
                                      couplingloss = 159;
                                    }
                                  else if (rar->rapId < 36)
                                    {
                                      couplingloss = 149;
                                    }
                                  else if (rar->rapId < 48)
                                    {
                                      couplingloss = 139;
                                    }
                                  subframesNpusch = m_Amc.getMsg3Subframes (
                                      couplingloss, size_mac_pdu, 15000, 15);
                                  ulgrant = GetNextAvailableMsg3UlGrantCandidate (
                                      *(npdschsubframes.end () - 1), subframesNpusch);
                                  if (ulgrant.first.success) // WE GOT AN UPLINK MSG3 CANDIDATE
                                    {
                                      scheduleSuccessful = true;
                                      rar->rarPayload.ulGrant = ulgrant.first;
                                      rar->rarPayload.ulGrant.subframes = ulgrant.second;
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
                                      NS_BUILD_DEBUG (std::cout << "\n");
                                      ++rar;
                                    }
                                  else
                                    {
                                      it->rars.erase (rar);
                                      // Stuff if cant schedule npusch
                                    }
                                }
                            }
                          else
                            {
                              // Create HARQ Ressource
                              int subframesNpuschHarq =
                                  4; // Have to be set by higher layer | 4 for debugging
                              std::vector<std::pair<int, std::vector<int>>> npuschharqsubframes =
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
                                  NS_BUILD_DEBUG (std::cout << "\n");
                                  it->dciN1.npuschOpportunity = npuschharqsubframes;
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

                              NS_BUILD_DEBUG (std::cout << "\n");
                              NS_BUILD_DEBUG (std::cout << "Scheduling NPDSCH at ");

                              for (size_t j = 0; j < npdschsubframes.size (); ++j)
                                {
                                  m_downlink[npdschsubframes[j]] = m_currenthyperindex;
                                  NS_BUILD_DEBUG (std::cout << npdschsubframes[j] << " ");
                                }

                              it->dciRepetitionsubframes = test;
                              it->dciN1.npdschOpportunity = npdschsubframes;
                              scheduledMessages.push_back (*(it));
                              NS_BUILD_DEBUG (std::cout << "\n");

                              m_NpdcchQueue.erase (it);
                              continue;
                            }
                          else
                            {
                              m_NpdcchQueue.erase (it);
                              continue;
                            }
                        }
                    }
                }
              else if (it->dciType == NbIotRrcSap::NpdcchMessage::DciType::n0)
                {
                  if (test.size () > 0)
                    {
                      int subframesNpusch = NbIotRrcSap::ConvertNumResourceUnits2int(it->dciN0) *
                          NbIotRrcSap::ConvertNumNpuschRepetitions2int(it->dciN0);
                       // Have to be set by higher layer | 4 for debugging
                      std::vector<std::pair<int, std::vector<int>>> npuschsubframes =
                          GetNextAvailableNpuschCandidate (*(test.end () - 1), 0, subframesNpusch, true);

                      if (npuschsubframes.size () > 0)
                        {
                          NS_BUILD_DEBUG (std::cout << "Scheduling NPUSCH at ");
                          scheduleSuccessful = true;
                          for (size_t i = 0; i < npuschsubframes[0].second.size (); i++)
                            {
                              m_uplink[npuschsubframes[0].first]
                                      [npuschsubframes[0].second[i]] = m_currenthyperindex;
                              NS_BUILD_DEBUG (std::cout << npuschsubframes[0].second[i] << " ");
                            }
                          NS_BUILD_DEBUG (std::cout << "\n");

                          it->dciRepetitionsubframes = test;
                          it->dciN0.npuschOpportunity = npuschsubframes;
                          scheduledMessages.push_back (*(it));
                          NS_BUILD_DEBUG (std::cout << "\n");

                          m_NpdcchQueue.erase (it);
                          continue;
                        }
                    }
                }
            }
          // TO FIX, HAS TO COUNT UP IN ALL CASSES that dont delete the current element
        }

      ++it;
      scheduleSuccessful = false;
    }

  return scheduledMessages;
}

std::vector<std::pair<int, std::vector<int>>>
NbiotScheduler::GetNextAvailableNpuschCandidate (int endSubframeNpdsch, int minSchedulingDelay,
                                                 int numSubframes, bool isHarq)
{
  std::vector<std::pair<int, std::vector<int>>> allocation;
  if (isHarq)
    {

      for (auto &i : m_HarqTimeOffsets)
        {
          for (size_t j = 0; j < 4; ++j)
            { // For subcarrier 0-3 for 15khz Subcarrier spacing | needs change for 3.75 Khz
              int candidate =
                  endSubframeNpdsch + NbIotRrcSap::HarqAckResource::ConvertHarqTimeOffset2int (i);
              std::vector<int> subframesOccupied =
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
              int candidate =
                  endSubframeNpdsch + NbIotRrcSap::ConvertNpuschSchedulingDelay2int(tmp);
              std::vector<int> subframesOccupied =
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
  return std::vector<std::pair<int, std::vector<int>>> ();
}

std::pair<NbIotRrcSap::UlGrant, std::pair<int, std::vector<int>>>
NbiotScheduler::GetNextAvailableMsg3UlGrantCandidate (int endSubframeMsg2, int numSubframes)
{
  for (auto &i : m_Msg3TimeOffset)
    {
      for (size_t j = 0; j < m_uplink.size (); ++j)
        {
          int candidate =
              endSubframeMsg2 + NbIotRrcSap::UlGrant::ConvertUlGrantSchedulingDelay2int (i);
          std::vector<int> subframesOccupied =
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
  return std::make_pair (ret, std::make_pair (int (), std::vector<int> ()));
}
std::vector<int>
NbiotScheduler::GetNextAvailableNpdschCandidate (int endSubframeDci, int minSchedulingDelay,
                                                 int numSubframes, int R_max)
{

  int npdschCandidate = endSubframeDci + minSchedulingDelay;
  if (R_max < 128)
    {
      for (auto &i : m_DciTimeOffsetRmaxSmall)
        {
          NbIotRrcSap::DciN1 tmp; /// FIX AS SOON AS POSSIBLE
          tmp.npdcchTimeOffset = i;
          int tmpCandidate = npdschCandidate + NbIotRrcSap::ConvertNpdcchTimeOffset2int (tmp);
          std::vector<int> subframesOccupied =
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
          int tmpCandidate = npdschCandidate + NbIotRrcSap::ConvertNpdcchTimeOffset2int (tmp);
          std::vector<int> subframesOccupied =
              GetDlSubframeRangeWithoutSystemResources (tmpCandidate, numSubframes);
          subframesOccupied =
              CheckforNContiniousSubframesDl (subframesOccupied, tmpCandidate, numSubframes);
          if (subframesOccupied.size () > 0)
            {
              return subframesOccupied;
            }
        }
    }
  return std::vector<int> ();
}

std::vector<int>
NbiotScheduler::GetDlSubframeRangeWithoutSystemResources (int overallSubframeNo, int numSubframes)
{
  std::vector<int> subframeIndexes;
  size_t i = 0;
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

std::vector<int>
NbiotScheduler::GetUlSubframeRangeWithoutSystemResources (int overallSubframeNo, int numSubframes,
                                                          int carrier)
{
  std::vector<int> subframeIndexes;
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
std::vector<std::pair<int, int>>
NbiotScheduler::GetAllPossibleSearchSpaceCandidates (std::vector<int> subframes, int R_max)
{
  std::vector<std::pair<int, int>> candidates;
  m_currenthyperindex = 1;
  int start_sf;
  int length = 0;
  int i = 0;
  start_sf = subframes[0];
  while (R_max > 0)
    {
      if (m_downlink[subframes[i]] != m_currenthyperindex)
        {
          length++;
        }
      else
        {
          if (length > 0)
            {
              candidates.push_back (std::make_pair (start_sf, length));
            }
          start_sf = subframes[i];
          length = 0;
        }
      R_max--;
      i++;
    }
  if (candidates.size () == 0 && length > 0)
    {
      candidates.push_back (std::make_pair (start_sf, length));
    }
  return candidates;
}

std::vector<int>
NbiotScheduler::CheckforNContiniousSubframesDl (std::vector<int> Subframes, int StartSubframe,
                                                uint N)
{
  int startSubframeIndex = -1;
  std::vector<int> range;
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
      return std::vector<int> ();
    }

  for (size_t i = 0; i < N; i++)
    {
      if (m_downlink[Subframes[startSubframeIndex + i]] == m_currenthyperindex)
        {
          return std::vector<int> ();
        }
      else
        {
          range.push_back (Subframes[startSubframeIndex + i]);
        }
    }
  return range;
}
std::vector<int>
NbiotScheduler::CheckforNContiniousSubframesUl (std::vector<int> Subframes, int StartSubframe,
                                                uint N, uint carrier)
{
  int startSubframeIndex = -1;
  std::vector<int> range;
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
      return std::vector<int> ();
    }

  for (size_t i = 0; i < N; i++)
    {
      if (m_uplink[carrier][Subframes[startSubframeIndex + i]] == m_currenthyperindex)
        {
          return std::vector<int> ();
        }
      else
        {
          range.push_back (Subframes[startSubframeIndex + i]);
        }
    }
  return range;
}
std::vector<int>
NbiotScheduler::GetNextAvailableSearchSpaceCandidate (int SearchSpaceStartFrame,
                                                      int SearchSpaceStartSubframe, int R_max,
                                                      int R)
{
  uint u_max = ((R_max / R) - 1);
  int overallSubframe = 10 * (SearchSpaceStartFrame) + SearchSpaceStartSubframe;
  std::vector<int> subframes = GetDlSubframeRangeWithoutSystemResources (overallSubframe, R_max);
  for (size_t i = 0; i <= u_max; ++i)
    {
      // Calculate start of dci candidate
      std::vector<int> subframes_to_use =
          CheckforNContiniousSubframesDl (subframes, subframes[i * R], R);

      if (subframes_to_use.size () > 0)
        {
          return subframes_to_use;
        }
    }
  return std::vector<int> ();
}

void
NbiotScheduler::ScheduleDlRlcBufferReq (LteMacSapProvider::ReportBufferStatusParameters params,
                                        NbIotRrcSap::NpdcchMessage::SearchSpaceType searchspace)
{
  /*
  Magic for defining modulation and coding scheme etc
 */
  double correction_factor =
      10 *
      log10 (
          1.0 /
          12.0); // correctionfactor applied to rsrp because it's for earch subcarrier and tx power is for full spectrum
  NS_BUILD_DEBUG (std::cout << "MCL of " << params.rnti << " is "
                            << m_rntiRsrpMap[params.rnti] - 43.0 - correction_factor << "\n");

  std::pair<NbIotRrcSap::DciN1, int> dci_tbs = m_Amc.getBareboneDci (
      m_rntiRsrpMap[params.rnti] - 43.0 - correction_factor, params.txQueueSize * 8, "inband");

  NbIotRrcSap::DciN1 dci = dci_tbs.first;
  dci.mCS = NbIotRrcSap::DciN1::MCS::one;
  dci.NDI = true;

  NbIotRrcSap::NprachParametersNb ceLevel;

  if (m_rntiRsrpMap[params.rnti] < m_sib2config.radioResourceConfigCommon.nprachConfig
                                       .rsrpThresholdsPrachInfoList.ce2_lowerbound)
    {
      dci.dciRepetitions = NbIotRrcSap::DciN1::DciRepetitions::r256;
      ceLevel = m_ce2;
    }
  else if (m_rntiRsrpMap[params.rnti] < m_sib2config.radioResourceConfigCommon.nprachConfig
                                            .rsrpThresholdsPrachInfoList.ce1_lowerbound)
    {
      dci.dciRepetitions = NbIotRrcSap::DciN1::DciRepetitions::r32;
      ceLevel = m_ce1;
    }
  else if (m_rntiRsrpMap[params.rnti] > m_sib2config.radioResourceConfigCommon.nprachConfig
                                            .rsrpThresholdsPrachInfoList.ce1_lowerbound)
    {
      dci.dciRepetitions = NbIotRrcSap::DciN1::DciRepetitions::r2;
      ceLevel = m_ce0;
    }

  NbIotRrcSap::NpdcchMessage msg;
  msg.dciType = NbIotRrcSap::NpdcchMessage::DciType::n1;
  msg.isRar = false;
  msg.rnti = params.rnti;
  msg.dciN1 = dci;
  msg.ce = ceLevel;
  msg.searchSpaceType = searchspace;
  msg.tbs = dci_tbs.second;
  msg.lcid = params.lcid;
  m_NpdcchQueue.push_back (msg);
}

void
NbiotScheduler::ScheduleMsg5Req (int rnti)
{

  NbIotRrcSap::DciN0 dci;
  dci.dciRepetitions = NbIotRrcSap::DciN0::DciRepetitions::r2;
  dci.mCS = NbIotRrcSap::DciN0::MCS::one;
  dci.NDI = true;
  dci.numNpuschRepetitions = NbIotRrcSap::DciN0::NumNpuschRepetitions::r4;
  dci.numResourceUnits = NbIotRrcSap::DciN0::NumResourceUnits::ru2;

  NbIotRrcSap::NpdcchMessage msg;
  msg.dciType = NbIotRrcSap::NpdcchMessage::DciType::n0;
  msg.isRar = false;
  msg.rnti = rnti;
  msg.dciN0 = dci;
  msg.ce = m_ce0;
  msg.searchSpaceType = NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2;

  m_NpdcchQueue.push_back (msg);
}

} // namespace ns3
