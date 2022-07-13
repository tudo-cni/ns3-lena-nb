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
 */



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
  MatlabNpdschMeasurement readNpdschF2CSV (std::istream &in);    
  NpdschMeasurementValues getNpdschParameters(double couplingloss, int dataSize, std::string opMode);
  NpuschMeasurementValues getNpuschParameters(double couplingloss, int dataSize, double scs, double bandwidth);
  NpuschMeasurementValues getMaxTbsforCl(double couplingloss,  double scs, double bandwidth);
  std::pair<NbIotRrcSap::DciN1, int> getBareboneDciN1(double couplingloss, int dataSize, std::string opMode);
  std::pair<NbIotRrcSap::DciN0, int> getBareboneDciN0(double couplingloss, int dataSize, double scs, double bandwidth);
  int getMsg3Subframes (double couplingloss, int dataSize, double scs, double bandwidth);

private:
  bool m_r13;
  MatlabNpuschMeasurement m_npusch_params;
  MatlabNpdschMeasurement m_npdsch_params;
  NbIotRrcSap::DciN1 mapMeasurementValuetoDciN1(NpdschMeasurementValues value);
  NbIotRrcSap::DciN0 mapMeasurementValuetoDciN0(NpuschMeasurementValues value);
  int mapMeasurementValuetoMSG3(NpuschMeasurementValues value);

  std::map<uint8_t, std::map<uint8_t, uint16_t>> TransportBlockSizeTableDlNb{

    /*TBSI 0*/  {0,  {{1, 16},           {2, 32},           {3, 56},           {4,   88},          {5,  120},          {6,  152},          {8,  208},          {10,  256}}},
    /*TBSI 1*/  {1,  {{1, 24},           {2, 56},           {3, 88},           {4,  144},          {5,  176},          {6,  208},          {8,  256},          {10,  344}}},
    /*TBSI 2*/  {2,  {{1, 32},           {2, 72},           {3, 144},          {4,  176},          {5,  208},          {6,  256},          {8,  328},          {10,  424}}},
    /*TBSI 3*/  {3,  {{1, 40},           {2, 104},          {3, 176},          {4,  208},          {5,  256},          {6,  328},          {8,  440},          {10,  568}}},
    /*TBSI 4*/  {4,  {{1, 56},           {2, 120},          {3, 208},          {4,  256},          {5,  328},          {6,  408},          {8,  552},          {10,  680}}},
    /*TBSI 5*/  {5,  {{1, 72},           {2, 144},          {3, 224},          {4,  328},          {5,  424},          {6,  504},          {8,  680},          {10,  872 /*Rel14*/}}},
    /*TBSI 6*/  {6,  {{1, 88},           {2, 176},          {3, 256},          {4,  424},          {5,  504},          {6,  600},          {8,  808/*Rel14*/}, {10, 1032 /*Rel14*/}}},
    /*TBSI 7*/  {7,  {{1, 104},          {2, 224},          {3, 328},          {4,  504},          {5,  584},          {6,  680},          {8,  968/*Rel14*/}, {10, 1224 /*Rel14*/}}},
    /*TBSI 8*/  {8,  {{1, 120},          {2, 256},          {3, 392},          {4,  584},          {5,  680},          {6,  808/*Rel14*/}, {8, 1096/*Rel14*/}, {10, 1352 /*Rel14*/}}},
    /*TBSI 9*/  {9,  {{1, 136},          {2, 296},          {3, 456},          {4,  616},          {5,  776/*Rel14*/}, {6,  936/*Rel14*/}, {8, 1256/*Rel14*/}, {10, 1544 /*Rel14*/}}},
    /*TBSI 10*/ {10, {{1, 144},          {2, 328},          {3, 504},          {4,  680},          {5,  872/*Rel14*/}, {6, 1032/*Rel14*/}, {8, 1384/*Rel14*/}, {10, 1736 /*Rel14*/}}},
    /*TBSI 11*/ {11, {{1, 176},          {2, 376},          {3, 584},          {4,  776/*Rel14*/}, {5, 1000/*Rel14*/}, {6, 1192/*Rel14*/}, {8, 1608/*Rel14*/}, {10, 2024 /*Rel14*/}}},
    /*TBSI 12*/ {12, {{1, 208},          {2, 440},          {3, 680},          {4,  904/*Rel14*/}, {5, 1128/*Rel14*/}, {6, 1352/*Rel14*/}, {8, 1800/*Rel14*/}, {10, 2280 /*Rel14*/}}},
    /*TBSI 13*/ {13, {{1, 224/*Rel14*/}, {2, 488/*Rel14*/}, {3, 744/*Rel14*/}, {4, 1128/*Rel14*/}, {5, 1256/*Rel14*/}, {6, 1544/*Rel14*/}, {8, 2024/*Rel14*/}, {10, 2536 /*Rel14*/}}} // New Release 14

};
  std::map<uint8_t, std::map<uint8_t, uint16_t>> TransportBlockSizeTableUlNb{

    /*TBSI 0*/  {0,  {{1,  16},          {2,  32},          {3,  56},          {4,   88},          {5,  120},          {6,  152},          {8,  208},          {10,  256}}},
    /*TBSI 1*/  {1,  {{1,  24},          {2,  56},          {3,  88},          {4,  144},          {5,  176},          {6,  208},          {8,  256},          {10,  344}}},
    /*TBSI 2*/  {2,  {{1,  32},          {2,  72},          {3, 144},          {4,  176},          {5,  208},          {6,  256},          {8,  328},          {10,  424}}},
    /*TBSI 3*/  {3,  {{1,  40},          {2, 104},          {3, 176},          {4,  208},          {5,  256},          {6,  328},          {8,  440},          {10,  568}}},
    /*TBSI 4*/  {4,  {{1,  56},          {2, 120},          {3, 208},          {4,  256},          {5,  328},          {6,  408},          {8,  552},          {10,  696}}},
    /*TBSI 5*/  {5,  {{1,  72},          {2, 144},          {3, 224},          {4,  328},          {5,  424},          {6,  504},          {8,  680},          {10,  872}}},
    /*TBSI 6*/  {6,  {{1,  88},          {2, 176},          {3, 256},          {4,  392},          {5,  504},          {6,  600},          {8,  808},          {10, 1000}}},
    /*TBSI 7*/  {7,  {{1, 104},          {2, 224},          {3, 328},          {4,  472},          {5,  584},          {6,  712},          {8, 1000},          {10, 1224/*Rel14*/}}},
    /*TBSI 8*/  {8,  {{1, 120},          {2, 256},          {3, 392},          {4,  536},          {5,  680},          {6,  808},          {8, 1096/*Rel14*/}, {10, 1384/*Rel14*/}}},
    /*TBSI 9*/  {9,  {{1, 136},          {2, 296},          {3, 456},          {4,  616},          {5,  776},          {6,  936},          {8, 1256/*Rel14*/}, {10, 1544/*Rel14*/}}},
    /*TBSI 10*/ {10, {{1, 144},          {2, 328},          {3, 504},          {4,  680},          {5,  872},          {6, 1000},          {8, 1384/*Rel14*/}, {10, 1736/*Rel14*/}}},
    /*TBSI 11*/ {11, {{1, 176},          {2, 376},          {3, 584},          {4,  776},          {5, 1000},          {6, 1192/*Rel14*/}, {8, 1608/*Rel14*/}, {10, 2024/*Rel14*/}}},
    /*TBSI 12*/ {12, {{1, 208},          {2, 440},          {3, 680},          {4, 1000},          {5, 1128/*Rel14*/}, {6, 1352/*Rel14*/}, {8, 1800/*Rel14*/}, {10, 2280/*Rel14*/}}},
    /*TBSI 13*/ {13, {{1, 224/*Rel14*/}, {2, 488/*Rel14*/}, {3, 744/*Rel14*/}, {4, 1128/*Rel14*/}, {5, 1256/*Rel14*/}, {6, 1544/*Rel14*/}, {8, 2024/*Rel14*/}, {10, 2536/*Rel14*/}}} // Release 14

};
};
} // namespace ns3

#endif