#include "nb-iot-amc.h"
#include <fstream>
#include <cmath>

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("NbiotAmc");

NS_OBJECT_ENSURE_REGISTERED (NbiotAmc);

NbiotAmc::NbiotAmc ()
{

  std::ifstream npusch_file ("src/lte/csv/clean_NPUSCH_100_TRBlks.csv");
  std::ifstream npdsch_file ("src/lte/csv/clean_NPDSCH_100_TRBlks.csv");
  m_npusch_params = readNpuschCSV (npusch_file);
  m_npdsch_params = readNpdschCSV (npdsch_file);
  m_r13 = true;
  m_highestmcl = m_npdsch_params.standalone.begin ()->Pathloss;
  m_lowestmcl = m_npdsch_params.standalone.begin ()->Pathloss;
  for (std::vector<NpdschMeasurementValues>::iterator it = m_npdsch_params.standalone.begin ();
       it != m_npdsch_params.standalone.end (); ++it)
    {
      if (it->Pathloss < m_lowestmcl)
        {
          m_lowestmcl = it->Pathloss;
        }
      if (it->Pathloss > m_highestmcl)
        {
          m_highestmcl = it->Pathloss;
        }
    }
}
void
NbiotAmc::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

std::vector<std::string>
NbiotAmc::readCSVRow (const std::string &row)
{
  CSVState state = CSVState::UnquotedField;
  std::vector<std::string> fields{""};
  size_t i = 0; // index of the current field
  for (char c : row)
    {
      switch (state)
        {
        case CSVState::UnquotedField:
          switch (c)
            {
            case ',': // end of field
              fields.push_back ("");
              i++;
              break;
            case '"':
              state = CSVState::QuotedField;
              break;
            default:
              fields[i].push_back (c);
              break;
            }
          break;
        case CSVState::QuotedField:
          switch (c)
            {
            case '"':
              state = CSVState::QuotedQuote;
              break;
            default:
              fields[i].push_back (c);
              break;
            }
          break;
        case CSVState::QuotedQuote:
          switch (c)
            {
            case ',': // , after closing quote
              fields.push_back ("");
              i++;
              state = CSVState::UnquotedField;
              break;
            case '"': // "" -> "
              fields[i].push_back ('"');
              state = CSVState::QuotedField;
              break;
            default: // end of quote
              state = CSVState::UnquotedField;
              break;
            }
          break;
        }
    }
  return fields;
}

/// Read CSV file, Excel dialect. Accept "quoted fields ""with quotes"""
MatlabNpdschMeasurement
NbiotAmc::readNpdschCSV (std::istream &in)
{
  MatlabNpdschMeasurement table;
  std::string row;
  std::getline (in, row);
  while (!in.eof ())
    {
      std::getline (in, row);
      if (in.bad () || in.fail ())
        {
          break;
        }
      auto fields = readCSVRow (row);
      NpdschMeasurementValues tmp;
      tmp.datatype = true;
      tmp.requiredSNR = stod (fields[2]);
      tmp.ISF = stoi (fields[3]);
      tmp.NSF = stoi (fields[4]);
      tmp.IMCS = stoi (fields[5]);
      tmp.TBS = stoi (fields[6]);
      tmp.numTrBlks = stoi (fields[7]);
      tmp.NRep = stoi (fields[8]);
      tmp.SNR = stoi (fields[9]);
      tmp.BLER = stod (fields[10]);
      tmp.TTI = stoi (fields[11]);
      tmp.Pathloss = stoi (fields[12]);
      tmp.datarate = stod (fields[13]);
      if (fields[0] == "inband")
        {
          table.inband.push_back (tmp);
        }
      else if (fields[0] == "guardband")
        {
          table.guardband.push_back (tmp);
        }
      else if (fields[0] == "standalone")
        {
          table.standalone.push_back (tmp);
        }
    }
  return table;
}

