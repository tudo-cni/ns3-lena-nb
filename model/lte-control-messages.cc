/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
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
 * Author: Giuseppe Piro  <g.piro@poliba.it>
 *         Marco Miozzo <marco.miozzo@cttc.es>
 */

#include "lte-control-messages.h"
#include "ns3/address-utils.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "lte-net-device.h"
#include "lte-ue-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteControlMessage");

LteControlMessage::LteControlMessage (void)
{
}


LteControlMessage::~LteControlMessage (void)
{
}


void
LteControlMessage::SetMessageType (LteControlMessage::MessageType type)
{
  m_type = type;
}


LteControlMessage::MessageType
LteControlMessage::GetMessageType (void)
{
  return m_type;
}


// ----------------------------------------------------------------------------------------------------------


DlDciLteControlMessage::DlDciLteControlMessage (void)
{
  SetMessageType (LteControlMessage::DL_DCI);
}


DlDciLteControlMessage::~DlDciLteControlMessage (void)
{

}

void
DlDciLteControlMessage::SetDci (DlDciListElement_s dci)
{
  m_dci = dci;

}


DlDciListElement_s
DlDciLteControlMessage::GetDci (void)
{
  return m_dci;
}


// ----------------------------------------------------------------------------------------------------------


UlDciLteControlMessage::UlDciLteControlMessage (void)
{
  SetMessageType (LteControlMessage::UL_DCI);
}


UlDciLteControlMessage::~UlDciLteControlMessage (void)
{

}

void
UlDciLteControlMessage::SetDci (UlDciListElement_s dci)
{
  m_dci = dci;

}


UlDciListElement_s
UlDciLteControlMessage::GetDci (void)
{
  return m_dci;
}


// ----------------------------------------------------------------------------------------------------------


DlCqiLteControlMessage::DlCqiLteControlMessage (void)
{
  SetMessageType (LteControlMessage::DL_CQI);
}


DlCqiLteControlMessage::~DlCqiLteControlMessage (void)
{

}

void
DlCqiLteControlMessage::SetDlCqi (CqiListElement_s dlcqi)
{
  m_dlCqi = dlcqi;

}


CqiListElement_s
DlCqiLteControlMessage::GetDlCqi (void)
{
  return m_dlCqi;
}



// ----------------------------------------------------------------------------------------------------------


BsrLteControlMessage::BsrLteControlMessage (void)
{
  SetMessageType (LteControlMessage::BSR);
}


BsrLteControlMessage::~BsrLteControlMessage (void)
{

}

void
BsrLteControlMessage::SetBsr (MacCeListElement_s bsr)
{
  m_bsr = bsr;

}


MacCeListElement_s
BsrLteControlMessage::GetBsr (void)
{
  return m_bsr;
}



// ----------------------------------------------------------------------------------------------------------


RachPreambleLteControlMessage::RachPreambleLteControlMessage (void)
{
  SetMessageType (LteControlMessage::RACH_PREAMBLE);
}

void
RachPreambleLteControlMessage::SetRapId (uint32_t rapId)
{
  m_rapId = rapId;
}

uint32_t 
RachPreambleLteControlMessage::GetRapId () const
{
  return m_rapId;
}


// ----------------------------------------------------------------------------------------------------------


RarLteControlMessage::RarLteControlMessage (void)
{
  SetMessageType (LteControlMessage::RAR);
}


void
RarLteControlMessage::SetRaRnti (uint16_t raRnti)
{
  m_raRnti = raRnti;
}

uint16_t 
RarLteControlMessage::GetRaRnti () const
{
  return m_raRnti;
}


void
RarLteControlMessage::AddRar (Rar rar)
{
  m_rarList.push_back (rar);
}

std::list<RarLteControlMessage::Rar>::const_iterator 
RarLteControlMessage::RarListBegin () const
{
  return m_rarList.begin ();
}

std::list<RarLteControlMessage::Rar>::const_iterator 
RarLteControlMessage::RarListEnd () const
{
  return m_rarList.end ();
}


// ----------------------------------------------------------------------------------------------------------



MibLteControlMessage::MibLteControlMessage (void)
{
  SetMessageType (LteControlMessage::MIB);
}


void
MibLteControlMessage::SetMib (LteRrcSap::MasterInformationBlock  mib)
{
  m_mib = mib;
}

LteRrcSap::MasterInformationBlock 
MibLteControlMessage::GetMib () const
{
  return m_mib;
}


// ----------------------------------------------------------------------------------------------------------



Sib1LteControlMessage::Sib1LteControlMessage (void)
{
  SetMessageType (LteControlMessage::SIB1);
}


void
Sib1LteControlMessage::SetSib1 (LteRrcSap::SystemInformationBlockType1 sib1)
{
  m_sib1 = sib1;
}

LteRrcSap::SystemInformationBlockType1
Sib1LteControlMessage::GetSib1 () const
{
  return m_sib1;
}


// ---------------------------------------------------------------------------



DlHarqFeedbackLteControlMessage::DlHarqFeedbackLteControlMessage (void)
{
  SetMessageType (LteControlMessage::DL_HARQ);
}


DlHarqFeedbackLteControlMessage::~DlHarqFeedbackLteControlMessage (void)
{

}

void
DlHarqFeedbackLteControlMessage::SetDlHarqFeedback (DlInfoListElement_s m)
{
  m_dlInfoListElement = m;
}


DlInfoListElement_s
DlHarqFeedbackLteControlMessage::GetDlHarqFeedback (void)
{
  return m_dlInfoListElement;
}

// ----------------------------------------------------------------------------------------------------------



