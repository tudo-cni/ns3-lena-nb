#include "nb-iot-scheduler.h"


namespace ns3{

NS_LOG_COMPONENT_DEFINE ("NbiotScheduler");

NS_OBJECT_ENSURE_REGISTERED (NbiotScheduler);

NbiotScheduler::NbiotScheduler(){
  m_DciTimeOffsetRmaxSmall.reserve(8);
  m_DciTimeOffsetRmaxBig.reserve(8);

  m_DciTimeOffsetRmaxSmall.insert(m_DciTimeOffsetRmaxSmall.begin(), {
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms0,
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms4,
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms8,
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms12,
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms16,
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms32,
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms64,
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms128
  });
  m_DciTimeOffsetRmaxBig.insert(m_DciTimeOffsetRmaxBig.begin(), {
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms0,
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms16,
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms32,
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms64,
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms128,
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms256,
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms512,
    NbIotRrcSap::DciN1::NpdcchTimeOffset::ms1024
  });

  int numHyperframes = 1024;
  int numFrames = 1024;
  int numSubframes = 10;
  m_downlink.resize(numHyperframes*numFrames*numSubframes, 0);
  for(size_t i =0; i < m_downlink.size(); ++i){
    if((i % 10) == 0){
      m_downlink[i] = -1; // MIB-NB
    }
    if((i % 10) == 5){
      m_downlink[i] = -1; // NPSS
    }
    if(((i % 10) == 9) && ((i/10) %2 == 1)){
      m_downlink[i] = -1; // NSSS
    }
  }
}

void
NbiotScheduler::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

void NbiotScheduler::SetCeLevel(NbIotRrcSap::NprachParametersNb ce0, NbIotRrcSap::NprachParametersNb ce1, NbIotRrcSap::NprachParametersNb ce2){
  m_ce0 = ce0;
  m_ce1 = ce1;
  m_ce2 = ce2;

}
bool NbiotScheduler::IsSeachSpaceType2Begin(NbIotRrcSap::NprachParametersNb ce){
  uint32_t searchSpacePeriodicity = NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int(ce) * NbIotRrcSap::ConvertNpdcchStartSfCssRa2double(ce);
  uint32_t searchSpaceConditionLeftSide = (10*(m_frameNo-1) + (m_subframeNo-1)) % searchSpacePeriodicity;
  uint32_t searchSpaceConditionRightSide = NbIotRrcSap::ConvertNpdcchOffsetRa2double(ce) * searchSpacePeriodicity;
  if(searchSpaceConditionLeftSide == searchSpaceConditionRightSide){
    return true;
  
  }
  return false;
}

void NbiotScheduler::ScheduleRarReq(int rnti, int rapid, NbIotRrcSap::NprachParametersNb ce,NbIotRrcSap::DciN1::DciRepetitions rep){
  NpdcchMessage rar;
  rar.npdcchFormat = NpdcchMessage::NpdcchFormat::format1;
  rar.dciType = NpdcchMessage::DciType::n1;
  rar.searchSpaceType = NpdcchMessage::SearchSpaceType::type2;
  // Dci set depending on coverage level.... yet static 
  rar.dciN1.dciRepetitions = rep;
  rar.dciN1.m_rapId = rapid;
  rar.dciN1.numNpdschSubframesPerRepetition = NbIotRrcSap::DciN1::NumNpdschSubframesPerRepetition::s2;
  rar.dciN1.numNpdschRepetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r2;
  rar.ue.ce = ce;
  rar.ue.ranti = rnti;
  m_NpdcchQueue.push_back(rar);
}

void NbiotScheduler::ScheduleNpdcchMessageReq(NpdcchMessage msg){
  m_NpdcchQueue.push_back(msg);
}

std::vector<NpdcchMessage> NbiotScheduler::Schedule(int frameNo, int subframeNo){
  m_frameNo = frameNo;
  m_subframeNo = subframeNo;
  std::vector<NpdcchMessage> ret = std::vector<NpdcchMessage>();
  if(frameNo == 1 && subframeNo == 1){
    return ret;
  }
  // check and Schedule DCIs for SearchSpaceType2 (RAR, HARQ, RRC)
  if(IsSeachSpaceType2Begin(m_ce0)){
   ret = ScheduleSearchSpace(NpdcchMessage::SearchSpaceType::type2, m_ce0);
  }
  if(IsSeachSpaceType2Begin(m_ce1)){
   ret = ScheduleSearchSpace(NpdcchMessage::SearchSpaceType::type2, m_ce1);
  }
 if(IsSeachSpaceType2Begin(m_ce2)){
   ret = ScheduleSearchSpace(NpdcchMessage::SearchSpaceType::type2, m_ce2);
 } 

  return ret;
}

std::vector<NpdcchMessage> NbiotScheduler::ScheduleSearchSpace(NpdcchMessage::SearchSpaceType seachspace,NbIotRrcSap::NprachParametersNb ce){
  int R_max;
  std::vector<NpdcchMessage> scheduledMessages;
  if (seachspace == NpdcchMessage::SearchSpaceType::type2){
    R_max = NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int(ce);
  }
  /*
  Scheduling Magic. For now FIFO
  */
  for(std::vector<NpdcchMessage>::iterator it = m_NpdcchQueue.begin(); it != m_NpdcchQueue.end();){
    if(it->searchSpaceType == seachspace){
      if(it->ue.ce.nprachSubcarrierOffset == ce.nprachSubcarrierOffset){
      std::vector<int> test = GetNextAvailableSearchSpaceCandidate(m_frameNo-1, m_subframeNo-1, R_max, NbIotRrcSap::ConvertDciRepetitions2int(it->dciN1));
      if(test.size()>0){

        int subframesNpdsch = NbIotRrcSap::ConvertNumNpdschSubframesPerRepetition2int(it->dciN1)*NbIotRrcSap::ConvertNumNpdschRepetitions2int(it->dciN1);
        std::vector<int> npdschsubframes = GetNextAvailableNpdschCandidate(*(test.end()-1), m_minSchedulingDelayDci2Downlink,subframesNpdsch, R_max);
        if (npdschsubframes.size()>0 ){
          std::cout << "Scheduling NPDCCH of " << it->ue.ranti << " at ";
          for(size_t j = 0; j < test.size(); ++j){
            m_downlink[test[j]] = m_currenthyperindex;
            std::cout << test[j] << " ";
          }
          std::cout << "\n";
          std::cout << "Scheduling NPDSCH of " << it->ue.ranti << " at ";
          for(size_t j = 0; j < npdschsubframes.size(); ++j){
            m_downlink[npdschsubframes[j]] = m_currenthyperindex;
            std::cout << npdschsubframes[j] << " ";
          }
          it->dciRepetitionsubframes = test;
          it->npdschOpportunity = npdschsubframes;
          scheduledMessages.push_back(*(it));
          std::cout << "\n";

          m_NpdcchQueue.erase(it);
        }
      }
      }
    else{
      ++it;
    }
    }
  }

  return scheduledMessages;
}

std::vector<int> NbiotScheduler::GetNextAvailableNpdschCandidate(int endSubframeDci, int minSchedulingDelay, int numSubframes, int R_max){

  int npdschCandidate = endSubframeDci+minSchedulingDelay;
  if(R_max < 128){
    for(auto &i : m_DciTimeOffsetRmaxSmall){
      NbIotRrcSap::DciN1 tmp; /// FIX AS SOON AS POSSIBLE 
      tmp.npdcchTimeOffset = i;
      int tmpCandidate = npdschCandidate+NbIotRrcSap::ConvertNpdcchTimeOffset2int(tmp);
      std::vector<int> subframesOccupied = GetSubframeRangeWithoutSystemResources(tmpCandidate, numSubframes);
      subframesOccupied = CheckforNContiniousSubframes(subframesOccupied,tmpCandidate,numSubframes);
      if(subframesOccupied.size()> 0){
        return subframesOccupied;
      }
    }
  }else{
    for(auto &i : m_DciTimeOffsetRmaxBig){
      NbIotRrcSap::DciN1 tmp; /// FIX AS SOON AS POSSIBLE 
      tmp.npdcchTimeOffset = i;
      int tmpCandidate = npdschCandidate+NbIotRrcSap::ConvertNpdcchTimeOffset2int(tmp);
      std::vector<int> subframesOccupied = GetSubframeRangeWithoutSystemResources(tmpCandidate, numSubframes);
      subframesOccupied = CheckforNContiniousSubframes(subframesOccupied,tmpCandidate,numSubframes);
      if(subframesOccupied.size()> 0){
        return subframesOccupied;
      }
    } 
  }
  return std::vector<int>();
}

std::vector<int> NbiotScheduler::GetSubframeRangeWithoutSystemResources(int overallSubframeNo, int numSubframes){
  std::vector<int> subframeIndexes;
  size_t i = 0;
  m_currenthyperindex = 1;
  while(numSubframes > 0){
    size_t currentindex = overallSubframeNo+i;
    if((m_downlink[currentindex] != -1)){
        subframeIndexes.push_back(currentindex);
        numSubframes--; 
    }
    i++;
  }
  return subframeIndexes;
}

std::vector<std::pair<int,int>> NbiotScheduler::GetAllPossibleSearchSpaceCandidates(std::vector<int> subframes, int R_max){
  std::vector<std::pair<int,int>> candidates;
  m_currenthyperindex = 1;
  int start_sf;
  int length = 0;
  int i = 0;
  start_sf = subframes[0];
  while(R_max > 0){
    if(m_downlink[subframes[i]] != m_currenthyperindex){
        length++;
    }else{
      if(length >0){
      candidates.push_back(std::make_pair(start_sf, length));
      }
      start_sf = subframes[i];
      length = 0;
    }
    R_max--;
    i++;
  }
  if(candidates.size() == 0 && length > 0 ){
    candidates.push_back(std::make_pair(start_sf,length));
  }
  return candidates;
}

std::vector<int> NbiotScheduler::CheckforNContiniousSubframes(std::vector<int> Subframes, int StartSubframe, uint N){
  int startSubframeIndex = -1;
  std::vector<int> range;
  for(size_t i = 0; i < Subframes.size(); ++i){
    if(Subframes[i] == StartSubframe){
      startSubframeIndex = i;
      break;
    }
  }
  if(startSubframeIndex == -1){
    return std::vector<int>();
  }

  for(size_t i = 0; i < N; i++){
    if (m_downlink[Subframes[startSubframeIndex+i]] == m_currenthyperindex){
      return std::vector<int>();
    }else{
      range.push_back(Subframes[startSubframeIndex+i]);
    }
  }
  return range;
}
std::vector<int> NbiotScheduler::GetNextAvailableSearchSpaceCandidate(int SearchSpaceStartFrame, int SearchSpaceStartSubframe, int R_max, int R){
  uint u_max = ((R_max/R)-1);
  int overallSubframe = 10*(SearchSpaceStartFrame)+SearchSpaceStartSubframe;
  std::vector<int> subframes = GetSubframeRangeWithoutSystemResources(overallSubframe, R_max);
  for(size_t i =0; i <= u_max; ++i){
    // Calculate start of dci candidate
    std::vector<int> subframes_to_use = CheckforNContiniousSubframes(subframes, subframes[i*R],R);
    
    if(subframes_to_use.size()> 0){
      return subframes_to_use;
    }
  }
  return std::vector<int>();
}

}