MatlabNpuschMeasurement
NbiotAmc::readNpuschCSV (std::istream &in)
{
  MatlabNpuschMeasurement table;
  std::string row;
  std::getline (in, row);
  while (!in.eof ())
    {
      std::getline (in, row);
      if (in.bad () || in.fail ())
        {
          break;
        }
      auto fields = readCSVRow (row);
      NpuschMeasurementValues tmp;
      tmp.SCS = stoi (fields[0]);
      tmp.ISC = stoi (fields[1]);
      tmp.NRUSC = stoi (fields[2]);
      tmp.tRu = stoi (fields[3]);
      tmp.requiredSNR = stod (fields[4]);
      tmp.IRU = stoi (fields[5]);
      tmp.NRU = stoi (fields[6]);
      tmp.ITBS = stoi (fields[7]);
      tmp.Qm = stoi (fields[8]);
      tmp.TBS = stoi (fields[9]);
      tmp.numTrBlks = stoi (fields[10]);
      tmp.NRep = stoi (fields[11]);
      tmp.SNR = stoi (fields[12]);
      tmp.BLER = stod (fields[13]);
      tmp.TTI = stoi (fields[14]);
      tmp.Pathloss = stoi (fields[15]);
      tmp.datarate = stod (fields[16]);
      tmp.bandwidth = stod (fields[17]);
      table.measurements.push_back (tmp);
    }
  return table;
}

NpdschMeasurementValues
NbiotAmc::getNpdschParameters (double couplingloss, int dataSize, std::string opMode)
{
  couplingloss = abs (floor (couplingloss));
  std::vector<NpdschMeasurementValues> *values;
  if (couplingloss < m_lowestmcl)
    {
      couplingloss = m_lowestmcl;
    }
  if (couplingloss > m_highestmcl)
    {
      couplingloss = m_highestmcl;
    }

  if (opMode == "inband")
    {
      values = &m_npdsch_params.inband;
    }
  else if (opMode == "guardband")
    {
      values = &m_npdsch_params.guardband;
    }
  else if (opMode == "standalone")
    {
      values = &m_npdsch_params.standalone;
    }

  NpdschMeasurementValues value;
  value.TTI = 10000;
  value.BLER = 1;
  int max_tbs;
  if(m_r13){
    max_tbs = 680;
  }
  for (std::vector<NpdschMeasurementValues>::iterator it = values->begin (); it != values->end ();
       ++it)
    {
      if (it->Pathloss == couplingloss)
        {
          if (it->TBS >= dataSize && it->TBS <= max_tbs) 
            {
              if (it->TTI < value.TTI)
                { 
                  if (it->BLER < value.BLER){
                    value = *it;
                  }
                }
            }
        }
    }
  return value;
}

NpuschMeasurementValues
NbiotAmc::getMaxTbsforCl(double couplingloss, double scs, double bandwidth)
{
  couplingloss = abs (floor (couplingloss));
  if (couplingloss < m_lowestmcl)
    {
      couplingloss = m_lowestmcl;
    }
  if (couplingloss > m_highestmcl)
    {
      couplingloss = m_highestmcl;
    }
  NpuschMeasurementValues value;
  value.TTI = 10000;
  value.BLER = 1;
  value.TBS = 100;
  for (std::vector<NpuschMeasurementValues>::iterator it = m_npusch_params.measurements.begin ();
       it != m_npusch_params.measurements.end (); ++it){
      if (it->SCS == scs){
        if(it->bandwidth == bandwidth){
          if(it->Pathloss > couplingloss){
            if(it->TBS > value.TBS){
              value = *it;
            }
          }
        }
      }
    }


  //std::cout << "Value: " << value.TBS << std::endl;
  return value;
}