MibNbiotControlMessage::MibNbiotControlMessage (void)
{
  SetMessageType (LteControlMessage::MIB_NB);
}


void
MibNbiotControlMessage::SetMib (NbIotRrcSap::MasterInformationBlockNb mib)
{
  m_mib = mib;
}

NbIotRrcSap::MasterInformationBlockNb MibNbiotControlMessage::GetMib () const
{
  return m_mib;
}


// ----------------------------------------------------------------------------------------------------------



Sib1NbiotControlMessage::Sib1NbiotControlMessage (void)
{
  SetMessageType (LteControlMessage::SIB1_NB);
}


void
Sib1NbiotControlMessage::SetSib1 (NbIotRrcSap::SystemInformationBlockType1Nb sib1)
{
  m_sib1 = sib1;
}

NbIotRrcSap::SystemInformationBlockType1Nb Sib1NbiotControlMessage::GetSib1 () const
{
  return m_sib1;
}


// ---------------------------------------------------------------------------


NprachPreambleNbiotControlMessage::NprachPreambleNbiotControlMessage (void)
{
  SetMessageType (LteControlMessage::NPRACH_PREAMBLE);
}

void
NprachPreambleNbiotControlMessage::SetRapId (uint8_t rapId)
{
  m_rapId = rapId;
}

uint8_t 
NprachPreambleNbiotControlMessage::GetRapId () const
{
  return m_rapId;
}
void
NprachPreambleNbiotControlMessage::SetSubcarrierOffset (uint8_t subcarrierOffset)
{
  m_subcarrierOffset = subcarrierOffset;
}

uint8_t 
NprachPreambleNbiotControlMessage::GetSubcarrierOffset() const
{
  return m_subcarrierOffset;
}
void
NprachPreambleNbiotControlMessage::SetRanti(uint32_t ranti)
{
  m_ranti= ranti;
}

uint32_t 
NprachPreambleNbiotControlMessage::GetRanti() const
{
  return m_ranti;
}
// ----------------------------------------------------------------------------------------------------------


DlDciN1NbiotControlMessage::DlDciN1NbiotControlMessage (void)
{
  SetMessageType (LteControlMessage::DL_DCI_NB);
}


DlDciN1NbiotControlMessage::~DlDciN1NbiotControlMessage (void)
{

}

void
DlDciN1NbiotControlMessage::SetDci (NbIotRrcSap::DciN1 dci)
{
  m_dci = dci;

}


NbIotRrcSap::DciN1
DlDciN1NbiotControlMessage::GetDci (void)
{
  return m_dci;
}

void
DlDciN1NbiotControlMessage::SetRnti (uint32_t rnti)
{
  m_rnti = rnti;

}


uint32_t
DlDciN1NbiotControlMessage::GetRnti (void)
{
  return m_rnti;
}

// ----------------------------------------------------------------------------------------------------------


UlDciN0NbiotControlMessage::UlDciN0NbiotControlMessage (void)
{
  SetMessageType (LteControlMessage::UL_DCI_NB);
}


UlDciN0NbiotControlMessage::~UlDciN0NbiotControlMessage (void)
{

}

void
UlDciN0NbiotControlMessage::SetDci (NbIotRrcSap::DciN0 dci)
{
  m_dci = dci;

}


NbIotRrcSap::DciN0
UlDciN0NbiotControlMessage::GetDci (void)
{
  return m_dci;
}

void
UlDciN0NbiotControlMessage::SetRnti (uint32_t rnti)
{
  m_rnti = rnti;

}


uint32_t
UlDciN0NbiotControlMessage::GetRnti (void)
{
  return m_rnti;
}
void
UlDciN0NbiotControlMessage::SetLc(uint8_t lc)
{
  m_lc = lc;

}


uint8_t
UlDciN0NbiotControlMessage::GetLc(void)
{
  return m_lc;
}
// ----------------------------------------------------------------------------------------------------------


RarNbiotControlMessage::RarNbiotControlMessage (void)
{
  SetMessageType (LteControlMessage::RAR_NB);
}


void
RarNbiotControlMessage::SetRaRnti (uint16_t raRnti)
{
  m_raRnti = raRnti;
}

uint16_t 
RarNbiotControlMessage::GetRaRnti () const
{
  return m_raRnti;
}


void
RarNbiotControlMessage::AddRar (NbIotRrcSap::Rar rar)
{
  m_rarList.push_back (rar);
}

std::list<NbIotRrcSap::Rar>::const_iterator 
RarNbiotControlMessage::RarListBegin () const
{
  return m_rarList.begin ();
}

std::list<NbIotRrcSap::Rar>::const_iterator 
RarNbiotControlMessage::RarListEnd () const
{
  return m_rarList.end ();
}

//-------------------------------------------------------------------------------------------

DlHarqFeedbackNbiotControlMessage::DlHarqFeedbackNbiotControlMessage(void){
  SetMessageType(LteControlMessage::DL_HARQ_NB);
}

DlHarqFeedbackNbiotControlMessage::~DlHarqFeedbackNbiotControlMessage(void){

}
void DlHarqFeedbackNbiotControlMessage::SetAcknowledgement(bool ack){
  m_ack = ack;
}
void DlHarqFeedbackNbiotControlMessage::SetRnti(int16_t rnti){
  m_rnti = rnti;
}
bool DlHarqFeedbackNbiotControlMessage::GetAcknowledgement(void){
  return m_ack;
}
int16_t DlHarqFeedbackNbiotControlMessage::GetRnti(void){
  return m_rnti;
}
} // namespace ns3

