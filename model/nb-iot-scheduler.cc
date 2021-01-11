#include "nb-iot-scheduler.h"


namespace ns3{

NS_LOG_COMPONENT_DEFINE ("NbiotScheduler");

NS_OBJECT_ENSURE_REGISTERED (NbiotScheduler);

NbiotScheduler::NbiotScheduler(){
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
  rar.ue.ce = ce;
  rar.ue.ranti = rnti;
  m_NpdcchQueue.push_back(rar);
}

std::vector<std::pair<int, LteControlMessage>> NbiotScheduler::Schedule(int frameNo, int subframeNo){
  m_frameNo = frameNo;
  m_subframeNo = subframeNo;
  // check and Schedule DCIs for SearchSpaceType2 (RAR, HARQ, RRC)
  if(IsSeachSpaceType2Begin(m_ce0)){
   ScheduleSearchSpace(NpdcchMessage::SearchSpaceType::type2, m_ce0);
  }
  //if(IsSeachSpaceType2Begin(m_ce1)){

  //}
 //if(IsSeachSpaceType1Begin(m_ce2)){

 // } 

  return std::vector<std::pair<int, LteControlMessage>>();
}

std::vector<std::pair<int, LteControlMessage>> NbiotScheduler::ScheduleSearchSpace(NpdcchMessage::SearchSpaceType seachspace,NbIotRrcSap::NprachParametersNb ce){
  int R_max;
  if (seachspace == NpdcchMessage::SearchSpaceType::type2){
    R_max = NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int(ce);
  }
  /*
  Scheduling Magic. For now FIFO
  */
  for(std::vector<NpdcchMessage>::iterator it = m_NpdcchQueue.begin(); it != m_NpdcchQueue.end();){
    if(it->searchSpaceType == seachspace){
      std::vector<int> test = GetNextAvailableSearchSpaceCandidate(m_frameNo-1, m_subframeNo-1, R_max, NbIotRrcSap::ConvertDciRepetitions2int(it->dciN1));
      if(test.size()>0){
        std::cout << "Scheduling NPDCCH of " << it->ue.ranti << " at ";
      for(size_t j = 0; j < test.size(); ++j){
        m_downlink[test[j]] = m_currenthyperindex;
        std::cout << test[j] << " ";
      }
      std::cout << "\n";
      m_NpdcchQueue.erase(it);
      }
    else{
      ++it;
    }
    }
    std::cout << "bla";
  }

  return std::vector<std::pair<int, LteControlMessage>>();
}

std::vector<int> NbiotScheduler::GetSubframeRangeWithoutSystemResources(int frameNo, int subframeNo, int numSubframes){
  std::vector<int> subframeIndexes;
  int tmpframe = (frameNo) % (1024*1024);
  size_t i = 0;
  m_currenthyperindex = 1;
  while(numSubframes > 0){
    size_t currentindex = (10*tmpframe+(subframeNo))+i;
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

  std::vector<int> subframes = GetSubframeRangeWithoutSystemResources(SearchSpaceStartFrame, SearchSpaceStartSubframe, R_max);
  for(size_t i =0; i <= u_max; ++i){
    // Calculate start of dci candidate
    std::vector<int> subframes_to_use = CheckforNContiniousSubframes(subframes, subframes[i*R],R);
    
    if(subframes_to_use.size()> 0){
      std::cout << "Can schedule\n";
      return subframes_to_use;
    }
  }
  return std::vector<int>();
}

}