NpuschMeasurementValues
NbiotAmc::getNpuschParameters (double couplingloss, int dataSize, double scs, double bandwidth)
{
  couplingloss = abs (floor (couplingloss));
  if (couplingloss < m_lowestmcl)
    {
      couplingloss = m_lowestmcl;
    }
  if (couplingloss > m_highestmcl)
    {
      couplingloss = m_highestmcl;
    }
  NpuschMeasurementValues value;
  value.TTI = 10000;
  value.BLER = 1;
  int max_tbs = 1000;
  int max_tbsi = 13;
  if(m_r13){
    max_tbs = 1000;
    max_tbsi = 12;
  }
  for (std::vector<NpuschMeasurementValues>::iterator it = m_npusch_params.measurements.begin ();
       it != m_npusch_params.measurements.end (); ++it)
    {
      if (it->SCS == scs)
        {
            if (it->Pathloss == couplingloss)
              {
                if (it->TBS >= dataSize && it->TBS <= max_tbs) 
                  {
                    if (it->TTI < value.TTI)
                      { 
                        if (it->BLER < value.BLER){
                          NpuschMeasurementValues tmp = *it;
                          if(tmp.ITBS <= max_tbsi){
                            value = *it;
                          }
                        }
                      }
                  }
              }
            
        }
    }
  return value;
}

int
NbiotAmc::mapMeasurementValuetoMSG3 (NpuschMeasurementValues value)
{
  //Currently only Singletone 15khz Scheduling supported.
  // Has to be extended later on
  return value.NRU * value.NRep;
}

int
NbiotAmc::getMsg3Subframes (double couplingloss, int dataSize, double scs, double bandwidth)
{
  NpuschMeasurementValues value = getNpuschParameters (couplingloss, dataSize, scs, bandwidth);
  // Only singletone 15khz supported yet
  // Extension later on
  return value.NRep * value.NRU;
}

NbIotRrcSap::DciN1
NbiotAmc::mapMeasurementValuetoDciN1 (NpdschMeasurementValues value)
{
  NbIotRrcSap::DciN1::NumNpdschSubframesPerRepetition subframes;
  switch (value.NSF)
    {
    case 1:
      subframes = NbIotRrcSap::DciN1::NumNpdschSubframesPerRepetition::s1;
      break;
    case 2:
      subframes = NbIotRrcSap::DciN1::NumNpdschSubframesPerRepetition::s2;
      break;
    case 3:
      subframes = NbIotRrcSap::DciN1::NumNpdschSubframesPerRepetition::s3;
      break;
    case 4:
      subframes = NbIotRrcSap::DciN1::NumNpdschSubframesPerRepetition::s4;
      break;
    case 5:
      subframes = NbIotRrcSap::DciN1::NumNpdschSubframesPerRepetition::s5;
      break;
    case 6:
      subframes = NbIotRrcSap::DciN1::NumNpdschSubframesPerRepetition::s6;
      break;
    case 8:
      subframes = NbIotRrcSap::DciN1::NumNpdschSubframesPerRepetition::s8;
      break;
    case 10:
    default:
      subframes = NbIotRrcSap::DciN1::NumNpdschSubframesPerRepetition::s10;
      break;
    }

  NbIotRrcSap::DciN1::NumNpdschRepetitions repetitions;
  switch (value.NRep)
    {
    case 1:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r1;
      break;
    case 2:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r2;
      break;
    case 4:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r4;
      break;
    case 8:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r8;
      break;
    case 16:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r16;
      break;
    case 32:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r32;
      break;
    case 64:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r64;
      break;
    case 128:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r128;
      break;
    case 192:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r192;
      break;
    case 256:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r256;
      break;
    case 384:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r384;
      break;
    case 512:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r512;
      break;
    case 768:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r768;
      break;
    case 1024:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r1024;
      break;
    case 1536:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r1536;
      break;
    case 2048:
    default:
      repetitions = NbIotRrcSap::DciN1::NumNpdschRepetitions::r2048;
      break;
    }

  NbIotRrcSap::DciN1 dci;
  dci.format = true;
  dci.numNpdschSubframesPerRepetition = subframes;
  dci.numNpdschRepetitions = repetitions;
  return dci;
}

