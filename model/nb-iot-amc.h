
#ifndef NBIOT_AMC_H
#define NBIOT_AMC_H

#include <map>
#include <ns3/object.h>
#include "nb-iot-rrc-sap.h"
#include <string>


namespace ns3 {



struct NpdschMeasurementValues
{
  bool datatype;
  double requiredSNR;
  uint8_t ISF;
  uint8_t NSF;
  uint8_t IMCS;
  int TBS;
  int numTrBlks;
  int NRep;
  int SNR;
  double BLER;
  int TTI;
  int Pathloss;
  double datarate;
};
struct MatlabNpdschMeasurement
{
  std::vector<NpdschMeasurementValues> inband;
  std::vector<NpdschMeasurementValues> standalone;
  std::vector<NpdschMeasurementValues> guardband;
};

struct NpuschMeasurementValues
{
  int SCS;
  int ISC;
  int NRUSC;
  int tRu;
  double requiredSNR;
  int IRU;
  int NRU;
  int ITBS;
  int Qm;
  int TBS;
  int numTrBlks;
  int NRep;
  int SNR;
  double BLER;
  int TTI;
  int Pathloss;
  double datarate;
  double bandwidth;

};

struct MatlabNpuschMeasurement
{
  std::vector<NpuschMeasurementValues> measurements;
};

enum class CSVState { UnquotedField, QuotedField, QuotedQuote };

class NbiotAmc : public Object
{

public:
  NbiotAmc ();

  //~NbiotScheduler();
  int m_highestmcl;
  int m_lowestmcl; 

  virtual void DoDispose (void);
  std::vector<std::string> readCSVRow (const std::string &row);
  MatlabNpuschMeasurement readNpuschCSV (std::istream &in);
  MatlabNpdschMeasurement readNpdschCSV (std::istream &in);    
  NpdschMeasurementValues getNpdschParameters(double couplingloss, int dataSize, std::string opMode);
  NpuschMeasurementValues getNpuschParameters(double couplingloss, int dataSize, double scs, double bandwidth);
  std::pair<NbIotRrcSap::DciN1, int> getBareboneDci(double couplingloss, int dataSize, std::string opMode);
  int getMsg3Subframes (double couplingloss, int dataSize, double scs, double bandwidth);

private:
  MatlabNpuschMeasurement m_npusch_params;
  MatlabNpdschMeasurement m_npdsch_params;
  NbIotRrcSap::DciN1 mapMeasurementValuetoDciN1(NpdschMeasurementValues value);
  NbIotRrcSap::DciN0 mapMeasurementValuetoDciN0(NpuschMeasurementValues value);
  int mapMeasurementValuetoMSG3(NpuschMeasurementValues value);

  std::map<uint8_t, std::map<uint8_t, uint16_t>> TransportBlockSizeTableNb{

    /*TBSI 0*/ {0, {{1, 16}, {2, 32}, {3, 56}, {4, 88}, {5, 120}, {6, 152}, {8, 208}, {10, 256}}},
    /*TBSI 1*/ {1, {{1, 24}, {2, 56}, {3, 88}, {4, 144}, {5, 176}, {6, 208}, {8, 256}, {10, 344}}},
    /*TBSI 2*/ {2, {{1, 32}, {2, 72}, {3, 144}, {4, 176}, {5, 208}, {6, 256}, {8, 328}, {10, 424}}},
    /*TBSI 3*/ {3, {{1, 40}, {2, 104}, {3, 176}, {4, 208}, {5, 256}, {6, 328}, {8, 440}, {10, 568}}},
    /*TBSI 4*/ {4, {{1, 56}, {2, 120}, {3, 208}, {4, 256}, {5, 328}, {6, 408}, {8, 552}, {10, 680}}},
    /*TBSI 5*/ {5, {{1, 72}, {2, 144}, {3, 224}, {4, 328}, {5, 424}, {6, 504}, {8, 680}, {10, 0}}},
    /*TBSI 6*/ {6, {{1, 88}, {2, 176}, {3, 256}, {4, 424}, {5, 504}, {6, 600}, {8, 0}, {10, 0}}},
    /*TBSI 7*/ {7, {{1, 104}, {2, 224}, {3, 328}, {4, 504}, {5, 584}, {6, 680}, {8, 0}, {10, 0}}},
    /*TBSI 8*/ {8, {{1, 120}, {2, 256}, {3, 392}, {4, 584}, {5, 680}, {6, 0}, {8, 0}, {10, 0}}},
    /*TBSI 9*/ {9, {{1, 136}, {2, 296}, {3, 456}, {4, 680}, {5, 0}, {6, 0}, {8, 0}, {10, 0}}},
    /*TBSI 10*/ {10, {{1, 144}, {2, 328}, {3, 504}, {4, 0}, {5, 0}, {6, 0}, {8, 0}, {10, 0}}},
    /*TBSI 11*/ {11, {{1, 176}, {2, 376}, {3, 584}, {4, 0}, {5, 0}, {6, 0}, {8, 0}, {10, 0}}},
    /*TBSI 12*/ {12, {{1, 208}, {2, 440}, {3, 680}, {4, 0}, {5, 0}, {6, 0}, {8, 0}, {10, 0}}}

};
};
} // namespace ns3

#endif