NbIotRrcSap::DciN0
NbiotAmc::mapMeasurementValuetoDciN0 (NpuschMeasurementValues value)
{
  NbIotRrcSap::DciN0::NumResourceUnits subframes;
  switch (value.NRU)
    {
    case 1:
      subframes = NbIotRrcSap::DciN0::NumResourceUnits::ru1;
      break;
    case 2:
      subframes = NbIotRrcSap::DciN0::NumResourceUnits::ru2;
      break;
    case 3:
      subframes = NbIotRrcSap::DciN0::NumResourceUnits::ru3;
      break;
    case 4:
      subframes = NbIotRrcSap::DciN0::NumResourceUnits::ru4;
      break;
    case 5:
      subframes = NbIotRrcSap::DciN0::NumResourceUnits::ru5;
      break;
    case 6:
      subframes = NbIotRrcSap::DciN0::NumResourceUnits::ru6;
      break;
    case 8:
      subframes = NbIotRrcSap::DciN0::NumResourceUnits::ru8;
      break;
    case 10:
    default:
      subframes = NbIotRrcSap::DciN0::NumResourceUnits::ru10;
      break;
    }

  NbIotRrcSap::DciN0::NumNpuschRepetitions repetitions;
  switch (value.NRep)
    {
    case 1:
      repetitions = NbIotRrcSap::DciN0::NumNpuschRepetitions::r1;
      break;
    case 2:
      repetitions = NbIotRrcSap::DciN0::NumNpuschRepetitions::r2;
      break;
    case 4:
      repetitions = NbIotRrcSap::DciN0::NumNpuschRepetitions::r4;
      break;
    case 8:
      repetitions = NbIotRrcSap::DciN0::NumNpuschRepetitions::r8;
      break;
    case 16:
      repetitions = NbIotRrcSap::DciN0::NumNpuschRepetitions::r16;
      break;
    case 32:
      repetitions = NbIotRrcSap::DciN0::NumNpuschRepetitions::r32;
      break;
    case 64:
      repetitions = NbIotRrcSap::DciN0::NumNpuschRepetitions::r64;
      break;
    case 128:
      repetitions = NbIotRrcSap::DciN0::NumNpuschRepetitions::r128;
      break;
    default:
      repetitions = NbIotRrcSap::DciN0::NumNpuschRepetitions::r128;
      break;
    }

  NbIotRrcSap::DciN0 dci;
  dci.format = true;
  dci.numResourceUnits = subframes;
  dci.numNpuschRepetitions = repetitions;
  return dci;
}

std::pair<NbIotRrcSap::DciN1, int>
NbiotAmc::getBareboneDciN1 (double couplingloss, int dataSize, std::string opMode)
{
  NpdschMeasurementValues value = getNpdschParameters (couplingloss, dataSize, opMode);
  //value.NSF=8;
  //value.NRep=16;
  //value.IMCS = 5;
  NbIotRrcSap::DciN1 dci = mapMeasurementValuetoDciN1 (value);
  uint16_t tbs = TransportBlockSizeTableDlNb[value.IMCS][value.NSF];
  return std::make_pair (dci, tbs);
}

std::pair<NbIotRrcSap::DciN0, int>
NbiotAmc::getBareboneDciN0 (double couplingloss, int dataSize, double scs, double bandwidth)
{
  NpuschMeasurementValues value = getNpuschParameters (couplingloss, dataSize, scs, bandwidth);
  //value.NRep = 4;
  //value.NRU = 10;
  //value.ITBS= 6;
  NbIotRrcSap::DciN0 dci = mapMeasurementValuetoDciN0 (value);
  uint16_t tbs = TransportBlockSizeTableUlNb[value.ITBS][value.NRU];
  return std::make_pair (dci, tbs);
}
} // namespace ns3